# This top-level Makefile can *not* be executed in parallel
.NOTPARALLEL:

define ifexist
$(shell if [ -a $(1) ]; then echo "exist"; else :; fi;)
endef

# absolute path
TOPDIR := $(shell pwd)

# we want bash as shell
SHELL:=$(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi; fi)

export SHELL

HOSTCC:=$(shell which gcc || echo gcc)
HOSTCXX:=$(shell which g++ || echo g++)
HOSTAR:=$(shell which ar || echo ar)
HOSTAS:=$(shell which as || echo as)
HOSTLD:=$(shell which ld || echo ld)
HOSTLN:=$(shell which ln || echo ln)
HOSTMAKE:=$(shell which make || echo make)
HOSTCMAKE:=$(shell which make || echo make)


# scale the maximum concurrency with the number of CPUs.
# An additional job is used in order to keep processors busy
# If the number of processors is not available, assume one.
PARALLEL_JOBS:=$(shell echo $$((1 + `getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1`)))

MAKE1:=$(HOSTMAKE) -j1
MAKE:=$(HOSTMAKE) -j$(PARALLEL_JOBS)
CMAKE:=cmake

export HOSTAR HOSTAS HOSTCC HOSTCXX HOSTLD HOSTMAKE HOSTCMAKE MAKE MAKE1 CMAKE

# Make sure pkg-config doesn't look outside the buildroot tree
unexport PKG_CONFIG_PATH
unexport PKG_CONFIG_SYSROOT_DIR

# Having DESTDIR set in the environment confuses the installation
# steps of some packages.
unexport DESTDIR


# Hide troublesome environment variables from sub processes
unexport CROSS_COMPILE
unexport ARCH
unexport CC
unexport CXX
unexport CPP
unexport CFLAGS
unexport CXXFLAGS
unexport LDLAGS
unexport GREP_OPTIONS
unexport CONFIG_SITE

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE=$(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE=0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet=
  Q=
ifndef VERBOSE
  VERBOSE=1
endif
else
  quiet=quiet_
  Q=@
endif


OUTPUT_DIR := $(TOPDIR)/output

MOZART_DIR := $(OUTPUT_DIR)/mozart/app
MOZART_APP_DIR := $(MOZART_DIR)
MOZART_UPDATER_DIR := $(OUTPUT_DIR)/mozart/updater
MOZART_TEST_DIR := $(OUTPUT_DIR)/mozart/test

MOLIB_DIR := $(OUTPUT_DIR)/molib/app
MOLIB_APP_DIR:= $(MOLIB_DIR)
MOLIB_UPDATER_DIR := $(OUTPUT_DIR)/molib/updater
MOLIB_TEST_DIR := $(OUTPUT_DIR)/molib/test

TARGET_DIR := $(OUTPUT_DIR)/target
UPDATEPKG_DIR := $(OUTPUT_DIR)/updatepkg

APPFS_DIR := $(OUTPUT_DIR)/fs/appfs
UPDATERFS_DIR := $(OUTPUT_DIR)/fs/updaterfs
USRDATAFS_DIR := $(OUTPUT_DIR)/fs/usrdata
TESTFS_DIR := $(OUTPUT_DIR)/fs/testfs

DEVICE_TOPDIR := $(TOPDIR)/device

export TOPDIR
export OUTPUT_DIR
export MOZART_DIR
export MOZART_APP_DIR
export MOZART_UPDATER_DIR
export MOZART_TEST_DIR
export MOLIB_DIR
export MOLIB_APP_DIR
export MOLIB_UPDATER_DIR
export MOLIB_TEST_DIR
export TARGET_DIR
export UPDATEPKG_DIR
export APPFS_DIR
export USRDATAFS_DIR
export UPDATERFS_DIR
export DEVICE_TOPDIR

TARGETS :=
TARGETS1 :=
TARGETS_CLEAN :=
TARGETS_DISTCLEAN :=

#########################################################################

ifeq ($(TOPDIR)/configs/config.mk,$(wildcard $(TOPDIR)/configs/config.mk))
ifeq ($(MOZART_DIR)/usr/include/mozart_config.h,$(wildcard $(MOZART_DIR)/usr/include/mozart_config.h))

include $(TOPDIR)/configs/config.mk

DEVICE_OTA_DIR := $(if $(and $(DEVICE_DIR), $(call ifexist,$(DEVICE_DIR)/ota)),$(DEVICE_DIR)/ota,$(DEVICE_TOPDIR)/common/ota)
export DEVICE_OTA_DIR

all: world rootfs

include configs/pkg.mk
include configs/mkfs.mk
include configs/*.mak

TARGETS_CLEAN:=$(patsubst %,%-clean,$(TARGETS1))
TARGETS_DISTCLEAN:=$(patsubst %,%-clean,$(TARGETS))

world: prepare $(TARGETS)

debug:
	@echo TOPDIR is $(TOPDIR)
	@echo TARGETS is $(TARGETS)

prepare: dirs develop_env

dirs: $(OUTPUT_DIR) $(TARGET_DIR) $(UPDATEPKG_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR) $(MOZART_TEST_DIR)


$(OUTPUT_DIR) $(UPDATEPKG_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR) $(MOZART_TEST_DIR):
	@mkdir -p $@
$(TARGET_DIR)::
	@rm -rf $@; mkdir -p $@

NODE_MODULES := ./node_modules/
GIT_HOOK_SCRIPT_DIR := ./.git/hooks/script/
GIT_HOOK_COMMIT_MSG := ./.git/hooks/commit-msg

develop_env: develop_env_message $(GIT_HOOK_SCRIPT_DIR) $(GIT_HOOK_COMMIT_MSG)

develop_env_message:
	@$(call MESSAGE,"Build Development Environment")

$(NODE_MODULES):
	$(if $(shell commitizen init cz-conventional-changelog --save-dev --save-exact --force),, \
		$(warning *** Please install node and commitizen.) \
		$(warning *** Download from https://nodejs.org/download/release/v5.9.0/) \
		$(warning *** Run 'sudo npm install -g commitizen') \
		$(warning Make again after installing))

$(GIT_HOOK_SCRIPT_DIR): $(NODE_MODULES)
	-ls $(NODE_MODULES) && rm -rf $@ && mkdir -p $@ && ln -s $(TOPDIR)/develop_env/commit-msg.script $@

$(GIT_HOOK_COMMIT_MSG):
	-ln -s $(TOPDIR)/develop_env/commit-msg $@

checkpatch:
	@./develop_env/commit-msg.script/checkpatch

changelog:
	@conventional-changelog -p angular -i CHANGELOG.md -s

else
all world debug prepare dirs $(OUTPUT_DIR) $(TARGET_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR):
	@echo "output/mozart/usr/include/mozart_config.h Not Found,System not configured - see 'make help'" >&2
	@exit 1
endif
else
all world debug prepare dirs $(OUTPUT_DIR) $(TARGET_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR):
	@echo "configs/config.mk Not Found,System not configured - see 'make help'" >&2
	@exit 1
endif

###########################################################################
# function config
###########################################################################
mozart_prepare:
	@echo "#" > $(TOPDIR)/configs/config.mk
	@echo "# Automatically generated make config: don't edit" >> $(TOPDIR)/configs/config.mk
	@echo "# Mozart SDK configure" >> $(TOPDIR)/configs/config.mk
	@echo "#" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

	@mkdir -p $(MOZART_DIR)/usr/include
	@echo "/*" > $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " * Automatically generated C config: don't edit" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " * Mozart SDK configure" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " */" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_RTK 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_BCM 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_AISPEECH 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_JIETONG 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_IFLYTEK 3" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_ATALK 4" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_VOICE_KEY_MIX 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_VOICE 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_KEY_SHORTPRESS 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_KEY_LONGPRESS 3" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define SONG_SUPPLYER_XIMALAYA 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define SONG_SUPPLYER_LAPSULE 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define SONG_SUPPLYER_JUSHANG 3" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define SONG_SUPPLYER_AISPEECH 4" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define TTS_BAIDU 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define TTS_AISPEECH 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define TTS_JIETONG 3" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define UI_SMARTUI 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define UI_MINIGUI 2" >> $(MOZART_DIR)/usr/include/mozart_config.h

# Main app define.
main_app_tv_audio:
	@echo "MAIN_APP=tv_audio" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# tv_audio mode define
tv_audio_planet_mode:
	@echo "SUPPORT_MODE=planet" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
tv_audio_moon_mode:
	@echo "SUPPORT_MODE=moon" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# wifi module config (SUPPORT_WIFI_MODULE=xxx) in config.mak
#   rtk_<module name>: use realtek's wifi module
#   bcm_<module name>: use broadcom's wifi module
#
# wifi module/chip config (#define SUPPORT_WIFI_MODULE xxx) in config.h
#   rtk_<module name>: use realtek's wifi module/chip
#   bcm_<module name>: use broadcom's wifi module/chip
wifi_rtl8723bs_config:
	@echo "SUPPORT_WIFI_MODULE=rtk_rtl8723bs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"rtk_rtl8723bs\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6212_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6212\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6212a_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6212a\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6181_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6181" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6181\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_bcm43438_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_bcm43438" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_bcm43438\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_bcm43362_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_bcm43362" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_bcm43362\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ssv_6xxx_config:
	@echo "SUPPORT_WIFI_MODULE=ssv_6xxx" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"ssv_6xxx\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6255_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6255" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6255\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6255_8021as_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6255_8021as" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6255_8021as\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h


# bluetooth config in config.mak
#                   0: DO NOT support bluetooth
#   rtk_<module name>: use realtek's bt
#   bcm_<module name>: use broadcom's bt
#
# bluetooth config in config.h(XXX: need define those micro or enum manually in somewhere.)
#	BT_RTK:  use realtek's bt
#	BT_BCM:  use broadcom's bt
#	BT_???:  use ???'s bt
bt_rtl8723bs_config:
	@echo "SUPPORT_BT_MODULE=rtk_rtl8723bs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_RTK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_ap6212_config:
	@echo "SUPPORT_BT_MODULE=bcm_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_ap6212a_config:
	@echo "SUPPORT_BT_MODULE=bcm_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_ap6255_config:
	@echo "SUPPORT_BT_MODULE=bcm_ap6255" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_bcm43438_config:
	@echo "SUPPORT_BT_MODULE=bcm_bcm43438" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_disable_config:

bt_hci_dev_ttyS0_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS1_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS2_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS3_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS3" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# webrtc
webrtc_aec_enable_config:
	@echo "SUPPORT_WEBRTC=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WEBRTC 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
webrtc_aec_disable_config:

# mplayer config in config.mak
#   float: float point decoder (default)
#   fixed: fixed point decoder
mplayer_float_config:
	@echo "SUPPORT_MPLAYER=float" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
mplayer_fixed_config:
	@echo "SUPPORT_MPLAYER=fixed" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# tts config in config.mak and config.h
tts_baidu_config:
	@echo "SUPPORT_TTS=baidu" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_TTS TTS_BAIDU" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
tts_aispeech_config:
	@echo "SUPPORT_TTS=aispeech" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_TTS TTS_AISPEECH" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
tts_jietong_config:
	@echo "SUPPORT_TTS=jietong" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_TTS TTS_JIETONG" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# voice recognition config in config.mak and config.h
#       1: enable
#   other: disable
vr_aispeech_config:
	@echo "SUPPORT_VR=aispeech" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_AISPEECH" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_jietong_config:
	@echo "SUPPORT_VR=jietong" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_JIETONG" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_iflytek_config:
	@echo "SUPPORT_VR=iflytek" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_IFLYTEK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_atalk_config:
	@echo "SUPPORT_VR=atalk" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_ATALK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_disable_config:

vr_wakeup_voice_key_mix_config:
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_voice_key_mix" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_VOICE_KEY_MIX" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_voice_config:
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_voice" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_VOICE" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_key_shortpress_config:
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_key_shortpress" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_KEY_SHORTPRESS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_key_longpress_config:
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_key_longpress" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_KEY_LONGPRESS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# appserver config in config.mak and config.h
#       1: enable
#   other: disable
appserver_enable_config:
	@echo "SUPPORT_APPSERVER=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_APPSERVER 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
appserver_disable_config:

# ingenicplayer config in config.mak and config.h
#       1: enable
#   other: disable
ingenicplayer_enable_config: appserver_enable_config
	@echo "SUPPORT_INGENICPLAYER=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_INGENICPLAYER 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
ingenicplayer_disable_config:

# lapsule config in config.mak and config.h
#       1: enable
#   other: disable
lapsule_enable_config:
	@echo "SUPPORT_LAPSULE=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LAPSULE 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
lapsule_disable_config:

# ximalaya search song config in config.mak and config.h
song_supplyer_ximalaya_config:
	@echo "SUPPORT_SONG_SUPPLYER=ximalaya" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SONG_SUPPLYER SONG_SUPPLYER_XIMALAYA" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
song_supplyer_lapsule_config:
	@echo "SUPPORT_SONG_SUPPLYER=lapsule" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SONG_SUPPLYER SONG_SUPPLYER_LAPSULE" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
song_supplyer_jushang_config:
	@echo "SUPPORT_SONG_SUPPLYER=jushang" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SONG_SUPPLYER SONG_SUPPLYER_JUSHANG" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
song_supplyer_aispeech_config:
	@echo "SUPPORT_SONG_SUPPLYER=aispeech" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SONG_SUPPLYER SONG_SUPPLYER_AISPEECH" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
song_supplyer_disable_config:

# atalk config in config.mak and config.h
#       1: enable
#   other: disable
atalk_enable_config:
	@echo "SUPPORT_ATALK=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ATALK 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
atalk_disable_config:

# dmr config in config.mak and config.h
#       1: enable
#   other: disable
dmr_enable_config:
	@echo "SUPPORT_DMR=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMR 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
dmr_disable_config:

# dms config in config.mak and config.h
#       1: enable
#   other: disable
dms_enable_config:
	@echo "SUPPORT_DMS=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMS 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
dms_disable_config:

# ui config in config.mak and config.h
ui_smartui_config:
	@echo "SUPPORT_UI=smartui" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_UI UI_SMARTUI" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
ui_minigui_config:
	@echo "SUPPORT_UI=minigui" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_UI UI_MINIGUI" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
ui_disable_config:

# alarm config in config.mak and config.h
#	1: enable
#	other: disable
alarm_enable_config:
	@echo "SUPPORT_ALARM=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ALARM 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
alarm_disable_config:

# airplay config in config.mak and config.h
#       1: enable
#   other: disable
airplay_enable_config:
	@echo "SUPPORT_AIRPLAY=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AIRPLAY 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
airplay_disable_config:

# localplayer config in config.mak and config.h
#       1: enable
#   other: disable
localplayer_enable_config:
	@echo "SUPPORT_LOCALPLAYER=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LOCALPLAYER 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
localplayer_disable_config:

# linein support in config.mak
#       1: enable
#   other: disable
#XXX: set to 0 in release version!!!
linein_enable_config:
	@echo "SUPPORT_LINEIN=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LINEIN 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
linein_disable_config:

# mulroom config in config.mak and config.h
# 	1: enable
#   other: disable
multiroom_enable_config:
	@echo "SUPPORT_MULROOM=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_MULROOM 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
multiroom_disable_config:

# adb debug support in config.mak
#       1: enable
#   other: disable
#XXX: set to 0 in release version!!!
adb_enable_config:
	@echo "SUPPORT_ADB_DEBUG=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
adb_disable_config:

# camera support in config.mak
#       1: enable
#   other: disable
#XXX: set to 0 in release version!!!
camera_enable_config:
	@echo "SUPPORT_CAMERA=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_CAMERA 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
camera_disable_config:

# battery support in config.mak
#       1: enable
#   other: disable
#XXX: set to 0 in release version!!!
battery_enable_config:
	@echo "SUPPORT_BATTERY=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BATTERY 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
battery_disable_config:

# rootfs type config in config.mak
#  ramdisk: build ramdisk rootfs
#     ext4: build ext4 rootfs
#   cramfs: build cramfs rootfs
#    ubifs: build ubifs rootfs
rootfs_ramdisk_config:
	@echo "SUPPORT_FS=ramdisk" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
rootfs_ext4_config:
	@echo "SUPPORT_FS=ext4" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
rootfs_cramfs_config:
	@echo "SUPPORT_FS=cramfs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
rootfs_ubifs_config:
	@echo "SUPPORT_FS=ubifs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# usrdata type config
#    jffs2: build jffs2 usrdata
#    ubifs: build ubifs usrdata
#     ext4: build ext4 usrdata
#   others: disable
usrdata_jffs2_config:
	@echo "SUPPORT_USRDATA=jffs2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
usrdata_ubifs_config:
	@echo "SUPPORT_USRDATA=ubifs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
usrdata_ext4_config:
	@echo "SUPPORT_USRDATA=ext4" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# audio type config
#    oss : oss (default)
#    alsa: alsa
audio_oss_config:
	@echo "SUPPORT_AUDIO=oss" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AUDIO_OSS 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

audio_alsa_config:
	@echo "SUPPORT_AUDIO=alsa" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AUDIO_ALSA 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

audio_exter_codec_config:
	@echo "AUDIO_CODEC=exter_codec" >> $(TOPDIR)/configs/config.mk

audio_inter_codec_config:
	@echo "AUDIO_CODEC=inter_codec" >> $(TOPDIR)/configs/config.mk

# filesystem optimize size config in config.mak
#        1: cutdown fs continous
#    other: DO NOT cutdown fs.
#XXX: Dangerous!!! set to 0 generally.
cutdown_fs_size:
	@echo "SUPPORT_CUT_CONTINUE=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# cpu model define.
cpu_model_m150:
	@echo "CPU_MODEL=M150" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
cpu_model_x1000:
	@echo "CPU_MODEL=X1000" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# product name define.
#trunk baseboard v1.0 with rtl8723bs
product_trunk_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#trunk baseboard v1.1 with ap6212
product_trunk_v1.1_ap6212_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.1_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#trunk baseboard v1.0 with ap6212
product_trunk_ap6212_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v1.0 with ap6212a
product_canna_v1.0_ap6212a_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v1.0_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v1.0 with ap6181
product_canna_v1.0_ap6181_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v1.0_ap6181" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v1.0 with ap6255
product_canna_v1.0_ap6255_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v1.0_ap6255" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v2.0 with ap6212
product_canna_v2.0_ap6212_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v2.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#pansy coreboard v1.3 with ap6181
product_pansy_v1.3_ap6181_config: cpu_model_x1000
	@echo "PRODUCT_NAME=pansy_v1.3_ap6181" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#halley2 coreboard v2.0 with ap6212(bcm43438)
product_halley2_v2.0_ap6212_config: cpu_model_x1000
	@echo "PRODUCT_NAME=halley2_v2.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
# audio device info.
audio_dev_m150_config:
	@echo "AUDIO_DEV_PLAYBACK=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_RECORD=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_PCM=/dev/dsp1" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_SPDIFOUT=/dev/dsp2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

audio_dev_x1000_config:
	@echo "AUDIO_DEV_PLAYBACK=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_RECORD=/dev/dsp3" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_PCM=/dev/dsp1" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_SPDIFOUT=/dev/dsp2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# flash info define.
#  spi nor we used now:
#      gd25q128c: 16MB
#      gd25lq64c: 8MB
#  pars:
#      FLASH_TYPE: flash type: such as spinor, spinand, emmc.
#      FLASH_PAGE_SIZE: flash page size
#      FLASH_ERASE_BLOCK_SIZE: flash earse block size
#      FLASH_LOGICAL_BLOCK_SIZE: flash logical erase block size
#      FLASH_USRDATA_PADSIZE: usrdata partition size
#      FLASH_ERASE_BLOCK_COUNT: maximum logical erase block count
flash_spi_gd25q128c_config:
	@echo "FLASH_TYPE=spinor" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x100" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_PADSIZE=0xa0000" >> $(TOPDIR)/configs/config.mk #640KB

flash_spi_gd25lq64c_config:
	@echo "FLASH_TYPE=spinor" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x100" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_PADSIZE=0x38000" >> $(TOPDIR)/configs/config.mk #224KB

flash_spi_gd5f1gq4uby1g_config:
	@echo "FLASH_TYPE=spinand" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x800" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x20000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_LOGICAL_BLOCK_SIZE=0x1f000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_BLOCK_COUNT=24" >> $(TOPDIR)/configs/config.mk #32 * 128K(PEB) = 4M - reserved size
	@echo "FLASH_UPDATER_BLOCK_COUNT=120" >> $(TOPDIR)/configs/config.mk #128 * 128K(PEB) = 16M - reserved size
	@echo "FLASH_APPFS_BLOCK_COUNT=220" >> $(TOPDIR)/configs/config.mk #256 * 128K(PEB) = 32M - reserved size
	@echo "FLASH_ERASE_BLOCK_COUNT=920" >> $(TOPDIR)/configs/config.mk #992 * 128K(PEB) = 124M - reserved size

flash_emmc_config:
	@echo "FLASH_TYPE=emmc" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk # virtual setting, ONLY used for padding nv.img

###########################################################################
#
# board config
#
###########################################################################
mozart_common_config: dmr_enable_config airplay_enable_config localplayer_enable_config ingenicplayer_enable_config \
		vr_aispeech_config vr_wakeup_voice_key_mix_config song_supplyer_aispeech_config tts_aispeech_config \
		adb_enable_config


trunk_rtl8723bs_cramfs_config: unconfig mozart_prepare product_trunk_config \
		wifi_rtl8723bs_config \
		audio_dev_m150_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for trunk board."

trunk_v1.1_ap6212_cramfs_config: unconfig mozart_prepare product_trunk_v1.1_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS1_config webrtc_aec_enable_config \
		audio_dev_m150_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for trunk v1.1 ap6212 board"

canna_ssv_ramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ssv_6xxx_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

canna_v1.0_ap6181_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6181_config \
		wifi_ap6181_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6181 board"

canna_v1.0_ap6255_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6255_config \
		wifi_ap6255_config bt_ap6255_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config dmr_enable_config multiroom_enable_config \
		airplay_enable_config localplayer_enable_config \
		ingenicplayer_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6255 board"

canna_v1.0_ap6255_cramfs_multiroom_config: unconfig mozart_prepare product_canna_v1.0_ap6255_config \
		wifi_ap6255_config bt_disable_config \
		webrtc_aec_enable_config audio_dev_x1000_config \
		dmr_enable_config dms_disable_config multiroom_enable_config \
		airplay_enable_config localplayer_enable_config \
		ingenicplayer_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs multiroom for canna v1.0 ap6255 board"

canna_v1.0_ap6255_cramfs_tv_audio_planet_config: unconfig mozart_prepare product_canna_v1.0_ap6255_config \
		main_app_tv_audio tv_audio_planet_mode \
		wifi_ap6255_config bt_disable_config \
		audio_dev_x1000_config \
		multiroom_enable_config localplayer_enable_config \
		ingenicplayer_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6255 board"

canna_v1.0_ap6255_cramfs_tv_audio_moon_config: unconfig mozart_prepare product_canna_v1.0_ap6255_config \
		main_app_tv_audio tv_audio_moon_mode \
		wifi_ap6255_config bt_disable_config \
		audio_dev_x1000_config \
		multiroom_enable_config localplayer_enable_config \
		ingenicplayer_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6255 board"

canna_v1.0_ap6212a_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

canna_v1.0_rtl8723bs_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_rtl8723bs_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

canna_v1.0_rayshine_ap6255_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6255_config \
		wifi_ap6255_config bt_ap6255_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6255 board"

canna_v1.0_ap6212a_ubifs_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_ubifs_config usrdata_ubifs_config flash_spi_gd5f1gq4uby1g_config
	@echo "Config complete sdk with ubifs for canna v1.0 ap6212a board"

canna_v1.0_ap6212a_ext4_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_ext4_config usrdata_ext4_config flash_emmc_config
	@echo "Config complete sdk with ext4 for canna v1.0 ap6212a board"

canna_v1.0_ap6212a_cramfs_alsa_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_alsa_config audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

exter_codec_voice_wakeup_ap6212a_cramfs_alsa_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_alsa_config audio_exter_codec_config audio_dev_x1000_config \
		mozart_common_config \
                rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for alsa voice wakeup with exter codec and ap6212a board"

canna_v2.0_43438_cramfs_config: unconfig mozart_prepare product_canna_v2.0_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v2.0 ap6212 board"

halley2_v2.0_43438_a1_cramfs_config: unconfig mozart_prepare product_halley2_v2.0_ap6212_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config camera_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for halley2 v2.0 43438 a1 board"

halley2_v2.0_43438_cramfs_config: unconfig mozart_prepare product_halley2_v2.0_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config camera_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for halley2 v2.0 43438 a0 board"

halley2_v2.0_43438_ubifs_config: unconfig mozart_prepare product_halley2_v2.0_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config camera_enable_config \
		rootfs_ubifs_config usrdata_ubifs_config flash_spi_gd5f1gq4uby1g_config
	@echo "Config complete sdk with ubifs for halley2 v2.0 43438 a0 board"

halley2_v2.0_43438_a1_ubifs_config: unconfig mozart_prepare product_halley2_v2.0_ap6212_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_dev_x1000_config mozart_common_config camera_enable_config \
		rootfs_ubifs_config usrdata_ubifs_config flash_spi_gd5f1gq4uby1g_config
	@echo "Config complete sdk with ubifs for halley2 v2.0 43438 a1 board"

halley2_v2.0_43438_ubifs_alsa_config: unconfig mozart_prepare product_halley2_v2.0_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config webrtc_aec_enable_config \
		audio_alsa_config audio_dev_x1000_config mozart_common_config \
		rootfs_ubifs_config usrdata_ubifs_config flash_spi_gd5f1gq4uby1g_config
	@echo "Config complete sdk with ubifs for halley2 v2.0 ap6212 board"

pansy_ap6181_cramfs_config: unconfig mozart_prepare product_pansy_v1.3_ap6181_config\
		wifi_ap6181_config \
		audio_dev_x1000_config mozart_common_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for pansy v1.3 ap6181 board"

unconfig:
	@echo "you are applying a new configure..."
	@echo -e $(shell test -d $(APPFS_DIR) && echo "You should run \'make clean\' to compile mozart in a clean environment")
	@echo

help:
	@echo "Configs:"
	@echo "    make <user>_config                      - make <user>'s config"
	@echo "                                              Should apply a valid config before build!!"
	@echo
	@echo "    trunk_v1.1_ap6212_cramfs_config         - for trunk board."
	@echo
	@echo "    canna_v1.0_ap6212a_cramfs_config        - for canna board."
	@echo
	@echo "Others:"
	@echo "    clean                                   - invoke main-clean app-config-clean updater-clean"
	@echo "    distclean                               - clean all targets"


clean: $(TARGETS_CLEAN)
	-rm -rf $(OUTPUT_DIR)/fs $(TARGET_DIR)

distclean: $(TARGETS_DISTCLEAN) clean
	-rm -rf $(MOZART_DIR) $(MOZART_UPDATER_DIR) $(OUTPUT_DIR)/host
	-rm -rf $(NODE_MODULES) $(GIT_HOOK_SCRIPT_DIR) $(GIT_HOOK_COMMIT_MSG)

show-targets:
	@echo "TOPDIR is $(TOPDIR)"
	@echo
	@echo "TARGETS(clean, distclean) is $(TARGETS)"
	@echo
	@echo "TARGETS1(clean) is $(TARGETS1)"
	@echo

.PHONY: all world dirs clean distclean \
	$(TARGETS) $(TARGETS_ALL) \
	$(TARGETS_DISTCLEAN) $(APPFS_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR)
