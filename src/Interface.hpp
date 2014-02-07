/**
 * @file Interface.hpp
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

#ifndef __EPAX_Interface_hpp__
#define __EPAX_Interface_hpp__

// keep this list as small as possible
#include <stdint.h>
#include <string>
#include <vector>

namespace EPAX {

    class Binary;
    class Section;
    class Function;
    class ControlFlow;
    class Loop;
    class BasicBlock;
    class Instruction;
    class Symbol;
    class FlowEquation; // TODO: DNE. will represent a scheme for general data flow analysis, a la http://en.wikipedia.org/wiki/Data-flow_analysis
    // TODO: do we really want stl in this interface? (std::string and std::vector)
    // TODO: "const" for input params, label all params as in/out?

    typedef Binary*        BIN;
    typedef Section*       SECT;
    typedef Function*      FUNC;
    typedef ControlFlow*   CFG;
    typedef Loop*          LOOP;
    typedef BasicBlock*    BBL;
    typedef Instruction*   INSN;
    typedef Symbol*        SYM;
    typedef FlowEquation*  FLOW;

    /**
     * Creates a BIN object
     * 
     * @param fileName The name of a binary file. Allowed formats are: ELF, MachO
     * @return a BIN object created using the input parameter
     */
    extern BIN BIN_create(std::string fileName);


    /**
     * returns the name of a BIN object
     *
     * @param bin a BIN
     * @return the name of the file used to create bin
     */
    extern std::string BIN_getName(BIN bin);

    /**
     * frees all memory associated with a BIN object
     *
     * @param bin a BIN object, which is set to NULL during this operation.
     * @return none
     */
    extern void BIN_destroy(BIN bin);

    /**
     * Runs a the program represented by BIN with arguments; does not return.
     *
     * @param bin a BIN object for which BIN_isExecutable returns true
     * @param argc the number of program arguments
     * @param argv the program arguments
     * @return none
     */
    extern void BIN_run(BIN bin, int argc, char** argv);

    /**
     * Gets the first function in a BIN object
     *
     * @param bin a BIN object
     * @return the first logical function in binary
     */
    extern FUNC BIN_firstFunc(BIN bin);
    
    /**
     * Gets the next logical function in a BIN object
     *
     * @param bin a BIN object 
     * @param func a FUNC from binary
     * @return the logical function following func from bin, or NULL if func is the last such function
     */
    extern FUNC BIN_nextFunc(BIN bin, FUNC func);

    /**
     * Is a FUNC the last logical function in its BIN
     *
     * @param bin a BIN
     * @param func a FUNC from bin
     * @return true iff func is the last logical function in bin
     */
    extern bool BIN_isLastFunc(BIN bin, FUNC func);

    /**
     * Count the functions in a BIN
     *
     * @param bin a BIN
     * @return the number of FUNCs in bin
     */
    extern uint32_t BIN_countFunc(BIN bin);

    /**
     * Is the BIN executable
     *
     * @param bin a BIN
     * @return true iff bin is an executable file
     */
    extern bool BIN_isExecutable(BIN bin);

    /**
     * Find the file size of a BIN
     *
     * @param bin a BIN
     * @return the size of the file used to create bin
     */
    extern uint32_t BIN_fileSize(BIN bin);

    /**
     * Print a static file containing detailed information about the structures
     * found in a BIN
     *
     * @param bin a BIN
     * @param fname the name of the output file to catch static analysis
     * @return none
     */
    extern void BIN_printStaticFile(BIN bin, std::string fname);

    /**
     * Find the function at a given virtual address
     *
     * @param bin a BIN
     * @param addr a virtual address
     * @return the FUNC at addr in bin
     */
    extern FUNC BIN_findFunc(BIN bin, uint64_t addr);

    /**
     * Generate a function using the supplied bytes. Note that the size of the function
     * found may be smaller than the size of the input buffer supplied. Use FUNC_size
     * on the returned FUNC to find its size.
     *
     * @param bytes a buffer of raw instruction bytes
     * @param size the size of the buffer
     * @return a FUNC generated using the bytes supplied in buf
     */
    extern FUNC FUNC_create(uint8_t* bytes, uint32_t size);

    /**
     * Destroy a function; note that it is an error to destroy a function that was
     * not created with FUNC_create
     *
     * @param func a FUNC object that was created with FUNC_Create
     * @return none
     */
    extern void FUNC_Destroy(FUNC func);

    /**
     * Print a FUNC
     *
     * @param func a FUNC object
     * @return none
     */
    extern void FUNC_print(FUNC func);

    /**
     * Get the name of a FUNC
     *
     * @param func a FUNC object
     * @return the name of func, or NULL if no name can be found
     */
    extern std::string FUNC_name(FUNC func);

    /**
     * Get the size of a FUNC
     *
     * @param func a FUNC object
     * @return the size of func in bytes
     */
    extern uint32_t FUNC_size(FUNC func);

    /**
     * Get the virtual address of a FUNC
     * 
     * @param func a FUNC object
     * @return the virtual address of func
     */
    extern uint64_t FUNC_addr(FUNC func);

    /**
     * Get the name of the section that contains a FUNC
     *
     * @param func a FUNC object
     * @return the name of the section containing func, or NULL if it is unknown
     */
    extern std::string FUNC_secName(FUNC func);

    /**
     * Get the BIN object that contains a FUNC
     *
     * @param func a FUNC object
     * @return the BIN object associated with func
     */
    extern BIN FUNC_bin(FUNC func);
    
    /**
     * Get the number of BBL objects in a FUNC
     *
     * @param func a FUNC object
     * @return the number of BBL objects in func
     */
    extern uint32_t FUNC_countBbl(FUNC func);

    /**
     * Find the BBL within a FUNC at a given address
     *
     * @param func a FUNC object
     * @param addr a virtual address
     * @return the BBL within FUNC that intersects with addr, or NULL if no such BBL exists
     */
    extern BBL FUNC_findBbl(FUNC func, uint64_t addr);

    /**
     * Get the first BBL object in a FUNC
     *
     * @param func a FUNC object
     * @return the first BBL in func
     */
    extern BBL FUNC_firstBbl(FUNC func);

    /**
     * Get the next BBL object in a FUNC
     *
     * @param func a FUNC object
     * @param bbl a BBL object
     * @return the BBL from func that is subsequent to bbl, or NULL if no such BBL exists
     */
    extern BBL FUNC_nextBbl(FUNC func, BBL bbl);

    /**
     * Tests whether a BBL is the last in a FUNC
     *
     * @param func a FUNC object
     * @param bbl a BBL object
     * @return true iff bbl is the last BBL object in func, false otherwise
     */
    extern bool FUNC_isLastBbl(FUNC func, BBL bbl);

    /**
     * Get the number of INSNs in a FUNC
     *
     * @param func a FUNC object
     * @return the number of INSNs in a FUNC
     */
    extern uint32_t FUNC_countInsn(FUNC func);

    /**
     * Find the INSN within a FUNC at a given address
     *
     * @param func a FUNC object
     * @param addr a virtual address
     * @return the INSN within FUNC that intersects with addr, or NULL if no such INSN exists
     */
    extern INSN FUNC_findInsn(FUNC func, uint64_t addr);

    /**
     * Get the first INSN object in a FUNC
     *
     * @param func a FUNC object
     * @return the first INSN in func
     */
    extern INSN FUNC_firstInsn(FUNC func);

    /**
     * Get the next INSN object in a FUNC
     *
     * @param func a FUNC object
     * @param insn a INSN object
     * @return the INSN from func that is subsequent to insn, or NULL if no such INSN exists
     */
    extern INSN FUNC_nextInsn(FUNC func, INSN insn);

    /**
     * Tests whether a INSN is the last in a FUNC
     *
     * @param func a FUNC object
     * @param insn a INSN object
     * @return true iff insn is the last INSN object in func, false otherwise
     */
    extern bool FUNC_isLastInsn(FUNC func, INSN insn);

    /**
     * Get the CFG attached to a FUNC
     *
     * @param func a FUNC object
     * @return the CFG attached to func
     */
    extern CFG FUNC_cfg(FUNC func);

    /**
     * Get the number of targets of (functions called by) a FUNC
     *
     * @param func a FUNC object
     * @return the number of unique targets of the text of func
     */
    extern uint32_t FUNC_countTargets(FUNC func);

    /**
     * Get the unique targets of (functions called by) a FUNC
     *
     * @param func a FUNC object
     * @param (out) funcList the unique targets of func
     * @return the number of unique targets of func
     */
    extern uint32_t FUNC_targets(FUNC func, std::vector<FUNC>& funcList);

    /**
     * Count the number of loops in a CFG
     *
     * @param cfg a CFG object
     * @return the number of loops in cfg
     */
    extern uint32_t CFG_countLoop(CFG cfg);

    /**
     * Find the LOOP within a CFG ad a given address
     *
     * @param cfg a CFG object
     * @param addr a virtual address
     * @return the loop within cfg at addr, or NULL if no such loop exists
     */
    extern LOOP CFG_findLoop(CFG cfg, uint64_t addr);

    /**
     * Get the first loop in a CFG
     *
     * @param cfg a CFG object
     * @return the first loop in cfg
     */
    extern LOOP CFG_firstLoop(CFG cfg);

    /**
     * Get the next loop in a CFG
     *
     * @param cfg a CFG object
     * @param loop a LOOP object
     * @return the successor to loop within cfg, or NULL if no such LOOP exists
     */
    extern LOOP CFG_nextLoop(CFG cfg, LOOP loop);

    /**
     * Tests whether a LOOP is the last in a CFG
     *
     * @param cfg a CFG object
     * @param loop a LOOP object
     * @return true iff loop is the last LOOP in cfg
     */
    extern bool CFG_isLastLoop(CFG cfg, LOOP loop);

    /**
     * Get the CFG associated with a LOOP
     *
     * @param loop a LOOP object
     * @return the CFG associated with loop
     */
    extern CFG LOOP_cfg(LOOP loop);

    /**
     * Get the FUNC associated with a LOOP
     *
     * @param loop a LOOP object
     * @return the FUNC associated with loop
     */
    extern FUNC LOOP_func(LOOP loop);

    /**
     * Get the size of a LOOP
     *
     * @param loop a LOOP object
     * @return the size in bytes of loop
     */
    extern uint32_t LOOP_size(LOOP loop);

    /**
     * Get the number of BBL objects in a LOOP
     *
     * @param loop a LOOP object
     * @return the number of BBL objects in loop
     */
    extern uint32_t LOOP_countBbl(LOOP loop);

    /**
     * Find the BBL within a LOOP at a given address
     *
     * @param loop a LOOP object
     * @param addr a virtual address
     * @return the BBL within LOOP that intersects with addr, or NULL if no such BBL exists
     */
    extern BBL LOOP_findBbl(LOOP loop, uint64_t addr);

    /**
     * Get the first BBL object in a LOOP
     *
     * @param loop a LOOP object
     * @return the first BBL in loop
     */
    extern BBL LOOP_firstBbl(LOOP loop);

    /**
     * Get the next BBL object in a LOOP
     *
     * @param loop a LOOP object
     * @param bbl a BBL object
     * @return the BBL from loop that is subsequent to bbl, or NULL if no such BBL exists
     */
    extern BBL LOOP_nextBbl(LOOP loop, BBL bbl);

    /**
     * Tests whether a BBL is the last in a LOOP
     *
     * @param loop a LOOP object
     * @param bbl a BBL object
     * @return true iff bbl is the last BBL object in loop, false otherwise
     */
    extern bool LOOP_isLastBbl(LOOP loop, BBL bbl);

    /**
     * Get the number of INSNs in a LOOP
     *
     * @param loop a LOOP object
     * @return the number of INSNs in a LOOP
     */
    extern uint32_t LOOP_countInsn(LOOP loop);

    /**
     * Find the INSN within a LOOP at a given address
     *
     * @param loop a LOOP object
     * @param addr a virtual address
     * @return the INSN within LOOP that intersects with addr, or NULL if no such INSN exists
     */
    extern INSN LOOP_findInsn(LOOP loop, uint64_t addr);

    /**
     * Get the first INSN object in a LOOP
     *
     * @param loop a LOOP object
     * @return the first INSN in loop
     */
    extern INSN LOOP_firstInsn(LOOP loop);

    /**
     * Get the next INSN object in a LOOP
     *
     * @param loop a LOOP object
     * @param insn a INSN object
     * @return the INSN from loop that is subsequent to insn, or NULL if no such INSN exists
     */
    extern INSN LOOP_nextInsn(LOOP loop, INSN insn);

    /**
     * Tests whether a INSN is the last in a LOOP
     *
     * @param loop a LOOP object
     * @param insn a INSN object
     * @return true iff insn is the last INSN object in loop, false otherwise
     */
    extern bool LOOP_isLastInsn(LOOP loop, INSN insn);

    /**
     * Get the head basic block from a LOOP
     *
     * @param loop a LOOP object
     * @return the head (target of the back edge) BBL in loop
     */
    extern BBL LOOP_head(LOOP loop);

    /**
     * Get the tail basic block from a LOOP
     *
     * @param loop a LOOP object
     * @return the tail (source of the back edge) BBL in loop
     */
    extern BBL LOOP_tail(LOOP loop);

    /**
     * Get the number of exit points from a LOOP
     *
     * @param loop a LOOP object
     * @return the number of exit points in loop
     */
    extern uint32_t LOOP_countExits(LOOP loop);

    /**
     * Get the instructions that are exit points from a particular LOOP
     *
     * @param loop a LOOP object
     * @param (out) insnList loop's exit points
     * @return the number of exit points in loop
     */
    extern uint32_t LOOP_exits(LOOP loop, std::vector<INSN>& insnList);

    /**
     * Find out whether a LOOP is an inner loop of another LOOP
     *
     * @param loop a LOOP object
     * @param other a LOOP object
     * @return true iff loop2 is an inner loop of loop1 
     */
    extern bool LOOP_isInnerLoop(LOOP loop1, LOOP loop2);

    /**
     * Get the parent LOOP of a LOOP
     *
     * @param loop a LOOP object
     * @return the parent LOOP of loop, or NULL no such loop exists
     */
    extern LOOP LOOP_parent(LOOP loop);

    /**
     * Get the index of a LOOP
     *
     * @param loop a LOOP object
     * @return the index of loop, which is unique within the containing FUNC/CFG
     */
    extern uint32_t LOOP_index(LOOP loop);

    /**
     * Get the depth of a LOOP
     *
     * @param loop a LOOP object
     * @return the depth of loop
     */
    extern uint32_t LOOP_depth(LOOP loop);

    /**
     * Is an insn the head of a BBL
     *
     * @param bbl a BBL object
     * @param insn an INSN object
     * @return true iff insn is the head of bbl
     */
    extern bool BBL_isHead(BBL bbl, INSN insn);

    /**
     * Is an insn the tail of a BBL
     *
     * @param bbl a BBL object
     * @param insn an INSN object
     * @return true iff insn is the tail of bbl
     */
    extern bool BBL_isTail(BBL bbl, INSN insn);

    /**
     * Get the head INSN of a BBL
     *
     * @param bbl a BBL object
     * @return the head INSN of bbl
     */
    extern INSN BBL_head(BBL bbl);

    /**
     * Get the tail INSN of a BBL
     *
     * @param bbl a BBL object
     * @return the tail INSN of bbl
     */
    extern INSN BBL_tail(BBL bbl);

    /**
     * Get the function containing a BBL
     *
     * @param bbl a BBL object
     * @return the FUNC containing bbl
     */
    extern FUNC BBL_func(BBL bbl);

    /**
     * Get the loop containing a BBL
     *
     * @param bbl a BBL object
     * @return the LOOP containing bbl, of NULL if no such LOOP exists
     */
    extern LOOP BBL_loop(BBL bbl);

    /**
     * Get the size of a BBL
     *
     * @param bbl a BBL object
     * @return the size in bytes of bbl
     */
    extern uint32_t BBL_size(BBL bbl);

    /*
     * Get the virtual address of a BBL
     *
     * @param bbl a BBL object
     * @return the virtual address of bbl
     */
    extern uint64_t BBL_addr(BBL bbl);

    /**
     * Get the number of INSNs in a BBL
     *
     * @param bbl a BBL object
     * @return the number of INSNs in a BBL
     */
    extern uint32_t BBL_countInsn(BBL bbl);

    /**
     * Find the INSN within a BBL at a given address
     *
     * @param bbl a BBL object
     * @param addr a virtual address
     * @return the INSN within BBL that intersects with addr, or NULL if no such INSN exists
     */
    extern INSN BBL_findInsn(BBL bbl, uint64_t addr);

    /**
     * Get the first INSN object in a BBL
     *
     * @param bbl a BBL object
     * @return the first INSN in bbl
     */
    extern INSN BBL_firstInsn(BBL bbl);

    /**
     * Get the next INSN object in a BBL
     *
     * @param bbl a BBL object
     * @param insn a INSN object
     * @return the INSN from bbl that is subsequent to insn, or NULL if no such INSN exists
     */
    extern INSN BBL_nextInsn(BBL bbl, INSN insn);

    /**
     * Tests whether a INSN is the last in a BBL
     *
     * @param bbl a BBL object
     * @param insn a INSN object
     * @return true iff insn is the last INSN object in bbl, false otherwise
     */
    extern bool BBL_isLastInsn(BBL bbl, INSN insn);

    /**
     * Gets the number of control flow targets for a BBL
     *
     * @param bbl a BBL object
     * @return the number of BBLs that are control flow targets for bbl
     */
    extern uint32_t BBL_countTargets(BBL bbl);

    /**
     * Gets the control flow targets for a BBL
     *
     * @param bbl a BBL object
     * @param (out) the BBLs that are control flow targets for bbl
     * @return the number of BBLs that are control targets for bbl
     */
    extern uint32_t BBL_targets(BBL bbl, std::vector<BBL>& bblList);

    /**
     * Tells whether control can fall through the end of a BBL
     *
     * @param bbl a BBL object
     * @return true iff control can fall through the end of bbl
     */
    extern bool BBL_hasFallthroughTarget(BBL bbl);

    /**
     * Gets the fallthrough target for a BBL
     *
     * @param bbl a BBL object
     * @return the BBL that is the fallthrough target of bbl, or NULL if no such BBL exists
     */
    extern BBL BBL_fallthroughTarget(BBL bbl);

    /**
     * Counts the number of non-fallthrough targets for a BBL
     *
     * @param bbl a BBL object
     * @return the number of targets for bbl that are not fallthrough targets
     */
    extern uint32_t BBL_countJumpTargets(BBL bbl);

    /**
     * Gets the non-fallthrough targets for a BBL
     *
     * @param bbl a BBL object
     * @param (out) the non-fallthrough targets for bbl
     * @return the number of non-fallthrough targets for bbl
     */
    extern uint32_t BBL_jumpTargets(BBL bbl, std::vector<BBL>& bblList);

    /**
     * Counts the number of control source blocks for a BBL
     *
     * @param bbl a BBL object
     * @return the number of control source blocks for bbl
     */
    extern uint32_t BBL_countSources(BBL bbl);

    /**
     * Gets the control source blocks for a BBL
     *
     * @param bbl a BBL object
     * @param (out) bblList the control source blocks for bbl
     * @return the number of control source blocks for bbl
     */
    extern uint32_t BBL_sources(BBL bbl, std::vector<BBL>& bblList);

    /**
     * Get the control target INSNs for an INSN
     *
     * @param insn an INSN object
     * @param tlist (out) the target INSNs of insn
     * @return the number of control targets of insn
     */
    extern uint32_t INSN_targets(INSN insn, std::vector<uint64_t>& tlist);

    /**
     * Get the basic block of an INSN
     *
     * @param insn an INSN object
     * @return the BBL that contains insn, or NULL if no such BBL exists
     */
    extern BBL INSN_bbl(INSN insn);

    /**
     * Get the function of an INSN
     *
     * @param insn an INSN object
     * @return the FUNC that contains insn, or NULL if no such FUNC exists
     */
    extern FUNC INSN_func(INSN insn);

    /**
     * Get the loop of an INSN
     *
     * @param insn an INSN object
     * @return the LOOP that contains insn, or NULL if no such LOOP exists
     */
    extern LOOP INSN_loop(INSN insn);

    /**
     * Get the virtual address of an INSN
     *
     * @param insn an INSN object
     * @return the virtual address of insn
     */
    extern uint64_t INSN_addr(INSN insn);

    /**
     * Get a string representation of an INSN
     *
     * @param insn an INSN object
     * @return the decoded string representation of insn
     */
    extern std::string INSN_string(INSN insn);

    /**
     * Get the call target of an INSN
     *
     * @param insn an INSN object
     * @return the address of the call target of insn, or 0 if the target cannot be found
     */
    extern uint64_t INSN_callTarget(INSN insn);

    /**
     * Is an INSN a branch
     *
     * @param insn an INSN object
     * @return true iff insn is a branch instruction of any kind
     */
    extern bool INSN_isBranch(INSN insn);

    /**
     * Is an INSN an fp op
     * @param insn an INSN object
     * @return true iff either source or destination operands is fp data
     */
    extern bool INSN_isFpop(INSN insn);

    /**
     * Is an INSN a mem op
     * @param insn an INSN object
     * @return true iff the insns touches memory
     */
    extern bool INSN_isMemop(INSN insn);

    /**
     * Get the size in of an INSN in bytes
     *
     * @param insn an INSN object
     * @return the size (in bytes) of insn
     */
    extern uint32_t INSN_size(INSN insn);

    /**
     * Get the string rep of the predicate condition of an INSN
     *
     * @param insn an INSN object
     * @return the string representation of the predicate condition of insn
     */
    extern std::string INSN_condName(INSN insn);

    /**
     * Can control fall through an INSN
     *
     * @param insn an INSN object
     * @return true iff control can fall through insn
     */
    extern bool INSN_fallsThrough(INSN insn);

    /**
     * Size of a source register in bits
     *
     * @param insn an INSN object
     * @return the number of bits in a source register
     */
    extern uint32_t INSN_sourceRegisterSizeInBits(INSN insn);

    /**
     * Size of source datatype in bits
     *
     * @param insn an INSN object
     * @return the number of bits in a source operand
     */
    extern uint32_t INSN_sourceDatatypeSizeInBits(INSN insn);

    /*
    // TODO: documentation/implementation needed starting here

    typedef void InstructionMode;
    extern InstructionMode INSN_mode(INSN insn); // regular, thumb, jazelle, etc
    typedef void InstructionType;
    extern InstructionType INSN_type(INSN insn); // data processing, ld/st, etc

    extern bool INSN_readsMem(INSN insn);
    extern bool INSN_writesMem(INSN insn);
    extern std::string INSN_mnemonic(INSN insn);
    */

} // namespace EPAX

#endif // __EPAX_Interface_hpp__
