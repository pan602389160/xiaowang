LCD ?= 0
CC = mipsel-linux-gcc
STRIP = mipsel-linux-strip

CFLAGS = -g -Wall -I$(MOLIB_UPDATER_DIR)/usr/include
CFLAGS += -I$(MOLIB_APP_DIR)/usr/include
CFLAGS += -I$(MOLIB_APP_DIR)/usr/include/upnp
CFLAGS += -I$(MOLIB_UPDATER_DIR)/usr/include/json-c/
CFLAGS += -I$(MOZART_APP_DIR)/usr/include
CFLAGS += -I$(MOZART_UPDATER_DIR)/usr/include
CFLAGS += -rdynamic -fasynchronous-unwind-tables

LDFLAGS = -L$(MOLIB_UPDATER_DIR)/usr/lib
LDFLAGS += -L$(MOLIB_APP_DIR)/usr/lib
LDFLAGS += -L$(MOZART_APP_DIR)/usr/lib
LDFLAGS += -L$(MOZART_UPDATER_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib

DESTDIR = $(MOZART_APP_DIR)
DESTDIR1 = $(MOZART_UPDATER_DIR)
PREFIX = /usr
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin
TESTDIR = $(PREFIX)/test
INCDIR = $(PREFIX)/include

PKG_CONFIG = pkg-config
INSTALL    = install

# test program for libsharememory.
TARGETS = getshm setshm
# test program for libini.
TARGETS += inirw
# test program for volume
TARGETS += volume
# test program for libtips
TARGETS += playtone
TARGETS += playkey
TARGETS += welcome
# test program for libpower
TARGETS += power
# test program for libplayer.
TARGETS += player_test
# test program for libalarm.
TARGETS += alarm_test
# test program for liblocalstream.
TARGETS += localstream_test
# test program for librender.
TARGETS += render_test
# test program for librecord.
TARGETS += record_test
# test program for event_manager.
TARGETS += event_test_recv event_test_send
# test program for libnvrw
TARGETS += nvparser

all:$(TARGETS)

getshm:libsharememory/getshm.o
	$(CC) $? -o $@ $(LDFLAGS) -lsharememory
setshm:libsharememory/setshm.o
	$(CC) $? -o $@ $(LDFLAGS) -lsharememory

inirw:inirw.o
	$(CC) $? -o $@ $(LDFLAGS) -lini
inirw.o:inirw.c
	$(CC) $< -c -o $@ $(CFLAGS)
inirw.c:
	ln -s $(TOPDIR)/tools/host-tools/inirw/inirw.c $(CURDIR)/inirw.c

volume:libvolume/volume.o
	$(CC) $? -o $@ $(LDFLAGS) -lini -lvolume -lasound

playtone:libtips/play_tone.o
	$(CC) $? -o $@ $(LDFLAGS) -ltips -lz -lplayer -lini -lutils

playkey:libtips/play_key.o
	$(CC) $? -o $@ $(LDFLAGS) -ltips -lz -lplayer -lini -lutils

ifeq ("$(UI)","lcd")
welcome:libtips/welcome.o
	$(CC) $? -o $@ $(LDFLAGS) -ltips -lz -lplayer -lini -llcd -llcdshow -lpng -lz -lutils
else
welcome:libtips/welcome.o
	$(CC) $? -o $@ $(LDFLAGS) -ltips -lz -lplayer -lini -lutils
endif
power:libpower/power.o
	$(CC) $? -o $@ $(LDFLAGS) -lpower

player_test:player/test.o
	$(CC) $? -o $@ $(LDFLAGS) -lplayer -lini -lutils

alarm_test:alarm/alarm_test.o
	$(CC) $? -o $@ $(LDFLAGS) -lalarm

localstream_test:localstream/test.o
	$(CC) $? -o $@ $(LDFLAGS) -lplayer -llocalstream

render_test:librender/test.o
	$(CC) $? -o $@ $(LDFLAGS) -lrender -lplayer -lini -lutils -llinklist -ltips -lsharememory -lasound -lpthread -lupnp -lixml -lvolume -lthreadutil -levent

record_test:librecord/test.o
	$(CC) $? -o $@ $(LDFLAGS) -lasound -lini -lrecord

event_test_recv:event_manager/test_recv.o
	$(CC) $? -o $@ $(LDFLAGS) -levent

event_test_send:event_manager/test_send.o
	$(CC) $? -o $@ $(LDFLAGS) -levent

nvparser:libnvrw/test.o
	$(CC) $? -o $@ $(LDFLAGS) -lnvrw

install:$(TARGETS)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR1)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(TESTDIR)
	$(INSTALL) -m 755 getshm $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 setshm $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 inirw $(DESTDIR1)$(BINDIR)/
	$(INSTALL) -m 755 volume $(DESTDIR1)$(BINDIR)/
	$(INSTALL) -m 755 welcome $(DESTDIR1)$(BINDIR)/
	$(INSTALL) -m 755 playtone $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 playkey $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 power $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 player_test $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 alarm_test $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 localstream_test $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 render_test $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 record_test $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 event_test_send $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 event_test_recv $(DESTDIR)$(TESTDIR)/
	$(INSTALL) -m 755 nvparser $(DESTDIR1)$(BINDIR)/

clean distclean:
	-rm -rf inirw.c *.o */*.o $(TARGETS)

uninstall:
	rm -rf $(DESTDIR1)/$(BINDIR)/{inirw,volume,welcome,nvparser}
	rm -rf $(DESTDIR)/$(TESTDIR)/{playtone,playtone}
	rm -rf $(DESTDIR)/$(TESTDIR)/{getshm,setshm}
	rm -rf $(DESTDIR)/$(TESTDIR)/{power,player_test,alarm_test,localstream_test,render_test,record_test}
	rm -rf $(DESTDIR)/$(TESTDIR)/{event_test_recv,event_test_send}

.PHONY:all clean install uninstall $(TARGETS)
