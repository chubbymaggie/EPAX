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

namespace EPAX {

    class BasicBlock;
    class Function;

    typedef enum {
        PredCondition_EQ = 0,
        PredCondition_NE,
        PredCondition_HS,
        PredCondition_LO,
        PredCondition_MI,
        PredCondition_PL,
        PredCondition_VS,
        PredCondition_VC, 
        PredCondition_HI,
        PredCondition_LS,
        PredCondition_GE,
        PredCondition_LT,
        PredCondition_GT,
        PredCondition_LE,
        PredCondition_AL,
        PredCondition_INVALID,
        PredCondition_total
    } PredCondition;

    typedef enum {
        DisasmMode_ARM,
        DisasmMode_THUMB,
        DisasmMode_THUMB2_16,
        DisasmMode_THUMB2,
        DisasmMode_ARM_VFP,
        DisasmMode_THUMB2_VFP,
        DisasmMode_ARM_NEON,
        DisasmMode_THUMB2_NEON,

        DisasmMode_INVLD = -1,
    } DisasmMode;

    class Instruction : public MemoryBase, public IndexBase, public EPAXExport {
    protected:
        bool disasm_res;

        Function* function;
        BasicBlock* basicblock;

    public:
        Instruction(const uint64_t a, const rawbyte_t* r, Function* func)
            : MemoryBase(a, 0),
              IndexBase(0),
              EPAXExport(EPAXExportClass_INSN),
              function(func)
        {}

        virtual ~Instruction(){}

        static void disassemble(rawbyte_t* buf, uint64_t addr, const uint32_t size, std::vector<Instruction*>& insns, fastmap<uint64_t, uint32_t>::map& insn_map, DisasmMode mode, Function* func, std::vector<void*>& handlers, bool isARMv8);
        static void freedisasm(std::vector<void*> handlers);

        void setBasicBlock(BasicBlock* bb){ basicblock = bb; }
        BasicBlock* getBasicBlock() { return basicblock; }
        Function* getFunction() { return function; }

        virtual const char* const getConditionName();
        virtual PredCondition getCondition() = 0;

        // control-related
        virtual bool isBranch();
        virtual bool isConditionalBranch() = 0;
        virtual bool isUnconditionalBranch() = 0;
        virtual bool hasFallthrough() = 0;
        virtual bool touchesPC() = 0;
        virtual bool isReturn() = 0;
        virtual bool isCall() = 0;
        virtual uint64_t fallthroughTarget() = 0;
        virtual uint64_t getBranchTarget() = 0;
        virtual uint32_t getControlTargets(std::vector<uint64_t>& tgts) = 0;

        virtual bool isFpop() = 0;
        virtual bool isLoad() = 0;
        virtual bool isStore() = 0;
        virtual bool isMemop();

        virtual uint32_t getSourceRegisterSizeInBits() = 0;
        virtual uint32_t getSourceDatatypeSizeInBits() = 0;

        virtual std::string stringRep() = 0;
        virtual void print(std::ostream& stream = std::cout) = 0;

        virtual uint32_t getGroupNames(std::vector<std::string>& cls);
    }; // class Instruction

} // namespace EPAX

#endif // __EPAX_Instruction_hpp__

