src_DIR := $(TOPDIR)/tools/host-tools/mtd-utils-1.5.1
flash_erase_DIR := $(TOPDIR)/tools/device-tools/flash_erase-src

#TARGETS += flash_erase
#TARGETS1 += flash_erase

define flash_erase_prepare_sourcecode
	(rm -rf $(@D); cp -af $(src_DIR) $(@D); \
		make -C $(@D) clean; \
		rm -fr $(@D)/{.stamp_patched,.stamp_configured,.stamp_built,.stamp_target_installed,.stamp_host_installed} \
	)
endef

flash_erase_PRE_PATCH_HOOKS += flash_erase_prepare_sourcecode

define flash_erase_BUILD_CMDS
	( \
		CROSS=mipsel-linux- \
		$(MAKE) V=1 -C $(@D) WITHOUT_XATTR=1 flash-utils \
	)
endef

define flash_erase_INSTALL_TARGET_CMDS
	( \
		CROSS=mipsel-linux- \
		$(MAKE) -C $(@D) WITHOUT_XATTR=1 DESTDIR=$(MOZART_UPDATER_DIR)/ install-sbinFLASH \
	)
endef

define flash_erase_DISTCLEAN_CMDS
        -rm -rf $(@D) $(MOZART_UPDATER_DIR)/usr/sbin/flash_erase
endef

$(eval $(call install_rules,flash_erase,$(flash_erase_DIR),target))
