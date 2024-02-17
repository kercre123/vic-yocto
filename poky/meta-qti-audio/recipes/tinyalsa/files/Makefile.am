pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tinyalsa.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

c_sources = git/src/mixer.c \
            git/src/pcm.c

h_sources = git/include/tinyalsa/asoundlib.h \
            git/include/tinyalsa/mixer.h \
            git/include/tinyalsa/pcm.h \
            git/include/tinyalsa/version.h

library_includedir = $(includedir)/tinyalsa/
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libtinyalsa.la
libtinyalsa_la_CC = @CC@
libtinyalsa_la_SOURCES = $(c_sources)
libtinyalsa_la_LIBADD = -lcutils
libtinyalsa_la_CFLAGS = $(AM_CFLAGS)
libtinyalsa_la_LDFLAGS = -shared -version-info 1:0:0

bin_PROGRAMS = tinyplay tinycap tinymix tinyhostless
tinyplay_SOURCES = git/utils/tinyplay.c
tinyplay_LDADD = libtinyalsa.la -lcutils
tinyplay_CFLAGS = $(AM_CFLAGS)

tinycap_SOURCES = git/utils/tinycap.c
tinycap_LDADD = libtinyalsa.la -lcutils
tinycap_CFLAGS = $(AM_CFLAGS)

tinymix_SOURCES = git/utils/tinymix.c
tinymix_LDADD =  libtinyalsa.la -lcutils
tinymix_CFLAGS = $(AM_CFLAGS)

tinyhostless_SOURCES = git/utils/tinyhostless.c
tinyhostless_LDADD =  libtinyalsa.la -lcutils
tinyhostless_CFLAGS = $(AM_CFLAGS)
