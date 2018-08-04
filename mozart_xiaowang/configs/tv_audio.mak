tv_audio_DIR := $(TOPDIR)/src/tv_audio

ifeq ($(MAIN_APP), tv_audio)
TARGETS += tv_audio
TARGETS1 += tv_audio
endif

SUPPORT_MULROOM ?= 0
SUPPORT_LOCALPLAYER ?= 0
SUPPORT_UI ?= 0

tv_audio_DEPENDENCIES := updater

# do nothing
define tv_audio_CONFIGURE_CMDS

endef

define tv_audio_OPTS
	RUN_MODE=$(SUPPORT_MODE) \
	MULROOM=$(SUPPORT_MULROOM) LOCALPLAYER=$(SUPPORT_LOCALPLAYER) \
	INGENICPLAYER=$(SUPPORT_INGENICPLAYER) UI=$(SUPPORT_UI)
endef

define tv_audio_BUILD_CMDS
	$(tv_audio_OPTS) \
			$(MAKE) -C $(tv_audio_DIR)
endef

define tv_audio_INSTALL_TARGET_CMDS
	$(tv_audio_OPTS) \
			$(MAKE) -C $(tv_audio_DIR) DESTDIR=$(MOZART_DIR) install
endef

define tv_audio_CLEAN_CMDS
	-$(tv_audio_OPTS) \
			$(MAKE) -C $(tv_audio_DIR) clean
endef

define tv_audio_UNINSTALL_TARGET_CMDS
	-$(tv_audio_OPTS) \
			$(MAKE1) -C $(tv_audio_DIR) DESTDIR=$(MOZART_DIR) uninstall
endef

$(eval $(call install_rules,tv_audio,$(tv_audio_DIR),target))
