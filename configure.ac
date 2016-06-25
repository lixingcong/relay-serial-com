#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([relay-serial-com], [1.0], [lixingcong@live.com])
AM_INIT_AUTOMAKE(prelay-serial-com, 1.0)
AC_CONFIG_SRCDIR([src/utils.h])

# Checks for programs.
AC_PROG_CC

# check modules
AC_ARG_ENABLE(serial,
[  --enable-serial    Turn on module for serial],
[case "${enableval}" in
  yes) serial=true ;;
  no)  serial=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-serial) ;;
esac],[serial=false])
AM_CONDITIONAL(BUILD_SERIAL, test x$serial = xtrue)

AC_ARG_ENABLE(bluetooth,
[  --enable-bluetooth    Turn on module for bluetooth],
[case "${enableval}" in
  yes) bluetooth=true ;;
  no)  bluetooth=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-bluetooth) ;;
esac],[bluetooth=false])
AM_CONDITIONAL([BUILD_BLUETOOTH], [test x$bluetooth = xtrue])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([inet_ntoa memset select socket strchr])

AC_OUTPUT(Makefile src/Makefile)
