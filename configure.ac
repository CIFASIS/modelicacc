#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_LANG([C++])
AC_INIT([ModelicaCC], [5.0.0], [fernandez@cifasis-conicet.gov.ar])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_LANG([C++])
AC_CHECK_HEADERS([ginac/ginac.h],[],[AC_MSG_ERROR([ModelicaCC needs libginac.])],[])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
