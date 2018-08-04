DIR_CUR := $(shell pwd)

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

ifndef RUN_MODE
$(error tv audio run mode NOT set.)
endif

LOCALPLAYER ?= 0
MULROOM ?= 0
INGENICPLAYER ?= 0

CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
STRIP = mipsel-linux-strip
INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d
SBINDIR = $(PREFIX)/sbin

SRCS = mozart_app.c mozart_key_function.c mozart_musicplayer.c tv_audio.c main.c
SRCS += modules/mozart_module_linein.c

# common opts.
CFLAGS = -fPIC -Wall -Werror -g -O2 -I. -I./include/ -I./modules/ -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include -I$(MOZART_UPDATER_DIR)/usr/include
CFLAGS += -rdynamic -fasynchronous-unwind-tables

LDFLAGS = -g -lpthread -lstdc++ -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib -L$(MOZART_UPDATER_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib

# basic depends.
LDFLAGS += -levent -lpower -lwifi -lvolume -ltips -lsharememory -lini -lutils -lpthread -lplayer -ljson-c -lm -lresample -lnvrw -lupdater -lmusiclist -llinklist -lasound

ifeq ($(LOCALPLAYER),1)
SRCS += modules/mozart_module_local_music.c
LDFLAGS += -llocalplayer
else
$(warning localplayer func CLOSED!!!!)
endif

ifeq ($(MULROOM),1)
SRCS += modules/mozart_module_mulroom.c
LDFLAGS += -lmulroom
else
$(warning multiroom func CLOSED!!!!)
endif

ifeq ($(INGENICPLAYER),1)
SRCS +=  ingenicplayer/appserver.c ingenicplayer/ingenicplayer.c ingenicplayer/device.c
LDFLAGS += -lappserver
else
$(warning ingenicplayer func CLOSED!!!!)
endif

ifeq ("$(UI)","0")
$(warning ui func are CLOSED!!!!)
else
SRCS += mozart_ui.c
endif
ifeq ("$(UI)","lcd")
LDFLAGS += -llcd -llcdshow -lpng -lz
endif
ifeq ("$(UI)","smartui")
LDFLAGS += -lsmartui
endif

OBJS = $(patsubst %.c, %.c.o, $(SRCS))
TARGET = tvaudio

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)
	if [ -e $(TARGET) ]; then \
		$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(TARGET) $(DESTDIR)$(SBINDIR)/; \
		$(INSTALL) -m 755 S10tvaudio_$(RUN_MODE).sh $(DESTDIR)$(INITDIR)/; \
	fi

clean:
	-rm -rf $(OBJS) $(TARGET)

uninstall:
	[ -e $(DESTDIR)$(SBINDIR)/$(TARGET) ] && rm -rf $(DESTDIR)$(SBINDIR)/$(TARGET)
	[ -e $(DESTDIR)$(INITDIR)/S10tvaudio_$(RUN_MODE).sh ] && rm -rf $(DESTDIR)$(INITDIR)/S10tvaudio_$(RUN_MODE).sh

.PHONY: all clean install uninstall $(TARGET)
