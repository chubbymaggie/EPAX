/**
 * @file Instruction.cpp
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

#include "Instruction.hpp"
#include "Function.hpp"

#include <assert.h>

namespace EPAX {

#define DARM_REGLIST_HASREG(__list, __reg) ((__list >> __reg) != 0)
#define DARM_PREDICATE_UNCOND(__pred) (C_AL == __pred)
 
    // TODO: assumes a 32-bit normal ARM instrution for now
    Instruction::Instruction(const uint64_t a, const rawbyte_t* r, Function* func, const darm_mode_t mode)
        : MemoryBase(a, 0),
          IndexBase(0),
          EPAXExport(EPAXExportClass_INSN),
          function(func)
    {
        uint32_t insn;
        uint16_t insn1, insn2;

        // TODO: assumes little endian
        bzero(&info, sizeof(info));

        switch((uint32_t)mode){

        case M_ARM:
            insn = (r[0] << 0) | (r[1] << 8) | (r[2] << 16) | (r[3] << 24);
            disasm_res = darm_armv7_disasm(&info, insn);
            if(disasm_res != 0) {
                EPAXErr << "Unable to disassemble   ARM: " << insn << "\n";
            }
            break;

        case M_THUMB2:
            insn1 = (r[0] << 0) | (r[1] << 8);
            insn2 = 0;
            if (IS_THUMB2_32BIT(insn1)){
                insn2 = (r[2] << 0) | (r[3] << 8);
            }
            disasm_res = darm_thumb2_disasm(&info, insn1, insn2);
            if(disasm_res != 0 && insn2 != 0) {
                EPAXErr << "Unable to disassemble THU32: " << insn1 << ", " << insn2 << "\n";
            }

            // TODO: this is a hack because darm messes up some of these
            if (!info.size){
                info.size = 2;
                if (IS_THUMB2_32BIT(insn1)){
                    info.size = 4;
                }
            }

	    if(disasm_res == 0 || insn2 != 0) {
                break;
            }

        case M_THUMB:
            insn1 = (r[0] << 0) | (r[1] << 8);
            disasm_res = darm_thumb_disasm(&info, insn1);
            if(disasm_res != 0) {
                EPAXErr << "Unable to disassemble THUMB: " << insn1 << "\n";
            }

            // TODO info size should be set by darm
            if(!info.size) {
                info.size = 2;
            }

            break;


        default:
            EPAXErr << "Unexpected mode given to Instruction constructor";
        }

        EPAXAssert(2 == info.size || 4 == info.size, "Unexpected instruction size: " << DEC(info.size));
        setMemorySize(info.size);
    }

    Instruction::~Instruction(){
    }

    std::string Instruction::stringRep(){
        darm_str_t s;
        if (!disasm_res && darm_str2(&info, &s, 1) == 0){
            return std::string(s.instr);
        }
        return std::string(NAME_UNKNOWN);
    }

    darm_cond_t Instruction::getCondition(){
        return info.cond;
    }

    void Instruction::print(std::ostream& stream){
        darm_str_t s;
        uint32_t blen = info.size * 2;

        if (!disasm_res){
            darm_str2(&info, &s, 1);
            stream << (isBranch()? (hasFallthrough()? "BrF": "Br"):"") << TAB << HEX(getMemoryAddress()) << ":" << TAB << std::setfill('0') << std::setw(blen) << std::hex << info.w << TAB << s.instr;
        } else {
            stream << TAB << HEX(getMemoryAddress()) << ":" << TAB << std::setfill('0') << std::setw(blen) << std::hex << info.w << TAB << NAME_UNKNOWN;
        }

        std::vector<uint64_t> targets;
        getControlTargets(targets);
        EPAXAssert(targets.size() > 0, "Every instruction should have at least one control flow target");

        uint64_t ft = hasFallthrough()? fallthroughTarget() : INVALID_ADDRESS;

        bool first = true;
        for (uint32_t i = 0; i < targets.size(); i++){
            if (targets[i] != ft){
                if (first){
                    first = false;
                    stream << TAB;
                }
                if (targets[i] == INVALID_ADDRESS){
                    stream << NAME_UNKNOWN;
                } else {
                    stream << HEX(targets[i]);
                    if (IS_VALID_PTR(function)){
                        if (function->inRange(targets[i])){
                            stream << "<" << function->getName() << "+" << HEX(targets[i] - function->getMemoryAddress()) << ">";
                        } else {
                            Function* tgt = function->getBinary()->findFunction(targets[i]);
                            if (IS_VALID_PTR(tgt)){
                                stream << "<" << tgt->getName() << "+" << HEX(targets[i] - tgt->getMemoryAddress()) << ">";
                            } else {
                                stream << "<" << NAME_UNKNOWN << ">";
                            }
                        }
                    }
                }
                if (i < targets.size() - 1){
                    stream << ",";
                }
            }
        }
        stream << ENDL;
        /*
        EPAXOut << TAB << "DEST" << TAB << HEX(info.destreg1) << TAB << HEX(info.destreg2) << ENDL;
        EPAXOut << TAB << "SRC" << TAB << HEX(info.opreg1) << TAB << HEX(info.opreg2) << TAB << HEX(info.opreg3) << ENDL;
        EPAXOut << TAB << "LIST" << TAB << HEX(info.regslist) << ENDL;
        */
    }

    bool Instruction::isBranch(){
        return (isConditionalBranch() || isUnconditionalBranch());
    }

    bool Instruction::isConditionalBranch(){
        if (touchesPC() && (!DARM_PREDICATE_UNCOND(info.cond) || I_CBZ == info.instr || I_CBNZ == info.instr)){
            return true;
        }
        return false;
    }

    bool Instruction::touchesPC(){
        if (I_B    == info.instr || 
            I_BL   == info.instr || 
            I_BX   == info.instr || 
            I_BLX  == info.instr || 
            I_BXJ  == info.instr ||
            I_CBZ  == info.instr ||
            I_CBNZ == info.instr ||
            PC  == info.Rd || 
            PC  == info.Rd ||
            DARM_REGLIST_HASREG(info.reglist, PC)){
            return true;
        }
        return false;
    }

    bool Instruction::isUnconditionalBranch(){
        if (touchesPC() && DARM_PREDICATE_UNCOND(info.cond)){
            return true;
        }
        return false;
    }

    bool Instruction::isCall(){
        if (I_BL  == info.instr ||
            I_BLX == info.instr){
            return true;
        }
        return false;
    }

    uint64_t Instruction::getBranchTarget(){
        EPAXAssert(isBranch(), "Cannot call this on a non-branch");
        if (info.I == B_INVLD){
            return INVALID_ADDRESS;
        }

        uint64_t baddr = getMemoryAddress();

        // if THUMB -> ARM, PC is 4-aligned before adding br offset
        if (M_THUMB2 == info.mode && I_BLX == info.instr){
            baddr = ALIGN_PWR2(baddr, 2);
        }

        // 16-bit modes
        if (M_THUMB == info.mode || M_THUMB2_16 == info.mode || M_THUMB2 == info.mode){
            baddr += 4;
        }
        // 32-bit modes
        else if (M_ARM == info.mode){
            baddr += 8;
        }
        else {
            EPAXAssert(false, "Unexpected info.mode found: " << DEC(info.mode));
        }
        return (0xffffffff & (baddr + info.imm));
    }

    bool Instruction::hasFallthrough(){
        if (!isBranch()){
            return true;
        }
        if (isConditionalBranch()){
            return true;
        }
        if (isCall()){
            return true;
        }
        return false;
    }

    uint64_t Instruction::fallthroughTarget(){
        EPAXAssert(hasFallthrough(), "Cannot call this on an instruction w/o fallthrough");
        return getMemoryAddress() + getMemorySize();
    }

    uint32_t Instruction::getControlTargets(std::vector<uint64_t>& tgts){
        if (hasFallthrough()){
            tgts.push_back(fallthroughTarget());
        }
        if (isBranch()){
            tgts.push_back(getBranchTarget());
        }
        EPAXAssert(tgts.size() > 0, "This instruction appears to have no control targets");
        return tgts.size();
    }

    bool Instruction::isFpop(){
        switch(info.dtype) {
            case D_F64:
            case D_F32:
            case D_F16:
                return true;
            default:
                break; // fallthrough
        }
        switch(info.stype) {
            case D_F64:
            case D_F32:
            case D_F16:
                return true;
            default:
                break; // fallthrough
        }
        return false;
    }

    bool Instruction::isLoad(){
        switch(info.instr) {
            case I_LDR:
            case I_VLD1:
            case I_VLD2:
            case I_VLD3:
            case I_VLD4:
            case I_VLDR:
            case I_VLDM:
            case I_VLDMIA:
            case I_VLDMDB:
            case I_POP:
            case I_VPOP:
                return true;
            default:
                return false;
        }
    }

    bool Instruction::isStore(){
        switch(info.instr){
            case I_STR:
            case I_VST1:
            case I_VST2:
            case I_VST3:
            case I_VST4:
            case I_VSTR:
            case I_VSTM:
            case I_VSTMIA:
            case I_VSTMDB:
            case I_PUSH:
            case I_VPUSH:
                return true;
            default:
                return false;
        }
    }

    bool Instruction::isMemop(){
        return isLoad() || isStore();
    }

    uint32_t Instruction::getSourceRegisterSizeInBits(){
        darm_reg_t rn = info.Rn;

        if(rn >= r0 && rn <= r15) {
            return 32;
        } else if(rn >= s0 && rn <= s31) {
            return 32;
        } else if(rn >= d0 && rn <= d31) {
            return 64;
        } else if(rn >= q0 && rn <= q15) {
            return 128;
        } else {
            return 0;
        }
    }

    uint32_t Instruction::getSourceDatatypeSizeInBits(){
        switch(info.stype) {
            case D_F64:
            case D_I64:
            case D_64:
                return 64;
            case D_F32:
            case D_S32:
            case D_U32:
            case D_I32:
            case D_32:
                return 32;
            case D_F16:
            case D_S16:
            case D_U16:
            case D_I16:
            case D_16:
                return 16;
            case D_8:
            case D_I8:
                return 8;
            case D_INVLD:
                return 0;

            // no default to encourage compiler warnings
        }
        assert(0); // unreachable
    }

} // namespace EPAX
