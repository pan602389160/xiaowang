SUBDIRS = src 3rd-party include rtos etc m4 cmake build

EXTRA_DIST = \
            GNUmakefile makefile.ng makefile.msvc rules.make \
            Version README INSTALL \
            COPYING LICENSE.POLICY

# The directory where the include files will be installed
libminiguiincludedir = $(includedir)/minigui

# Which header files to install
libminiguiinclude_HEADERS = mgconfig.h

docs:
	doxygen Doxyfile

dist-nolicense: dist
	./clean_license.sh $(DIST_ARCHIVES)

dist:
distcheck-nolicense:dist-nolicense distcheck

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = minigui.pc

