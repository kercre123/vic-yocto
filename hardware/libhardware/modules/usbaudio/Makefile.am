AM_CFLAGS := -Wno-unused-parameter

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = usb_audio.pc
EXTRA_DIST = $(pkgconfig_DATA)

c_sources = audio_hal.c

lib_LTLIBRARIES = audio.usb.default.la
audio_usb_default_la_SOURCES  = $(c_sources)
audio_usb_default_la_CFLAGS = $(AM_CFLAGS) -D__unused=__attribute__\(\(__unused__\)\)
audio_usb_default_la_LIBADD = -llog -lcutils -ltinyalsa -lalsautils -laudioutils
audio_usb_default_la_LDFLAGS = -module -shared -avoid-version
