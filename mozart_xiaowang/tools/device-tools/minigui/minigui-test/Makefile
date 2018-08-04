CC = mipsel-linux-gcc
PWDDIR = $(shell pwd)
MOLIB_DIR = $(PWDDIR)/../../../../output/molib/app
MOLIB_UPDATER_DIR = $(PWDDIR)/../../../../output/molib/updater
MOZART_DIR = $(PWDDIR)/../../../../output/mozart/app
MOZART_UPDATER_DIR = $(PWDDIR)/../../../../output/mozart/updater

CFLAGS = -Wall -I$(PWDDIR)/../minigui-gpl/include
CFLAGS += -I$(PWDDIR)/../minigui-gpl/

LDFLAGS = -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib -L$(MOZART_UPDATER_DIR)/usr/lib
LDFLAGS += -lminigui_ths -ldl -lm -lc -lpthread

all:minigui_demo

minigui_demo:helloworld.o
	$(CC) $? -o $@ $(CFLAGS) $(LDFLAGS)

clean distclean:
	-rm -rf *.o minigui_demo
