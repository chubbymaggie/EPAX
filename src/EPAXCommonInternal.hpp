/**
 * @file EPAXCommonInternal.hpp
 *
 * @section LICENSE
 * This file is part of the EPAX toolkit.
 * 
 * Copyright (c) 2013, EP Analytics, Inc.
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __EPAX_EPAXCommonInternal_hpp__
#define __EPAX_EPAXCommonInternal_hpp__

#ifdef HAVE_CONFIG_H
#include "Config.h"
#else
#error "Cannot continue without Config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <bitset>
#include <stack>
#include <set>
#include <algorithm>

#define INVALID_PTR (NULL)
#define IS_VALID_PTR(__p__) ((__p__ != INVALID_PTR))
#define MAX_STRING_SIZE (1024)
#define NAME_UNKNOWN "__unknown__"

#define EPAX_PREFACE "-EPAX- "
#define ENDL "\n"
#define TAB "\t"
#define DEC(__n__) std::dec << (__n__)
#define HEX(__n__) std::hex << "0x" << (__n__)

#define BACKTRACE_LIMIT 64
static void* _backtraceArray[BACKTRACE_LIMIT];
static size_t _backtraceSize;
static char** _backtraceStrings;
static int _arrayBacktraceIterator;

#define EPAXErr std::cerr << EPAX_PREFACE
#define EPAXOut std::cout << EPAX_PREFACE
#define EPAXWarn EPAXErr << " warning: "

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#define EPAXAssert(__stmt__, __msg__)                                 \
    if (!(__stmt__)){ _backtraceSize = backtrace(_backtraceArray, BACKTRACE_LIMIT); _backtraceStrings = backtrace_symbols(_backtraceArray, _backtraceSize); \
        EPAXErr << "Assert failure: " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__ << ENDL; \
        EPAXErr << __msg__ << ENDL;                                   \
        for (_arrayBacktraceIterator = 0; _arrayBacktraceIterator < _backtraceSize; _arrayBacktraceIterator++){ EPAXErr << TAB << _backtraceStrings[_arrayBacktraceIterator] << ENDL; } \
        free(_backtraceStrings);                                        \
        exit(1); }
#else // !HAVE_EXECINFO_H
#define EPAXAssert(__stmt__, __msg__)                                 \
    if (!(__stmt__)){                                                   \
        EPAXErr << "Assert failure: " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__ << ENDL; \
        EPAXErr << __msg__ << ENDL;                                   \
        exit(1); }
#endif // HAVE_EXECINFO_H


#define EPAXDie(__msg__) EPAXAssert(false, __msg__)
#define ShouldNotArrive EPAXDie("This function is not yet implemented")

typedef enum {
    EPAXExportClass_undefined = 0,
    EPAXExportClass_BIN,
    EPAXExportClass_SECT,
    EPAXExportClass_FUNC,
    EPAXExportClass_CFG,
    EPAXExportClass_LOOP,
    EPAXExportClass_BBL,
    EPAXExportClass_INSN,
    EPAXExportClass_SYM,
    EPAXExportClass_FLOW,
    EPAXExportClass_total
} EPAXExportClass;

class EPAXExport {
protected:
    EPAXExportClass expclass;
public:
    EPAXExport(EPAXExportClass cls) { expclass = cls; }
    virtual ~EPAXExport() {}

    EPAXExportClass getClass() { return expclass; }
}; // class EPAXExport

#define EPAXVerifyType(__type__, __obj__)\
    EPAXAssert(IS_VALID_PTR(__obj__), "invalid object (NULL) found instead of " # __type__);\
    EPAXAssert(__obj__->getClass() == EPAXExportClass_ ## __type__, "Non-" # __type__ << " object found");

#define __do_not_call__ EPAXAssert(false, "This function cannot be called.")
#define ALIGN_PWR2(__addr, __exp) (__addr & ~((1 << __exp) - 1))

typedef char rawbyte_t;

#define INVALID_ADDRESS (0x0)
#define ADDRESS_IS_THUMB(__a) ((__a & 0x1) == 1)

#endif // __EPAX_EPAXCommonInternal_hpp__
