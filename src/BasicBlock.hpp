/**
 * @file BasicBlock.hpp
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


#ifndef __EPAX_BasicBlock_hpp__
#define __EPAX_BasicBlock_hpp__

#include "BaseClass.hpp"

namespace EPAX {

    class ControlFlow;
    class Function;
    class Instruction;
    class Loop;

    class BasicBlock : public MemoryBase, public IndexBase, public EPAXExport {
    private:
        std::vector<Instruction*> instructions;

        std::vector<BasicBlock*> sources;
        std::vector<BasicBlock*> targets;

        Function* function;
        Loop* loop;

        bool reachable;

    public:
        BasicBlock(Function* f, uint64_t a, uint32_t i);
        virtual ~BasicBlock();

        uint32_t countInstructions();
        void addInstruction(Instruction* insn);
        Instruction* getInstruction(uint32_t idx);

        Instruction* head();
        Instruction* tail();
        Instruction* findInstruction(uint64_t addr);

        void addSource(BasicBlock* bb);
        void addTarget(BasicBlock* bb);
        uint32_t countSources() { return sources.size(); }
        uint32_t countTargets() { return targets.size(); }
        BasicBlock* getSource(uint32_t idx);
        BasicBlock* getTarget(uint32_t idx);

        bool isReachable() { return reachable; }
        void setUnreachable() { reachable = false; }

        Function* getFunction() { return function; }

        Loop* getLoop() { return loop; }
        void setLoop(Loop* l) { loop = l; }

        ControlFlow* getControlFlow();

        bool isFallThrough();

        void print(std::ostream& stream = std::cout);

    }; // class BasicBlock

} // namespace EPAX

#endif // __EPAX_BasicBlock_hpp__

