DIR_CUR := $(shell pwd)
CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
INSTALL = install
DESTDIR =
PREFIX = /usr
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin
VRDATADIR = $(PREFIX)/share/vr

# common opts.
CFLAGS = -fPIC -Wall -Werror -g -O2 -I$(MOLIB_DIR)/usr/include\
		 -I$(MOLIB_UPDATER_DIR)/usr/include/ -I$(DIR_CUR)/include\
		 -I$(MOZART_DIR)/usr/include/ \
		 -I$(MOLIB_DIR)/usr/include/opencore-amrnb/ \
		 -I$(MOLIB_DIR)/usr/include/opencore-amrwb/ 
CFLAGS += $(VENDOR_CFLAGS)

# basic depends.
#comm LDFLAGS add VENDOR LDFLAGS
LDFLAGS = -g -lpthread -L$(MOLIB_DIR)/usr/lib\
		  -L$(MOLIB_UPDATER_DIR)/usr/lib\
		  -L$(DIR_CUR)/$(VENDOR)/libs/\
		  -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib/
LDFLAGS += $(VENDOR_LDFLAGS)
LDFLAGS += $(MOLIB_DIR)/usr/lib/libvo-amrwbenc.a

TEST_CFLAGS = -Wall -Werror -g -O2 -I$(DIR_CUR)/test\
			  -I$(DIR_CUR)/include -I$(DIR_CUR)/$(VENDOR)/include/\
			  -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOLIB_DIR)/usr/include\
			  -I$(MOZART_DIR)/usr/include/ -I$(MOLIB_DIR)/usr/include/opencore-amrwb/ 
TEST_LDFLAGS = $(MOLIB_DIR)/usr/lib/libvo-amrwbenc.a -L. -lpthread -lplayer -ltips -lvolume -lvr -lstdc++\
			   -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOLIB_DIR)/usr/lib\
			   -lopencore-amrnb -Wl,-rpath-link,$(MOLIB_UPDATER_DIR)/usr/lib:$(MOLIB_DIR)/usr/lib:$(DIR_CUR)/$(VENDOR)/libs/
			   

VR_SRCS = $(wildcard *.c)
VR_OBJS = $(patsubst %.c, %.c.vr.o, $(VR_SRCS))

TEST_SRCS = $(wildcard test/*.c)
TEST_OBJS = $(patsubst %.c, %.c.test.o, $(TEST_SRCS))

TARGET = libvr.so

all:$(TARGET) vr_test

$(TARGET):$(VR_OBJS) VENDOROBJS
	echo "XXXXXXXXXX"
	$(CC) -o $@  $(VR_OBJS) $(VENDOR_OBJS)  $(LDFLAGS)

%.c.vr.o : %.c
	echo "AAAAAAAAAAAAAAAAAAAAAAAAA"
	$(CC) -c -o $@ $< $(CFLAGS)
doxygen:
	@doxygen $(VENDOR_DIR)/$(VENDOR_DOXGEN)

doxygen_clean:
	@rm -rf $(VENDOR_DIR)/$(VENDOR_DOXCLEAN)

vr_test: $(TEST_OBJS)
	echo "BBBBBBBBBBBBBBBBBBBB"
	$(CC) -o test/$@ $^ $(TEST_CFLAGS) $(TEST_LDFLAGS)

test/%.c.test.o:test/%.c
	echo "CCCCCCCCCCCCCCCCCCCCCCCCCCC"
	$(CC) $(TEST_CFLAGS) -c $^ -o $@

install:vr_install vendor_install

vr_install:
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(INCDIR)
	$(INSTALL) -d $(DESTDIR)$(VRDATADIR)
	$(INSTALL) -m 755 $(DIR_CUR)/$(TARGET) $(DESTDIR)$(LIBDIR)/
	$(INSTALL) -m 755 $(DIR_CUR)/test/vr_test $(DESTDIR)$(BINDIR)/
	$(INSTALL) -m 755 $(DIR_CUR)/include/vr_interface.h $(DESTDIR)$(INCDIR)/

clean:vr_clean vendor_clean
vr_clean:
	-rm -rf $(VR_OBJS) $(TEST_OBJS) test/vr_test $(TARGET)

uninstall:vendor_uninstall vr_uninstall
vr_uninstall:
	rm -rf $(DESTDIR)$(INCDIR)/vr_interface.h
	rm -rf $(DESTDIR)$(LIBDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(BINDIR)/vr_test
	rm -rf $(DESTDIR)$(VRDATADIR)/res

include $(VENDOR)/vendor.mk
.PHONY:all clean install uninstall $(TARGET) vr_test
