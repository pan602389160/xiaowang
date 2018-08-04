define ifgt
$(shell file_size=`du -b $1 | cut -f1` && \
if [[ $$file_size -gt $2 ]];then echo "greater"; else :; fi;)
endef

# $1: string
define strlen
$(shell echo `expr length $(1)`)
endef

# $1: dec number
# $2: hex number
# return $1 x $2
define mul
$(shell expr $(1) \* `printf "%d" $(2)`)
endef

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

define install_molib_package
	if [ ! -f $(1) ];then\
		echo "**** [Error] $(1): No such file";\
		false;\
	else\
		echo "install $(1)";\
		for src in `cat $(1)`;\
		do\
			target=`echo $$$$src | sed -e "s/^output\/molib\/\([^/]*\)\/.*$$$$/\1/g" -e "s/^output\/molib\/\([^/]*\)$$$$/\1/g"`;\
			path=`echo $$$$src | sed -e "s/^output\/molib\/[^/]*\///g" -e "s/^output\/molib\/\([^/]*\)$$$$//g"`;\
			if [ x"app" == x$$$$target ];then \
				dst=$(APPFS_DIR)/$$$$path;\
			elif [ x"updater" == x$$$$target ];then\
				dst=$(UPDATERFS_DIR)/$$$$path;\
			elif [ x"test" == x$$$$target ];then\
				dst=$(TESTFS_DIR)/$$$$path;\
			fi;\
			if [ -d $$$$src ];then\
				mkdir -p $$$$dst;\
			else\
				mkdir -p `echo $$$$dst | sed -e "s/\/[^\/]*$$$$//g"`;\
				cp -d $$$$src $$$$dst;\
			fi;\
		done;\
	fi
endef

define prepare_package
	@$(call install_molib_package,output/molib/package/openssl.list)
	@$(call install_molib_package,output/molib/package/curl.list)
	@$(call install_molib_package,output/molib/package/iw.list)
	@$(call install_molib_package,output/molib/package/json-c.list)
	@$(call install_molib_package,output/molib/package/zlib.list)

	@$(call install_molib_package,output/molib/package/linklist.list)
	@$(call install_molib_package,output/molib/package/libutils.list)
	@$(call install_molib_package,output/molib/package/libini.list)
	@$(call install_molib_package,output/molib/package/libmad.list)
	@$(call install_molib_package,output/molib/package/libnl.list)
	@$(call install_molib_package,output/molib/package/libchannels.list)
	@$(call install_molib_package,output/molib/package/librecord.list)
	@$(call install_molib_package,output/molib/package/liburl.list)
	@$(call install_molib_package,output/molib/package/libresample.list)
	@$(call install_molib_package,output/molib/package/liblocalstream.list)
	@$(call install_molib_package,output/molib/package/libtips.list)
	@$(call install_molib_package,output/molib/package/volume.list)
	@$(call install_molib_package,output/molib/package/libsharememory.list)
	@$(call install_molib_package,output/molib/package/dsd2pcm.list)
	@$(call install_molib_package,output/molib/package/power.list)
	@$(call install_molib_package,output/molib/package/player.list)
	@$(call install_molib_package,output/molib/package/network-manager-v2.list)
	@$(call install_molib_package,output/molib/package/event-manager.list)

#	@$(call install_molib_package,output/molib/package/libcgi.list)
	@$(call install_molib_package,output/molib/package/jansson.list)
	@$(call install_molib_package,output/molib/package/libnvrw.list)
	@$(call install_molib_package,output/molib/package/libmusiclist.list)

	@$(call install_molib_package,output/molib/package/libcunit.list)
	@$(call install_molib_package,output/molib/package/liblivemulti.list)
# adb
ifeq ("$(SUPPORT_ADB_DEBUG)","1")
	@$(call install_molib_package,output/molib/package/adbd.list)
endif

# mplayer
	@$(call install_molib_package,output/molib/package/mplayer.list)
ifeq ("$(SUPPORT_MPLAYER)","fixed") # fixed
	@mv $(APPFS_DIR)/usr/bin/mplayer-fixed $(APPFS_DIR)/usr/bin/mplayer
	@rm $(APPFS_DIR)/usr/bin/mplayer-float
else # fixed
	@mv $(APPFS_DIR)/usr/bin/mplayer-float $(APPFS_DIR)/usr/bin/mplayer
	@rm $(APPFS_DIR)/usr/bin/mplayer-fixed
endif # fixed

# ingenicplayer
ifeq ("$(SUPPORT_INGENICPLAYER)","1")
	@$(call install_molib_package,output/molib/package/libappserver.list)
endif

# lapsule
ifeq ("$(SUPPORT_LAPSULE)","1")
	@$(call install_molib_package,output/molib/package/lapsule.list)
endif

# ui
## lcd
ifeq ("$(SUPPORT_UI)", "lcd")
	@$(call install_molib_package,output/molib/package/liblcd.list)
endif # lcd
## smartui
ifeq ("$(SUPPORT_UI)", "smartui")
	@$(call install_molib_package,output/molib/package/libsmartui.list)
endif #smartui

# alarm
ifeq ("$(SUPPORT_ALARM)","1")
	@$(call install_molib_package,output/molib/package/alarm.list)
endif

# dms
ifeq ("$(SUPPORT_DMS)","1")
	@$(call install_molib_package,output/molib/package/dms.list)
	@$(call install_molib_package,output/molib/package/ushare.list)
	@$(call install_molib_package,output/molib/package/libupnp.list)
endif

# dmr
ifeq ("$(SUPPORT_DMR)","1")
	@$(call install_molib_package,output/molib/package/render.list)
	@$(call install_molib_package,output/molib/package/libupnp.list)
endif

# mulroom
ifeq ("$(SUPPORT_MULROOM)","1")
	@$(call install_molib_package,output/molib/package/multiroomer.list)
endif

ifeq ("$(SUPPORT_AIRPLAY)","1")
	@$(call install_molib_package,output/molib/package/shairport.list)
endif

ifeq ("$(SUPPORT_LOCALPLAYER)","1")
	@$(call install_molib_package,output/molib/package/moplayer.list)
endif

ifeq ("$(SUPPORT_WEBRTC)","1")
	@$(call install_molib_package,output/molib/package/webrtc.list)
endif

# atalk
ifeq ("$(SUPPORT_ATALK)","1")
	@$(call install_molib_package,output/molib/package/atalk.list)
endif

# audio_type
## alsa
ifeq ("$(SUPPORT_AUDIO)", "alsa")
	@$(call install_molib_package,output/molib/package/alsa-lib.list)
	@$(call install_molib_package,output/molib/package/alsa-utils.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v $(SUPPORT_AUDIO)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/player.ini -w -s mplayer -k ao -v $(SUPPORT_AUDIO)
	@-$(STRIP) $(APPFS_DIR)/usr/lib/alsa-lib/smixer/*
	@-ln -sf /usr/fs/etc/alsa/asound.conf $(UPDATERFS_DIR)/.asoundrc
	@-ln -sf /usr/fs/usr/lib/alsa-lib $(UPDATERFS_DIR)/usr/lib/alsa-lib
	@-ln -sf /usr/fs/etc/alsa $(UPDATERFS_DIR)/etc/alsa
endif # alsa
## oss
ifeq ("$(SUPPORT_AUDIO)", "oss")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v $(SUPPORT_AUDIO)
endif # oss
## empty
ifeq ("$(SUPPORT_AUDIO)", "")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v oss
endif # empty
	@audio_type=`$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -r -s audio -k type`;\
	if [ x$$$$audio_type == x ];then\
		echo "**** [Error] $(SUPPORT_AUDIO): NOT supported audio type!!!!" && exit 1;\
	fi

# song supplyer
## lapsule
ifeq ("$(SUPPORT_SONG_SUPPLYER)", "lapsule")
	@$(call install_molib_package,output/molib/package/lapsule.list)
endif # lapsule
## ximalaya
#ifeq ("$(SUPPORT_SONG_SUPPLYER)", "ximalaya") TODO
	@$(call install_molib_package,output/molib/package/ximalaya.list)
#endif # ximalaya
## jushang
#ifeq ("$(SUPPORT_SONG_SUPPLYER)", "jushang") TODO
	@$(call install_molib_package,output/molib/package/vr-jushang.list)
#endif # jushang

## tts_jietong
ifeq ("$(SUPPORT_TTS)", "jietong")
	@$(call install_molib_package,output/molib/package/libtts-jietong.list)
endif # tts_jietong

# vr
## vr_jietong
ifeq ("$(SUPPORT_VR)", "jietong")
	@$(call install_molib_package,output/molib/package/vr-jietong.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	@$(call install_molib_package,output/molib/package/libwakeup-jietong.list)
endif # vr_jietong

## vr_baidu
ifeq ("$(SUPPORT_VR)", "baidu")
	@$(call install_molib_package,output/molib/package/baidu.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
endif # vr_baidu
## vr_speech
ifeq ("$(SUPPORT_VR)", "aispeech")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
endif # vr_speech
## vr_iflytek
ifeq ("$(SUPPORT_VR)", "iflytek")
	@$(call install_molib_package,output/molib/package/vr-iflytek.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
endif # vr_iflytek
## vr_unisound
ifeq ("$(SUPPORT_VR)", "unisound")
	@: #TODO
#	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
endif # vr_unisound
## vr_atalk
ifeq ("$(SUPPORT_VR)", "atalk")
	@$(call install_molib_package,output/molib/package/vr-atalk.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
endif # vr_atalk

ifneq ("$(SUPPORT_VR)", "")
	@vr_type=`$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -r -s product -k vr`;\
	if [ x$$$$vr_type == x ];then\
		echo "**** [Error] $(SUPPORT_VR): NOT supported vr!!!!" && exit 1;\
	fi
endif

# bt
## realtek bt
ifneq ("$(strip $(patsubst rtk_%, %, $(SUPPORT_BT_MODULE)))", "")
	@: #TODO bluz
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k bt -v rtk
ifeq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "rtl8723bs") # RTL8723BS
	@cp -a $(TOPDIR)/configs/firmwares/rtl8723bs/bt/* $(UPDATERFS_DIR)/lib/firmware/
endif # RTL8723BS
endif # realtek bt
## broadcom bt
ifneq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "")
	@$(call install_molib_package,output/molib/package/broadcom.list)
	@$(call install_molib_package,output/molib/package/bsa.list)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k bt -v bcm
	@sed -i 's!-d /dev/ttyS1!-d $(SUPPORT_BT_HCI_DEV)!g' $(APPFS_DIR)/etc/init.d/S04bsa.sh
ifeq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "ap6212")  # ap6212 module, bcm43438a0 inside
	@cp -a $(TOPDIR)/configs/firmwares/ap6212/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
endif # ap6212
ifeq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "ap6212a")  # ap6212a module, bcm43438a1 inside
	@cp -a $(TOPDIR)/configs/firmwares/ap6212a/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
endif # ap6212a
ifeq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "ap6255")  # ap6255 module, bcm43455 inside
	@cp -a $(TOPDIR)/configs/firmwares/ap6255/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
endif # ap6255

ifeq ("$(strip $(patsubst bcm_%, %, $(SUPPORT_BT_MODULE)))", "bcm43438") # bcm43438a0, core on broad
	@cp -a $(TOPDIR)/configs/firmwares/43438/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
endif # bcm43438
endif # broadcom bt

# wifi
## realtek wifi
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "rtk")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k wifi -v rtk
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_cli_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_cli
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	@mv $(APPFS_DIR)/usr/sbin/hostapd_broadcom $(APPFS_DIR)/usr/sbin/hostapd
	@mv $(APPFS_DIR)/usr/sbin/hostapd_cli_broadcom $(APPFS_DIR)/usr/sbin/hostapd_cli
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_realtek
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_cli_realtek
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_realtek
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_realtek
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_cli_realtek
endif # realtek
## broadcom wifi
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "bcm")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k wifi -v bcm
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_cli_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_cli
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	@mv $(APPFS_DIR)/usr/sbin/hostapd_broadcom $(APPFS_DIR)/usr/sbin/hostapd
	@mv $(APPFS_DIR)/usr/sbin/hostapd_cli_broadcom $(APPFS_DIR)/usr/sbin/hostapd_cli
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_realtek
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_cli_realtek
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_realtek
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_realtek
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_cli_realtek
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6212") # ap6212 module, bcm43438a0 inside
	@cp -r $(TOPDIR)/configs/firmwares/ap6212/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # ap6212
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6212a") # ap6212a module, bcm43438a1 inside
	@cp -r $(TOPDIR)/configs/firmwares/ap6212a/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # ap6212a
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "bcm43438") # bcm43438a0, core on broad
	@cp -r $(TOPDIR)/configs/firmwares/43438/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # bcm43438
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6181") # ap6181 module, bcm43362 inside.
	@cp -r $(TOPDIR)/configs/firmwares/ap6181/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # ap6181
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6255") # ap6255 module, bcm43455 inside.
	@cp -r $(TOPDIR)/configs/firmwares/ap6255/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # ap6255
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6255_8021as") # ap6255_8021as module, bcm43455 inside.
	@cp -r $(TOPDIR)/configs/firmwares/ap6255_8021as/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # ap6255_8021as
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "bcm43362") # bcm43362, core on broad.
	@cp -r $(TOPDIR)/configs/firmwares/43362/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # bcm43362
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "iw8101") # iw8101 module, bcm43362 inside.
	@cp -r $(TOPDIR)/configs/firmwares/iw8101/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif # iw8101
endif # broadcom
## ssv wifi
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "ssv")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k wifi -v ssv
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_cli_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_cli
	@mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	@mv $(APPFS_DIR)/usr/sbin/hostapd_realtek $(APPFS_DIR)/usr/sbin/hostapd
	@mv $(APPFS_DIR)/usr/sbin/hostapd_cli_realtek $(APPFS_DIR)/usr/sbin/hostapd_cli
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_broadcom
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_cli_broadcom
	@rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_broadcom
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_broadcom
	@rm -f $(APPFS_DIR)/usr/sbin/hostapd_cli_broadcom
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "6xxx") # ssv-6xxx module.
	@-$(STRIP) $(TOPDIR)/configs/firmwares/ssv-6xxx/wifi/ssv6051.ko --strip-unneeded
	@cp -r $(TOPDIR)/configs/firmwares/ssv-6xxx/wifi/* $(UPDATERFS_DIR)/lib/firmware/
	@mv $(UPDATERFS_DIR)/lib/firmware/load.sh $(UPDATERFS_DIR)/etc/init.d/S01wifi.sh
	@chmod +x $(UPDATERFS_DIR)/etc/init.d/S01wifi.sh
endif # ssv-6xxx
endif # ssv

endef # prepare_package

rootfs:
	@echo
	@echo
	@echo
	@$(call MESSAGE,"Targets:")
	@for file in $(TARGET_DIR)/*; do \
		echo -e "$$file: `$(TOPDIR)/configs/scripts/get_readable_filesize.sh $$file`"; \
	done
	@echo


update:
	@echo
	@$(call MESSAGE,"Ota Packages:")
	@echo
	@for product in output/updatepkg/*; do \
		echo "product: `basename $$product`"; \
		for version in $$product/v*; do \
			echo "    version: `basename $$version`"; \
			for file in $$version/*; do \
				echo -e "        $$file: `$(TOPDIR)/configs/scripts/get_readable_filesize.sh $$file`"; \
			done \
		done \
	done
	@echo

usrdata:

##############################################
#         install usrdata rules           #
##############################################
define install_usrdata_rules
# Params 1: fs type:
#      jffs2: make jffs2
#      ubifs: make ubifs
#      ext4: make ext4

$(TARGET_DIR)/usrdata.$(1): usrdata-$(1)-prepare
	@$(call MESSAGE,"Build usrdata.$(1)")
	$(call USRDATA_$(1)_CMDS,$(USRDATAFS_DIR),$$@)

usrdata-$(1)-prepare: $$(USRDATA_$(1)_DEPENDENCIES)
	-rm -rf $(USRDATAFS_DIR); mkdir -p $(USRDATAFS_DIR)
	cp $(UPDATERFS_DIR)/usr/share/data/* $(USRDATAFS_DIR)/

usrdata: $(TARGET_DIR)/usrdata.$(1)
endef

##############################################
#         install rootfs rules           #
##############################################
fakeroot_bin=$(OUTPUT_DIR)/host/usr/bin/fakeroot
FAKEROOT_SCRIPT = $(TARGET_DIR)/fakeroot.fs
DEVICE_TABLES = $(TARGET_DIR)/devs.txt

define install_fs_rules
# Params 1: fs type:
#   ramdisk: make ramdisk
#      ext4: make ext4
#    cramfs: make cramfs
#     ubifs: make ubifs
# Params 2: need compress the target after make fs done?
#    1: compress with gzip -9
#    0: do not compress

ROOTFS_$(1)_DEPENDENCIES += host-fakeroot host-makedevs host-inirw

rootfs-$(1)-show-depends:
	@echo $$(ROOTFS_$(1)_DEPENDENCIES)

$(TARGET_DIR)/updater.$(1)::
	@$(call MESSAGE,"Build updater.$(1)")
	@cp -f $(TOPDIR)/configs/devs.txt $(DEVICE_TABLES)
	echo "chown -R 0:0 $(UPDATERFS_DIR)" > $(FAKEROOT_SCRIPT)
	echo "makedevs -d $(DEVICE_TABLES) $(UPDATERFS_DIR) 2>/dev/null" >> $(FAKEROOT_SCRIPT)
	echo "$(call ROOTFS_UPDATER_$(1)_CMDS,$(UPDATERFS_DIR),$$@)" >> $(FAKEROOT_SCRIPT)
	chmod a+x $(FAKEROOT_SCRIPT)
	PATH=$(OUTPUT_DIR)/host/usr/bin:$(OUTPUT_DIR)/host/usr/sbin:$(PATH) \
	     $(fakeroot_bin) -l $(OUTPUT_DIR)/host/usr/lib/libfakeroot.so -f $(OUTPUT_DIR)/host/usr/bin/faked $(FAKEROOT_SCRIPT)
ifeq ($(2),1)
	gzip -9 -c $$@ > $$@.gz
endif
	@rm -f $(FAKEROOT_SCRIPT) $(DEVICE_TABLES)

$(TARGET_DIR)/appfs.$(1)::
	@$(call MESSAGE,"Build appfs.$(1)")
	echo "chown -R 0:0 $(APPFS_DIR)" > $(FAKEROOT_SCRIPT)
	echo "$(call ROOTFS_APP_$(1)_CMDS,$(APPFS_DIR),$$@)" >> $(FAKEROOT_SCRIPT)
	chmod a+x $(FAKEROOT_SCRIPT)
	PATH=$(OUTPUT_DIR)/host/usr/bin:$(OUTPUT_DIR)/host/usr/sbin:$(PATH) \
	     $(fakeroot_bin) -l $(OUTPUT_DIR)/host/usr/lib/libfakeroot.so -f $(OUTPUT_DIR)/host/usr/bin/faked $(FAKEROOT_SCRIPT)
ifeq ($(2),1)
	gzip -9 -c $$@ > $$@.gz
endif
	rm -f $(FAKEROOT_SCRIPT)

rootfs-$(1)-prepare: rootfs-pre-prepare rootfs-prepare
	mkdir -p $(UPDATERFS_DIR)/usr/share/data/
	mkdir -p $(UPDATERFS_DIR)/usr/data $(APPFS_DIR)/usr/data # avoid cp error next 2 lines.
	cp -rf $(UPDATERFS_DIR)/usr/data $(UPDATERFS_DIR)/usr/share/
	cp -rf $(APPFS_DIR)/usr/data $(UPDATERFS_DIR)/usr/share/
	rm -rf $(APPFS_DIR)/usr/data

rootfs-$(1)-fs: $(TARGET_DIR)/updater.$(1) $(TARGET_DIR)/appfs.$(1)

rootfs-$(1):$$(ROOTFS_$(1)_DEPENDENCIES) host-$(1)-fakeroot-check rootfs-$(1)-prepare rootfs-$(1)-fs

rootfs:rootfs-$(1) usrdata nvimg

rootfs-pre-prepare:

rootfs-prepare:
	@$(call MESSAGE,"Prepare $(UPDATERFS_DIR) and $(APPFS_DIR)")
	@-rm -rf $(UPDATERFS_DIR); mkdir -p $(UPDATERFS_DIR)
	@mkdir -p $(UPDATERFS_DIR)/{bin,lib,sbin,usr}
	@mkdir -p $(UPDATERFS_DIR)/lib/firmware
	@mkdir -p $(UPDATERFS_DIR)/{dev,root,var,tmp,proc,mnt,sys}
	@mkdir -p $(UPDATERFS_DIR)/{usr/lib,usr/bin,usr/sbin}
	@ln -sf /tmp $(UPDATERFS_DIR)/run
	@-rm -rf $(APPFS_DIR); mkdir -p $(APPFS_DIR)
	@-rm -rf $(TESTFS_DIR); mkdir -p $(TESTFS_DIR)

	@$(call MESSAGE,"Prepare package")
	@cp $(TOPDIR)/rootfs/updater/. $(UPDATERFS_DIR)/ -a
	@cp $(MOZART_UPDATER_DIR)/. $(UPDATERFS_DIR)/ -a
	@cp $(TOPDIR)/rootfs/app/. $(APPFS_DIR)/ -a
	@cp $(MOZART_APP_DIR)/. $(APPFS_DIR)/ -a
	@cp $(MOZART_TEST_DIR)/. $(TESTFS_DIR)/ -a
	@$(call prepare_package)

	@$(call MESSAGE,"Prepare nv settings")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k storage -v $(FLASH_TYPE)
ifeq ("$(FLASH_TYPE)","spinor")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mtdblock1
endif
ifeq ("$(FLASH_TYPE)","spinand")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mtdblock1
endif
ifeq ("$(FLASH_TYPE)","emmc")
	# reset nv blkname.
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mmcblk0p2
endif

	@$(call MESSAGE,"Prepare device info")
# correct fs type
ifeq ("$(1)","ext4")
	# auto-mount extral mmc card.
	sed -i 's/^mmcblk\[0/^mmcblk\[1/g' $(UPDATERFS_DIR)/etc/mdev.conf
	sed -i 's/\(mount -t USRDATAFS\ \)\(.*\)\(\ \/usr\/data\)/\1\/dev\/mmcblk0p3\3/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
	sed -i 's/\(mount -t USRFSFS\ \)\(.*\)\(\ \/usr\/fs\)/\1\/dev\/mmcblk0p6\3/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
endif
ifeq ("$(1)","ubifs")
	sed -i 's/\(mount -t USRDATAFS\ \)\(.*\)\(\ \/usr\/data\)/\1ubi0:usrdata\3/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
	sed -i 's/\(mount -t USRFSFS\ \)\(.*\)\(\ \/usr\/fs\)/\1ubi2:app\3/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
endif
	sed -i 's/USRFSFS/$(SUPPORT_FS)/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
	sed -i 's/USRDATAFS/$(SUPPORT_USRDATA)/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh

# cpu model filter.
ifneq ("$(CPU_MODEL)","")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k cpu -v $(CPU_MODEL)
endif
# product filter.
ifneq ("$(PRODUCT_NAME)","")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k name -v $(PRODUCT_NAME)
endif
# audio device filter.
ifneq ("$(AUDIO_DEV_PLAYBACK)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_playback -v $(AUDIO_DEV_PLAYBACK)
endif
ifneq ("$(AUDIO_DEV_RECORD)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_record -v $(AUDIO_DEV_RECORD)
endif
ifneq ("$(AUDIO_DEV_PCM)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_pcm -v $(AUDIO_DEV_PCM)
endif
ifneq ("$(AUDIO_DEV_SPDIFOUT)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_spdifout -v $(AUDIO_DEV_SPDIFOUT)
endif
ifneq ("$(AUDIO_CODEC)","")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k codec -v $(AUDIO_CODEC)
endif

	@$(call MESSAGE,"Remove useless files")
	find $(UPDATERFS_DIR) \( -name '*.a' -o -name '*.la' \) | xargs rm -rf
	find $(UPDATERFS_DIR) \( -iname '.svn' -o -name '*.git' -o -name '.gitignore' \) | xargs rm -rf
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/ppp*
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/chat
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/usb_modeswitch
	-rm -rf $(UPDATERFS_DIR)/usr/include
	-rm -rf $(UPDATERFS_DIR)/usr/man
	-rm -rf $(UPDATERFS_DIR)/usr/lib/pkgconfig
	-rm -rf $(UPDATERFS_DIR)/usr/share/{gdb,man,info,doc}
	-rm -rf $(UPDATERFS_DIR)/usr/share/aclocal
	ln -sf /usr/fs/var/www $(UPDATERFS_DIR)/var/www
	ln -sf /usr/fs/usr/share/render $(UPDATERFS_DIR)/usr/share/render
	ln -sf /usr/fs/usr/share/vr/res $(UPDATERFS_DIR)/usr/share/vr/res
	ln -sf /usr/data/player.ini $(UPDATERFS_DIR)/etc/player.ini
	ln -sf /tmp/resolv.conf $(UPDATERFS_DIR)/etc/resolv.conf
	find $(APPFS_DIR) \( -name '*.a' -o -name '*.la' \) | xargs rm -rf
	find $(APPFS_DIR) \( -name '.svn' -o -name '*.git' -o -name '.gitignore' \) | xargs rm -rf
	-rm -rf $(APPFS_DIR)/usr/include
	-rm -rf $(APPFS_DIR)/usr/man
	-rm -rf $(APPFS_DIR)/usr/lib/pkgconfig
	-rm -rf $(APPFS_DIR)/usr/share/{gdb,man,info,doc}
	-rm -rf $(APPFS_DIR)/usr/share/aclocal
	-rm -rf $(APPFS_DIR)/usr/lib/{dbus-1.0,glib-2.0}
	-rm -rf $(APPFS_DIR)/usr/lib/libfoo.so
	-rm -rf $(APPFS_DIR)/firmware
	find $(TESTFS_DIR) \( -name '*.a' -o -name '*.la' \) | xargs rm -rf
	find $(TESTFS_DIR) \( -name '.svn' -o -name '*.git' -o -name '.gitignore' \) | xargs rm -rf
	-rm -rf $(TESTFS_DIR)/usr/doc
	-rm -rf $(TESTFS_DIR)/usr/include
	-rm -rf $(TESTFS_DIR)/usr/share/CUnit
	-rm -rf $(TESTFS_DIR)/usr/share/man
	-rm -rf $(TESTFS_DIR)/usr/lib/pkgconfig
	-mv $(APPFS_DIR)/usr/sbin/iwconfig $(UPDATERFS_DIR)/usr/sbin/
	-mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase $(APPFS_DIR)/usr/sbin/

# cutdown fs size filter
ifeq ("$(SUPPORT_CUT_CONTINUE)","1")
	-rm -f $(UPDATERFS_DIR)/lib/{libanl*,libBrokenLocale*,libcidn*,libmemusage.so,libnsl*,libnss*,libpcprofile.so,libSegFault.so,libutil*,libthread_db*}
	-rm -rf $(APPFS_DIR)/usr/test
	-rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	-rm -f $(UPDATERFS_DIR)/usr/sbin/{ifrename,iwevent,iwspy,iwconfig,iwgetid,iwpriv}
	-rm -f $(UPDATERFS_DIR)/usr/bin/{linuxrw}
endif

ifneq ("$(DEVICE_DIR)","")
	$(DEVICE_DIR)/board.sh $(DEVICE_DIR) $(UPDATERFS_DIR) $(APPFS_DIR)
endif

	@$(call MESSAGE,"Strip ELF files")
	@-$(STRIP) $(UPDATERFS_DIR)/bin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/lib/*so* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/bin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/lib/*so* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/bin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/lib/*so* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/bin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/lib/*so* > /dev/null 2>&1

host-$(1)-fakeroot-check:
	@test -x $(fakeroot_bin) || \
		(echo "fakeroot is broken, please run 'make host-fakeroot-rebuild' Firstly!!" && exit 1)

update: update-prepare update-$(1)-updater update-$(1)-appfs update-nvimg updatepkg-gen

updatepkg-gen:
	@echo "DEVICE_OTA_DIR: $(DEVICE_OTA_DIR)"
	@$(TOPDIR)/tools/host-tools/update_pack/tool/gen_updatepkg.sh \
		-v `sed -n 's/\(  "version": "\)\(.*\)\(",\)/\2/p' $(DEVICE_OTA_DIR)/package.json` \
		-m $(TOPDIR) \
		-o $(UPDATEPKG_DIR) \
		-s $(FLASH_TYPE) \
		-p $(PRODUCT_NAME)
	@rm $(UPDATEPKG_DIR)/temp -rf

update-prepare:
	@mkdir -p $(UPDATEPKG_DIR)/temp
update-$(1)-updater:
	@$(call UPDATE_UPDATER_$(1)_CMDS)
update-$(1)-appfs:
	@$(call UPDATE_APP_$(1)_CMDS)

endef

nvimg: nvimg-prepare
	@$(call MESSAGE,"Build nv.img")
	@$(TOPDIR)/output/host/usr/bin/nvgen \
		-c $(TOPDIR)/configs/nvimage.ini \
		-o $(TARGET_DIR)/nv.img \
		-p $(call mul,2,$(FLASH_ERASE_BLOCK_SIZE))

nvimg-prepare:
	@cp $(DEVICE_OTA_DIR)/ota.ini $(TOPDIR)/configs/nvimage.ini
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k product -v $(PRODUCT_NAME)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k storage -v $(FLASH_TYPE)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k current_version \
		-v `sed -n 's/\(  "version": "\)\(.*\)\(",\)/\2/p' $(DEVICE_OTA_DIR)/package.json`
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k url -v \
		`$(OUTPUT_DIR)/host/usr/bin/inirw -f $(DEVICE_OTA_DIR)/ota.ini -r -s ota -k url`
ifeq ("$(FLASH_TYPE)","spinor")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_times
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mtdblock5
endif
ifeq ("$(FLASH_TYPE)","spinand")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_once
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mtdblock6
endif
ifeq ("$(FLASH_TYPE)","emmc")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_once
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mmcblk0p7
endif

update-nvimg:
	@#cp -f $(TARGET_DIR)/nv.img $(UPDATEPKG_DIR)/temp

