#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([oem-recovery],
	1.0.0)
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
#AC_CONFIG_SRCDIR([bootloader.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_CPP
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_CXX
PKG_PROG_PKG_CONFIG

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gettimeofday memmove memset mkdir munmap pow rmdir strchr strdup strerror strncasecmp strstr strtol strtoul utime])

AC_ARG_WITH([sanitized-headers],
      AC_HELP_STRING([--with-sanitized-headers=@<:@dir@:>@],
         [Specify the location of the sanitized Linux kernel headers]),
      [CPPFLAGS="$CPPFLAGS -idirafter $withval"],
      with_sanitized-headers=no)

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
        AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")

AC_ARG_WITH([core-headers],
      AC_HELP_STRING([--with-core-headers=@<:@dir@:>@],
         [Specify the location of the system core headers]),
      [CPPFLAGS="$CPPFLAGS -I$withval"],
      with_core-headers=no)

AC_SUBST(CFLAGS)
AC_CONFIG_FILES([     \
        Makefile      \
        ])
AC_OUTPUT
