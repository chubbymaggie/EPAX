/**
 * @file MachOBinary.cpp
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

#include "MachO/loader.h"

#include "InputFile.hpp"
#include "MachOBinary.hpp"
#include "BaseClass.hpp"

namespace EPAX {

    namespace MachO {

#define MHDR32_ENTRY ((mach_header*)entry)
#define MHDR64_ENTRY ((mach_header_64*)entry)

        MachOBinary::MachOBinary(std::string n)
            : BaseBinary(n), machheader(INVALID_PTR)
        {
        }

        MachOBinary::~MachOBinary(){
            if (IS_VALID_PTR(machheader)){
                delete machheader;
            }
        }

        uint64_t MachOBinary::functionEndAddress(Function* f, Function* nextf){
            __do_not_call__;
        }

        bool MachOBinary::insideTextRange(uint64_t a){
            __do_not_call__;
        }

        bool MachOBinary::is32Bit(){
            return (getFormat() == BinaryFormat_MachO32);
        }

        bool MachOBinary::is64Bit(){
            return (getFormat() == BinaryFormat_MachO64);
        }

        uint64_t MachOBinary::getStartAddr(){
            return machheader->getStartAddr();
        }

        MachOBinary32::MachOBinary32(std::string n)
            : MachOBinary(n)
        {
            machheader = new MachHeader32(this, 0);
        }

        MachOBinary64::MachOBinary64(std::string n)
            : MachOBinary(n)
        {
            machheader = new MachHeader64(this, 0);
        }

        void MachOBinary::emit(std::string n){
            __do_not_call__;
        }

        bool MachOBinary::verify(){
            return machheader->verify();
        }

        bool MachOBinary::isARM(){
            return machheader->isARM();
        }

        void MachOBinary::describe(){
            machheader->describe();
        }

        void MachOBinary::findFunctions(){
            __do_not_call__;
        }

        void MachOBinary::findSymbols(){
            __do_not_call__;
        }

        void MachOBinary::findSections(){
            __do_not_call__;
        }

        void MachOBinary::printSections(std::ostream& stream){
            __do_not_call__;
        }

        void MachOBinary::printFunctions(std::ostream& stream){
            __do_not_call__;
        }

        MachHeader::MachHeader(BaseBinary* b, uint64_t o, uint64_t s)
            : FileBase(b, o, s),
              entry(INVALID_PTR)
        {
        }

        MachHeader::~MachHeader(){
            if (IS_VALID_PTR(entry)){
                delete entry;
            }
        }

        MachHeader32::MachHeader32(BaseBinary* b, uint64_t o)
            : MachHeader(b, o, sizeof(mach_header))
        {
            entry = (rawbyte_t*)new mach_header();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        MachHeader64::MachHeader64(BaseBinary* b, uint64_t o)
            : MachHeader(b, o, sizeof(mach_header_64))
        {
            entry = (rawbyte_t*)new mach_header_64();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        uint64_t MachHeader32::getStartAddr(){
            return MHDR32_ENTRY->cputype;
        }

        uint64_t MachHeader64::getStartAddr(){
            return MHDR64_ENTRY->cputype;
        }

        bool MachHeader32::verify(){
            if (MHDR32_ENTRY->magic == MH_MAGIC){
                return true;
            }
            return false;
        }

       bool MachHeader64::verify(){
            if (MHDR64_ENTRY->magic == MH_MAGIC_64){
                return true;
            }
            return false;
        }

        bool MachHeader32::isARM(){
            if (MHDR32_ENTRY->cputype == CPU_TYPE_ARM){
                return true;
            }
        }

        bool MachHeader64::isARM(){
            // TODO: have a good idea, but not totally sure what ARM64 looks like here
            return false;
        }

        uint32_t MachHeader32::getFileType(){
            return MHDR32_ENTRY->filetype;
        }

        uint32_t MachHeader64::getFileType(){
            return MHDR64_ENTRY->filetype;
        }

        void MachHeader::describe(){
            EPAXOut << "format=mach-o" << TAB;
        }

        bool MachOBinary::isExecutable(){
            return (machheader->getFileType() == MH_EXECUTE);
        }

        void MachHeader::describeISA(int32_t ctype, int32_t stype){
            if (ctype == CPU_TYPE_ARM){
                switch(stype){
#define SCASE(__typ__) case CPU_SUBTYPE_ARM_ ## __typ__: std::cout << "isa=ARM_" << #__typ__; break
                    SCASE(ALL);
                    SCASE(V4T);
                    SCASE(V6);
                    SCASE(V5TEJ);
                    SCASE(XSCALE);
                    SCASE(V7);
                    SCASE(V7F);
                    SCASE(V7K);
                default:
                    std::cout << "isa=ARM";
                }
            } else {
                switch(ctype & 0xffff){
#define CCASE(__typ__) case CPU_TYPE_ ## __typ__: std::cout << "isa=" << #__typ__; break
                    CCASE(VAX);
                    CCASE(MC680x0);
                    CCASE(X86);
                    CCASE(MC98000);
                    CCASE(HPPA);
                    CCASE(MC88000);
                    CCASE(SPARC);
                    CCASE(I860);
                    CCASE(POWERPC);
                default:
                    std::cout << "isa=unknown";
                }
            }
            std::cout << ENDL;
        }

        void MachHeader32::describe(){
            MachHeader::describe();
            std::cout << "bits=32" << TAB;
            MachHeader::describeISA(MHDR32_ENTRY->cputype, MHDR32_ENTRY->cpusubtype);
        }

        void MachHeader64::describe(){
            MachHeader::describe();
            std::cout << "bits=64" << TAB;
            MachHeader::describeISA(MHDR64_ENTRY->cputype, MHDR64_ENTRY->cpusubtype);
        }
        
    } // namespace MachO

} // namespace EPAX
