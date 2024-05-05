# Build libcrypthelper-mediameta.so
lib_LTLIBRARIES        := libcrypthelper_mediameta.la

libcrypthelper_mediameta_la_SOURCES   := crypthelper_mediameta.c

libcrypthelper_mediameta_la_CFLAGS   := -I.
libcrypthelper_mediameta_la_CFLAGS   += -Wall -fpic

library_includedir = $(includedir)/crypthelper_mediameta
library_include_HEADERS = $(srcdir)/crypthelper_mediameta.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcrypthelper_mediameta.pc
EXTRA_DIST = $(pkgconfig_DATA)

