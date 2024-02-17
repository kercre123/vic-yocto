AC_PREREQ(2.61)
AC_INIT([binder], 1.0.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
LT_INIT
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

AS_CASE([$host],
        [arm*], [ARM=yes],
        [ARM=no]
)

ARM=yes

AC_ARG_ENABLE(32bit-binder-ipc,
      AC_HELP_STRING([--enable-32bit-binder-ipc],
        [enables IPC on 32bit kernel]),
        [enable_32bit_ipc=${enableval}])

AM_CONDITIONAL(IPC_32BIT, [test "x${enable_32bit_ipc}" = "xyes"])

AC_ARG_WITH([core_includes],
      AC_HELP_STRING([--with-core-includes=@<:@dir@:>@],
         [Specify the location of the core headers]),
      [core_incdir=$withval],
      with_core_includes=no)

if test "x$with_core_includes" != "xno"; then
     CPPFLAGS="${CPPFLAGS} -I${core_incdir}"
     AC_SUBST(core_incdir,${core_incdir})
fi

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

AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([LIBS])

AC_CONFIG_FILES([ \
        Makefile  \
        libs/binder/Makefile \
        libs/binder/libbinder.pc \
        libs/ui/Makefile \
        libs/ui/libui.pc \
        cmds/servicemanager/Makefile \
        include/Makefile \
        opengl/Makefile
        ])
AC_OUTPUT
