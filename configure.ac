#
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([EPAX], [0.01], [help@epaxtoolkit.com])
AC_CONFIG_SRCDIR([include/EPAX.hpp])
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_python.m4])
m4_include([m4/ax_cxx_compile_stdcxx_11.m4])

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

# c++11, for unordered_map
AX_CXX_COMPILE_STDCXX_11([],[optional])
CXX11_DEFS=""
if test "$HAVE_CXX11" == "1"; then
   CXX11_DEFS="-DHAVE_CXX11"
fi
AC_SUBST(CXX11_DEFS)

# check for working python to collect line information
AC_ARG_WITH(lineinfo, [allow build without line information], allow_noline=yes, allow_noline=no)

AX_PYTHON()
PYTHON_FULL_PATH=$(which $PYTHON_BIN)
HAVE_LINEINFO=1
if test "$PYTHON_FULL_PATH" == ""; then
   AC_MSG_WARN($PYTHON_BIN cannot be resolved and line information will not be supported)
   if test "$allow_noline" == "no"; then
      AC_MSG_ERROR(Use --without-lineinfo to allow build to proceed)
   else
      AC_WARN([Building without line information support])
      HAVE_LINEINFO=0
   fi
else
   EPAX_CONFIG_CHECK=1 $PYTHON_FULL_PATH $(pwd)/etc/epax_lineinfo_lookup.py $(pwd)/external/pyelftools/
   pyres=$?
   if test "$pyres" != "0"; then
      AC_MSG_WARN(Your python does not support pyelftools)
      if test "$allow_noline" == "no"; then
         AC_MSG_ERROR(Use --without-lineinfo to allow build to proceed)
      else
         AC_WARN([Building without line information support])
         HAVE_LINEINFO=0
      fi
   fi
fi

AC_SUBST(PYTHON_FULL_PATH)
AC_SUBST(HAVE_LINEINFO)

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([bzero memset])

DISASM_INCLUDE=""
DISASM_LIBDIR=""
DISASM_LINK=""
DISASM_SOURCE=""
AC_ARG_WITH(darm,
            [AS_HELP_STRING([--with-darm=<path>],[path to darm header])],
            [darm=$withval
             AC_CHECK_HEADER([$darm/darm.h],[DISASM_INCLUDE=-I$darm
                                                        DISASM_LIBDIR=$darm
                                                        DISASM_LINK="-L$DISASM_LIBDIR -ldarm"
                                                        DISASM_SOURCE=DARM],
                             [AC_MSG_ERROR($darm/darm.h not found)])],)

AC_ARG_WITH(capstone,
            [AS_HELP_STRING([--with-capstone=<path>],[path to capstone installation])],
            [capstone=$withval
             AC_CHECK_HEADER([$capstone/include/capstone.h],[DISASM_INCLUDE=-I$capstone/include
                                                        DISASM_LIBDIR=$capstone
                                                        DISASM_LINK="-L$capstone -lcapstone"
                                                        DISASM_SOURCE=CAPSTONE],
                             [AC_MSG_ERROR($darm/include/capstone.h not found)])],)

found_disasm=""
if test "$capstone" != ""; then
    AC_CHECK_HEADER([$capstone/include/arm.h],,
                [AC_MSG_ERROR($capstone/include/arm.h not found)])
    AC_CHECK_HEADER([$capstone/include/arm64.h],,
                [AC_MSG_ERROR($capstone/include/arm64.h not found)])
    found_disasm="capstone"
fi

if test "$darm" != ""; then
    if test "$capstone" != ""; then
        AC_MSG_ERROR(cannot use both --with-darm and --with-capstone)
    fi
    found_disasm="darm"
fi

if test "$found_disasm" == ""; then
    AC_MSG_ERROR(you must use either --with-darm or --with-capstone)
fi

AC_SUBST(DISASM_INCLUDE)
AC_SUBST(DISASM_LIBDIR)
AC_SUBST(DISASM_LINK)
AC_SUBST(DISASM_SOURCE)

# for Elf
AC_CHECK_HEADERS([elf.h])

# for all platforms
AC_CHECK_HEADERS([execinfo.h])
AC_CHECK_HEADERS([stdint.h stdlib.h stddef.h string.h sys/ptrace.h sys/user.h errno.h sys/types.h sys/wait.h unistd.h],,AC_ERROR("required C header file missing"))
AC_CHECK_HEADERS([iostream iomanip fstream string vector map],,AC_ERROR("required C++ header file missing"))

AC_MSG_CHECKING([sys/user.h has struct user_regs])
AC_TRY_RUN([
#include <sys/user.h>
struct user_regs u;
int main() { return 0; }
],[AC_MSG_RESULT([yes])
CXXFLAGS="${CXXFLAGS} -DHAVE_ARMV7_NATIVE"],
[AC_MSG_RESULT([no])
AC_WARN([sys/user.h lacks struct user_regs. we are assuming you are not on an ARMv7 platform and will disable runtime functionality])])

AC_MSG_CHECKING([asm/ptrace.h has struct user_pt_regs])
AC_TRY_RUN([
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <linux/uio.h>
struct iovec i;
struct user_pt_regs u;
int main() { return 0; }
],[AC_MSG_RESULT([yes])
CXXFLAGS="${CXXFLAGS} -DHAVE_ARMV8_NATIVE"],
[AC_MSG_RESULT([no])
AC_WARN([asm/ptrace.h lacks struct user_pt_regs. we are assuming you are not on an ARMv8 platform and will disable runtime functionality])])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 examples/Makefile
                 env/bashrc
                 env/cshrc
                 src/EPAXDefs.hpp])
AC_OUTPUT
chmod +x etc/epax_lineinfo_lookup.py
