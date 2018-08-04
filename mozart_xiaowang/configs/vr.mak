define makedir
$(shell if [ ! -f $(1) ]; then mkdir -p $(1);touch $(1)/Makefile; fi;)
endef

vr_DIR := $(TOPDIR)/src/vr

ifneq ($(SUPPORT_VR), 0)
TARGETS += vr
TARGETS1 += vr
endif

$(call makedir,$(vr_DIR))

define vr_OPTS
	VENDOR=$(SUPPORT_VR)
endef

define vr_BUILD_CMDS
	$(vr_OPTS) \
		$(MAKE1) -C $(@D)
endef

define vr_INSTALL_TARGET_CMDS
	$(vr_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_DIR) install
endef

define vr_CLEAN_CMDS
	$(vr_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_DIR) clean
endef

define vr_UNINSTALL_TARGET_CMDS
	$(vr_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_DIR) uninstall
endef

$(eval $(call install_rules,vr,$(vr_DIR),target))
