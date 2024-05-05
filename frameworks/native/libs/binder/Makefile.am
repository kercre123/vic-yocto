common_sources := AppOpsManager.cpp
common_sources += Binder.cpp
common_sources += BpBinder.cpp
common_sources += BufferedTextOutput.cpp
common_sources += Debug.cpp
common_sources += IAppOpsCallback.cpp
common_sources += IAppOpsService.cpp
common_sources += IBatteryStats.cpp
common_sources += IInterface.cpp
common_sources += IMemory.cpp
common_sources += IPCThreadState.cpp
common_sources += IPermissionController.cpp
common_sources += IProcessInfoService.cpp
common_sources += ProcessInfoService.cpp
common_sources += IServiceManager.cpp
common_sources += MemoryDealer.cpp
common_sources += MemoryBase.cpp
common_sources += MemoryHeapBase.cpp
common_sources += Parcel.cpp
common_sources += PermissionCache.cpp
common_sources += ProcessState.cpp
common_sources += Static.cpp
common_sources += TextOutput.cpp

lib_LTLIBRARIES = libbinder.la
libbinder_la_SOURCES = $(common_sources)
libbinder_la_CPPFLAGS := -I$(top_srcdir)/include
libbinder_la_CPPFLAGS += -std=c++11 -DHAVE_PTHREADS -DHAVE_ANDROID_OS
if IPC_32BIT
libbinder_la_CPPFLAGS += -DBINDER_IPC_32BIT=1
endif
libbinder_la_CPPFLAGS += -DUSE_GLIB -D__linux__
libbinder_la_CPPFLAGS += -DDEBUG_REFS_CALLSTACK_ENABLED=0
libbinder_la_LDFLAGS := -llog -lcutils -lpthread -lutils

# Build binder testapp
bin_PROGRAMS := test_binder

test_binder_SOURCES  := tests/test_binder.cpp
test_binder_CPPFLAGS := $(libbinder_la_CPPFLAGS)
test_binder_LDADD    := libbinder.la
test_binder_LDFLAGS  := -lutils -lcutils -llog -lpthread

# Export headers
binder_includedir=$(includedir)/binder
binder_include_HEADERS := $(top_srcdir)/include/binder/AppOpsManager.h
binder_include_HEADERS += $(top_srcdir)/include/binder/Binder.h
binder_include_HEADERS += $(top_srcdir)/include/binder/BinderService.h
binder_include_HEADERS += $(top_srcdir)/include/binder/BpBinder.h
binder_include_HEADERS += $(top_srcdir)/include/binder/BufferedTextOutput.h
binder_include_HEADERS += $(top_srcdir)/include/binder/Debug.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IAppOpsCallback.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IAppOpsService.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IBatteryStats.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IBinder.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IInterface.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IMemory.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IPCThreadState.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IPermissionController.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IProcessInfoService.h
binder_include_HEADERS += $(top_srcdir)/include/binder/IServiceManager.h
binder_include_HEADERS += $(top_srcdir)/include/binder/MemoryBase.h
binder_include_HEADERS += $(top_srcdir)/include/binder/MemoryDealer.h
binder_include_HEADERS += $(top_srcdir)/include/binder/MemoryHeapBase.h
binder_include_HEADERS += $(top_srcdir)/include/binder/Parcel.h
binder_include_HEADERS += $(top_srcdir)/include/binder/PermissionCache.h
binder_include_HEADERS += $(top_srcdir)/include/binder/ProcessInfoService.h
binder_include_HEADERS += $(top_srcdir)/include/binder/ProcessState.h
binder_include_HEADERS += $(top_srcdir)/include/binder/TextOutput.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbinder.pc
EXTRA_DIST = $(pkgconfig_DATA)
