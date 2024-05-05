pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = audio_route.pc

AM_CFLAGS = -Werror -Wall

library_includedir = $(includedir)/audio_route
library_include_HEADERS = include/audio_route/audio_route.h

c_sources = audio_route.c

lib_LTLIBRARIES = libaudioroute.la
libaudioroute_la_SOURCES = $(c_sources)
libaudioroute_la_LIBADD = -ltinyalsa -lexpat -llog -lcutils
libaudioroute_la_CFLAGS = $(AM_CFLAGS)
