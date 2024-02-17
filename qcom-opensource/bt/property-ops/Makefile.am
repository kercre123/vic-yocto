# sources and intermediate files are separated

AM_CFLAGS = -Wall \
    -Wundef \
    -Wstrict-prototypes \
    -Wno-trigraphs \
    -std=c99

AM_CPPFLAGS= \
    -Iinclude/

# HAL layer
c_sources = \
    src/ll.c \
    src/property_ops.c \
    src/sock_ops.c

if USE_GLIB
 btproperty_CFLAGS  = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
 btproperty_LDFLAGS = -lpthread @GLIB_LIBS@
else
 btproperty_CFLAGS = $(AM_CFLAGS)
 btproperty_LDFLAGS = -lpthread
endif

library_includedir = $(pkgincludedir)
bin_PROGRAMS = btproperty
btproperty_CC = @CC@
btproperty_SOURCES = $(c_sources)