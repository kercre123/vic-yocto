ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS := \
              -I. \
              -I$(WORKSPACE)/system/core/include \
              $(LIBSELINUX_CFLAGS)

c_sources := fs_config.c
bin_PROGRAMS := fs_config

fs_config_SOURCES := $(c_sources)
fs_config_CPPFLAGS := $(AM_CPPFLAGS)
if BUILD_HOST
fs_config_CPPFLAGS += -DHOST_BUILD
endif

fs_config_LDFLAGS := -lselinux -lcutils

fs_config_LDADD := -lselinux -lcutils



