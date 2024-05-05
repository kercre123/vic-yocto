#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for usb audio hal
#

# Requires autoconf tool later than 2.65
AC_PREREQ([2.65])
# Initialize the usb audio hal package version 1.0.0
AC_INIT("usbaudio", "1.0.0", "")
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Verifies the --srcdir is correct by checking for the path
AC_CONFIG_SRCDIR([audio_hal.c])
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([config.h])

# Checks for programs
AC_PROG_CXX
AC_PROG_CC
LT_INIT

AC_CONFIG_FILES([Makefile \
                 usb_audio.pc
                 ])

AC_OUTPUT
