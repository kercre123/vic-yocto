# Makefile.am

ACLOCAL_AMFLAGS := -I m4

AM_CFLAGS := -Wundef \
             -Wstrict-prototypes \
             -Wno-trigraphs \
             -g -O0 \
             -fno-inline \
             -fno-short-enums \
             -fpic \
             -I$(WORKSPACE)/bootable/recovery

AM_CPPFLAGS := \
              -I. \
              -I$(WORKSPACE)/bootable/recovery \
	      -std=c++11

miniui_sources := msm_recovery_ui.cpp

library_includedir := $(pkgincludedir)

lib_LTLIBRARIES := librecovery_ui_msm.la
librecovery_ui_msm_la_SOURCES := $(miniui_sources)

if USE_GLIB
librecovery_ui_msm_la_CFLAGS := $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
librecovery_ui_msm_la_CPPFLAGS := $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
librecovery_ui_msm_la_LDFLAGS := -all-static
else
librecovery_ui_msm_la_CFLAGS := $(AM_CFLAGS)
librecovery_ui_msm_la_CPPFLAGS := $(AM_CPPFLAGS)
librecovery_ui_msm_la_LDFLAGS := -all-static
endif
