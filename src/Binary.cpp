/**
 * @file Binary.cpp
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

#include "EPAXCommonInternal.hpp"

#include "Binary.hpp"
#include "ElfBinary.hpp"
#include "Function.hpp"
#include "Instruction.hpp"
#include "LineInformation.hpp"
#include "MachOBinary.hpp"

namespace EPAX {

    Binary::Binary(std::string n)
        : EPAXExport(EPAXExportClass_BIN), binary(INVALID_PTR), lineinfo(INVALID_PTR)
    {
        construct(n, BinaryFormat_undefined);
    }

    Binary::Binary(std::string n, BinaryFormat f)
        : EPAXExport(EPAXExportClass_BIN), binary(INVALID_PTR), lineinfo(INVALID_PTR)
    {
        construct(n, f);
    }

    Binary::~Binary(){
        if (IS_VALID_PTR(binary)){
            delete binary;
        }
        if (IS_VALID_PTR(lineinfo)){
            delete lineinfo;
        }
    }

    void Binary::printStaticFile(std::string& fname){
        printStaticFile(fname.c_str());
    }

    void Binary::printStaticFile(const char* fname){
    }


#ifdef HAVE_ARMV7_NATIVE

#define PTRACE_AND_CHECK(__opt, __pid, __addr, __data)                  \
    res = ptrace(__opt, __pid,__addr,__data);                           \
    if (res == -1){                                                     \
        EPAXErr << "ptrace failed with " << DEC(errno) << ENDL;       \
    }                                                                   \
    EPAXAssert(res == 0, "call to ptrace(" << #__opt << ", pid=" << DEC(__pid) << ", addr=" << HEX(__addr) << ", data=" << HEX(__data) << ") failed with error " << DEC(res));
    
    // does not return
    void Binary::runBasic(int argc, char* argv[]){
        long res;

        pid_t pid = fork();
        if (pid == 0){
            // child that runs bin
            PTRACE_AND_CHECK(PTRACE_TRACEME, 0, NULL, NULL);

            EPAXOut << "child (" << getName();
            for (int i = 1; i < argc; i++){
                std::cout << " " << argv[i];
            }
            std::cout << ") will start at " << HEX(getStartAddr()) << ENDL;

            int ret = execve(argv[0], argv, NULL);
            EPAXAssert(false, "exec returned with an error: " << DEC(ret));
        }

        // parent/epax process
        siginfo_t s;
        struct user_regs bregs;

	// wait, then be sure the signal caught is a SIGTRAP from child's execve
        wait(NULL);
        PTRACE_AND_CHECK(PTRACE_GETSIGINFO, pid, NULL, &s);
        EPAXAssert(s.si_signo == SIGTRAP, "unexpected signal caught (expect SIGTRAP resulting from execve)");

	// grab the PC of the child process
        PTRACE_AND_CHECK(PTRACE_GETREGS, pid, NULL, &bregs);
        EPAXOut << getName() << " PC: " << HEX(bregs.uregs[15]) << ENDL; // TODO: get magic num 15 from armd

        long b = ptrace(PTRACE_PEEKTEXT, pid, bregs.uregs[15], NULL);
        EPAXOut << getName() << " instruction bytes: " << HEX(b) << ENDL; // TODO: get magic num 15 from armd

        // plan: here we need to inject code that copies the contents of the parent process (epax) into the child.
        // this code can then shepherd the child process, taking control back at every BB, for example.
        PTRACE_AND_CHECK(PTRACE_POKETEXT, pid, bregs.uregs[15], b);

        // prints out the instruction stream, 1 PC at a time
//#define SINGLE_STEP_THROUGH
#ifdef SINGLE_STEP_THROUGH
        while (1){
            PTRACE_AND_CHECK(PTRACE_SINGLESTEP, pid, NULL, NULL);
            wait(NULL);
            PTRACE_AND_CHECK(PTRACE_GETREGS, pid, NULL, &bregs);
            EPAXOut << getName() << " PC: " << HEX(bregs.rip) << ENDL;
        }
#else
        PTRACE_AND_CHECK(PTRACE_CONT, pid, NULL, NULL);
#endif
        wait(NULL);
        exit(0);        
    }

    // TODO: write the ARMv8 version of this function (see examples/pcsample.cpp for some useful bits)
#else // HAVE_ARMV7_NATIVE
    void Binary::runBasic(int argc, char* argv[]){
	EPAXDie("ARMv7 Native support not enabled. You cannot run executables.");
    }
#endif // HAVE_ARMV7_NATIVE

    const char* Binary::getFormatName(){
        return BaseBinary::getFormatName(format);
    }

    void Binary::construct(std::string n, BinaryFormat f){
        if (f == BinaryFormat_undefined){
            f = detectFormat(n);
        }

        format = f;
        EPAXOut << "File " << n << " is a " << getFormatName() << " file" << ENDL;

        EPAXAssert(format != BinaryFormat_undefined, "Cannot determine format of " << n << ". Try specifying it manually.");

        switch (format){
        case BinaryFormat_Elf32:
            binary = new Elf::ElfBinary32(n);
            break;
        case BinaryFormat_Elf64:
            binary = new Elf::ElfBinary64(n);
            break;
        case BinaryFormat_MachO32:
            binary = new MachO::MachOBinary32(n);
            break;
        case BinaryFormat_MachO64:
            binary = new MachO::MachOBinary64(n);
            break;
        default:
            EPAXDie("Unimplemented binary format " << getFormatName() << " given.");
        }

        EPAXOut << "Program entry point at vaddr " << HEX(binary->getStartAddr()) << ENDL;

        lineinfo = new LineInformation(binary);

        binary->describe();
        //EPAXAssert(binary->isARM(), "This binary contains non-ARM code... bailing");
    }

    BinaryFormat Binary::detectFormat(std::string n){
        BinaryFormat f = BinaryFormat_undefined;
        BaseBinary* bin;

#define VERIFY_SINGLE_FORMAT(__fmt__) EPAXAssert(f == BinaryFormat_undefined, "This binary appears to be valid for two formats: " << BaseBinary::getFormatName(__fmt__) << " and " << BaseBinary::getFormatName(f));

        // check for Elf32
        bin = new Elf::ElfBinary32(n);
        if (bin->verify()){
            VERIFY_SINGLE_FORMAT(BinaryFormat_Elf32);
            f = BinaryFormat_Elf32;
        }
        delete bin;

        // check for Elf64
        bin = new Elf::ElfBinary64(n);
        if (bin->verify()){
            VERIFY_SINGLE_FORMAT(BinaryFormat_Elf64);
            f = BinaryFormat_Elf64;
        }
        delete bin;

        // check for MachO32
        bin = new MachO::MachOBinary32(n);
        if (bin->verify()){
            VERIFY_SINGLE_FORMAT(BinaryFormat_MachO32);
            f = BinaryFormat_MachO32;
        }
        delete bin;

        // check for MachO64
        bin = new MachO::MachOBinary64(n);
        if (bin->verify()){
            VERIFY_SINGLE_FORMAT(BinaryFormat_MachO64);
            f = BinaryFormat_MachO64;
        }
        delete bin;

        return f;
    }

    uint64_t Binary::getStartAddr(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->getStartAddr();
    }

    std::string Binary::getName(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->getName();
    }

    Function* Binary::getFirstFunction(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->getFirstFunction();
    }

    Function* Binary::getNextFunction(Function* f){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->getNextFunction(f);
    }

    bool Binary::isLastFunction(Function* f){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->isLastFunction(f);
    }

    Function* Binary::findFunctionAt(uint64_t addr){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->findFunctionAt(addr);
    }

    uint32_t Binary::countFunctions(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->countFunctions();
    }

    bool Binary::isExecutable(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->isExecutable();
    }

    uint32_t Binary::getFileSize(){
        EPAXAssert(IS_VALID_PTR(binary), "Binary is not valid");
        return binary->getFileSize();
    }

    bool Binary::hasDebugLineInfo(){
        if (IS_VALID_PTR(lineinfo)){
            return lineinfo->hasInformation();
        }
        return false;
    }

    uint32_t Binary::getDebugLineNumber(uint64_t addr){
        if (hasDebugLineInfo()){
            return lineinfo->getLineNumber(addr);
        }
        return 0;
    }

    std::string Binary::getDebugLineFile(uint64_t addr){
        if (hasDebugLineInfo()){
            return lineinfo->getLineFile(addr);
        }
        return NAME_UNKNOWN;
    }
} // namespace EPAX
