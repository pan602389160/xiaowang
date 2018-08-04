target := test-musiclist
test-musiclist_DIR := $(TOPDIR)/tools/device-tools/test/musiclist

TARGETS += test-musiclist
TARGETS1 += test-musiclist

SUPPORT_UI ?= 0


# do nothing
define test-musiclist_CONFIGURE_CMDS


endef

define test-musiclist_BUILD_CMDS
	$(MAKE) -C $(test-musiclist_DIR)
endef

define test-musiclist_INSTALL_TARGET_CMDS
	$(MAKE) -C $(test-musiclist_DIR) install
endef

define test-musiclist_CLEAN_CMDS
	$(MAKE) -C $(test-musiclist_DIR) clean
endef

define test-musiclist_UNINSTALL_TARGET_CMDS
	$(MAKE1) -C $(test-musiclist_DIR) uninstall
endef

$(eval $(call install_rules,test-musiclist,$(test-musiclist_DIR),target))
