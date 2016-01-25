/**
 * @file BasicBlock.cpp
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

#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "Function.hpp"

namespace EPAX {
 
    BasicBlock::BasicBlock(Function* f, uint64_t a, uint32_t i)
        : MemoryBase(a, 0),
          IndexBase(i),
          EPAXExport(EPAXExportClass_BBL),
          function(f),
          loop(INVALID_PTR),
          reachable(true)
    {
    }

    BasicBlock::~BasicBlock(){
        for (std::vector<Instruction*>::const_iterator it = instructions.begin(); it != instructions.end(); it++){
            Instruction* insn = (*it);
            delete insn;
        }
    }

    void BasicBlock::addSource(BasicBlock* bb){
        sources.push_back(bb);
    }

    void BasicBlock::addTarget(BasicBlock* bb){
        targets.push_back(bb);
    }

    BasicBlock* BasicBlock::getSource(uint32_t idx){
        if (idx < sources.size()){
            return sources[idx];
        }
        return INVALID_PTR;
    }

    BasicBlock* BasicBlock::getTarget(uint32_t idx){
        if (idx < targets.size()){
            return targets[idx];
        }
        return INVALID_PTR;
    }

    Instruction* BasicBlock::head(){
        if (instructions.size() == 0){
            return INVALID_PTR;
        }
        return instructions.front();
    }

    Instruction* BasicBlock::tail(){
        if (instructions.size() == 0){
            return INVALID_PTR;
        }
        return instructions.back();
    }

    Instruction* BasicBlock::findInstruction(uint64_t addr){
        if (instructions.size() == 0 || !inRange(addr)){
            return INVALID_PTR;
        }
        // TODO: binary search?
        for (std::vector<Instruction*>::const_iterator it = instructions.begin(); it != instructions.end(); it++){
            Instruction* insn = (*it);
            if (insn->inRange(addr)){
                return insn;
            }
        }
        return INVALID_PTR;
    }
    
    void BasicBlock::print(std::ostream& stream){
        uint64_t addr = 0;
        if (instructions.size() > 0){
            addr = instructions.front()->getMemoryAddress();
        }

        stream << "";
        if (!isReachable()){
            stream << "unreachable ";
        }
        stream << "BasicBlock " << DEC(getIndex()) << " at " << HEX(addr) << TAB;
        stream << "{ ";
        for (std::vector<BasicBlock*>::const_iterator it = sources.begin(); it != sources.end(); it++){
            BasicBlock* bb = (*it);
            if (it != sources.begin()){
                stream << ", ";
            }
            stream << DEC(bb->getIndex());
        }
        stream << " } -> SELF -> { ";
        for (std::vector<BasicBlock*>::const_iterator it = targets.begin(); it != targets.end(); it++){
            BasicBlock* bb = (*it);
           if (it != targets.begin()){
                stream << ", ";
            }
            stream << DEC(bb->getIndex());
        }
        stream << " }" << ENDL;
        /*
        for (std::vector<Instruction*>::const_iterator it = instructions.begin(); it != instructions.end(); it++){
            Instruction* insn = (*it);
            insn->print(stream);
        }
        */
    }

    uint32_t BasicBlock::countInstructions(){
        return instructions.size();
    }

    void BasicBlock::addInstruction(Instruction* i){
        instructions.push_back(i);
        setMemorySize(getMemorySize() + i->getMemorySize());
    }

    Instruction* BasicBlock::getInstruction(uint32_t idx){
        if (idx < instructions.size()){
            return instructions[idx];
        }
        return INVALID_PTR;
    }

    ControlFlow* BasicBlock::getControlFlow(){
        if (IS_VALID_PTR(function)){
            return function->getControlFlow();
        }
        return INVALID_PTR;
    }

    bool BasicBlock::isFallThrough(){
        Instruction* t = tail();
        if (inRange(t->fallthroughTarget())){
            return true;
        }
        return false;
    }

} // namespace EPAX
