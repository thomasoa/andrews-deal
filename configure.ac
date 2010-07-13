#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.61)
AC_INIT([deal],[3.1.9], [deal@thomasoandrews.com])
dnl Most of these things are boiler plate (from autoscan);
dnl but here's some of my stuff
dnl BEGIN Thomas's BLOCK OF STUFF

AC_CHECK_LIB([tcl],[Tcl_CreateInterp], [], [
    echo "Tcl library is required for this program"
    exit -1])

dnl END Thomas's BLOCK OF STUFF

#--------------------------------------------------------------------
# Load the tclConfig.sh file
#--------------------------------------------------------------------

AM_INIT_AUTOMAKE([deal], [3.1.9])
AC_CONFIG_SRCDIR([additive.c])
AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h limits.h stddef.h stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset sqrt])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
