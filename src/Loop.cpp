/**
 * @file Loop.cpp
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
#include "ControlFlow.hpp"
#include "Function.hpp"
#include "Loop.hpp"

namespace EPAX {

    Loop::Loop(ControlFlow* c, uint32_t h, uint32_t t, uint32_t d, dyn_bitset* m, uint32_t i)
        : IndexBase(i),
          EPAXExport(EPAXExportClass_LOOP),
          cfg(c),headidx(h),tailidx(t),depth(d),members(m)
    {
        EPAXAssert(members->has(tailidx), "tail must be part of loop");
        EPAXAssert(members->has(headidx), "head must be part of loop");

        for (uint32_t i = 0; i < members->size(); i++){
            if (members->has(i)){
                BasicBlock* bb = cfg->getBasicBlock(i);
                bb->setLoop(this);
            }
        }
    }

    Loop::~Loop(){
        if (IS_VALID_PTR(members)){
            delete members;
        }
    }

    BasicBlock* Loop::head(){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        if (!members->has(headidx)){
            EPAXOut << "bad head" << ENDL;
            return INVALID_PTR;
        }

        return cfg->getBasicBlock(headidx);
    }

    BasicBlock* Loop::tail(){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        if (!members->has(tailidx)){
            EPAXOut << "bad tail" << ENDL;
            return INVALID_PTR;
        }

        return cfg->getBasicBlock(tailidx);
    }

    Instruction* Loop::findInstruction(uint64_t addr){
        BasicBlock* bb = findBasicBlock(addr);
        if (IS_VALID_PTR(bb)){
            return bb->findInstruction(addr);
        }
        return INVALID_PTR;
    }

    uint32_t Loop::countInstructions(){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        uint32_t icnt = 0;
        for (uint32_t i = 0; i < members->size(); i++){
            if (members->has(i)){
                icnt += cfg->getBasicBlock(i)->countInstructions();
            }
        }
        return icnt;
    }

    uint32_t Loop::countBasicBlocks(){
        uint32_t bbcnt = 0;
        for (uint32_t i = 0; i < members->size(); i++){
            if (members->has(i)){
                bbcnt++;
            }
        }
        return bbcnt;
    }

    uint32_t Loop::getSize(){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        uint32_t lpsize = 0;
        for (uint32_t i = 0; i < members->size(); i++){
            if (members->has(i)){
                lpsize += cfg->getBasicBlock(i)->getMemorySize();
            }
        }
        return lpsize;
    }

    BasicBlock* Loop::findBasicBlock(uint64_t addr){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        BasicBlock* bb = cfg->findBasicBlock(addr);
        if (!IS_VALID_PTR(bb)){
            return INVALID_PTR;
        }

        if (members->has(bb->getIndex())){
            return bb;
        }

        return INVALID_PTR;
    }

    bool Loop::hasBasicBlock(uint32_t idx){
        return members->has(idx);
    }

    BasicBlock* Loop::getBasicBlock(uint32_t idx){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        if (!members->has(idx)){
            return INVALID_PTR;
        }

        return cfg->getBasicBlock(idx);
    }
    
    BasicBlock* Loop::getNextBasicBlock(uint32_t idx){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        if (!members->has(idx)){
            return INVALID_PTR;
        }

        uint32_t n = cfg->countBasicBlocks();
        for (uint32_t i = idx + 1; i < n; i++){
            if (members->has(i)){
                return cfg->getBasicBlock(i);
            }
        }
        return INVALID_PTR;
    }

    bool Loop::isLastBasicBlock(uint32_t idx){
        EPAXAssert(IS_VALID_PTR(cfg), "Loop should be connected to a valid CFG");

        if (!members->has(idx)){
            return false;
        }

        uint32_t n = cfg->countBasicBlocks();
        for (uint32_t i = idx + 1; i < n; i++){
            if (members->has(i)){
                return false;
            }
        }

        return true;
    }

    bool Loop::isChildOf(Loop* lp){
        if (this->getControlFlow()->getFunction()->getIndex() != lp->getControlFlow()->getFunction()->getIndex()){
            return false;
        }
        EPAXAssert(lp->getControlFlow()->countBasicBlocks() == this->getControlFlow()->countBasicBlocks(), "block counts for these loops should be identical: " << lp->countBasicBlocks() << TAB << this->countBasicBlocks());

        for (uint32_t i = 0; i < members->size(); i++){
            if (members->has(i) && !lp->hasBasicBlock(i)){
                return false;
            }
        }

        return true;
    }

    void Loop::setDepth(uint32_t d){
        depth = d;
    }

    uint32_t Loop::getDepth(){
        return depth;
    }

} // namespace EPAX
