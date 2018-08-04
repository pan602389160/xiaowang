target := aec_delay_test
aectest_DIR := $(TOPDIR)/tools/device-tools/test/aec_delay/src

TARGETS += aec_delay_test
TARGETS1 += aec_delay_test

# do nothing
define aectest_CONFIGURE_CMDS
endef

define aec_delay_test_BUILD_CMDS
	$(MAKE) -C $(aectest_DIR)
endef

define aec_delay_test_INSTALL_TARGET_CMDS
endef

define aec_delay_test_CLEAN_CMDS
	$(MAKE) -C $(aectest_DIR) clean
endef

define aec_delay_test_UNINSTALL_TARGET_CMDS
endef

$(eval $(call install_rules,aec_delay_test,$(aectest_DIR),target))
