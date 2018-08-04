host-mtd-utils_DIR := $(TOPDIR)/tools/host-tools/mtd-utils-1.5.1
host-mtd-utils_DEPENDENCIES := host-e2fsprogs host-lzo host-zlib

mtd_DEPENDS_PREFIX := $(OUTPUT_DIR)/host/usr

mtd_ZLIBFLAGS := \
	ZLIBCPPFLAGS=-I$(mtd_DEPENDS_PREFIX)/include \
	ZLIBLDFLAGS=-L$(mtd_DEPENDS_PREFIX)/lib

mtd_LZOFLAGS := \
	LZOCPPFLAGS=-I$(mtd_DEPENDS_PREFIX)/include/lzo \
	LZOLDFLAGS=-L$(mtd_DEPENDS_PREFIX)/lib

define host-mtd-utils_BUILD_CMDS
	( \
		$(mtd_ZLIBFLAGS) \
		$(mtd_LZOFLAGS) \
		$(MAKE) V=1 -C $(host-mtd-utils_DIR) WITHOUT_XATTR=1 \
	)
endef

define host-mtd-utils_INSTALL_HOST_CMDS
	$(MAKE) -C $(host-mtd-utils_DIR) WITHOUT_XATTR=1 DESTDIR=$(OUTPUT_DIR)/host install
endef

define host-mtd-utils_DISTCLEAN_CMDS
        -$(MAKE1) -C $(host-mtd-utils_DIR) clean
endef

define host-mtd-utils_UNINSTALL_CMDS
        -$(MAKE1) -C $(host-mtd-utils_DIR) DESTDIR=$(OUTPUT_DIR)/host uninstall-sbinUBI
endef

$(eval $(call install_rules,host-mtd-utils,$(host-mtd-utils_DIR),host))

mtd-utils_DIR := $(TOPDIR)/tools/device-tools/mtd-utils-1.5.1
mtd-utils_DEPENDENCIES := host-e2fsprogs lzo

#TARGETS += mtd-utils
#TARGETS1 += mtd-utils

define mtd-utils_prepare_sourcecode
	(rm -rf $(@D); cp -af $(host-mtd-utils_DIR) $(@D); \
		make -C $(@D) clean; \
		rm -fr $(@D)/{.stamp_patched,.stamp_configured,.stamp_built,.stamp_target_installed,.stamp_host_installed} \
	)
endef

mtd-utils_PRE_PATCH_HOOKS += mtd-utils_prepare_sourcecode

define mtd-utils_BUILD_CMDS
	( \
		CROSS=mipsel-linux- \
		$(MAKE) V=1 -C $(@D) WITHOUT_XATTR=1 ubi-utils \
	)
endef

define mtd-utils_INSTALL_TARGET_CMDS
	( \
		CROSS=mipsel-linux- \
		$(MAKE) -C $(@D) WITHOUT_XATTR=1 DESTDIR=$(MOZART_UPDATER_DIR)/ install-sbinUBI \
	)
endef

define mtd-utils_DISTCLEAN_CMDS
        -rm -rf $(mtd-utils_DIR)
endef

define mtd-utils_UNINSTALL_TARGET_CMDS
        -$(MAKE1) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) uninstall-sbinUBI
endef

$(eval $(call install_rules,mtd-utils,$(mtd-utils_DIR),target))
