DIR_CUR := $(shell pwd)

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

BT ?= 0
VR ?= 0
TTS ?= 0
LAPSULE ?= 0
INGENICPLAYER ?= 0
DMR ?= 0
WEBRTC ?= 0
DMS ?= 0
AIRPLAY ?= 0
LOCALPLAYER ?= 0
ALARM ?= 0
MULROOM ?= 0
CAMERA ?= 0

CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
STRIP = mipsel-linux-strip
INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin

SRCS = mozart.c mozart_app.c mozart_key_function.c mozart_tts.c mozart_musicplayer.c my_fft.c
SRCS += misc/battery_capacity.c
SRCS += modules/mozart_module_linein.c
SRCS += client_shared.c sub_client.c
SRCS += mqtt_lib/logging_mosq.c  mqtt_lib/mosquitto.c   mqtt_lib/memory_mosq.c \
			mqtt_lib/messages_mosq.c mqtt_lib/net_mosq.c mqtt_lib/read_handle.c mqtt_lib/read_handle_client.c \
			mqtt_lib/read_handle_shared.c mqtt_lib/send_mosq.c mqtt_lib/send_client_mosq.c  mqtt_lib/socks_mosq.c \
			mqtt_lib/srv_mosq.c mqtt_lib/thread_mosq.c  mqtt_lib/time_mosq.c  mqtt_lib/tls_mosq.c  \
			mqtt_lib/util_mosq.c   mqtt_lib/will_mosq.c
# common opts.
CFLAGS = -fPIC -Wall  -g -O2 -I. -I./include/ -I./modules/ -I./include/modules/ -I./mqtt_lib -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include -I$(MOZART_UPDATER_DIR)/usr/include -I./mqtt_client  -I../vr/include/
CFLAGS += -rdynamic -fasynchronous-unwind-tables

LDFLAGS = -g -lpthread -lstdc++ -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib -L$(MOZART_UPDATER_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib

# basic depends.
LDFLAGS += -levent -lpower -lwifi -lvolume -ltips -lsharememory -lini -lutils -lpthread -lplayer -ljson-c -lm -lresample -lnvrw -lupdater -lmusiclist -llinklist -lasound -llocalstream -lairkiss_log -llivemulti_ingenic_x1000
#LDFLAGS += -llivemulti_ingenic -lwebrtc -lcrypto -ldl -lairkiss 

SRCS += modules/mozart_module_cloud_music.c

SRCS += modules/mozart_module_rayshine.c modules/callbackVideo.c modules/callbackAudio.c modules/callbackAudioQueue.c modules/hardware_api.c

ifeq ("$(SONG_SUPPLYER)","aispeech")
SRCS += modules/song_supplyer/speech.c
endif

ifeq ("$(SONG_SUPPLYER)","ximalaya")
SRCS += modules/song_supplyer/ximalaya.c
LDFLAGS += -lximalaya
endif

ifeq ("$(SONG_SUPPLYER)","jushang")
SRCS += modules/song_supplyer/jushang.c
LDFLAGS += -lvr_jushang
endif

ifeq ($(LOCALPLAYER),1)
SRCS += modules/mozart_module_local_music.c
LDFLAGS += -llocalplayer
else
$(warning localplayer func CLOSED!!!!)
endif

ifeq ($(DMR),1)
CFLAGS += -I$(MOLIB_DIR)/usr/include/upnp -I$(MOLIB_DIR)/usr/include/opencore-amrnb/
LDFLAGS += -lrender -lopencore-amrnb
SRCS += modules/mozart_module_dmr.c
else
$(warning dmr func CLOSED!!!!)
endif

ifeq ($(MULROOM),1)
SRCS += modules/mozart_module_mulroom.c
LDFLAGS += -lmulroom
else
$(warning multiroom func CLOSED!!!!)
endif

ifeq ($(CAMERA),1)
SRCS += modules/camera/camera.c \
	modules/camera/lcd.c\
	modules/camera/video.c\
	modules/camera/savebmp.c\
	modules/camera/convert.c\
	modules/mozart_module_camera.c
else
$(warning camera func CLOSED!!!!)
endif

ifeq ("$(TTS)","baidu")
SRCS += misc/tts_baidu.c
else
ifeq ("$(TTS)","jietong")
LDFLAGS +=  -lhci_sys -lhci_curl -lhci_tts -ltts_jietong
else
ifeq ("$(TTS)","aispeech")
LDFLAGS +=  -lvr
endif
endif
endif

ifeq ($(AIRPLAY),1)
SRCS += modules/mozart_module_airplay.c
else
$(warning airplay func CLOSED!!!!)
endif

ifeq ($(BT),0)
$(warning bt func are CLOSED!!!!)
else
SRCS += modules/mozart_module_bt.c
endif
ifeq ("$(call substring,$(BT),1,3)", "rtk") # realtek's bt module.
endif
ifeq ("$(call substring,$(BT),1,3)", "bcm") # broadcom's bt module.
LDFLAGS += -lbsa -lbt -lappcommon -lchannels
endif

ifeq ($(INGENICPLAYER),1)
SRCS +=  ingenicplayer/appserver.c ingenicplayer/ingenicplayer.c ingenicplayer/device.c
LDFLAGS += -lappserver
else
$(warning ingenicplayer func CLOSED!!!!)
endif

ifeq ("$(VR)","0")
$(warning vr func are CLOSED!!!!)
else
SRCS += modules/mozart_module_vr.c
SRCS += modules/md5.c
endif
ifeq ("$(VR)","aispeech")
LDFLAGS +=  -lvr
endif
ifeq ("$(VR)","iflytek")
LDFLAGS += -lvr_iflytek
endif
ifeq ("$(VR)","jietong")
LDFLAGS += -lvr_jietong -lrecord -lwakeup_jietong
endif

ifeq ("$(UI)","0")
$(warning ui func are CLOSED!!!!)
else
SRCS += mozart_ui.c
endif
ifeq ("$(UI)","lcd")
LDFLAGS += -llcd -llcdshow -lpng -lz
endif
ifeq ("$(UI)","smartui")
LDFLAGS += -lsmartui
endif

ifeq ($(ALARM),1)
LDFLAGS += -lalarm
else
$(warning alarm func CLOSED!!!!)
endif

ifeq ($(WEBRTC),1)
LDFLAGS += -lwebrtc_audio_processing -lwebrtc -lrt -lcurl
else
$(warning webrtc func CLOSED!!!!)
endif

ifeq ($(DMS),1)
SRCS += misc/dms.c
else
$(warning dms func CLOSED!!!!)
endif

ifeq ($(LAPSULE),1)
SRCS += misc/lapsule_control.c
else
$(warning lapsule func CLOSED!!!!) 
endif

OBJS = $(patsubst %.c, %.o, $(SRCS))
#OBJS += ./modules/mozart_module_rayshine.o \
#	./modules/callbackVideo.o \
#	./modules/callbackAudio.o \
#	./modules/callbackAudioQueue.o \
#	./modules/hardware_api.o

TARGET = mozart

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	$(STRIP) $(TARGET)
%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
#	$(CXX) $(CFLAGS)  -c ./modules/mozart_module_rayshine.c   -o ./modules/mozart_module_rayshine.o
#	$(CXX) $(CFLAGS)  -c ./modules/callbackVideo.c -o ./modules/callbackVideo.o
#	$(CXX) $(CFLAGS)  -c ./modules/callbackAudio.c -o ./modules/callbackAudio.o
#	$(CXX) $(CFLAGS)  -c ./modules/callbackAudioQueue.c -o ./modules/callbackAudioQueue.o
#	$(CXX) $(CFLAGS)  -c ./modules/hardware_api.c -o ./modules/hardware_api.o

install:
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)
	if [ -e $(TARGET) ]; then \
		$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(TARGET) $(DESTDIR)$(SBINDIR)/; \
		$(INSTALL) -m 755 S10mozart.sh $(DESTDIR)$(INITDIR)/; \
	fi

clean:
	-rm -rf $(OBJS) $(TARGET)

uninstall:
	if [ -e $(DESTDIR)$(SBINDIR)/$(TARGET) ]; then \
		rm -rf $(DESTDIR)$(SBINDIR)/$(TARGET); \
		rm -rf $(DESTDIR)$(INITDIR)/S10mozart.sh; \
	fi

.PHONY:all clean install uninstall $(TARGET)
