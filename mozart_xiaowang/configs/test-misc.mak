target := test-misc
test-misc_DIR := $(TOPDIR)/tools/device-tools/test/misc

TARGETS += test-misc
TARGETS1 += test-misc

SUPPORT_UI ?= 0


# do nothing
define test-misc_CONFIGURE_CMDS


endef

define test-misc_OPTS
	UI=$(SUPPORT_UI)
endef

define test-misc_BUILD_CMDS
	$(test-misc_OPTS) \
		$(MAKE) -C $(test-misc_DIR)
endef

define test-misc_INSTALL_TARGET_CMDS
	$(test-misc_OPTS) \
		$(MAKE) -C $(test-misc_DIR) install
endef

define test-misc_CLEAN_CMDS
	$(test-misc_OPTS) \
		$(MAKE) -C $(test-misc_DIR) clean
endef

define test-misc_UNINSTALL_TARGET_CMDS
	$(test-misc_OPTS) \
		$(MAKE1) -C $(test-misc_DIR) uninstall
endef

$(eval $(call install_rules,test-misc,$(test-misc_DIR),target))
