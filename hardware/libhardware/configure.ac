AC_PREREQ([2.65])
AC_INIT("libhardware", "0.0.1", "")
AM_INIT_AUTOMAKE([1.10 no-define foreign])
AC_CONFIG_SRCDIR([hardware.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile \
                 include/Makefile \
                 hardware.pc
                 ])


AC_PROG_CXX
AC_PROG_CC

LT_INIT

AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strerror])

AC_OUTPUT
