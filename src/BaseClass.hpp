/**
 * @file BaseClass.hpp
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


#ifndef __EPAX_BaseClass_hpp__
#define __EPAX_BaseClass_hpp__

#include "Binary.hpp"

namespace EPAX {

    class BaseBinary;
    class Function;
    class InputFile;
    class Section;
    class StringTable;
    class Symbol;
    class SymbolTable;


    class FileBase {
    private:
        BaseBinary* binary;
        uint64_t offset;
        uint64_t size;

    public:
        FileBase(BaseBinary* b, uint64_t o, uint64_t s)
            : binary(b), offset(o), size(s) {}
        virtual ~FileBase() {}

        BaseBinary* getBinary() { return binary; }
        InputFile* getInputFile();
        bool is32Bit();
        uint64_t  getFileOffset() { return offset; }
        uint64_t getFileSize() { return size; }
        void setFileSize(uint64_t s) { size = s; }
    }; // class FileBase
    
    class MemoryBase {
    private:
        uint64_t addr;
        uint64_t size;

    public:
        MemoryBase(uint64_t a, uint64_t s)
            : addr(a), size(s) {}
        MemoryBase()
            : addr(0), size(0) {}
        virtual ~MemoryBase() {}

        uint64_t getMemoryAddress() { return addr; }
        uint64_t getMemorySize() { return size; }
        void setMemorySize(uint64_t s) { size = s; }

        bool inRange(uint64_t a);
    }; // class MemoryBase

    class SymbolBase { 
    private:
        Symbol* sym;
    public:
        SymbolBase(Symbol* s)
            : sym(s) {}
        virtual ~SymbolBase() {}

        Symbol* getSymbol() { return sym; }
        void setSymbol(Symbol* s) { sym = s; }

        std::string getName();
    }; // class SymbolBase

    class IndexBase {
    private:
        uint32_t index;
        
    public:
        IndexBase(uint32_t i)
            : index(i) {}
        virtual ~IndexBase() {}

        uint32_t getIndex() { return index; }
        void setIndex(uint32_t i) { index = i; }
    }; // class IndexBase

    class NameBase {
    private:
        std::string name;

    public:
        NameBase(std::string n) { setName(n); }
        NameBase() {}
        virtual ~NameBase() {}

        std::string getName() { return name; }
        void setName(std::string n) { name.clear(); name.append(n); }
    }; // class NameBase

    /**
     * Binary format names
     */
    static const char* BinaryFormatName[BinaryFormat_total] = {
        "undefined",
        "Elf32",
        "Elf64",
        "MachO32",
        "MachO64",
        "PE"
    };

    class BaseBinary : public NameBase {
    protected:

        /**
         * The image file
         */
        InputFile* inputfile;

        /**
         * Finds and internally stores all functions in the image
         *
         * @return none
         */
        virtual void findFunctions() = 0;
        void lazyFunctions();
        bool foundfunctions;
        std::vector<Function*>* functions;

        /**
         * Finds and internally stores all symbols in the image
         *
         * @return none
         */
        virtual void findSymbols() = 0;
        void lazySymbols();
        bool foundsymbols;
        std::vector<SymbolTable*>* symtabs;
        std::vector<StringTable*>* strtabs;

    public:
        BaseBinary(std::string n);
        virtual ~BaseBinary();

        static const char* getFormatName(BinaryFormat f);
        virtual BinaryFormat getFormat() = 0;
        virtual uint64_t getStartAddr() = 0;
        virtual void emit(std::string n) = 0;
        virtual bool verify() = 0;
        virtual bool isARM() = 0;
        virtual void describe() = 0;

        virtual bool is32Bit() = 0;
        virtual bool is64Bit() = 0;
        virtual bool isExecutable() = 0;

        virtual uint64_t functionEndAddress(Function* f, Function* nextf) = 0;

        uint32_t countFunctions();
        Function* getFirstFunction();
        Function* getNextFunction(Function* f);
        bool isLastFunction(Function* f);

        Function* findFunction(uint64_t addr);

        InputFile* getInputFile() { return inputfile; }
        
        virtual uint64_t getFileSize();

        virtual uint32_t getID() { return 0; } // TODO: should be some kind of unique id
        virtual bool insideTextRange(uint64_t a) = 0;

        virtual void printSections(std::ostream& stream = std::cout) = 0;
        virtual void printFunctions(std::ostream& stream = std::cout) = 0;

    }; // class BaseBinary

    extern bool compareMemory(MemoryBase* m1, MemoryBase* m2);

} // namespace EPAX

#endif // __EPAX_BaseClass_hpp__

