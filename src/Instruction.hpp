/**
 * @file Instruction.hpp
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


#ifndef __EPAX_Instruction_hpp__
#define __EPAX_Instruction_hpp__

#include "BaseClass.hpp"

extern "C" {
#include "darm.h"
}

namespace EPAX {

    class BasicBlock;
    class Function;

    class Instruction : public MemoryBase, public IndexBase, public EPAXExport {
    private:
        darm_t info;
        int disasm_res;

        Function* function;
        BasicBlock* basicblock;

    public:
        Instruction(const uint64_t a, const rawbyte_t* r, Function* func, const darm_mode_t mode);
        virtual ~Instruction();

        darm_t* source() { return &info; }

        void setBasicBlock(BasicBlock* bb) { basicblock = bb; }
        BasicBlock* getBasicBlock() { return basicblock; }

        darm_cond_t getCondition();

        // control-related
        bool isBranch();
        bool isConditionalBranch();
        bool isUnconditionalBranch();
        bool hasFallthrough();
        bool touchesPC();
        bool isCall();
        uint64_t fallthroughTarget();
        uint64_t getBranchTarget();
        uint32_t getControlTargets(std::vector<uint64_t>& tgts);

        bool isFpop();
        bool isLoad();
        bool isStore();
        bool isMemop();

        uint32_t getSourceRegisterSizeInBits();
        uint32_t getSourceDatatypeSizeInBits();

        std::string stringRep();
        void print(std::ostream& stream = std::cout);
    }; // class Instruction

} // namespace EPAX

#endif // __EPAX_Instruction_hpp__

