VENDOR_DIR = $(DIR_CUR)/$(VENDOR)
VENDOR_SRCS = $(wildcard $(DIR_DIR)$(VENDOR_DIR)/src/*.c)
VENDOR_OBJS = $(patsubst %.c, %.c.vendor.o, $(VENDOR_SRCS))
VENDOR_CFLAGS = -I$(VENDOR_DIR)/include -I$(VENDOR_DIR)/include/aispeech
VENDOR_LDFLAGS += -lstdc++ -shared -lrecord -ljson-c -lutils -laiengine -ltips
VENDOR_DOXGEN = doc/Doxyfile
VENDOR_DOXCLEAN = doc/AISpeech

AUTHORIZATION_SRCS = $(wildcard $(DIR_DIR)$(VENDOR_DIR)/src/ai_vr.c)
AUTHORIZATION_SRCS += $(wildcard $(DIR_DIR)$(VENDOR_DIR)/src/authorization/authorization.c)
AUTHORIZATION_OBJS = $(patsubst %.c, %.c.vendor.o, $(AUTHORIZATION_SRCS))
AUTHORIZATION_LDFLAGS = $(patsubst -shared, , $(LDFLAGS))

VENDOROBJS: $(VENDOR_OBJS) aispeech_authorization

%.c.vendor.o : %.c
	@echo $(VENDOR_SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

aispeech_authorization: $(AUTHORIZATION_OBJS)
	$(CC) -o  $(VENDOR_DIR)/$@ $(AUTHORIZATION_OBJS) -L$(AUTHORIZATION_LDFLAGS)

vendor_install:
	cp -a $(VENDOR_DIR)/include/aispeech $(DESTDIR)$(INCDIR)
	cp -a $(VENDOR_DIR)/libs/libaiengine* $(DESTDIR)$(LIBDIR)
	cp -a $(VENDOR_DIR)/res $(DESTDIR)/$(VRDATADIR)
	cp -a $(VENDOR_DIR)/aispeech_authorization $(DESTDIR)/$(BINDIR)

vendor_clean:
	-rm -rf $(VENDOR_OBJS) $(AUTHORIZATION_OBJS) aispeech_authorization

vendor_uninstall:
	rm -rf $(DESTDIR)$(INCDIR)/aispeech
	rm -rf $(DESTDIR)$(LIBDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(LIBDIR)/libaiengine*
	rm -rf $(DESTDIR)$(VRDATADIR)/res
	rm -rf $(DESTDIR)/$(BINDIR)/aispeech_authorization

.PHONY: vendor_clean vendor_install vendor_uninstall
