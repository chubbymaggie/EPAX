#
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([EPAX], [0.01], [help@epaxtoolkit.com])
AC_CONFIG_SRCDIR([include/EPAX.hpp])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG_PUSH([C++])

AC_CONFIG_HEADERS([src/Config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([bzero memset])

DARM_INCLUDE=""
DARM_LIBDIR=""
DARM_LIB=""
AC_ARG_WITH(darm,
            [AS_HELP_STRING([--with-darm=<path>],[path to darm header])],
            [darm=$withval
             AC_CHECK_HEADER([$darm/darm.h],[DARM_INCLUDE=-I$darm
                                                        DARM_LIBDIR=$darm
                                                        DARM_LIB=-L$DARM_LIBDIR],
                             [AC_MSG_ERROR($darm/darm.h not found)])],
            [AC_CHECK_HEADERS([darm],,AC_ERROR("required header file darm.h missing"))])
AC_SUBST(DARM_INCLUDE)
AC_SUBST(DARM_LIBDIR)
AC_SUBST(DARM_LIB)

# for Elf
AC_CHECK_HEADERS([elf.h])

# for all platforms
AC_CHECK_HEADERS([execinfo.h])
AC_CHECK_HEADERS([stdint.h stdlib.h stddef.h string.h sys/ptrace.h sys/user.h errno.h sys/types.h sys/wait.h unistd.h],,AC_ERROR("required C header file missing"))
AC_CHECK_HEADERS([iostream iomanip fstream string vector map],,AC_ERROR("required C++ header file missing"))

CTORS_FLAGS=""
AC_MSG_CHECKING([sys/user.h has struct user_regs])
AC_TRY_RUN([
#include <sys/user.h>
struct user_regs u;
int main() { return 0; }
],[AC_MSG_RESULT([yes])], [AC_MSG_RESULT([no])
AC_ERROR([sys/user.h lacks struct user_regs... you are probably not an ARM platform])])


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 env/bashrc
                 env/cshrc])
AC_OUTPUT
