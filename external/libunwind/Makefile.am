AM_CFLAGS := \
    -W \
    -Wall \
    -Wextra \
    -fpermissive \
    -Wunused
ACLOCAL_AMFLAGS = -I m4
unwind_sources := src/mi/init.c
unwind_sources += src/mi/flush_cache.c
unwind_sources += src/mi/mempool.c
unwind_sources += src/mi/strerror.c
unwind_sources += src/mi/backtrace.c
unwind_sources += src/mi/dyn-cancel.c
unwind_sources += src/mi/dyn-info-list.c
unwind_sources += src/mi/dyn-register.c
unwind_sources += src/mi/map.c
unwind_sources += src/mi/Lmap.c
unwind_sources += src/mi/Ldyn-extract.c
unwind_sources += src/mi/Lfind_dynamic_proc_info.c
unwind_sources += src/mi/Lget_proc_info_by_ip.c
unwind_sources += src/mi/Lget_proc_name.c
unwind_sources += src/mi/Lput_dynamic_unwind_info.c
unwind_sources += src/mi/Ldestroy_addr_space.c
unwind_sources += src/mi/Lget_reg.c
unwind_sources += src/mi/Lset_reg.c
unwind_sources += src/mi/Lget_fpreg.c
unwind_sources += src/mi/Lset_fpreg.c
unwind_sources += src/mi/Lset_caching_policy.c
unwind_sources += src/mi/Gdyn-extract.c
unwind_sources += src/mi/Gdyn-remote.c
unwind_sources += src/mi/Gfind_dynamic_proc_info.c
unwind_sources += src/mi/Gget_accessors.c
unwind_sources += src/mi/Gget_proc_info_by_ip.c
unwind_sources += src/mi/Gget_proc_name.c
unwind_sources += src/mi/Gput_dynamic_unwind_info.c
unwind_sources += src/mi/Gdestroy_addr_space.c
unwind_sources += src/mi/Gget_reg.c
unwind_sources += src/mi/Gset_reg.c
unwind_sources += src/mi/Gget_fpreg.c
unwind_sources += src/mi/Gset_fpreg.c
unwind_sources += src/mi/Gset_caching_policy.c
unwind_sources += src/dwarf/Lexpr.c
unwind_sources += src/dwarf/Lfde.c
unwind_sources += src/dwarf/Lparser.c
unwind_sources += src/dwarf/Lpe.c
unwind_sources += src/dwarf/Lstep_dwarf.c
unwind_sources += src/dwarf/Lfind_proc_info-lsb.c
unwind_sources += src/dwarf/Lfind_unwind_table.c
unwind_sources += src/dwarf/Gexpr.c
unwind_sources += src/dwarf/Gfde.c
unwind_sources += src/dwarf/Gfind_proc_info-lsb.c
unwind_sources += src/dwarf/Gfind_unwind_table.c
unwind_sources += src/dwarf/Gparser.c
unwind_sources += src/dwarf/Gpe.c
unwind_sources += src/dwarf/Gstep_dwarf.c
unwind_sources += src/dwarf/global.c
unwind_sources += src/os-common.c
unwind_sources += src/os-linux.c
unwind_sources += src/Los-common.c
unwind_sources += src/ptrace/_UPT_accessors.c
unwind_sources += src/ptrace/_UPT_access_fpreg.c
unwind_sources += src/ptrace/_UPT_access_mem.c
unwind_sources += src/ptrace/_UPT_access_reg.c
unwind_sources += src/ptrace/_UPT_create.c
unwind_sources += src/ptrace/_UPT_destroy.c
unwind_sources += src/ptrace/_UPT_find_proc_info.c
unwind_sources += src/ptrace/_UPT_get_dyn_info_list_addr.c
unwind_sources += src/ptrace/_UPT_put_unwind_info.c
unwind_sources += src/ptrace/_UPT_get_proc_name.c
unwind_sources += src/ptrace/_UPT_reg_offset.c
unwind_sources += src/ptrace/_UPT_resume.c
unwind_sources += src/$(arch)/is_fpreg.c
unwind_sources += src/$(arch)/regname.c
unwind_sources += src/$(arch)/Gcreate_addr_space.c
unwind_sources += src/$(arch)/Gget_proc_info.c
unwind_sources += src/$(arch)/Gget_save_loc.c
unwind_sources += src/$(arch)/Gglobal.c
unwind_sources += src/$(arch)/Ginit.c
unwind_sources += src/$(arch)/Ginit_local.c
unwind_sources += src/$(arch)/Ginit_remote.c
unwind_sources += src/$(arch)/Gregs.c
unwind_sources += src/$(arch)/Gresume.c
unwind_sources += src/$(arch)/Gstep.c
unwind_sources += src/$(arch)/Lcreate_addr_space.c
unwind_sources += src/$(arch)/Lget_proc_info.c
unwind_sources += src/$(arch)/Lget_save_loc.c
unwind_sources += src/$(arch)/Lglobal.c
unwind_sources += src/$(arch)/Linit.c
unwind_sources += src/$(arch)/Linit_local.c
unwind_sources += src/$(arch)/Linit_remote.c
unwind_sources += src/$(arch)/Lregs.c
unwind_sources += src/$(arch)/Lresume.c
unwind_sources += src/$(arch)/Lstep.c
unwind_headers := $(AM_FLAGS)
unwind_headers += -I${top_srcdir}/include/tdep
unwind_headers += -I${top_srcdir}/src

if ARCH_AARCH64
unwind_headers += -I$(top_srcdir)/include/tdep-aarch64/
unwind_arm64_sources := src/aarch64/Gis_signal_frame.c
unwind_arm64_sources += src/aarch64/Lis_signal_frame.c
unwind_arm64_sources += src/elf64.c
target_sources := $(unwind_sources) $(unwind_arm64_sources)
endif

if ARCH_ARM
unwind_headers += -I$(top_srcdir)/include/tdep-arm/
unwind_arm32_sources := src/arm/getcontext.S
unwind_arm32_sources += src/arm/Gis_signal_frame.c
unwind_arm32_sources += src/arm/Gex_tables.c
unwind_arm32_sources += src/arm/Lis_signal_frame.c
unwind_arm32_sources += src/arm/Lex_tables.c
unwind_arm32_sources += src/elf32.c
target_sources := $(unwind_sources) $(unwind_arm32_sources)
endif



lib_LTLIBRARIES = libunwind.la
libunwind_la_CPPFLAGS := $(unwind_headers)
libunwind_la_SOURCES  = $(target_sources)
libunwind_la_CPPFLAGS += -DANDROID

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ./src/unwind/libunwind.pc
EXTRA_DIST = $(pkgconfig_DATA)

# Export headers
unwind_includedir=$(includedir)/libunwind
unwind_include_HEADERS :=${top_srcdir}/include/libunwind.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-arm.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-aarch64.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-common.h
unwind_include_HEADERS +=${top_srcdir}/include/compiler.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-coredump.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-dynamic.h
unwind_include_HEADERS +=${top_srcdir}/include/libunwind-ptrace.h
unwind_include_HEADERS +=${top_srcdir}/include/map_info.h
unwind_include_HEADERS +=${top_srcdir}/include/unwind.h
unwind_include_HEADERS +=${top_srcdir}/include/mempool.h
unwind_include_HEADERS +=${top_srcdir}/include/remote.h

