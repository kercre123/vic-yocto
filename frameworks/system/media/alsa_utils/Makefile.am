pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = alsa_utils.pc

AM_CFLAGS = -Werror -Wall

AM_CFLAGS += -I $(top_srcdir)/audio/include

libraryalsautils_includedir = $(includedir)
libraryalsautils_include_HEADERS = include/alsa_device_proxy.h
libraryalsautils_include_HEADERS += include/alsa_device_profile.h
libraryalsautils_include_HEADERS += include/alsa_logging.h

c_sources = alsa_device_profile.c \
            alsa_device_proxy.c \
            alsa_format.c \
            alsa_logging.c

lib_LTLIBRARIES = libalsautils.la
libalsautils_la_SOURCES = $(c_sources)
libalsautils_la_LIBADD = -ltinyalsa -llog -lcutils ../audio_utils/libaudioutils.la
libalsautils_la_CFLAGS = $(AM_CFLAGS)
libalsautils_la_CFLAGS += -Dstrlcat=g_strlcat -include glib.h -DUSE_GLIB $(GLIB_CFLAGS)
libalsautils_la_LDFLAGS = -shared -avoid-version $(GLIB_LIBS)
