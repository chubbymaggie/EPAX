/**
 * @file ControlFlow.cpp
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
#include "Instruction.hpp"
#include "Loop.hpp"

namespace EPAX {
 
    ControlFlow::ControlFlow(Function* f, std::vector<BasicBlock*> bbs)
        : EPAXExport(EPAXExportClass_CFG),
          function(f)
    {
        initialize(bbs);
    }

    ControlFlow::~ControlFlow(){ 
        for (std::vector<BasicBlock*>::const_iterator it = basicblocks.begin(); it != basicblocks.end(); it++){
            BasicBlock* bb = (*it);
            if (IS_VALID_PTR(bb)){
                delete bb;
            }
        }

        for (std::vector<Loop*>::const_iterator it = loops.begin(); it != loops.end(); it++){
            Loop* lp = (*it);
            if (IS_VALID_PTR(lp)){
                delete lp;
            }
        }
    }

    void DFS(std::vector<BasicBlock*>& backedg, BasicBlock* start, dyn_bitset& v, dyn_bitset& c){

        v.set(start->getIndex());
        const uint32_t size = start->countTargets();
        for (uint32_t i = 0; i < size; i++){
            BasicBlock* tgt = start->getTarget(i);

            if (!v.has(tgt->getIndex())){
                DFS(backedg, tgt, v, c);
            } else if (!c.has(tgt->getIndex())){
                // each pair is tail -> head
                backedg.push_back(tgt);
                backedg.push_back(start);
            }
        }

        c.set(start->getIndex());
    }

    // uses the classic worklist algorithm
    void findDominators(std::vector<dyn_bitset*>& dominators, BasicBlock* start, std::vector<BasicBlock*>& bbs){
        uint32_t l = bbs.size();

        for (uint32_t i = 0; i < l; i++){
            dominators[i]->set();
        }

        std::set<BasicBlock*> worklist;
        worklist.insert(start);
        dyn_bitset touched(l);
        touched.clear();

        while (!worklist.empty()){
            BasicBlock* bb = *(worklist.begin());            
            worklist.erase(worklist.begin());

            touched.set(bb->getIndex());
            dyn_bitset n = dyn_bitset(l);
            n.clear();

            if (bb->countSources() > 0){
                n = *(dominators[bb->getSource(0)->getIndex()]);
                for (uint32_t i = 1; i < bb->countSources(); i++){
                    BasicBlock* source = bb->getSource(i);
                    n &= *(dominators[source->getIndex()]);
                }
            }
            n.set(bb->getIndex());

            if (n != *(dominators[bb->getIndex()])){
                *(dominators[bb->getIndex()]) = n;
                for (uint32_t i = 0; i < bb->countTargets(); i++){
                    worklist.insert(bb->getTarget(i));
                }
            }

            
        }

        for (uint32_t i = 0; i < l; i++){
            if (!touched.has(i)){
                dominators[i]->clear();
            }
        }

        /*
        for (uint32_t i = 0; i < l; i++){
            EPAXOut << "Dominator list for block " << DEC(i) << ":";
            dominators[i]->print();
        }
        */
    }

    void findBackEdges(std::vector<BasicBlock*>& backedg, BasicBlock* start, std::vector<BasicBlock*> bbs){
        // first depth first search to find back edges with Lengauer Tarjan algorithm
        dyn_bitset v = dyn_bitset(bbs.size());
        dyn_bitset c = dyn_bitset(bbs.size());
        v.clear();
        c.clear();

        DFS(backedg, start, v, c);
        EPAXAssert(backedg.size() % 2 == 0, "Expecting back edges to come in head/tail pairs");
    }
    
    void ControlFlow::initialize(std::vector<BasicBlock*> bbs){
        if (bbs.size() < 1){
            return;
        }

        uint32_t bcount = bbs.size();
        std::map<uint64_t, uint32_t> bb_map;
        uint32_t i = 0;
        for (uint32_t i = 0; i < bcount; i++){
            BasicBlock* bb = bbs[i];

            bb_map[bb->head()->getMemoryAddress()] = i;
            basicblocks.push_back(bb);

            for (uint32_t j = 0; j < bb->countInstructions(); j++){
                Instruction* insn = bb->getInstruction(j);
                insn->setIndex(j);
                instructions.push_back(insn);
            }
        }

        for (std::vector<BasicBlock*>::const_iterator it = bbs.begin(); it != bbs.end(); it++){
            BasicBlock* bb = (*it);

            std::vector<uint64_t> tgts;
            bb->tail()->getControlTargets(tgts);

            for (std::vector<uint64_t>::const_iterator iit = tgts.begin(); iit != tgts.end(); iit++){
                uint64_t tgt = (*iit);

                if (function->inRange(tgt)){
                    // could make this assertion if we also knew the tgt was reachable
                    //EPAXAssert(bb_map.count(tgt) > 0, "Target address " << HEX(tgt) << " from insn at " << HEX(bb->tail()->getMemoryAddress()) << " should be present in map");
                    if (bb_map.count(tgt) > 0){
                        BasicBlock* targetBlock = basicblocks[bb_map[tgt]];

                        bb->addTarget(targetBlock);
                        targetBlock->addSource(bb);
                    }
                }
            }
        }

        // dominators[i][j] == true iff BB_i is dominated by BB_j
        std::vector<dyn_bitset*> dominators;
        for (int32_t i = 0; i < bbs.size(); i++){
            dominators.push_back(new dyn_bitset(bbs.size()));
        }
        
        BasicBlock* start = bbs[0];
        findDominators(dominators, start, bbs);

        for (std::vector<BasicBlock*>::const_iterator it = bbs.begin(); it != bbs.end(); it++){
            BasicBlock* bb = (*it);
            if (!dominators[bb->getIndex()]->has(start->getIndex())){
                bb->setUnreachable();
            }
        }

        std::vector<BasicBlock*> backedg;
        findBackEdges(backedg, bbs[0], bbs);

        uint32_t loopidx = 0;
        while (backedg.size()){
            BasicBlock* tail = backedg.back(); 
            backedg.pop_back();
            BasicBlock* head = backedg.back(); 
            backedg.pop_back();

            if (dominators[tail->getIndex()]->has(head->getIndex())){
                dyn_bitset* members = new dyn_bitset(bbs.size());
                members->clear();
                members->set(head->getIndex());
                members->set(tail->getIndex());

                std::stack<BasicBlock*> check;

                // starting at tail, follow all sources until a member block is reached
                check.push(tail);
                while (!check.empty()){
                    BasicBlock* cur = check.top();
                    check.pop();

                    if (cur->getIndex() == head->getIndex()){
                        continue;
                    }

                    for (uint32_t i = 0; i < cur->countSources(); i++){
                        BasicBlock* other = cur->getSource(i);
                        if (!members->has(other->getIndex())){
                            check.push(other);
                            members->set(other->getIndex());
                        }
                    }
                }

                // TODO: get correct depth
                // TODO: add exit nodes
                loops.push_back(new Loop(this, head->getIndex(), tail->getIndex(), 0, members, loopidx++));
            }
        }

        for (uint32_t i = 0; i < loops.size(); i++){
            uint32_t d = 1;
            for (uint32_t j = 0; j < loops.size(); j++){
                if (i == j) continue;
                if (loops[i]->isChildOf(loops[j])){
                    d++;
                }
            }
            loops[i]->setDepth(d);
        }

        for (std::vector<dyn_bitset*>::const_iterator it = dominators.begin(); it != dominators.end(); it++){
            dyn_bitset* d = (*it);
            delete d;
        }
    }

    void ControlFlow::print(std::ostream& stream){
        for (std::vector<BasicBlock*>::const_iterator it = basicblocks.begin(); it != basicblocks.end(); it++){
            BasicBlock* bb = (*it);
            bb->print(stream);
        }
    }

    // http://graphviz-dev.appspot.com/
    void ControlFlow::dot_print(std::ostream& stream){
        stream << "digraph " << (IS_VALID_PTR(function)? function->getName(): NAME_UNKNOWN) << " {" << ENDL;
        // TODO: add edge labels, block ranges
        for (std::vector<BasicBlock*>::const_iterator it = basicblocks.begin(); it != basicblocks.end(); it++){
            BasicBlock* bb = (*it);
            for (uint32_t i = 0; i < bb->countTargets(); i++){
                stream << TAB << "\"" << DEC(bb->getIndex()) << "@" <<HEX(bb->getMemoryAddress()) << "\" -> \"" << DEC(bb->getTarget(i)->getIndex()) << "@" << HEX(bb->getTarget(i)->getMemoryAddress()) << "\";" << ENDL;
            }

            if (bb->countSources() == 0 && bb->countTargets() == 0){
                stream << TAB << "\"" << DEC(bb->getIndex()) << "@" << HEX(bb->getMemoryAddress()) << "\";" << ENDL;                
            }
        }
        stream << "}" << ENDL;
    }

    Loop* ControlFlow::getParentOf(Loop* loop){
        for (uint32_t i = 0; i < loops.size(); i++){
            if (loop->isChildOf(loops[i]) && loop->getDepth() == loops[i]->getDepth() + 1){
                return loops[i];
            }
        }
        return INVALID_PTR;
    }

    uint32_t ControlFlow::countBasicBlocks(){
        return basicblocks.size();
    }

    BasicBlock* ControlFlow::findBasicBlock(uint64_t addr){
        // TODO: should binary search
        for (std::vector<BasicBlock*>::const_iterator it = basicblocks.begin(); it != basicblocks.end(); it++){
            BasicBlock* bb = (*it);
            if (bb->inRange(addr)){
                return bb;
            }
        }
        return INVALID_PTR;
    }

    BasicBlock* ControlFlow::getBasicBlock(uint32_t idx){
        if (idx < basicblocks.size()){
            return basicblocks[idx];
        }
        return INVALID_PTR;
    }

    uint32_t ControlFlow::countInstructions(){
        return instructions.size();
    }

    Instruction* ControlFlow::findInstruction(uint64_t addr){
        // TODO: should binary search
        for (std::vector<Instruction*>::const_iterator it = instructions.begin(); it != instructions.end(); it++){
            Instruction* insn = (*it);
            if (insn->inRange(addr)){
                return insn;
            }
        }
        return INVALID_PTR;
    }

    Instruction* ControlFlow::getInstruction(uint32_t idx){
        if (idx < instructions.size()){
            return instructions[idx];
        }
        return INVALID_PTR;
    }

    uint32_t ControlFlow::countLoops(){
        return loops.size();
    }

    Loop* ControlFlow::findLoop(uint64_t addr){
        BasicBlock* bb = findBasicBlock(addr);
        if (!IS_VALID_PTR(bb)){
            return INVALID_PTR;
        }

        uint32_t bidx = bb->getIndex();
        uint32_t maxd = 0;
        Loop* lp = INVALID_PTR;
        for (uint32_t i = 0; i < loops.size(); i++){
            if (loops[i]->hasBasicBlock(bidx)){
                if (0 == maxd || loops[i]->getDepth() > maxd){
                    lp = loops[i];
                    maxd = lp->getDepth();
                }
            }
        }
        return lp;
    }

    Loop* ControlFlow::getLoop(uint32_t idx){
        if (idx < loops.size()){
            return loops[idx];
        }
        return INVALID_PTR;
    }

} // namespace EPAX
