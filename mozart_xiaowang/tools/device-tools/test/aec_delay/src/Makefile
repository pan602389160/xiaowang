CC = mipsel-linux-gcc
STRIP = mipsel-linux-strip

CFLAGS =  -I$(MOLIB_UPDATER_DIR)/usr/include
CFLAGS += -I$(MOLIB_APP_DIR)/usr/include
CFLAGS += -I$(MOZART_APP_DIR)/usr/include
CFLAGS += -I$(MOZART_UPDATER_DIR)/usr/include

LDFLAGS = -L$(MOLIB_UPDATER_DIR)/usr/lib
LDFLAGS += -L$(MOLIB_APP_DIR)/usr/lib
LDFLAGS += -L$(MOZART_APP_DIR)/usr/lib
LDFLAGS += -L$(MOZART_UPDATER_DIR)/usr/lib -lutils -lini -lasound -lrecord -lpthread

all:aec_delay_test
	cp $< ../test

aec_delay_test:aec_delay_test.o
	$(CC) $? -o $@ $(LDFLAGS)

aec_delay_test.o:aec_delay_test.c
	$(CC) $(CFLAGS) -c -o $@ $^
clean distclean:
	-rm -rf *.o aec_delay_test
