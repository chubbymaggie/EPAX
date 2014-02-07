/**
 * @file Function.hpp
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


#ifndef __EPAX_Function_hpp__
#define __EPAX_Function_hpp__

#include "BaseClass.hpp"

extern "C" {
#include "darm.h"
}

namespace EPAX {

    class BaseBinary;
    class BasicBlock;
    class ControlFlow;
    class Instruction;
    class Symbol;

    class DetachedText : public FileBase, public MemoryBase, public IndexBase {
    protected:
        std::vector<void*> instructions; // TODO: or blocks or something?

    public:
        DetachedText(BaseBinary* b, uint64_t o, uint64_t s, uint64_t a, uint32_t i);
        virtual ~DetachedText() {}

        virtual void print(std::ostream& stream = std::cout) {}

    }; // class DetachedText

    class Function : public DetachedText, public SymbolBase, public EPAXExport {
    private:
        ControlFlow* controlflow;
        void disasm(std::vector<BasicBlock*>& bbs);
        darm_mode_t disassembleMode();

    public:
        Function(BaseBinary* b, uint64_t o, uint64_t s, uint64_t a, uint32_t i, Symbol* y);
        virtual ~Function();

        void print(std::ostream& stream = std::cout);
        static void printHeader(std::ostream& stream = std::cout);

        ControlFlow* getControlFlow() { return controlflow; }

        uint32_t countBasicBlocks();
        BasicBlock* findBasicBlock(uint64_t addr);
        BasicBlock* getBasicBlock(uint32_t idx);

        uint32_t countInstructions();
        Instruction* findInstruction(uint64_t addr);
        Instruction* getInstruction(uint32_t idx);

        void disassemble();
    }; // class Function

} // namespace EPAX

#endif // __EPAX_Function_hpp__

