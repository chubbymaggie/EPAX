/**
 * @file Symbol.hpp
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


#ifndef __EPAX_Symbol_hpp__
#define __EPAX_Symbol_hpp__

#include "Binary.hpp"
#include "BaseClass.hpp"
#include "Section.hpp"

namespace EPAX {

    class Symbol : public MemoryBase, public FileBase, public IndexBase, public NameBase, public EPAXExport {
    public:
        Symbol(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i);
        virtual ~Symbol() {}

        virtual bool isFunction() = 0;
        virtual bool isThumbFunction() = 0;
    }; // class Symbol

    class SymbolTable : public Section {
    protected:
        std::vector<Symbol*>* symbols;

    public:
        SymbolTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n);
        virtual ~SymbolTable();

        uint32_t countSymbols() { return symbols->size(); }
        Symbol* getSymbol(uint32_t i);

        virtual void print(std::ostream& stream = std::cout) = 0;

        bool isSymbol() { return true; }
    }; // class SymbolTable

    class StringTable : public Section {
    public:
        StringTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n);
        virtual ~StringTable() {}

        virtual char* getStringAt(uint32_t i) = 0;

        bool isString() { return true; }
    }; // class StringTable

} // namespace EPAX

#endif // __EPAX_Symbol_hpp__

