CC = mipsel-linux-gcc
STRIP = mipsel-linux-strip
INSTALL = install

CFLAGS = -g -O2 -fPIC -Wall -Werror -rdynamic -fasynchronous-unwind-tables
CFLAGS += -I./include/ -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include
CFLAGS += -I$(MOLIB_TEST_DIR)/usr/include

LDFLAGS = -g -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib
LDFLAGS += -L$(MOLIB_TEST_DIR)/usr/lib -lcunit
LDFLAGS += -lmusiclist

SRCS = main.c
OBJS = $(patsubst %.c, %.o, $(SRCS))

TARGET = musiclist_functional_test

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	$(INSTALL) -d $(MOZART_TEST_DIR)/usr/bin
	$(INSTALL) -m 755 $(TARGET) $(MOZART_TEST_DIR)/usr/bin/

clean:
	-rm -rf $(OBJS) $(TARGET)

uninstall:
	rm -rf $(MOZART_TEST_DIR)/usr/bin/$(TARGET)

.PHONY:all clean install uninstall $(TARGET)
