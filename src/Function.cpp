/**
 * @file Function.cpp
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
#include "InputFile.hpp"
#include "Instruction.hpp"
#include "Symbol.hpp"

namespace EPAX {

    DetachedText::DetachedText(BaseBinary* b, uint64_t o, uint64_t s, uint64_t a, uint32_t i)
        : FileBase(b, o, s),
          MemoryBase(a, s),
          IndexBase(i)
    {
    }

    darm_mode_t Function::disassembleMode(){
        Symbol* sym = getSymbol();
        if (IS_VALID_PTR(sym) && sym->isThumbFunction()){
            return M_THUMB2;
        }
        return M_ARM;
    }

    void Function::disasm(std::vector<BasicBlock*>& bbs){
        if (getFileOffset() == 0){
            return;
        }

        uint32_t limit = getMemorySize();
        rawbyte_t* buf = new rawbyte_t[limit];
        getInputFile()->getBytes(getFileOffset(), limit, buf);

        // disassemble instructions
        std::vector<Instruction*> insns;
        std::map<uint64_t, uint32_t> insn_map;

        // TODO: mode changes inside functions (BLX, BXJ, etc)
        darm_mode_t mode = disassembleMode();
        for (uint32_t cur = 0; cur < limit; ){
            uint64_t addr = getMemoryAddress() + cur;

            // machinations to make sure we don't go out of bounds on buf
            uint32_t len = 4;
            if (M_ARM == mode){
                len = 4;
            } else if (M_THUMB == mode){
                len = 2;
            } else if (M_THUMB2 == mode){
                if (IS_THUMB2_32BIT(buf[cur] | buf[cur+1])){
                    len = 4;
                } else {
                    len = 2;
                }
            }

            if (M_THUMB == mode){
                len = 2;
            } else if (M_THUMB2 == mode){
                if (cur + 2 > limit) break;
                if (!IS_THUMB2_32BIT((buf[cur] << 8) | (buf[cur+1] << 0))){
                    len = 2;
                }
            }

            if (cur + len > limit) break;

            Instruction* insn = new Instruction(addr, &(buf[cur]), this, mode);
            cur += insn->getMemorySize();

            insn_map[addr] = insns.size();
            insns.push_back(insn);
        }

        delete[] buf;

        // no instructions found
        if (insns.size() == 0){
            return;
        }

        // find block leaders
        std::vector<bool> leaders;
        leaders.push_back(true);
        for (uint32_t i = 1; i < insns.size(); i++){
            leaders.push_back(false);
        }
        for (std::vector<Instruction*>::const_iterator it = insns.begin(); it != insns.end(); it++){
            Instruction* insn = (*it);

            if (insn->isBranch()){

                // the instruction after a branch is a leader
                uint64_t ft = insn->getMemoryAddress() + insn->getMemorySize();

                if (inRange(ft)){
                    leaders[insn_map[ft]] = true;
                }

                // branch target is a leader
                uint64_t tgt = insn->getBranchTarget();
                if (inRange(tgt)){
                    leaders[insn_map[tgt]] = true;
                }
            }
        }

        // partition instructions into basic blocks
        BasicBlock* bb = new BasicBlock(this, insns.front()->getMemoryAddress(), 0);
        EPAXAssert(leaders.front() == true, "First instruction in function should head a BB");
        bb->addInstruction(insns.front());
        insns.front()->setBasicBlock(bb);
        for (uint32_t i = 1; i < insns.size(); i++){
            if (leaders[i]){
                bbs.push_back(bb);
                bb = new BasicBlock(this, insns[i]->getMemoryAddress(), bbs.size());
            }
            bb->addInstruction(insns[i]);
            insns[i]->setBasicBlock(bb);
        }
        if (bb->countInstructions() == 0){
            delete bb;
        } else {
            bbs.push_back(bb);
        }
    }

    Function::Function(BaseBinary* b, uint64_t o, uint64_t s, uint64_t a, uint32_t i, Symbol* y)
        : DetachedText(b, o, s, a, i),
          SymbolBase(y),
          EPAXExport(EPAXExportClass_FUNC),
          controlflow(INVALID_PTR)
    {
        if (IS_VALID_PTR(getSymbol())){
            EPAXAssert(getSymbol()->isFunction(), "Functions have to be tied to function symbols");
        }

    }

    Function::~Function(){
        if (IS_VALID_PTR(controlflow)){
            delete controlflow;
        }
    }

    void Function::disassemble(){
        std::vector<BasicBlock*> bbs;
        disasm(bbs);
        controlflow = new ControlFlow(this, bbs);
    }

    void Function::printHeader(std::ostream& stream){
        stream << "FUNC [" << std::setw(2) << "ID" << "]"
               << TAB << "VIRTADDR"
               << TAB << "SYM_SIZE"
               << TAB << "NAME"
               << ENDL;
    }

    void Function::print(std::ostream& stream){
        stream << "FUNC [" << std::setw(2) << DEC(getIndex()) << "]"
               << TAB << HEX(getMemoryAddress())
               << TAB << HEX(getMemorySize()) << "|" << HEX(getFileSize())
               << TAB << getName() //std::setw(40) << getName().substr(0,40)
               << ENDL;

        if (IS_VALID_PTR(controlflow)){
            controlflow->print(stream);
        }
    }

    uint32_t Function::countBasicBlocks(){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->countBasicBlocks();
        }
        return 0;
    }

    BasicBlock* Function::findBasicBlock(uint64_t addr){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->findBasicBlock(addr);
        }

        return INVALID_PTR;
    }

    BasicBlock* Function::getBasicBlock(uint32_t idx){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->getBasicBlock(idx);
        }
        return INVALID_PTR;
    }

    uint32_t Function::countInstructions(){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->countInstructions();
        }
        return 0;
    }

    Instruction* Function::findInstruction(uint64_t addr){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->findInstruction(addr);
        }
        return INVALID_PTR;
    }

    Instruction* Function::getInstruction(uint32_t idx){
        if (IS_VALID_PTR(controlflow)){
            return controlflow->getInstruction(idx);
        }
        return INVALID_PTR;
    }

} // namespace EPAX
