host-lzo_DIR := $(TOPDIR)/tools/host-tools/lzo-2.09

TARGETS += host-lzo


define host-lzo_CONFIGURE_CMDS
	(cd $(@D); \
		./configure --prefix=$(OUTPUT_DIR)/host/usr \
	)
endef

define host-lzo_BUILD_CMDS
	$(MAKE) -C $(host-lzo_DIR)
endef

define host-lzo_INSTALL_HOST_CMDS
	$(MAKE) -C $(host-lzo_DIR) install
endef

define host-lzo_CLEAN_CMDS
	-$(MAKE1) -C $(host-lzo_DIR) clean
endef

define host-lzo_DISTCLEAN_CMDS
	-$(MAKE1) -C $(host-lzo_DIR) distclean
endef

define host-lzo_UNINSTALL_HOST_CMDS
	-$(MAKE1) -C $(host-lzo_DIR) uninstall
endef

$(eval $(call install_rules,host-lzo,$(host-lzo_DIR),host))



lzo_DIR := $(TOPDIR)/tools/device-tools/lzo-2.09

define lzo_prepare_sourcecode
	(cp -af $(host-lzo_DIR) $(@D); \
		rm -fr $(@D)/{.stamp_patched,.stamp_configured,.stamp_built,.stamp_target_installed,.stamp_host_installed} \
	)
endef

lzo_PRE_PATCH_HOOKS += lzo_prepare_sourcecode

define lzo_CONFIGURE_CMDS
	(cd $(@D); \
		./configure --prefix=$(MOZART_UPDATER_DIR)/usr --host=mipsel-linux \
	)
endef

define lzo_BUILD_CMDS
	$(MAKE) -C $(lzo_DIR)
endef

define lzo_INSTALL_TARGET_CMDS
	$(MAKE) -C $(lzo_DIR) install
endef

define lzo_CLEAN_CMDS
	-$(MAKE1) -C $(lzo_DIR) clean
endef

define lzo_DISTCLEAN_CMDS
	-$(MAKE1) -C $(lzo_DIR) distclean
endef

define lzo_UNINSTALL_HOST_CMDS
	-$(MAKE1) -C $(lzo_DIR) uninstall
endef

$(eval $(call install_rules,lzo,$(lzo_DIR),target))
