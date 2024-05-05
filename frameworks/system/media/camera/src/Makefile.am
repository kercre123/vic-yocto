# Build libcamera_metadata.so
lib_LTLIBRARIES = libcamera_metadata.la

libcamera_metadata_la_SOURCES := camera_metadata.c

libcamera_metadata_la_CFLAGS := -I$(top_srcdir)/camera/include
libcamera_metadata_la_CFLAGS += -I$(top_srcdir)/private/camera/include
libcamera_metadata_la_CFLAGS += -std=c99

# Export headers
camera_metadata_includedir=$(includedir)/system
camera_metadata_include_HEADERS := $(top_srcdir)/camera/include/system/camera_metadata.h
camera_metadata_include_HEADERS += $(top_srcdir)/camera/include/system/camera_metadata_tags.h
camera_metadata_include_HEADERS += $(top_srcdir)/camera/include/system/camera_vendor_tags.h
