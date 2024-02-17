##### audioutils #####
AM_CFLAGS = -Werror -Wall

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = audio_utils.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS += -I $(top_srcdir)/audio/include
AM_CFLAGS += -I $(top_srcdir)/audio_utils/include

audioutils_includedir = $(includedir)/audio_utils
audioutils_include_HEADERS  = include/audio_utils/primitives.h
audioutils_include_HEADERS += include/audio_utils/format.h
audioutils_include_HEADERS += include/audio_utils/channels.h

audioutils_sources = format.c \
            channels.c \
            primitives.c

lib_LTLIBRARIES = libaudioutils.la
libaudioutils_la_SOURCES = $(audioutils_sources)
libaudioutils_la_CFLAGS = $(AM_CFLAGS)
libaudioutils_la_LDFLAGS = -shared

##### tinysndfile #####

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA += tinysndfile.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS += -I $(top_srcdir)/audio_utils
AM_CFLAGS += -I $(top_srcdir)/audio_utils/include

tinysnd_includedir = $(includedir)/tinysndfile
tinysnd_include_HEADERS = include/audio_utils/sndfile.h

tinysnd_sources = tinysndfile.c \
            primitives.c

lib_LTLIBRARIES += libtinysndfile.la
libtinysndfile_la_SOURCES = $(tinysnd_sources)
libtinysndfile_la_CFLAGS = $(AM_CFLAGS)
