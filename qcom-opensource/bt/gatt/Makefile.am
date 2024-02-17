AM_CPPFLAGS = -O2 \
              -D_GNU_SOURCE \
             -I${WORKSPACE}/qcom-opensource/bt/gatt/include \
             -I${WORKSPACE}/qcom-opensource/bt/bt-app/include \
             -I${WORKSPACE}/qcom-opensource/bt/bt-app/ \
             -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include/ \
             -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/vhal/include \
             -I${WORKSPACE}/system/bt \
             -I${WORKSPACE}/system/core/include

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -DLINUX \
        -std=c99 \
        -I${WORKSPACE}/qcom-opensource/bt/gatt/include \
        -I${WORKSPACE}/qcom-opensource/bt/bt-app/include \
        -I${WORKSPACE}/qcom-opensource/bt/bt-app/ \
        -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/hal/include/ \
        -I${WORKSPACE}/vendor/qcom/opensource/bluetooth/vhal/include \
        -I${WORKSPACE}/system/core/include

ACLOCAL_AMFLAGS = -I m4

c_sources = src/Gatt.cpp

AM_LDFLAGS := $(LDFLAFGS)

library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libgengatt.la
libgengatt_la_CC = @CC@
libgengatt_la_SOURCES = $(c_sources)
libgengatt_la_CPPFLAGS = $(AM_CPPFLAGS)
libgengatt_la_CFLAGS = $(AM_CFLAGS)
libgengatt_la_LDFLAGS = $(AM_LDFLAGS) -static -lpthread -lm -ldl -lrt
