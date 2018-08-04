minigui-gpl_DIR := $(TOPDIR)/tools/device-tools/minigui/minigui-gpl
resource_DIR := $(TOPDIR)/tools/device-tools/minigui/resource

ifeq ("$(SUPPORT_UI)", "minigui")
TARGETS += minigui-gpl
endif

define minigui-gpl_CONFIGURE_CMDS
	(cd $(@D); \
		./configure --enable-shared --host=mips-linux-gnu --prefix=$(MOZART_APP_DIR)/usr \
	)
endef

define minigui-gpl_BUILD_CMDS
	$(MAKE) -C $(minigui-gpl_DIR)
endef

define minigui-gpl_INSTALL_TARGET_CMDS
	$(MAKE) -C $(minigui-gpl_DIR) install; \
	mkdir -p $(MOZART_UPDATER_DIR)/etc; \
	mkdir -p $(MOZART_UPDATER_DIR)/usr/local; \
	cp -a $(resource_DIR)/MiniGUI.cfg $(MOZART_UPDATER_DIR)/etc; \
	cp -a $(resource_DIR)/share $(MOZART_UPDATER_DIR)/usr/local
endef

define minigui-gpl_CLEAN_CMDS
	-$(MAKE) -C $(minigui-gpl_DIR) clean
endef

define minigui-gpl_DISTCLEAN_CMDS
	-$(MAKE) -C $(minigui-gpl_DIR) distclean; rm -rf $(minigui-gpl_DIR)/.stamp_*
endef

define minigui-gpl_UNINSTALL_TARGET_CMDS
	-$(MAKE) -C $(minigui-gpl_DIR) uninstall; \
	rm -f $(MOZART_UPDATER_DIR)/etc/MiniGUI.cfg; \
	rm -rf $(MOZART_UPDATER_DIR)/usr/local
endef

$(eval $(call install_rules,minigui-gpl,$(minigui-gpl_DIR),target))
