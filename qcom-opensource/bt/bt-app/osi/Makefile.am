# sources and intermediate files are separated

AM_CFLAGS = -Wall \
            -Wundef \
            -Wstrict-prototypes \
            -Wno-trigraphs \
            -std=c99 \
            -DHAS_NO_BDROID_BUILDCFG \
            -D__GLIBC__

AM_CPPFLAGS = -I${WORKSPACE}/qcom-opensource/bt/bt-app \
               -I${WORKSPACE}/qcom-opensource/bt/bt-app/osi/include

c_sources = \
    src/alarm.c \
    src/allocator.c \
    src/allocation_tracker.c \
    src/config.c \
    src/fixed_queue.c \
    src/hash_functions.c \
    src/hash_map.c \
    src/list.c \
    src/reactor.c \
    src/semaphore.c \
    src/thread.c \
    src/future.c

requiredlibs = -lpthread
library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
if USE_GST
lib_LTLIBRARIES = libgstbtapposi.la
libgstbtapposi_la_CC = @CC@
libgstbtapposi_la_SOURCES = $(c_sources)
libgstbtapposi_la_CPPFLAGS =$(AM_CPPFLAGS)
libgstbtapposi_la_LIBADD = $(requiredlibs)
libgstbtapposi_la_CFLAGS = $(AM_CFLAGS) -fPIC -D_GNU_SOURCE -DUSE_GLIB
libgstbtapposi_la_LDFLAGS = -static
else
lib_LTLIBRARIES = libbtapposi.la
libbtapposi_la_CC = @CC@
libbtapposi_la_SOURCES = $(c_sources)
libbtapposi_la_CPPFLAGS =$(AM_CPPFLAGS)
libbtapposi_la_LIBADD = $(requiredlibs)
libbtapposi_la_CFLAGS = $(AM_CFLAGS) -fPIC -D_GNU_SOURCE -DUSE_GLIB
libbtapposi_la_LDFLAGS = -static
endif
