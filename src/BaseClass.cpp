/**
 * @file BaseClass.cpp
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

#include "BaseClass.hpp"
#include "InputFile.hpp"
#include "Function.hpp"
#include "Symbol.hpp"
#include "Section.hpp"

namespace EPAX {

    bool compareMemory(MemoryBase* m1, MemoryBase* m2){
        return (m1->getMemoryAddress() < m2->getMemoryAddress());
    }

    std::string SymbolBase::getName(){
        if (IS_VALID_PTR(sym)){
            return sym->getName();
        }
        return NAME_UNKNOWN;
    }

    InputFile* FileBase::getInputFile(){
        return binary->getInputFile();
    }

    bool FileBase::is32Bit(){
        return binary->is32Bit();
    }

    bool MemoryBase::inRange(uint64_t a){
        return (a >= addr && a < addr + size);
    }

    BaseBinary::BaseBinary(std::string n)
        : NameBase(n),
          inputfile(INVALID_PTR),
          foundfunctions(false), functions(INVALID_PTR),
          foundsymbols(false), symtabs(INVALID_PTR), strtabs(INVALID_PTR)
    {
        inputfile = new InputFile(getName());
    }

    BaseBinary::~BaseBinary(){
        if (IS_VALID_PTR(inputfile)){
            delete inputfile;
        }

        if (IS_VALID_PTR(functions)){
            while (functions->size()){
                delete functions->back();
                functions->pop_back();
            }
            delete functions;
        }

        if (IS_VALID_PTR(symtabs)){
            while (symtabs->size()){
                delete symtabs->back();
                symtabs->pop_back();
            }
            delete symtabs;
        }

        if (IS_VALID_PTR(strtabs)){
            while (strtabs->size()){
                delete strtabs->back();
                strtabs->pop_back();
            }
            delete strtabs;
        }

    }

    const char* BaseBinary::getFormatName(BinaryFormat f){
        EPAXAssert(f >= BinaryFormat_undefined && f < BinaryFormat_total, "Invalid BinaryFormat (" << DEC(f) << ").");
        return BinaryFormatName[f];
    }

    void BaseBinary::lazyFunctions(){
        if (!foundfunctions){
            findFunctions();
        }
    }

    void BaseBinary::lazySymbols(){
        if (!foundsymbols){
            findSymbols();
        }
    }

    Function* BaseBinary::getFirstFunction(){
        lazyFunctions();

        if (functions->size() == 0){
            return INVALID_PTR;
        }

        return functions->front();
    }

    uint32_t BaseBinary::countFunctions(){
        lazyFunctions();
        return functions->size();
    }

    Function* BaseBinary::getNextFunction(Function* f){
        EPAXAssert(foundfunctions, "Call getFirstFunction() before calling this");
        EPAXAssert(f->getBinary()->getID() == getID(), "The Function passed in is not from this binary");

        return functions->at(f->getIndex() + 1);
    }


    bool BaseBinary::isLastFunction(Function* f){
        EPAXAssert(foundfunctions, "Call firstfunction() before calling this");
        EPAXAssert(IS_VALID_PTR(f), "Null pointer found");
        EPAXAssert(f->getBinary()->getID() == getID(), "The Function passed in is not from this binary");

        if (functions->size() == f->getIndex() + 1){
            return true;
        }
        return false;
    }

    uint64_t BaseBinary::getFileSize(){
        return inputfile->getFileSize();
    }

    Function* BaseBinary::findFunctionAt(uint64_t addr){
        lazyFunctions();

        // TODO: binary search... they are sorted
        for (std::vector<Function*>::const_iterator it = functions->begin(); it != functions->end(); it++){
            Function* func = (*it);
            if (IS_VALID_PTR(func) && (func->getMemoryAddress() == addr || func->inRange(addr))){
                return func;
            }
        }
        return INVALID_PTR;
    }

} // namespace EPAX
