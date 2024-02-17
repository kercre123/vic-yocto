# Default options
USE_BSDIFF ?= y

EXECUTABLES-y := bspatch
LIBRARIES-y := libbspatch.so
EXECUTABLES-$(USE_BSDIFF) += bsdiff
LIBRARIES-$(USE_BSDIFF) += libbsdiff.so

BINARIES := $(EXECUTABLES-y) $(LIBRARIES-y)

INSTALL = install
CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++11 -O3 -Wall -Werror -fPIC

DESTDIR ?=
PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
MANDIR = $(DATADIR)/man
MAN1DIR = $(MANDIR)/man1
INCLUDEDIR ?= $(PREFIX)/include
GENTOO_LIBDIR ?= lib
LIBDIR ?= $(PREFIX)/$(GENTOO_LIBDIR)
INSTALL_PROGRAM ?= $(INSTALL) -c -m 755
INSTALL_MAN ?= $(INSTALL) -c -m 444

.PHONY: all test clean install
all: $(BINARIES)
test: bsdiff_unittest
clean:
	rm -f *.o $(BINARIES) bsdiff_unittest

### List of source files for each project. Keep in sync with the Android.mk.
# "bsdiff" program.
bsdiff_src_files := \
    bsdiff.cc

# "bspatch" program.
bspatch_src_files := \
    bspatch.cc \
    buffer_file.cc \
    extents.cc \
    extents_file.cc \
    file.cc \
    memory_file.cc \
    sink_file.cc

# Unit test files.
bsdiff_common_unittests := \
    bsdiff_unittest.cc \
    bspatch_unittest.cc \
    extents_file_unittest.cc \
    extents_unittest.cc \
    test_utils.cc \
    testrunner.cc


BSDIFF_LIBS := -lbz2 -ldivsufsort -ldivsufsort64
BSDIFF_OBJS := $(bsdiff_src_files:.cc=.o)

BSPATCH_LIBS := -lbz2
BSPATCH_OBJS := $(bspatch_src_files:.cc=.o)

UNITTEST_LIBS = -lgmock -lgtest -lpthread
UNITTEST_OBJS := $(bsdiff_common_unittests:.cc=.o)

bsdiff: $(BSDIFF_OBJS) bsdiff_main.o
bsdiff: LDLIBS += $(BSDIFF_LIBS)
libbsdiff.so: $(BSDIFF_OBJS)
libbsdiff.so: LDLIBS += $(BSDIFF_LIBS)

bspatch: $(BSPATCH_OBJS) bspatch_main.o
bspatch: LDLIBS += $(BSPATCH_LIBS)
libbspatch.so: $(BSPATCH_OBJS)
libbspatch.so: LDLIBS += $(BSPATCH_LIBS)

bsdiff_unittest: LDLIBS += $(BSDIFF_LIBS) $(BSPATCH_LIBS) $(UNITTEST_LIBS)
bsdiff_unittest: $(BSPATCH_OBJS) $(BSDIFF_OBJS) $(UNITTEST_OBJS)

bsdiff_unittest bsdiff bspatch:
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

libbsdiff.so libbspatch.so:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,-soname,$@ -shared -o $@ $^ $(LDLIBS)

# Source file dependencies.
bspatch.o: bspatch.cc include/bsdiff/bspatch.h \
 include/bsdiff/extents_file.h include/bsdiff/file_interface.h \
 buffer_file.h extents.h file.h memory_file.h sink_file.h
bspatch_main.o: bspatch_main.cc include/bsdiff/bspatch.h \
 include/bsdiff/extents_file.h include/bsdiff/file_interface.h
bspatch_unittest.o: bspatch_unittest.cc include/bsdiff/bspatch.h \
 include/bsdiff/extents_file.h include/bsdiff/file_interface.h \
 test_utils.h
buffer_file.o: buffer_file.cc buffer_file.h \
 include/bsdiff/file_interface.h include/bsdiff/bspatch.h \
 include/bsdiff/extents_file.h
extents.o: extents.cc extents.h include/bsdiff/extents_file.h \
 include/bsdiff/file_interface.h
extents_file.o: extents_file.cc include/bsdiff/extents_file.h \
 include/bsdiff/file_interface.h
extents_file_unittest.o: extents_file_unittest.cc \
 include/bsdiff/extents_file.h include/bsdiff/file_interface.h
extents_unittest.o: extents_unittest.cc extents.h \
 include/bsdiff/extents_file.h include/bsdiff/file_interface.h
file.o: file.cc file.h include/bsdiff/file_interface.h
memory_file.o: memory_file.cc memory_file.h \
 include/bsdiff/file_interface.h
sink_file.o: sink_file.cc sink_file.h include/bsdiff/file_interface.h
testrunner.o: testrunner.cc test_utils.h
test_utils.o: test_utils.cc test_utils.h

install:
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(MAN1DIR) \
	  $(DESTDIR)/$(INCLUDEDIR)/bsdiff
	$(INSTALL_PROGRAM) $(EXECUTABLES-y) $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) $(LIBRARIES-y) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -c -m 644 include/bsdiff/*.h $(DESTDIR)/$(INCLUDEDIR)/bsdiff
ifndef WITHOUT_MAN
	$(INSTALL_MAN) $(EXECUTABLES-y:=.1) $(DESTDIR)$(MAN1DIR)
endif
