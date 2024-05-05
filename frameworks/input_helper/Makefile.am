# Build shared lib.
lib_LTLIBRARIES := libinputhelper.la

libinputhelper_la_SOURCES := input_helper.c

libinputhelper_la_CFLAGS := -I$(top_srcdir)/include

# Export headers
h_sources = input_helper.h

libinputhelper_includedir=$(includedir)/
libinputhelper_include_HEADERS = $(h_sources)
