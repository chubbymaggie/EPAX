/**
 * @file Interface.cpp
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
#include "Interface.hpp"
#include "Interface.h"

#include "BasicBlock.hpp"
#include "Binary.hpp"
#include "ControlFlow.hpp"
#include "Function.hpp"
#include "Instruction.hpp"
#include "Loop.hpp"
#include "Symbol.hpp"
#include "Section.hpp"

#include <iostream>
#include <fstream>

namespace EPAX {

    BIN BIN_create(std::string fileName){
        return new Binary(fileName, BinaryFormat_undefined);
    }

    std::string BIN_getName(BIN bin){
        EPAXVerifyType(BIN, bin);
        return bin->getName();
    }

    void BIN_destroy(BIN bin){
        EPAXVerifyType(BIN, bin);

        delete bin;
        bin = NULL;
    }

    // does not return
    void BIN_run(BIN bin, int argc, char* argv[]){
        EPAXVerifyType(BIN, bin);

        EPAXAssert(BIN_isExecutable(bin), "This function only operates on executable binaries");
	bin->runBasic(argc, argv);

        ShouldNotArrive;
    }

    FUNC BIN_firstFunc(BIN bin){
        EPAXVerifyType(BIN, bin);

        if (bin->countFunctions() == 0){
            return NULL;
        }
        return bin->getFirstFunction();
    }

    FUNC BIN_nextFunc(BIN bin, FUNC func){
        EPAXVerifyType(BIN, bin);
        EPAXVerifyType(FUNC, func);

        if (bin->isLastFunction(func)){
            return NULL;
        }
        return bin->getNextFunction(func);
    }

    bool BIN_isLastFunc(BIN bin, FUNC func){
        EPAXVerifyType(BIN, bin);
        EPAXVerifyType(FUNC, func);
        return bin->isLastFunction(func);
    }

    uint32_t BIN_countFunc(BIN bin){
        EPAXVerifyType(BIN, bin);
        return bin->countFunctions();
    }

    bool BIN_isExecutable(BIN bin){
        EPAXVerifyType(BIN, bin);
        return bin->isExecutable();
    }

    uint32_t BIN_fileSize(BIN bin){
        EPAXVerifyType(BIN, bin);
        return bin->getFileSize();
    }

    void BIN_printStaticFile(BIN bin, std::string fname){
        EPAXVerifyType(BIN, bin);

        EPAXOut << "Printing static file to " << fname << ENDL;

        std::filebuf fb;
        fb.open(fname.c_str(), std::ios::out);

        std::ostream staticfile(&fb);

        staticfile << "# appname        = " << BIN_getName(bin) << ENDL;
        staticfile << "# appsize        = " << BIN_fileSize(bin) << ENDL;

        uint32_t bbcount = 0, insncount = 0;
        for (FUNC f = BIN_firstFunc(bin); !BIN_isLastFunc(bin, f); f = BIN_nextFunc(bin, f)){
            bbcount += FUNC_countBbl(f);
            insncount += FUNC_countInsn(f);
        }

        // TODO: several fields to be filled in here
        staticfile << "# blocks         = " << DEC(bbcount) << ENDL;
        //staticfile << "# memops         = " << DEC(memcount) << ENDL;
        //staticfile << "# memopbyte      = " << DEC(membyte) << ENDL;
        staticfile << "# fpops          = " << "???" << ENDL;
        staticfile << "# insns          = " << DEC(insncount) << ENDL;
        //staticfile << "# sha1sum        = " << sha1str << ENDL;
        staticfile << "# <sequence> <vaddr> <funcname> <funcid> <bbid> <line>" << ENDL;
        staticfile << "# +str <mnemonic> [<and> <ops>]" << ENDL;
        staticfile << "# +isa <groups> <bytes>" << ENDL;
        staticfile << "# +prd <pred_condition>" << ENDL;
        staticfile << "# +flw <list> <of> <known> <control> <targets>" << ENDL;
        staticfile << "# +lpi <loopcnt> <loopid> <ldepth> <loop_head_addr> <loop_tail_addr>" << ENDL;
        staticfile << "# +lpc <parent_loop_head> <parent_loop_tail>" << ENDL;
        staticfile << "# +cnt <branch_op> <fp_op> <load_op> <store_op>" << ENDL;
        staticfile << "# +srg <bits>x<elements>:<fp>:<int>" << ENDL;
        //staticfile << "# +mem <total_mem_op> <total_mem_bytes> <bytes/op>" << ENDL;
        //staticfile << "# +dud <dudist1>:<duint1>:<dufp1> <dudist2>:<ducnt2>:<dufp2>..." << ENDL;
        //staticfile << "# +dxi <count_def_use_cross> <count_call>" << ENDL;
        // staticfile << "# +blk " << ENDL; // TODO: block-specific flags (e.g, reachability)
        staticfile << "# +ipa <call_target_addr> <call_target_name>" << ENDL << ENDL;

        uint32_t insnid = 0;
        uint32_t bblid = 0;
        uint32_t funcid = 0;
        uint32_t loopid = 0;
        for (FUNC func = BIN_firstFunc(bin); IS_VALID_PTR(func); func = BIN_nextFunc(bin, func)){
            if (!FUNC_countBbl(func)){
                continue;
            }
            CFG cfg = FUNC_cfg(func);

            for (BBL bbl = FUNC_firstBbl(func); IS_VALID_PTR(bbl); bbl = FUNC_nextBbl(func, bbl)){
                if (!BBL_countInsn(bbl)){
                    continue;
                }
                for (INSN insn = BBL_firstInsn(bbl); IS_VALID_PTR(insn); insn = BBL_nextInsn(bbl, insn)){
                    staticfile << DEC(insnid)
                               << TAB << HEX(INSN_addr(insn))
                               << TAB << FUNC_name(func)
                               << TAB << DEC(funcid)
                               << TAB << DEC(bblid)
                               << TAB << BIN_debugFileName(bin, INSN_addr(insn)) << ":" << BIN_debugLineNumber(bin, INSN_addr(insn))
                               << ENDL;

                    staticfile << TAB << "+str"
                               << TAB << INSN_string(insn)
                               << ENDL;

                    std::vector<std::string> groups;
                    INSN_groupNames(insn, groups);
                    staticfile << TAB << "+isa";
                    if (groups.size() == 0){
                        staticfile << TAB << NAME_UNKNOWN;
                    } else {
                        staticfile << TAB;
                        for (uint32_t i = 0; i < groups.size(); i++){
                            if (i > 0){
                                staticfile << ",";
                            }
                            staticfile << groups[i];
                        }
                    }
                    staticfile << TAB << INSN_size(insn)
                               << ENDL;

                    std::string condname = INSN_condName(insn);
                    if (condname.compare("INVALID") != 0){
                        staticfile << TAB << "+prd"
                                   << TAB << condname
                                   << ENDL;
                    }

                    std::vector<uint64_t> tlist;
                    INSN_targets(insn, tlist);
		    if (tlist.size() > 0){
		        staticfile << TAB << "+flw";
		        for (std::vector<uint64_t>::const_iterator it = tlist.begin(); it != tlist.end(); it++){
			    uint64_t addr = *it;
			    if (INVALID_ADDRESS == addr) continue;
			    staticfile << TAB << HEX(*it);
			}
			staticfile << ENDL;
		    }

                    CFG cfg = FUNC_cfg(func);
                    LOOP loop = CFG_findLoop(cfg, INSN_addr(insn));
                    if (loop){
                        staticfile << TAB << "+lpi"
                                   << TAB << DEC(CFG_countLoop(cfg))
                                   << TAB << DEC(LOOP_index(loop))
                                   << TAB << DEC(LOOP_depth(loop))
                                   << TAB << HEX(INSN_addr(BBL_head(LOOP_head(loop))))
                                   << TAB << HEX(INSN_addr(BBL_tail(LOOP_tail(loop))))
                                   << ENDL;

                        LOOP parent = LOOP_parent(loop);
                        if (parent){
                            staticfile << TAB << "+lpc"
                                       << TAB << HEX(INSN_addr(BBL_head(LOOP_head(parent))))
                                       << TAB << HEX(INSN_addr(BBL_tail(LOOP_tail(parent))))
                                       << ENDL;
                        }
                    }

                    staticfile << TAB << "+cnt"
                               << TAB << DEC(INSN_isBranch(insn))
                               << TAB << DEC(INSN_isFpop(insn))
                               << TAB << DEC(INSN_isLoad(insn))
                               << TAB << DEC(INSN_isStore(insn))
                               << ENDL;

                    uint32_t bitsInReg = INSN_sourceRegisterSizeInBits(insn);
                    uint32_t bitsInElem = INSN_sourceDatatypeSizeInBits(insn);

                    if(bitsInReg != 0 && bitsInElem != 0) {
                        staticfile << TAB << "+srg";
                        uint32_t isFp = INSN_isFpop(insn);
                        uint32_t elemInReg = bitsInReg / bitsInElem;
                        staticfile << TAB << elemInReg << "x" << bitsInElem << ":" << isFp << ":" << !isFp;
                        staticfile << ENDL;
                    }

                    uint64_t tgt = INSN_callTarget(insn);
                    if (tgt){
                        FUNC ftgt = BIN_findFuncAt(bin, tgt);
                        staticfile << TAB << "+ipa"
                                   << TAB << HEX(tgt)
                                   << TAB << (IS_VALID_PTR(ftgt)? FUNC_name(ftgt):NAME_UNKNOWN)
                                   << ENDL;
                    }
                    insnid++;
                }
                bblid++;
            }
            funcid++;
        }

        staticfile.flush();
        fb.close();

        return;

        // TODO: move the meat of this function down into the Binary class?
        bin->printStaticFile(fname);
    }

    FUNC BIN_findFuncAt(BIN bin, uint64_t addr){
        EPAXVerifyType(BIN, bin);
        return bin->findFunctionAt(addr);
    }

    bool BIN_hasDebugLineInfo(BIN bin){
        EPAXVerifyType(BIN, bin);
        return bin->hasDebugLineInfo();
    }

    uint32_t BIN_debugLineNumber(BIN bin, uint64_t addr){
        EPAXVerifyType(BIN, bin);
        return bin->getDebugLineNumber(addr);
    }

    std::string BIN_debugFileName(BIN bin, uint64_t addr){
        EPAXVerifyType(BIN, bin);
        return bin->getDebugLineFile(addr);
    }

    FUNC FUNC_create(uint8_t* bytes, uint32_t size){
        ShouldNotArrive; // TODO        
        return INVALID_PTR;
    }

    void FUNC_destroy(FUNC func){
        EPAXVerifyType(FUNC, func);
        ShouldNotArrive; // TODO
    }

    void FUNC_print(FUNC func){
        EPAXVerifyType(FUNC, func);
        func->print();
    }

    std::string FUNC_name(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getName();
    }

    uint32_t FUNC_size(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getMemorySize(); // TODO: count up real bytes used in instructions?
    }

    uint64_t FUNC_addr(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getMemoryAddress();
    }

    std::string FUNC_secName(FUNC func){
        EPAXVerifyType(FUNC, func);
        ShouldNotArrive; // TODO
    }

    BIN FUNC_bin(FUNC func){
        EPAXVerifyType(FUNC, func);
        ShouldNotArrive; // TODO
    }

    uint32_t FUNC_countBbl(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->countBasicBlocks();
    }

    BBL FUNC_findBbl(FUNC func, uint64_t addr){
        EPAXVerifyType(FUNC, func);
        return func->findBasicBlock(addr);
    }

    BBL FUNC_firstBbl(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getBasicBlock(0);
    }

    BBL FUNC_nextBbl(FUNC func, BBL bbl){
        EPAXVerifyType(FUNC, func);
        EPAXVerifyType(BBL, bbl);
        return func->getBasicBlock(bbl->getIndex() + 1);
    }

    bool FUNC_isLastBbl(FUNC func, BBL bbl){
        EPAXVerifyType(FUNC, func);
        EPAXVerifyType(BBL, bbl);
        return (func->countBasicBlocks() == bbl->getIndex() - 1);
    }

    uint32_t FUNC_countInsn(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->countInstructions();
    }

    INSN FUNC_findInsn(FUNC func, uint64_t addr){
        EPAXVerifyType(FUNC, func);
        return func->findInstruction(addr);
    }

    INSN FUNC_firstInsn(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getInstruction(0);
    }

    INSN FUNC_nextInsn(FUNC func, INSN insn){
        EPAXVerifyType(FUNC, func);
        EPAXVerifyType(INSN, insn);

        BBL bbl = INSN_bbl(insn);
        EPAXVerifyType(BBL, bbl);

        INSN nexti = BBL_nextInsn(bbl, insn);
        if (IS_VALID_PTR(nexti)){
            return nexti;
        }

        BBL nextb = FUNC_nextBbl(func, bbl);
        if (!IS_VALID_PTR(nextb)){
            return INVALID_PTR;
        }

        return BBL_firstInsn(nextb);
    }

    bool FUNC_isLastInsn(FUNC func, INSN insn){
        EPAXVerifyType(FUNC, func);
        EPAXVerifyType(INSN, insn);

        BBL bbl = INSN_bbl(insn);
        EPAXVerifyType(BBL, bbl);

        INSN nexti = BBL_nextInsn(bbl, insn);
        BBL nextb = FUNC_nextBbl(func, bbl);
        if (!IS_VALID_PTR(nexti) && !IS_VALID_PTR(nextb)){
            return true;
        }

        return false;
    }

    CFG FUNC_cfg(FUNC func){
        EPAXVerifyType(FUNC, func);
        return func->getControlFlow();
    }

    uint32_t FUNC_countTargets(FUNC func){
        EPAXVerifyType(FUNC, func);
        ShouldNotArrive; // TODO
    }

    uint32_t FUNC_targets(FUNC func, std::vector<FUNC>& funcList){
        EPAXVerifyType(FUNC, func);
        ShouldNotArrive; // TODO
    }

    uint32_t CFG_countLoop(CFG cfg){
        EPAXVerifyType(CFG, cfg);
        return cfg->countLoops();
    }

    LOOP CFG_findLoop(CFG cfg, uint64_t addr){
        EPAXVerifyType(CFG, cfg);
        return cfg->findLoop(addr);
    }

    LOOP CFG_firstLoop(CFG cfg){
        EPAXVerifyType(CFG, cfg);
        return cfg->getLoop(0);
    }

    LOOP CFG_nextLoop(CFG cfg, LOOP loop){
        EPAXVerifyType(CFG, cfg);
        EPAXVerifyType(LOOP, loop);
        return cfg->getLoop(loop->getIndex() + 1);
    }

    bool CFG_isLastLoop(CFG cfg, LOOP loop){
        EPAXVerifyType(CFG, cfg);
        EPAXVerifyType(LOOP, loop);
        return (cfg->countLoops() == loop->getIndex() - 1);
    }

    CFG LOOP_cfg(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->getControlFlow();
    }

    FUNC LOOP_func(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        ControlFlow* cfg = loop->getControlFlow();
        if (IS_VALID_PTR(cfg)){
            return cfg->getFunction();
        }
        return INVALID_PTR;
    }

    uint32_t LOOP_size(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->getSize();
    }

    uint32_t LOOP_countBbl(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->countBasicBlocks();
    }

    BBL LOOP_findBbl(LOOP loop, uint64_t addr){
        EPAXVerifyType(LOOP, loop);
        return loop->findBasicBlock(addr);
    }

    BBL LOOP_firstBbl(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->getBasicBlock(0);
    }

    BBL LOOP_nextBbl(LOOP loop, BBL bbl){
        EPAXVerifyType(LOOP, loop);
        EPAXVerifyType(BBL, bbl);
        return loop->getNextBasicBlock(bbl->getIndex());
    }

    bool LOOP_isLastBbl(LOOP loop, BBL bbl){
        EPAXVerifyType(LOOP, loop);
        EPAXVerifyType(BBL, bbl);
        return loop->isLastBasicBlock(bbl->getIndex());
    }

    uint32_t LOOP_countInsn(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->countInstructions();
    }

    INSN LOOP_findInsn(LOOP loop, uint64_t addr){
        EPAXVerifyType(LOOP, loop);
        return loop->findInstruction(addr);
    }

    INSN LOOP_firstInsn(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        BasicBlock* bbl = loop->getBasicBlock(0);
        if (IS_VALID_PTR(bbl)){
            return bbl->getInstruction(0);
        }
        return INVALID_PTR;
    }

    INSN LOOP_nextInsn(LOOP loop, INSN insn){
        EPAXVerifyType(LOOP, loop);
        BasicBlock* bbl = insn->getBasicBlock();
        if (IS_VALID_PTR(bbl)){
            Instruction* tail = bbl->tail();
            if (IS_VALID_PTR(tail) && tail->getIndex() == insn->getIndex()){
                bbl = loop->getNextBasicBlock(bbl->getIndex());
                if (IS_VALID_PTR(bbl)){
                    return bbl->head();
                } else {
                    return INVALID_PTR;
                }
            } else {
                return bbl->getInstruction(insn->getIndex() + 1);
            }
        }
        return INVALID_PTR;
    }

    bool LOOP_isLastInsn(LOOP loop, INSN insn){
        EPAXVerifyType(LOOP, loop);
        BasicBlock* bbl = insn->getBasicBlock();
        if (IS_VALID_PTR(bbl)){
            Instruction* tail = bbl->tail();
            if (loop->isLastBasicBlock(bbl->getIndex()) && tail->getIndex() == insn->getIndex()){
                return true;
            }
        }
        return false;
    }

    BBL LOOP_head(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->head();
    }

    BBL LOOP_tail(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->tail();
    }

    uint32_t LOOP_countExits(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        ShouldNotArrive; // TODO
    }

    uint32_t LOOP_exits(LOOP loop, std::vector<EPAX::INSN>& insnList){
        EPAXVerifyType(LOOP, loop);
        ShouldNotArrive; // TODO
    }

    bool LOOP_isInnerLoop(LOOP loop1, LOOP loop2){
        EPAXVerifyType(LOOP, loop2);
        ShouldNotArrive; // TODO
    }

    LOOP LOOP_parent(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        CFG cfg = LOOP_cfg(loop);
        return cfg->getParentOf(loop);
    }

    uint32_t LOOP_index(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->getIndex();
    }

    uint32_t LOOP_depth(LOOP loop){
        EPAXVerifyType(LOOP, loop);
        return loop->getDepth();
    }

    bool BBL_isHead(BBL bbl, INSN insn){
        EPAXVerifyType(BBL, bbl);
        EPAXVerifyType(INSN, insn);

        INSN head = BBL_head(bbl);
        BBL obbl = INSN_bbl(insn);
        if (!IS_VALID_PTR(obbl)){
            return false;
        }
        if (BBL_addr(bbl) != BBL_addr(obbl)){
            return false;
        }
        if (INSN_addr(head) != INSN_addr(insn)){
            return false;
        }
        return true;
    }

    bool BBL_isTail(BBL bbl, INSN insn){
        EPAXVerifyType(BBL, bbl);
        EPAXVerifyType(INSN, insn);

        INSN tail = BBL_tail(bbl);
        BBL obbl = INSN_bbl(insn);
        if (!IS_VALID_PTR(obbl)){
            return false;
        }
        if (BBL_addr(bbl) != BBL_addr(obbl)){
            return false;
        }
        if (INSN_addr(tail) != INSN_addr(insn)){
            return false;
        }
        return true;
    }

    INSN BBL_head(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->head();
    }

    INSN BBL_tail(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->tail();
    }

    FUNC BBL_func(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->getFunction();
    }

    LOOP BBL_loop(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->getLoop();
    }

    uint32_t BBL_size(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->getMemorySize();
    }

    uint64_t BBL_addr(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->getMemoryAddress();
    }

    uint32_t BBL_countInsn(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->countInstructions();
    }

    INSN BBL_findInsn(BBL bbl, uint64_t addr){
        EPAXVerifyType(BBL, bbl);
        return bbl->findInstruction(addr);
    }

    INSN BBL_firstInsn(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->getInstruction(0);
    }

    INSN BBL_nextInsn(BBL bbl, INSN insn){
        EPAXVerifyType(BBL, bbl);
        EPAXVerifyType(INSN, insn);
        return bbl->getInstruction(insn->getIndex() + 1);
    }

    bool BBL_isLastInsn(BBL bbl, INSN insn){
        EPAXVerifyType(BBL, bbl);
        EPAXVerifyType(INSN, insn);
        return (bbl->countInstructions() == insn->getIndex() - 1);
    }    

    uint32_t BBL_countTargets(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->countTargets();
    }

    uint32_t BBL_targets(BBL bbl, std::vector<EPAX::BBL>& bblList){
        EPAXVerifyType(BBL, bbl);
        uint32_t c = bbl->countTargets();
        for (uint32_t i = 0; i < c; i++){
            bblList.push_back(bbl->getTarget(i));
        }
        return c;
    }

    bool BBL_hasFallthroughTarget(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->isFallThrough();
    }

    BBL BBL_fallthroughTarget(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        ShouldNotArrive; // TODO
    }

    uint32_t BBL_countJumpTargets(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        ShouldNotArrive; // TODO        
    }

    uint32_t BBL_jumpTargets(BBL bbl, std::vector<EPAX::BBL>& bblList){
        EPAXVerifyType(BBL, bbl);
        ShouldNotArrive; // TODO
    }

    uint32_t BBL_countSources(BBL bbl){
        EPAXVerifyType(BBL, bbl);
        return bbl->countSources();
    }

    uint32_t BBL_sources(BBL bbl, std::vector<EPAX::BBL>& bblList){
        EPAXVerifyType(BBL, bbl);
        uint32_t c = bbl->countTargets();
        for (uint32_t i = 0; i < c; i++){
            bblList.push_back(bbl->getSource(i));
        }
        return c;
    }

    uint32_t INSN_targets(INSN insn, std::vector<uint64_t>& tlist){
        EPAXVerifyType(INSN, insn);
        uint32_t s = tlist.size();
        insn->getControlTargets(tlist);
        return tlist.size() - s;
    }

    BBL INSN_bbl(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->getBasicBlock();
    }

    FUNC INSN_func(INSN insn){
        EPAXVerifyType(INSN, insn);
        BasicBlock* bbl = insn->getBasicBlock();
        if (IS_VALID_PTR(bbl)){
            return bbl->getFunction();
        }
        return INVALID_PTR;
    }

    // TODO: return a list of loops? inner loop?
    LOOP INSN_loop(INSN insn){
        EPAXVerifyType(INSN, insn);
        ShouldNotArrive; // TODO
    }

    uint64_t INSN_addr(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->getMemoryAddress();
    }

    std::string INSN_string(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->stringRep();
    }

    uint64_t INSN_callTarget(INSN insn){
        EPAXVerifyType(INSN, insn);

        if (insn->isCall()){
            uint64_t tgt = insn->getBranchTarget();
            if (INVALID_ADDRESS == tgt){
                return 0;
            }
            return tgt;
        }
        return 0;
    }

    bool INSN_isBranch(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->isBranch();
    }

    bool INSN_isFpop(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->isFpop();
    }

    bool INSN_isMemop(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->isMemop();
    }

    bool INSN_isLoad(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->isLoad();
    }

    bool INSN_isStore(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->isStore();
    }

    uint32_t INSN_size(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->getMemorySize();        
    }

    std::string INSN_condName(INSN insn){
        EPAXVerifyType(INSN, insn);
        return std::string(insn->getConditionName());
    }

    bool INSN_fallsThrough(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->hasFallthrough();
    }

    uint32_t INSN_sourceRegisterSizeInBits(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->getSourceRegisterSizeInBits();
    }

    uint32_t INSN_sourceDatatypeSizeInBits(INSN insn){
        EPAXVerifyType(INSN, insn);
        return insn->getSourceDatatypeSizeInBits();
    }

    uint32_t INSN_groupNames(INSN insn, std::vector<std::string>& groups){
        EPAXVerifyType(INSN, insn);
        return insn->getGroupNames(groups);
    }

    uint32_t INSN_groupCount(INSN insn){
        EPAXVerifyType(INSN, insn);
        std::vector<std::string> groups;
        uint32_t count = INSN_groupNames(insn, groups);
        return count;
    }

} // namespace EPAX

/* these C implementations are just a thin layer around the C++ implementations */
extern "C" {

    EPAX_bin EPAX_bin_create(const char* fileName){
        std::string s(fileName);
        return (EPAX_bin)EPAX::BIN_create(s);
    }

    const char* EPAX_bin_getName(EPAX_bin bin){
        std::string s(EPAX::BIN_getName((EPAX::BIN)bin));
        return s.c_str();
    }

    void EPAX_bin_destroy(EPAX_bin bin){
        EPAX::BIN_destroy((EPAX::BIN)bin);
    }

    void EPAX_bin_run(EPAX_bin bin, int argc, char** argv){
        EPAX::BIN_run((EPAX::BIN)bin, argc, argv);
    }

    EPAX_func EPAX_bin_firstFunc(EPAX_bin bin){
        return (EPAX_func)EPAX::BIN_firstFunc((EPAX::BIN)bin);
    }

    EPAX_func EPAX_bin_nextFunc(EPAX_bin bin, EPAX_func func){
        return (EPAX_func)EPAX::BIN_nextFunc((EPAX::BIN)bin, (EPAX::FUNC)func);
    }

    uint32_t EPAX_bin_isLastFunc(EPAX_bin bin, EPAX_func func){
        return (uint32_t)EPAX::BIN_isLastFunc((EPAX::BIN)bin, (EPAX::FUNC)func);
    }

    uint32_t EPAX_bin_countFunc(EPAX_bin bin){
        return EPAX::BIN_countFunc((EPAX::BIN)bin);
    }

    uint32_t EPAX_bin_isExecutable(EPAX_bin bin){
        return (uint32_t)EPAX::BIN_isExecutable((EPAX::BIN)bin);
    }

    uint32_t EPAX_bin_fileSize(EPAX_bin bin){
        return EPAX::BIN_fileSize((EPAX::BIN)bin);
    }

    void EPAX_bin_printStaticFile(EPAX_bin bin, const char* fname){
        std::string s(fname);
        EPAX::BIN_printStaticFile((EPAX::BIN)bin, s);
    }

    EPAX_func EPAX_bin_fundFunc(EPAX_bin bin, uint64_t addr){
        return (EPAX_func)EPAX::BIN_findFuncAt((EPAX::BIN)bin, addr);
    }

    EPAX_func EPAX_func_create(uint8_t* bytes, uint32_t size){
        return (EPAX_func)EPAX::FUNC_create(bytes, size);
    }

    void EPAX_func_destroy(EPAX_func func){
        EPAX::FUNC_destroy((EPAX::FUNC)func);
    }

    void EPAX_func_print(EPAX_func func){
        EPAX::FUNC_print((EPAX::FUNC)func);
    }

    const char* EPAX_func_name(EPAX_func func){
        std::string s = EPAX::FUNC_name((EPAX::FUNC)func);
        return s.c_str();
    }

    uint32_t EPAX_func_size(EPAX_func func){
        return EPAX::FUNC_size((EPAX::FUNC)func);
    }

    uint64_t EPAX_func_addr(EPAX_func func){
        return EPAX::FUNC_addr((EPAX::FUNC)func);
    }

    const char* EPAX_func_secName(EPAX_func func){
        std::string s = EPAX::FUNC_secName((EPAX::FUNC)func);
        return s.c_str();
    }

    EPAX_bin EPAX_func_bin(EPAX_func func){
        return (EPAX_bin)EPAX::FUNC_bin((EPAX::FUNC)func);
    }

    uint32_t EPAX_func_countBbl(EPAX_func func){
        return EPAX::FUNC_countBbl((EPAX::FUNC)func);
    }

    EPAX_bbl EPAX_func_findBbl(EPAX_func func, uint64_t addr){
        return (EPAX_bbl)EPAX::FUNC_findBbl((EPAX::FUNC)func, addr);
    }

    EPAX_bbl EPAX_func_firstBbl(EPAX_func func){
        return (EPAX_bbl)EPAX::FUNC_firstBbl((EPAX::FUNC)func);
    }

    EPAX_bbl EPAX_func_nextBbl(EPAX_func func, EPAX_bbl bbl){
        return (EPAX_bbl)EPAX::FUNC_nextBbl((EPAX::FUNC)func, (EPAX::BBL)bbl);
    }

    uint32_t EPAX_func_isLastBbl(EPAX_func func, EPAX_bbl bbl){
        return (uint32_t)EPAX::FUNC_isLastBbl((EPAX::FUNC)func, (EPAX::BBL)bbl);
    }

    uint32_t EPAX_func_countInsn(EPAX_func func){
        return EPAX::FUNC_countInsn((EPAX::FUNC)func);
    }

    EPAX_insn EPAX_func_findInsn(EPAX_func func, uint64_t addr){
        return (EPAX_insn)EPAX::FUNC_findInsn((EPAX::FUNC)func, addr);
    }

    EPAX_insn EPAX_func_firstInsn(EPAX_func func){
        return (EPAX_insn)EPAX::FUNC_firstInsn((EPAX::FUNC)func);
    }

    EPAX_insn EPAX_func_nextInsn(EPAX_func func, EPAX_insn insn){
        return (EPAX_insn)EPAX::FUNC_nextInsn((EPAX::FUNC)func, (EPAX::INSN)insn);
    }

    uint32_t EPAX_func_isLastInsn(EPAX_func func, EPAX_insn insn){
        return (uint32_t)EPAX::FUNC_isLastInsn((EPAX::FUNC)func, (EPAX::INSN)insn);
    }

    EPAX_cfg EPAX_func_cfg(EPAX_func func){
        return (EPAX_cfg)EPAX::FUNC_cfg((EPAX::FUNC)func);
    }

    uint32_t EPAX_func_countTargets(EPAX_func func){
        return EPAX::FUNC_countTargets((EPAX::FUNC)func);
    }

    uint32_t EPAX_func_targets(EPAX_func func, EPAX_func* funcList){
        EPAXAssert(IS_VALID_PTR(funcList), "NULL pointer cannot be passed for output parameter");
        std::vector<EPAX::FUNC> fv;
        uint32_t res = EPAX::FUNC_targets((EPAX::FUNC)func, fv);

        uint32_t cur = 0;
        for (std::vector<EPAX::FUNC>::const_iterator it = fv.begin(); it != fv.end(); it++, cur++){
            funcList[cur] = (*it);
        }
        return res;
    }

    uint32_t EPAX_cfg_countLoop(EPAX_cfg cfg){
        return EPAX::CFG_countLoop((EPAX::CFG)cfg);
    }

    EPAX_loop EPAX_cfg_findLoop(EPAX_cfg cfg, uint64_t addr){
        return (EPAX_loop)EPAX::CFG_findLoop((EPAX::CFG)cfg, addr);
    }

    EPAX_loop EPAX_cfg_firstLoop(EPAX_cfg cfg){
        return (EPAX_loop)EPAX::CFG_firstLoop((EPAX::CFG)cfg);
    }

    EPAX_loop EPAX_cfg_nextLoop(EPAX_cfg cfg, EPAX_loop loop){
        return (EPAX_loop)EPAX::CFG_nextLoop((EPAX::CFG)cfg, (EPAX::LOOP)loop);
    }

    uint32_t EPAX_cfg_isLastLoop(EPAX_cfg cfg, EPAX_loop loop){
        return (uint32_t)EPAX::CFG_isLastLoop((EPAX::CFG)cfg, (EPAX::LOOP)loop);
    }

    EPAX_cfg EPAX_loop_cfg(EPAX_loop loop){
        return (EPAX_cfg)EPAX::LOOP_cfg((EPAX::LOOP)loop);
    }

    EPAX_func EPAX_loop_func(EPAX_loop loop){
        return (EPAX_func)EPAX::LOOP_func((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_size(EPAX_loop loop){
        return EPAX::LOOP_size((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_countBbl(EPAX_loop loop){
        return EPAX::LOOP_countBbl((EPAX::LOOP)loop);
    }

    EPAX_bbl EPAX_loop_findBbl(EPAX_loop loop, uint64_t addr){
        return (EPAX_bbl)EPAX::LOOP_findBbl((EPAX::LOOP)loop, addr);
    }

    EPAX_bbl EPAX_loop_firstBbl(EPAX_loop loop){
        return (EPAX_bbl)EPAX::LOOP_firstBbl((EPAX::LOOP)loop);
    }

    EPAX_bbl EPAX_loop_nextBbl(EPAX_loop loop, EPAX_bbl bbl){
        return (EPAX_bbl)EPAX::LOOP_nextBbl((EPAX::LOOP)loop, (EPAX::BBL)bbl);
    }

    uint32_t EPAX_loop_isLastBbl(EPAX_loop loop, EPAX_bbl bbl){
        return (uint32_t)EPAX::LOOP_isLastBbl((EPAX::LOOP)loop, (EPAX::BBL)bbl);
    }

    uint32_t EPAX_loop_countInsn(EPAX_loop loop){
        return EPAX::LOOP_countInsn((EPAX::LOOP)loop);
    }

    EPAX_insn EPAX_loop_findInsn(EPAX_loop loop, uint64_t addr){
        return (EPAX_insn)EPAX::LOOP_findInsn((EPAX::LOOP)loop, addr);
    }

    EPAX_insn EPAX_loop_firstInsn(EPAX_loop loop){
        return (EPAX_insn)EPAX::LOOP_firstInsn((EPAX::LOOP)loop);
    }

    EPAX_insn EPAX_loop_nextInsn(EPAX_loop loop, EPAX_insn insn){
        return (EPAX_insn)EPAX::LOOP_nextInsn((EPAX::LOOP)loop, (EPAX::INSN)insn);
    }

    uint32_t EPAX_loop_isLastInsn(EPAX_loop loop, EPAX_insn insn){
        return (uint32_t)EPAX::LOOP_isLastInsn((EPAX::LOOP)loop, (EPAX::INSN)insn);
    }

    EPAX_bbl EPAX_loop_head(EPAX_loop loop){
        return (EPAX_bbl)EPAX::LOOP_head((EPAX::LOOP)loop);
    }

    EPAX_bbl EPAX_loop_tail(EPAX_loop loop){
        return (EPAX_bbl)EPAX::LOOP_tail((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_countExits(EPAX_loop loop){
        return EPAX::LOOP_countExits((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_exits(EPAX_loop loop, EPAX_insn* insnList){
        EPAXAssert(IS_VALID_PTR(insnList), "NULL pointer cannot be passed for output parameter");
        std::vector<EPAX::INSN> iv;
        uint32_t res = EPAX::LOOP_exits((EPAX::LOOP)loop, iv);

        uint32_t cur = 0;
        for (std::vector<EPAX::INSN>::const_iterator it = iv.begin(); it != iv.end(); it++, cur++){
            insnList[cur] = (*it);
        }
        return res;
    }

    uint32_t EPAX_loop_isInnerLoop(EPAX_loop loop1, EPAX_loop loop2){
        return (uint32_t)EPAX::LOOP_isInnerLoop((EPAX::LOOP)loop1, (EPAX::LOOP)loop2);
    }

    EPAX_loop EPAX_loop_parent(EPAX_loop loop){
        return (EPAX_loop)EPAX::LOOP_parent((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_index(EPAX_loop loop){
        return EPAX::LOOP_index((EPAX::LOOP)loop);
    }

    uint32_t EPAX_loop_depth(EPAX_loop loop){
        return EPAX::LOOP_depth((EPAX::LOOP)loop);
    }

    uint32_t EPAX_bbl_isHead(EPAX_bbl bbl, EPAX_insn insn){
        return (uint32_t)EPAX::BBL_isHead((EPAX::BBL)bbl, (EPAX::INSN)insn);
    }

    uint32_t EPAX_bbl_isTail(EPAX_bbl bbl, EPAX_insn insn){
        return (uint32_t)EPAX::BBL_isTail((EPAX::BBL)bbl, (EPAX::INSN)insn);
    }

    EPAX_insn EPAX_bbl_head(EPAX_bbl bbl){
        return (EPAX_insn)EPAX::BBL_head((EPAX::BBL)bbl);
    }

    EPAX_insn EPAX_bbl_tail(EPAX_bbl bbl){
        return (EPAX_insn)EPAX::BBL_tail((EPAX::BBL)bbl);
    }

    EPAX_func EPAX_bbl_func(EPAX_bbl bbl){
        return (EPAX_func)EPAX::BBL_func((EPAX::BBL)bbl);
    }

    EPAX_loop EPAX_bbl_loop(EPAX_bbl bbl){
        return (EPAX_loop)EPAX::BBL_loop((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_size(EPAX_bbl bbl){
        return EPAX::BBL_size((EPAX::BBL)bbl);
    }

    uint64_t EPAX_bbl_addr(EPAX_bbl bbl){
        return EPAX::BBL_addr((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_countInsn(EPAX_bbl bbl){
        return EPAX::BBL_countInsn((EPAX::BBL)bbl);
    }

    EPAX_insn EPAX_bbl_findInsn(EPAX_bbl bbl, uint64_t addr){
        return (EPAX_insn)EPAX::BBL_findInsn((EPAX::BBL)bbl, addr);
    }

    EPAX_insn EPAX_bbl_firstInsn(EPAX_bbl bbl){
        return (EPAX_insn)EPAX::BBL_firstInsn((EPAX::BBL)bbl);
    }

    EPAX_insn EPAX_bbl_nextInsn(EPAX_bbl bbl, EPAX_insn insn){
        return (EPAX_insn)EPAX::BBL_nextInsn((EPAX::BBL)bbl, (EPAX::INSN)insn);
    }

    uint32_t EPAX_bbl_isLastInsn(EPAX_bbl bbl, EPAX_insn insn){
        return (uint32_t)EPAX::BBL_isLastInsn((EPAX::BBL)bbl, (EPAX::INSN)insn);
    }

    uint32_t EPAX_bbl_countTargets(EPAX_bbl bbl){
        return EPAX::BBL_countTargets((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_targets(EPAX_bbl bbl, EPAX_bbl* bblList){
        EPAXAssert(IS_VALID_PTR(bblList), "NULL pointer cannot be passed for output parameter");
        std::vector<EPAX::BBL> bv;
        uint32_t res = EPAX::BBL_targets((EPAX::BBL)bbl, bv);

        uint32_t cur = 0;
        for (std::vector<EPAX::BBL>::const_iterator it = bv.begin(); it != bv.end(); it++, cur++){
            bblList[cur] = (*it);
        }
        return res;
    }

    uint32_t EPAX_bbl_hasFallthroughTarget(EPAX_bbl bbl){
        return (uint32_t)EPAX::BBL_hasFallthroughTarget((EPAX::BBL)bbl);
    }

    EPAX_bbl EPAX_bbl_fallthroughTarget(EPAX_bbl bbl){
        return (EPAX_bbl)EPAX::BBL_fallthroughTarget((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_countJumpTargets(EPAX_bbl bbl){
        return EPAX::BBL_countJumpTargets((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_jumpTargets(EPAX_bbl bbl, EPAX_bbl* bblList){
        EPAXAssert(IS_VALID_PTR(bblList), "NULL pointer cannot be passed for output parameter");
        std::vector<EPAX::BBL> bv;
        uint32_t res = EPAX::BBL_jumpTargets((EPAX::BBL)bbl, bv);

        uint32_t cur = 0;
        for (std::vector<EPAX::BBL>::const_iterator it = bv.begin(); it != bv.end(); it++, cur++){
            bblList[cur] = (*it);
        }
        return res;
    }

    uint32_t EPAX_bbl_countSources(EPAX_bbl bbl){
        return EPAX::BBL_countSources((EPAX::BBL)bbl);
    }

    uint32_t EPAX_bbl_sources(EPAX_bbl bbl, EPAX_bbl* bblList){
        EPAXAssert(IS_VALID_PTR(bblList), "NULL pointer cannot be passed for output parameter");
        std::vector<EPAX::BBL> bv;
        uint32_t res = EPAX::BBL_sources((EPAX::BBL)bbl, bv);

        uint32_t cur = 0;
        for (std::vector<EPAX::BBL>::const_iterator it = bv.begin(); it != bv.end(); it++, cur++){
            bblList[cur] = (*it);
        }
        return res;
    }

    uint32_t EPAX_insn_targets(EPAX_insn insn, uint64_t* tlist){
        EPAXAssert(IS_VALID_PTR(tlist), "NULL pointer cannot be passed for output parameter");
        std::vector<uint64_t> tv;
        uint32_t res = EPAX::INSN_targets((EPAX::INSN)insn, tv);

        uint32_t cur = 0;
        for (std::vector<uint64_t>::const_iterator it = tv.begin(); it != tv.end(); it++, cur++){
            tlist[cur] = (*it);
        }
        return res;
    }

    EPAX_bbl EPAX_insn_bbl(EPAX_insn insn){
        return (EPAX_bbl)EPAX::INSN_bbl((EPAX::INSN)insn);
    }

    EPAX_func EPAX_insn_func(EPAX_insn insn){
        return (EPAX_func)EPAX::INSN_func((EPAX::INSN)insn);
    }

    EPAX_loop EPAX_insn_loop(EPAX_insn insn){
        return (EPAX_loop)EPAX::INSN_loop((EPAX::INSN)insn);
    }

    uint64_t EPAX_insn_addr(EPAX_insn insn){
        return EPAX::INSN_addr((EPAX::INSN)insn);
    }

    const char* EPAX_insn_string(EPAX_insn insn){
        std::string s = EPAX::INSN_string((EPAX::INSN)insn);
        return s.c_str();
    }

    uint64_t EPAX_insn_callTarget(EPAX_insn insn){
        return EPAX::INSN_callTarget((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_isBranch(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_isBranch((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_isFpop(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_isFpop((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_isMemop(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_isMemop((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_isLoad(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_isLoad((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_isStore(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_isStore((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_size(EPAX_insn insn){
        return EPAX::INSN_size((EPAX::INSN)insn);
    }

    const char* EPAX_insn_condName(EPAX_insn insn){
        std::string s(EPAX::INSN_condName((EPAX::INSN)insn));
        return s.c_str();
    }

    uint32_t EPAX_insn_fallsThrough(EPAX_insn insn){
        return (uint32_t)EPAX::INSN_fallsThrough((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_sourceRegisterSizeInBits(EPAX_insn insn){
        return EPAX::INSN_sourceRegisterSizeInBits((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_sourceDatatypeSizeInBits(EPAX_insn insn){
        return EPAX::INSN_sourceDatatypeSizeInBits((EPAX::INSN)insn);
    }

    uint32_t EPAX_insn_groupNames(EPAX_insn insn, char** groups){
        EPAXAssert(IS_VALID_PTR(groups), "NULL pointer cannot be passed for output parameter");
        std::vector<std::string> grps;
        EPAX::INSN_groupNames((EPAX::INSN)insn, grps);

        for (uint32_t i = 0; i < grps.size(); i++){
            char* name = (char*)(grps[i].c_str());
            groups[i] = name;
        }
        return grps.size();
    }

    uint32_t EPAX_insn_groupCount(EPAX_insn insn){
        std::vector<std::string> grps;
        EPAX::INSN_groupNames((EPAX::INSN)insn, grps);

        return grps.size();
    }
}

