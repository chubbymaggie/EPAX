/**
 * @file MachOBinary.hpp
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


#ifndef __EPAX_MachOBinary_hpp__
#define __EPAX_MachOBinary_hpp__

#include "BaseClass.hpp"

namespace EPAX {

    namespace MachO {

        class MachHeader;

        class MachOBinary : public BaseBinary {
        protected:
            MachHeader* machheader;
        
        public:
            MachOBinary(std::string n);
            virtual ~MachOBinary();

            virtual BinaryFormat getFormat() = 0;
            uint64_t getStartAddr();
            void emit(std::string n);

            bool verify();
            bool isARM();
            void describe();

            bool is32Bit();
            bool is64Bit();
            bool isExecutable();

            void findFunctions();
            void findSymbols();
            void findSections();

            bool insideTextRange(uint64_t a);
            uint64_t functionEndAddress(Function* f, Function* nextf);

            void printSections(std::ostream& stream = std::cout);
            void printFunctions(std::ostream& stream = std::cout);

        }; // class MachOBinary

        class MachOBinary32 : public MachOBinary {
        public:
            MachOBinary32(std::string n);
            virtual ~MachOBinary32() {}

            BinaryFormat getFormat() { return BinaryFormat_MachO32; }
        }; // class MachOBinary32

        class MachOBinary64 : public MachOBinary {
        public:
            MachOBinary64(std::string n);
            virtual ~MachOBinary64() {}

            BinaryFormat getFormat() { return BinaryFormat_MachO64; }
        }; // class MachOBinary64

        class MachHeader : public FileBase {
        protected:
            rawbyte_t* entry;

            static void describeISA(int32_t ctype, int32_t stype);

        public:
            MachHeader(BaseBinary* b, uint64_t o, uint64_t s);
            virtual ~MachHeader();

            virtual uint64_t getStartAddr() = 0;
            virtual bool verify() = 0;
            virtual bool isARM() = 0;
            virtual void describe() = 0;
            virtual uint32_t getFileType() = 0;
        }; // class MachHeader

        class MachHeader32 : public MachHeader {
        public:

            MachHeader32(BaseBinary* b, uint64_t o);
            virtual ~MachHeader32() {}

            uint64_t getStartAddr();
            bool verify();
            bool isARM();
            void describe();
            uint32_t getFileType();
        }; // class MachHeader32

        class MachHeader64 : public MachHeader {
        public:
            MachHeader64(BaseBinary* b, uint64_t o);
            virtual ~MachHeader64() {}

            uint64_t getStartAddr();
            bool verify();
            bool isARM();
            void describe();
            uint32_t getFileType();
        }; // class MachHeader64

    } // namespace MachO    

} // namespace EPAX

#endif // __EPAX_MachOBinary_hpp__

