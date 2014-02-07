/**
 * @file Loop.hpp
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

#ifndef __EPAX_Loop_hpp__
#define __EPAX_Loop_hpp__

#include "DataStruct.hpp"
#include "BaseClass.hpp"

namespace EPAX {

    class BasicBlock;
    class ControlFlow;
    class Instruction;

    class Loop : public IndexBase, public EPAXExport {
    private:
        uint32_t headidx;
        uint32_t tailidx;
        dyn_bitset* members;

        uint32_t depth;
        ControlFlow* cfg;

    public:
        Loop(ControlFlow* c, uint32_t h, uint32_t t, uint32_t d, dyn_bitset* m, uint32_t i);
        virtual ~Loop();

        ControlFlow* getControlFlow() { return cfg; }
        uint32_t countBasicBlocks();
        uint32_t countInstructions();
        uint32_t getSize();

        BasicBlock* findBasicBlock(uint64_t addr);

        bool hasBasicBlock(uint32_t idx);
        BasicBlock* getBasicBlock(uint32_t idx);
        BasicBlock* getNextBasicBlock(uint32_t idx);
        bool isLastBasicBlock(uint32_t idx);

        BasicBlock* head();
        BasicBlock* tail();

        Instruction* findInstruction(uint64_t addr);
        Instruction* getInstruction(uint32_t idx);
        bool isLastInstruction(uint32_t idx);

        void setDepth(uint32_t d);
        uint32_t getDepth();

        bool isChildOf(Loop* lp);

    }; // class Loop

} // namespace EPAX

#endif // __EPAX_Loop_hpp__

