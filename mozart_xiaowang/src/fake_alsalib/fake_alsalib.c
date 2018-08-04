#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/soundcard.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

static int alsa2oss_debug = 1;

#define pr_debug(fmt, args...)                      \
	do {                                \
		if (alsa2oss_debug)                  \
		printf("[alsa2oss] [DEBUG] %s %s %d: "fmt,   \
			   __FILE__, __func__, __LINE__, ##args);   \
	} while (0)

#define pr_info(fmt, args...)                       \
	printf("[alsa2oss] [INFO] "fmt, ##args)

#define pr_err(fmt, args...)                        \
	fprintf(stderr, "[alsa2oss] [ERROR] %s %s %d: "fmt, __FILE__, __func__, __LINE__, ##args)

//#define OSS_WITH_NONBLOCK
#define DSP_LOW_DELAY
#define VOL_MAX 100

/* default 20 millisecond buffer */
#define AO_OSS_BUFFER_TIME 20000

typedef struct {
	int	channels;	/* number of audio channels */
	int	bytes;		/* bytes per sample */
	int	format;		/* pcm format */
	int	rate;		/* samples per second */
} ao_format_t;

typedef struct {
	char	dev[12];
	int		id;
	int		fd;
	ao_format_t	fmt;

	int		blocking;
	unsigned int	buffer_time;
	int		outburst;
	int		buffersize;
} ao_oss_internal_t;

struct _snd_pcm {
	void *private_data;
};

int snd_lib_error_set_handler(snd_lib_error_handler_t handler)
{return 0;}
int snd_mixer_attach(snd_mixer_t *mixer, const char *name)
{return 0;}
int snd_mixer_close(snd_mixer_t *mixer)
{return 0;}
snd_mixer_elem_t *snd_mixer_find_selem(snd_mixer_t *mixer, const snd_mixer_selem_id_t *id)
{return NULL;}
int snd_mixer_load(snd_mixer_t *mixer)
{return 0;}
int snd_mixer_open(snd_mixer_t **mixer, int mode)
{return 0;}
int snd_mixer_selem_get_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value)
{return 0;}
int snd_mixer_selem_get_playback_volume_range(snd_mixer_elem_t *elem, long *min, long *max)
{return 0;}
int snd_mixer_selem_has_playback_switch(snd_mixer_elem_t *elem)
{return 0;}
int snd_mixer_selem_has_playback_switch_joined(snd_mixer_elem_t *elem)
{return 0;}
unsigned int snd_mixer_selem_id_get_index(const snd_mixer_selem_id_t *obj)
{return 0;}
const char *snd_mixer_selem_id_get_name(const snd_mixer_selem_id_t *obj)
{return 0;}
void snd_mixer_selem_id_set_index(snd_mixer_selem_id_t *obj, unsigned int val)
{return;}
void snd_mixer_selem_id_set_name(snd_mixer_selem_id_t *obj, const char *val)
{return;}
size_t snd_mixer_selem_id_sizeof(void)
{return 0;}
int snd_mixer_selem_register(snd_mixer_t *mixer, struct snd_mixer_selem_regopt *options, snd_mixer_class_t **classp)
{return 0;}
int snd_mixer_selem_set_playback_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value)
{return 0;}
int snd_mixer_selem_set_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value)
{
	int mixer_fd = -1;

	if (channel != SND_MIXER_SCHN_FRONT_RIGHT)
		return 0;

	mixer_fd = open("/dev/mixer", O_RDWR, 0);
	if (mixer_fd < 0) {
		pr_err("open /dev/mixer error:%s\n", strerror(errno));
		return -1;
	}

	if(value > 100)
		value = 100;
	if(value < 0)
		value = 0;

	//set volume
	if (-1 == ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &value)) {
		pr_err("set volume to %ld error: %s\n", value, strerror(errno));
		return -1;
	}

	pr_debug("set volume to %ld\n", value);

	return 0;
}

int snd_pcm_close(snd_pcm_t *pcm)
{
	ao_oss_internal_t *oss_internal = NULL;

	if (!pcm)
		return 0;

	oss_internal = (ao_oss_internal_t *)pcm->private_data;

	if (oss_internal) {
		if (oss_internal->fd >= 0) {
			close(oss_internal->fd);
			oss_internal->fd = -1;
		}

		free(oss_internal);
		oss_internal = NULL;
	}

	if (pcm) {
		free(pcm);
		pcm = NULL;
	}

	return 0;
}

int snd_pcm_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{return 0;}
int snd_pcm_drain(snd_pcm_t *pcm)
{return 0;}
int snd_pcm_drop(snd_pcm_t *pcm)
{return 0;}
const char *snd_pcm_format_description(const snd_pcm_format_t format)
{return NULL;}
snd_pcm_sframes_t snd_pcm_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{return 0;}

#ifdef DSP_LOW_DELAY
static int ilog(long x)
{
	int ret = -1;

	while (x > 0) {
		x >>= 1;
		ret++;
	}

	return ret;
}
#endif

static int alsa2oss_device_open(ao_oss_internal_t *ao_oss, ao_format_t *fmt)
{
	static audio_buf_info zz;
#ifdef DSP_LOW_DELAY
	long bytesperframe;
	int fraglog, fragment, fragcheck;
#endif

	if (ao_oss->dev) {
		if (ao_oss->blocking)
			ao_oss->fd = open(ao_oss->dev, O_WRONLY);
		else
#ifdef OSS_WITH_NONBLOCK
			ao_oss->fd = open(ao_oss->dev, O_WRONLY | O_NONBLOCK);
#else
		ao_oss->fd = open(ao_oss->dev, O_WRONLY);
#endif

		if (ao_oss->fd < 0) {
			pr_err("Open \'%s\': %s", ao_oss->dev, strerror(errno));
			goto err_open;
		}
	} else {
		pr_err("Can not open oss device");
		goto err_open;
	}

	/* Now set all of the parameters */

	/* We only use SNDCTL_DSP_CHANNELS for >2 channels,
	 * in case some drivers don't have it.
	 */
	if (fmt->channels > 2) {
		int channels = fmt->channels;
		if (ioctl(ao_oss->fd, SNDCTL_DSP_CHANNELS, &channels) < 0) {
			pr_err("Cannot set channels to %d", fmt->channels);
			goto err_out;
		}
		fmt->channels = channels;
	} else {
		int channels = fmt->channels - 1;
		if (ioctl(ao_oss->fd, SNDCTL_DSP_STEREO, &channels) < 0) {
			pr_err("Cannot set channels to %d", fmt->channels);
			goto err_out;
		}
		fmt->channels = channels + 1;
	}

	if (ioctl(ao_oss->fd, SNDCTL_DSP_SETFMT, &fmt->format) < 0) {
		pr_err("Cannot set format to %d", fmt->bytes * 8);
		goto err_out;
	}

	/* Some cards aren't too accurate with their clocks and set to the
	 * exact data rate, but something close.  Fail only if completely out
	 * of whack.
	 */
	if (ioctl(ao_oss->fd, SNDCTL_DSP_SPEED, &fmt->rate) < 0) {
		pr_err("Cannot set rate to %d", fmt->rate);
		goto err_out;
	}

#ifdef DSP_LOW_DELAY
	/* try to lower the DSP delay; this ioctl may fail gracefully */
	bytesperframe = fmt->bytes * fmt->channels *
		fmt->rate * (ao_oss->buffer_time / 1000000.);
	fraglog = ilog(bytesperframe);
	fragment = 0x00040000 | fraglog;
	fragcheck=fragment;

	if (ioctl(ao_oss->fd, SNDCTL_DSP_SETFRAGMENT, &fragment) ||
		fragcheck != fragment) {
		pr_err("Could not set DSP fragment size; continuing.");
	}
#endif

	/* Update OSS buffer size */
	if (ioctl(ao_oss->fd, SNDCTL_DSP_GETOSPACE, &zz) < 0) {
		pr_err("Cannot get oss buffer info");
		ao_oss->buffersize = 0;
		goto err_out;
	}
	ao_oss->buffersize = zz.fragstotal * zz.fragsize;

	/* this calculates and sets the fragment size */
	ao_oss->outburst = -1;
	if ((ioctl(ao_oss->fd, SNDCTL_DSP_GETBLKSIZE, &(ao_oss->outburst)) < 0) ||
		ao_oss->outburst <= 0) {
		/* Some versions of the *BSD OSS drivers use a subtly
		 * different SNDCTL_DSP_GETBLKSIZE ioctl implementation
		 * which is, oddly, incompatible with the shipped
		 * declaration in soundcard.h.  This ioctl isn't necessary
		 * anyway, it's just tuning.  Soldier on without,
		 */
		pr_err("Cannot get buffer size for device; using a default of 1024kB");
		ao_oss->outburst = 1024;
	}

	return 0; /* Open successful */

err_out:
	if (ao_oss->fd >= 0) {
		close(ao_oss->fd);
		ao_oss->fd = -1;
	}

err_open:
	return -1; /* Failed to open device */
}

int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	ao_oss_internal_t *oss_internal = NULL;

	if (!pcm) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}

	oss_internal = (ao_oss_internal_t *)pcm->private_data;

	return alsa2oss_device_open(oss_internal, &oss_internal->fmt);
}

int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{return 0;}
int snd_pcm_hw_params_can_pause(const snd_pcm_hw_params_t *params)
{return 0;}
int snd_pcm_hw_params_get_buffer_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{return 0;}
int snd_pcm_hw_params_get_period_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *frames, int *dir)
{return 0;}
int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access)
{return 0;}
int snd_pcm_hw_params_set_buffer_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{return 0;}
int snd_pcm_hw_params_set_channels_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val)
{
	ao_oss_internal_t *oss_internal = NULL;
	int channels = *val;

	if (!pcm) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}

	oss_internal = (ao_oss_internal_t *)pcm->private_data;
	oss_internal->fmt.channels = channels;

	pr_debug("channels: %d\n", channels);

	return 0;
}
int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{
	ao_oss_internal_t *oss_internal = NULL;
	int format = 0;

	if (!pcm) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}

	/* translate alsa pcm format to oss pcm format */
	switch (val) {
	case SND_PCM_FORMAT_S8:
		format = AFMT_S8;
		break;
	case SND_PCM_FORMAT_S16:
		format = AFMT_S16_LE;
		break;
	default:
		return -1;
	}

	oss_internal = (ao_oss_internal_t *)pcm->private_data;
	oss_internal->fmt.format = format;
	oss_internal->fmt.bytes = 2;

	pr_debug("format: %d\n", format);

	return 0;
}
int snd_pcm_hw_params_set_periods_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{return 0;}

int snd_pcm_hw_params_set_rate_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
	ao_oss_internal_t *oss_internal = NULL;
	int rate = *val;

	if (!pcm) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}

	oss_internal = (ao_oss_internal_t *)pcm->private_data;
	oss_internal->fmt.rate = rate;

	pr_debug("rate: %d\n", rate);

	return 0;
}

int snd_pcm_hw_params_set_rate_resample(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{return 0;}
size_t snd_pcm_hw_params_sizeof(void)
{return 0;}
int snd_pcm_hw_params_test_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{return 0;}
int snd_pcm_nonblock(snd_pcm_t *pcm, int nonblock)
{return 0;}
int snd_pcm_open(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode)
{
	int ret = 0;
	snd_pcm_t *tmp = NULL;

	tmp = malloc(sizeof(snd_pcm_t));
	if (!tmp) {
		pr_err("alloc memory fail: %s.\n", strerror(errno));
		ret = -1;
		goto err_out;
	}
	memset(tmp, 0, sizeof(snd_pcm_t));

	ao_oss_internal_t *oss_internal = malloc(sizeof(ao_oss_internal_t));
	if (!tmp) {
		pr_err("alloc memory fail: %s.\n", strerror(errno));
		ret = -1;
		goto err_out;
	}
	memset(oss_internal, 0, sizeof(ao_oss_internal_t));
	tmp->private_data = oss_internal;

	/* init params */
	strcpy(oss_internal->dev, "/dev/dsp");
	oss_internal->id	= -1;
	oss_internal->fd			= -1;
	oss_internal->blocking		= 1;
	oss_internal->buffer_time	= AO_OSS_BUFFER_TIME;

	*pcm = tmp;

	return 0;

err_out:
	if (tmp) {
		free(tmp);
		tmp = NULL;
	}

	return ret;
}

int snd_pcm_pause(snd_pcm_t *pcm, int enable)
{return 0;}
int snd_pcm_prepare(snd_pcm_t *pcm)
{return 0;}
int snd_pcm_resume(snd_pcm_t *pcm)
{return 0;}
snd_pcm_state_t snd_pcm_state(snd_pcm_t *pcm)
{return 0;}
int snd_pcm_status(snd_pcm_t *pcm, snd_pcm_status_t *status)
{return 0;}
snd_pcm_uframes_t snd_pcm_status_get_avail(const snd_pcm_status_t *obj)
{return 0;}
size_t snd_pcm_status_sizeof(void)
{return 0;}
int snd_pcm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{return 0;}
int snd_pcm_sw_params_current(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{return 0;}
int snd_pcm_sw_params_get_boundary(const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val)
{return 0;}
int snd_pcm_sw_params_set_silence_size(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{return 0;}
int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{return 0;}
int snd_pcm_sw_params_set_stop_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{return 0;}
size_t snd_pcm_sw_params_sizeof(void)
{return 0;}

snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buf, snd_pcm_uframes_t samples)
{
	ao_oss_internal_t *ao_oss = NULL;
	static audio_buf_info zz;
	char *out_buffer = (char *)buf;
	int len, send;
	int retval;

	if (!pcm) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}
	ao_oss = pcm->private_data;
	if (!ao_oss) {
		pr_err("snd_pcm not init.\n");
		return -1;
	}

	if (ao_oss->fd < 0)
		return -1;

	len = samples * ao_oss->fmt.bytes * ao_oss->fmt.channels;

	while (len > 0) {
		if (ioctl(ao_oss->fd, SNDCTL_DSP_GETOSPACE, &zz) == -1) {
			pr_err("%s: Can not get availade space.", __func__);
			return -1;
		}

		if (zz.bytes < ao_oss->outburst) {
			usleep(100);
			continue;
		}

		send = len > ao_oss->outburst ?
			ao_oss->outburst : len;
		retval = write(ao_oss->fd, out_buffer, send);
		if (retval < 0){
			if(errno == -EINTR)
				continue;
			return -1;
		}

		len -= retval;
		out_buffer += retval;
	}

	return samples;
}
const char *snd_strerror(int errnum)
{return NULL;}
int snd_pcm_format_physical_width(snd_pcm_format_t format)
{return 0;}
int snd_pcm_hw_params_get_buffer_time_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{return 0;}
int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{return 0;}
int snd_pcm_hw_params_set_period_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{return 0;}
snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{return 0;}
int snd_pcm_wait(snd_pcm_t *pcm, int timeout)
{return 0;}
int snd_mixer_selem_set_playback_volume_range(snd_mixer_elem_t *elem, long min, long max)
{return 0;}
int snd_mixer_handle_events(snd_mixer_t *mixer)
{return 0;}
int snd_ctl_open(snd_ctl_t **ctl, const char *name, int mode)
{return 0;}
int snd_ctl_close(snd_ctl_t *ctl)
{return 0;}
int snd_ctl_elem_value_malloc(snd_ctl_elem_value_t **ptr)
{return 0;}
void snd_ctl_elem_value_set_integer(snd_ctl_elem_value_t *obj, unsigned int idx, long val)
{}
void snd_ctl_elem_value_set_interface(snd_ctl_elem_value_t *obj, snd_ctl_elem_iface_t val)
{}
void snd_ctl_elem_value_set_name(snd_ctl_elem_value_t *obj, const char *val)
{}
void snd_ctl_elem_value_set_numid(snd_ctl_elem_value_t *obj, unsigned int val)
{}
int snd_ctl_elem_write(snd_ctl_t *ctl, snd_ctl_elem_value_t *value)
{return 0;}
