/**
 * @file ElfBinary.hpp
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


#ifndef __EPAX_ElfBinary_hpp__
#define __EPAX_ElfBinary_hpp__

#include "BaseClass.hpp"
#include "Section.hpp"
#include "Symbol.hpp"

namespace EPAX {

    namespace Elf {

        class ElfStringTable;
        class FileHeader;
        class SectionHeader;
        class ProgramHeader;

        class ElfBinary : public BaseBinary {
        protected:
            FileHeader* fileheader;

            bool foundsections;
            std::vector<SectionHeader*>* sections;
            bool foundsegments;
            std::vector<ProgramHeader*>* segments;
        
        public:
            ElfBinary(std::string n);
            virtual ~ElfBinary();

            virtual BinaryFormat getFormat() = 0;
            uint64_t getStartAddr();
            void emit(std::string n);

            bool verify();
            bool isARM();
            void describe();

            void findFunctions();
            void findSymbols();
            void findSections();
            void findSegments();

            bool is32Bit();
            bool is64Bit();

            bool isExecutable();

            ElfStringTable* findStringtable(uint32_t i);
            bool insideTextRange(uint64_t a);

            void printSections(std::ostream& stream = std::cout);
            void printFunctions(std::ostream& stream = std::cout);

            uint64_t vaddrToFile(uint64_t v);
            uint64_t functionEndAddress(Function* f, Function* nextf);

        }; // class ElfBinary

        class ElfBinary32 : public ElfBinary {
        public:
            ElfBinary32(std::string n);
            virtual ~ElfBinary32() {}

            BinaryFormat getFormat() { return BinaryFormat_Elf32; }
        }; // class ElfBinary32

        class ElfBinary64 : public ElfBinary {
        public:
            ElfBinary64(std::string n);
            virtual ~ElfBinary64() {}

            BinaryFormat getFormat() { return BinaryFormat_Elf64; }
        }; // class ElfBinary64


        class FileHeader : public FileBase {
        protected:
            rawbyte_t* entry;

            static void describeISA(uint32_t ctype);

        public:
            FileHeader(BaseBinary* b, uint64_t o, uint64_t s);
            virtual ~FileHeader();

            virtual uint64_t getStartAddr() = 0;
            virtual bool verify() = 0;
            virtual bool isARM() = 0;
            virtual void describe() = 0;

            virtual uint32_t getSectionCount() = 0;
            virtual uint64_t getSecTableOffset() = 0;
            virtual uint32_t getShdrSize() = 0;
            virtual uint32_t getShdrStringIndex() = 0;
            virtual uint32_t getSegmentCount() = 0;
            virtual uint64_t getSegTableOffset() = 0;
            virtual uint32_t getPhdrSize() = 0;
            virtual uint32_t getFileType() = 0;

        }; // class FileHeader

        class FileHeader32 : public FileHeader {
        public:

            FileHeader32(BaseBinary* b, uint64_t o);
            virtual ~FileHeader32() {}

            uint64_t getStartAddr();
            bool verify();
            bool isARM();
            void describe();

            uint32_t getSectionCount();
            uint64_t getSecTableOffset();
            uint32_t getShdrSize();
            uint32_t getShdrStringIndex();
            uint32_t getSegmentCount();
            uint64_t getSegTableOffset();
            uint32_t getPhdrSize();
            uint32_t getFileType();

        }; // class FileHeader32

        class FileHeader64 : public FileHeader {
        public:
            FileHeader64(BaseBinary* b, uint64_t o);
            virtual ~FileHeader64() {}

            uint64_t getStartAddr();
            bool verify();
            bool isARM();
            void describe();

            uint32_t getSectionCount();
            uint64_t getSecTableOffset();
            uint32_t getShdrSize();
            uint32_t getShdrStringIndex();
            uint32_t getSegmentCount();
            uint64_t getSegTableOffset();
            uint32_t getPhdrSize();
            uint32_t getFileType();

        }; // class FileHeader64

        class ElfSymbol : public Symbol {
        protected:
            rawbyte_t* entry;

        public:
            ElfSymbol(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~ElfSymbol();

            void print(std::ostream& stream = std::cout);

            virtual uint64_t getNameIndex() = 0;
            virtual uint64_t getValue() = 0;
            virtual uint32_t getSection() = 0;
            virtual uint32_t getSize() = 0;
            virtual uint32_t getType() = 0;
            virtual uint32_t getBinding() = 0;
            virtual uint64_t getVisibility() = 0;

            bool isFunction();
            bool isThumbFunction();
            uint64_t getFunctionAddress();
        }; // class ElfSymbol

        class ElfSymbol32 : public ElfSymbol {
        public:
            ElfSymbol32(BaseBinary* b, uint64_t o, uint32_t i);
            virtual ~ElfSymbol32() {}

            uint64_t getNameIndex();
            uint64_t getValue();
            uint32_t getSection();
            uint32_t getSize();
            uint32_t getType();
            uint32_t getBinding();
            uint64_t getVisibility();

        }; // class ElfSymbol32

        class ElfSymbol64 : public ElfSymbol {
        public:
            ElfSymbol64(BaseBinary* b, uint64_t o, uint32_t i);
            virtual ~ElfSymbol64() {}

            uint64_t getNameIndex();
            uint64_t getValue();
            uint32_t getSection();
            uint32_t getSize();
            uint32_t getType();
            uint32_t getBinding();
            uint64_t getVisibility();

        }; // class ElfSymbol64

        class ElfStringTable : public StringTable {
        private:
            rawbyte_t* entry;

        public:
            ElfStringTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n);
            ~ElfStringTable();

            char* getStringAt(uint32_t i);

            void print(std::ostream& stream = std::cout);
        }; // class ElfStringTable

        class ElfSymbolTable : public SymbolTable {
        private:
            ElfStringTable* stringtab;

        public:
            ElfSymbolTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n, ElfStringTable* st);
            ~ElfSymbolTable() {}

            void print(std::ostream& stream = std::cout);
            
        }; // class ElfSymbolTable

        class SectionHeader : public FileBase, public NameBase, public IndexBase {
        protected:
            rawbyte_t* entry;

        public:
            SectionHeader(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~SectionHeader();

            void print(std::ostream& stream = std::cout);

            bool isText();
            bool isData();
            bool isBSS();
            bool isDebug();
            bool isString();
            bool isSymbol();

            bool isRead();
            bool isWrite();
            bool isExec();
            bool isAlloc();
            bool isMerge();

            bool inRange(uint64_t a);

            virtual uint64_t getNameIndex() = 0;
            virtual uint64_t getType() = 0;
            virtual uint64_t getVirtAddr() = 0;
            virtual uint64_t getFileOffset() = 0;
            virtual uint64_t getSectionLink() = 0;
            virtual uint64_t getAlignment() = 0;
            virtual uint64_t getEntrySize() = 0;
            virtual uint64_t getFlags() = 0;
            virtual uint64_t getSize() = 0;

        }; // class SectionHeader

        class SectionHeader32 : public SectionHeader {
        public:
            SectionHeader32(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~SectionHeader32() {}

            uint64_t getNameIndex();
            uint64_t getType();
            uint64_t getVirtAddr();
            uint64_t getFileOffset();
            uint64_t getSectionLink();
            uint64_t getAlignment();
            uint64_t getEntrySize();
            uint64_t getFlags();
            uint64_t getSize();

        }; // class SectionHeader32

        class SectionHeader64 : public SectionHeader {
        public:
            SectionHeader64(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~SectionHeader64() {}

            uint64_t getNameIndex();
            uint64_t getType();
            uint64_t getVirtAddr();
            uint64_t getFileOffset();
            uint64_t getSectionLink();
            uint64_t getAlignment();
            uint64_t getEntrySize();
            uint64_t getFlags();
            uint64_t getSize();

        }; // class SectionHeader64

        class ProgramHeader : public FileBase, public MemoryBase, public IndexBase {
        protected:
            rawbyte_t* entry;

        public:
            ProgramHeader(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~ProgramHeader();

            bool isValidVaddr(uint64_t v);
            uint64_t vaddrToFileaddr(uint64_t v);

            virtual uint64_t getVaddr() = 0;
            virtual uint64_t getPaddr() = 0;
            virtual uint64_t getFSize() = 0;
            virtual uint64_t getMSize() = 0;
            virtual uint32_t getSegmentType() = 0;
            virtual uint64_t getFlags() = 0;
            virtual uint32_t getAlignment() = 0;
            virtual uint64_t getFOffset() = 0;
        }; // class ProgramHeader

        class ProgramHeader32 : public ProgramHeader {
        public:
            ProgramHeader32(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~ProgramHeader32() {}

            uint64_t getVaddr();
            uint64_t getPaddr();
            uint64_t getFSize();
            uint64_t getMSize();
            uint32_t getSegmentType();
            uint64_t getFlags();
            uint32_t getAlignment();
            uint64_t getFOffset();
        }; // class ProgramHeader32

        class ProgramHeader64 : public ProgramHeader {
        public:
            ProgramHeader64(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
            virtual ~ProgramHeader64() {}

            uint64_t getVaddr();
            uint64_t getPaddr();
            uint64_t getFSize();
            uint64_t getMSize();
            uint32_t getSegmentType();
            uint64_t getFlags();
            uint32_t getAlignment();
            uint64_t getFOffset();
        }; // class ProgramHeader32

    } // namespace Elf    

} // namespace EPAX

#endif // __EPAX_ElfBinary_hpp__

