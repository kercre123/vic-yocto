# Build libcamera_client.so
lib_LTLIBRARIES = libcamera_client.la

# Only enable Metadata for now.
libcamera_client_la_SOURCES := CameraMetadata.cpp
libcamera_client_la_SOURCES += CameraParameters.cpp
libcamera_client_la_SOURCES += VendorTagDescriptor.cpp

libcamera_client_la_CPPFLAGS := -I$(top_srcdir)/include
libcamera_client_la_CPPFLAGS += -std=c++11
libcamera_client_la_CPPFLAGS += -I$(WORKSPACE)/system/media/private/camera/include
libcamera_client_la_LDFLAGS  := -lutils -lcamera_metadata

# Export headers
camera_client_includedir=$(includedir)/camera
camera_client_include_HEADERS := $(top_srcdir)/include/camera/CameraMetadata.h
camera_client_include_HEADERS += $(top_srcdir)/include/camera/CameraParameters.h
camera_client_include_HEADERS += $(top_srcdir)/include/camera/VendorTagDescriptor.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcamera_client.pc
EXTRA_DIST = $(pkgconfig_DATA)
