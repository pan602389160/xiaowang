target := test-network-manager
test-network-manager_DIR := $(TOPDIR)/tools/device-tools/test/network-manager

TARGETS += test-network-manager
TARGETS1 += test-network-manager

SUPPORT_UI ?= 0


# do nothing
define test-network-manager_CONFIGURE_CMDS


endef

define test-network-manager_BUILD_CMDS
	$(MAKE) -C $(test-network-manager_DIR)
endef

define test-network-manager_INSTALL_TARGET_CMDS
	$(MAKE) -C $(test-network-manager_DIR) install
endef

define test-network-manager_CLEAN_CMDS
	$(MAKE) -C $(test-network-manager_DIR) clean
endef

define test-network-manager_UNINSTALL_TARGET_CMDS
	$(MAKE1) -C $(test-network-manager_DIR) uninstall
endef

$(eval $(call install_rules,test-network-manager,$(test-network-manager_DIR),target))
