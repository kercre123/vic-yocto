common_sources := Fence.cpp
common_sources += FramebufferNativeWindow.cpp
common_sources += FrameStats.cpp
common_sources += GraphicBuffer.cpp
common_sources += GraphicBufferAllocator.cpp
common_sources += GraphicBufferMapper.cpp
common_sources += PixelFormat.cpp
common_sources += Rect.cpp
common_sources += Region.cpp
common_sources += UiConfig.cpp

lib_LTLIBRARIES = libui.la
libui_la_SOURCES = $(common_sources)
libui_la_CPPFLAGS := -I$(top_srcdir)/include -I$(top_srcdir)/opengl/include
libui_la_CPPFLAGS += -std=c++11  -DANDROID
libui_la_CPPFLAGS += -DDEBUG_REFS_CALLSTACK_ENABLED=0
libui_la_LDFLAGS := -llog -lcutils -lsync -lhardware -lutils

# Export headers
ui_includedir=$(includedir)/ui
ui_include_HEADERS := $(top_srcdir)/include/ui/ANativeObjectBase.h
ui_include_HEADERS += $(top_srcdir)/include/ui/Fence.h
ui_include_HEADERS += $(top_srcdir)/include/ui/GraphicBufferAllocator.h
ui_include_HEADERS += $(top_srcdir)/include/ui/mat4.h
ui_include_HEADERS += $(top_srcdir)/include/ui/Rect.h
ui_include_HEADERS += $(top_srcdir)/include/ui/TVecHelpers.h
ui_include_HEADERS += $(top_srcdir)/include/ui/vec3.h
ui_include_HEADERS += $(top_srcdir)/include/ui/DisplayInfo.h
ui_include_HEADERS += $(top_srcdir)/include/ui/FramebufferNativeWindow.h
ui_include_HEADERS += $(top_srcdir)/include/ui/GraphicBuffer.h
ui_include_HEADERS += $(top_srcdir)/include/ui/PixelFormat.h
ui_include_HEADERS += $(top_srcdir)/include/ui/Region.h
ui_include_HEADERS += $(top_srcdir)/include/ui/UiConfig.h
ui_include_HEADERS += $(top_srcdir)/include/ui/vec4.h
ui_include_HEADERS += $(top_srcdir)/include/ui/DisplayStatInfo.h
ui_include_HEADERS += $(top_srcdir)/include/ui/FrameStats.h
ui_include_HEADERS += $(top_srcdir)/include/ui/GraphicBufferMapper.h
ui_include_HEADERS += $(top_srcdir)/include/ui/Point.h
ui_include_HEADERS += $(top_srcdir)/include/ui/TMatHelpers.h
ui_include_HEADERS += $(top_srcdir)/include/ui/vec2.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libui.pc
EXTRA_DIST = $(pkgconfig_DATA)
