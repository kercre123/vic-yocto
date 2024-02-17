AC_PREREQ(2.61)
AC_INIT([bt-app], 1.0.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AM_MAINTAINER_MODE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

AC_ARG_WITH([glib],
     AC_HELP_STRING([--with-glib],
        [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
       PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                               AC_MSG_ERROR(GThread >= 2.16 is required))
       PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                               AC_MSG_ERROR(GLib >= 2.16 is required))
       GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
       GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

       AC_SUBST(GLIB_CFLAGS)
       AC_SUBST(GLIB_LIBS)

       AC_ARG_WITH([gstreamer],
           AS_HELP_STRING([--with-gstreamer],[use gstreamer pipeline for audio playback]))
       USE_GST=NO
       if test "x$with_gstreamer" = "xyes"; then
           PKG_CHECK_MODULES(GSTREAMER, [gstreamer-1.0 gstreamer-tag-1.0], USE_GST=yes, USE_GST=no)
       fi
       AC_SUBST(GSTREAMER_LIBS)
       AC_SUBST(GSTREAMER_CFLAGS)
       AC_SUBST(USE_GST)
       AM_CONDITIONAL(USE_GST, test "x$USE_GST" = "xyes")
       if test "x$USE_GST" = "xyes"; then
           AC_DEFINE(USE_GST, 1, [Define if use GStreamer])
       fi
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")

# Library configs
AC_ARG_WITH([common_includes],
      AC_HELP_STRING([--with-common-includes=@<:@dir@:>@],
         [Specify the location of the common headers]),
      [common_incdir=$withval],
      with_common_includes=no)

if test "x$with_common_includes" != "xno"; then
   CFLAGS="${CFLAGS} -I${common_incdir}"
fi

AC_ARG_WITH([lib_path],
      AC_HELP_STRING([--with-lib-path=@<:@dir@:>@],
         [Specify the location of the libraries]),
      [std_libdir=$withval],
      with_lib_path=no)

if test "x$with_lib_path" != "xno"; then
   SYS_LIB=${std_libdir}
fi

AC_ARG_WITH([btobex],
     AC_HELP_STRING([--with-btobex],
        [enable btobex, building BT-APP which uses BTOBEX]))

AM_CONDITIONAL(USE_BT_OBEX, test "x${with_btobex}" = "xyes")

AC_ARG_ENABLE(target,
  [AS_HELP_STRING([--enable-target=TARGET], [Specify the target product to build])],
  [TARGET=$enableval],
  [TARGET=none]
)
AM_CONDITIONAL([AUDIO_HAL_SUPPORTED], [test "x$TARGET" = "xapq8009" -o "x$TARGET" = "xapq8017" -o "x$TARGET" = "xapq8096" -o "x$TARGET" = "xapq8053" -o "x$TARGET" = "xapq8098"])

AC_SUBST(SYS_LIB)
AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([Makefile\
    osi/Makefile\
    main/Makefile])
AC_OUTPUT
