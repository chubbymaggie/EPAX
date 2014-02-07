/**
 * @file ControlFlow.hpp
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

#ifndef __EPAX_ControlFlow_hpp__
#define __EPAX_ControlFlow_hpp__

#include "BaseClass.hpp"

namespace EPAX {

    class BasicBlock;
    class Function;
    class Instruction;
    class Loop;

    class ControlFlow : public EPAXExport {
    private:
        Function* function;
        std::vector<BasicBlock*> basicblocks;
        std::vector<Loop*> loops;

        // all instructions in the cfg. their IndexBase index is kept per-block
        std::vector<Instruction*> instructions;

        void initialize(std::vector<BasicBlock*> bbs);

    public:
        ControlFlow(Function* f, std::vector<BasicBlock*> bbs);
        virtual ~ControlFlow();

        Function* getFunction() { return function; }

        void print(std::ostream& stream = std::cout);
        void dot_print(std::ostream& stream = std::cout);

        uint32_t countBasicBlocks();
        BasicBlock* findBasicBlock(uint64_t addr);
        BasicBlock* getBasicBlock(uint32_t idx);

        uint32_t countInstructions();
        Instruction* findInstruction(uint64_t addr);
        Instruction* getInstruction(uint32_t idx);

        uint32_t countLoops();
        Loop* findLoop(uint64_t addr);
        Loop* getLoop(uint32_t idx);
        Loop* getParentOf(Loop* loop);

    }; // class ControlFlow

} // namespace EPAX

#endif // __EPAX_ControlFlow_hpp__

