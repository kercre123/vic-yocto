# Makefile.am

ACLOCAL_AMFLAGS := -I m4

AM_CFLAGS := -Wundef \
             -Wstrict-prototypes \
             -Wno-trigraphs \
             -g -O0 \
             -fno-inline \
             -fno-short-enums \
             -fpic \
              -I$(WORKSPACE)/bootable/recovery \
              -I$(WORKSPACE)/system/core/libsparse

AM_CPPFLAGS := \
              -I. \
              -I$(WORKSPACE)/system/core/include

#AM_LDFLAGS := -shared -version-info 1:0:0

oem_recovery_sources := gpt-utils.c dec.c oem-updater.c

library_includedir := $(pkgincludedir)

lib_LTLIBRARIES := librecovery_updater_msm.la
librecovery_updater_msm_la_SOURCES := $(oem_recovery_sources)

if USE_GLIB
librecovery_updater_msm_la_CFLAGS := $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
librecovery_updater_msm_la_CPPFLAGS := $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
librecovery_updater_msm_la_LIBADD := -lm -lpthread @GLIB_LIBS@ -lsparse
librecovery_updater_msm_la_LDFLAGS := -all-static
else
librecovery_updater_msm_la_CFLAGS := $(AM_CFLAGS)
librecovery_updater_msm_la_CPPFLAGS := $(AM_CPPFLAGS)
librecovery_updater_msm_la_LIBADD := -lm -lpthread -lsparse
librecovery_updater_msm_la_LDFLAGS := -all-static
endif
