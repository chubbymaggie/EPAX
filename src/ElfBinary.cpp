/**
 * @file ElfBinary.cpp
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

#include "Elf/elf.h"

#include "Binary.hpp"
#include "ElfBinary.hpp"
#include "Function.hpp"
#include "InputFile.hpp"

namespace EPAX {

    namespace Elf {

#define EHDR32_ENTRY ((Elf32_Ehdr*)entry)
#define EHDR64_ENTRY ((Elf64_Ehdr*)entry)
#define SYM32_ENTRY ((Elf32_Sym*)entry)
#define SYM64_ENTRY ((Elf64_Sym*)entry)
#define SHDR32_ENTRY ((Elf32_Shdr*)entry)
#define SHDR64_ENTRY ((Elf64_Shdr*)entry)
#define PHDR32_ENTRY ((Elf32_Phdr*)entry)
#define PHDR64_ENTRY ((Elf64_Phdr*)entry)

        ElfBinary::ElfBinary(std::string n)
            : BaseBinary(n),
              fileheader(INVALID_PTR),
              foundsections(false), sections(INVALID_PTR),
              foundsegments(false), segments(INVALID_PTR)
        {
        }

        ElfBinary::~ElfBinary(){
            if (IS_VALID_PTR(fileheader)){
                delete fileheader;
            }

            if (IS_VALID_PTR(sections)){
                while (sections->size()){
                    delete sections->back();
                    sections->pop_back();
                }
                delete sections;
            }

	    if (IS_VALID_PTR(segments)){
                while (segments->size()){
                    delete segments->back();
                    segments->pop_back();
                }
                delete segments;
	    }

        }

        bool ElfBinary::insideTextRange(uint64_t a){
            for (std::vector<SectionHeader*>::const_iterator it = sections->begin(); it != sections->end(); it++){
                SectionHeader* shdr = (*it);
                if (shdr->isText() && shdr->inRange(a)){
                    return true;
                }
            }
            return false;
        }

        uint64_t ElfBinary::functionEndAddress(Function* f, Function* nextf){
            uint64_t addr = INVALID_ADDRESS;

            ElfSymbol* s = (ElfSymbol*)f->getSymbol();
            if (IS_VALID_PTR(s)){
                if (s->getSection() < sections->size()){
                    SectionHeader* sh = (*sections)[s->getSection()];
                    addr = sh->getFileOffset() + sh->getSize();
                }
            }
            
            if (IS_VALID_PTR(nextf)){
                if (nextf->getFileOffset() < addr){
                    addr = nextf->getFileOffset();
                }
            }

            return addr;
        }

        uint64_t ElfBinary::getStartAddr(){
            return fileheader->getStartAddr();
        }

        bool ElfBinary::verify(){
            return fileheader->verify();
        }

        bool ElfBinary::isARM(){
            return fileheader->isARM();
        }

        void ElfBinary::describe(){
            fileheader->describe();
        }

        ElfBinary32::ElfBinary32(std::string n)
            : ElfBinary(n)
        {
            fileheader = new FileHeader32(this, 0);
        }

        ElfBinary64::ElfBinary64(std::string n)
            : ElfBinary(n)
        {
            fileheader = new FileHeader64(this, 0);
        }

        bool ElfBinary::is32Bit(){
            return (getFormat() == BinaryFormat_Elf32);
        }

        bool ElfBinary::is64Bit(){
            return (getFormat() == BinaryFormat_Elf64);
        }

        void ElfBinary::emit(std::string n){
            __do_not_call__;
        }

        uint64_t ElfBinary::vaddrToFile(uint64_t v){
            for (std::vector<ProgramHeader*>::const_iterator it = segments->begin(); it != segments->end(); it++){
                ProgramHeader* h = (ProgramHeader*)(*it);
                if (h->isValidVaddr(v)){
                    return h->vaddrToFileaddr(v);
                }
            }
            return 0;
        }

        void ElfBinary::findFunctions(){
            EPAXAssert(!foundfunctions, "this function should only be called once per binary");
            if (foundfunctions){
                return;
            }

            findSymbols();

            functions = new std::vector<Function*>();
            foundfunctions = true;

            uint32_t cur = 0;
            for (std::vector<SymbolTable*>::const_iterator it = symtabs->begin(); it != symtabs->end(); it++){
                ElfSymbolTable* symt = (ElfSymbolTable*)(*it);
                for (uint32_t i = 0; i < symt->countSymbols(); i++){
                    ElfSymbol* s = (ElfSymbol*)symt->getSymbol(i);
                    if (s->isFunction() && insideTextRange(s->getFunctionAddress())){
                        functions->push_back(new Function(this, vaddrToFile(s->getFunctionAddress()), s->getSize(), s->getFunctionAddress(), cur, s, fileheader->getBits() == 64));
                        cur++;
                    }
                }
            }

            cur = 0;
            std::sort(functions->begin(), functions->end(), compareMemory);
            for (std::vector<Function*>::const_iterator it = functions->begin(); it != functions->end(); it++){
                Function* f = (*it);
                f->setIndex(cur++);
            }

            for (uint32_t i = 0; i < functions->size(); i++){
                Function* f = (*functions)[i];
                uint64_t end_addr = INVALID_ADDRESS;
                BaseBinary* b = f->getBinary();
                if (IS_VALID_PTR(b)){
                    end_addr = b->functionEndAddress(f, i+1 < functions->size()? (*functions)[i+1]:INVALID_PTR);
                }
                //EPAXAssert(end_addr != INVALID_ADDRESS, "No end address for function " << f->getName() << " found");
                if (INVALID_ADDRESS != end_addr){
                    f->setMemorySize(end_addr - f->getFileOffset());
                }
            }

            for (std::vector<Function*>::const_iterator it = functions->begin(); it != functions->end(); it++){
                Function* f = (*it);
                f->disassemble();
            }


            Function* prev = INVALID_PTR;
            for (std::vector<Function*>::const_iterator it = functions->begin(); it != functions->end(); it++){
                Function* f = *it;
                if (IS_VALID_PTR(prev)){
                    EPAXAssert(prev->getMemoryAddress() < f->getMemoryAddress(), "Functions should be sorted");
                }
            }
        }

        void ElfBinary::printFunctions(std::ostream& stream){
            stream << ENDL;
            stream << "Printing all functions in " << getName() << ENDL;
            Function::printHeader();
            for (std::vector<Function*>::const_iterator it = functions->begin(); it != functions->end(); it++){
                Function* f = (*it);
                f->print(stream);
            }
        }

        ElfStringTable* ElfBinary::findStringtable(uint32_t i){
            for (std::vector<StringTable*>::const_iterator it = strtabs->begin(); it != strtabs->end(); it++){
                ElfStringTable* s = (ElfStringTable*)(*it);
                if (s->getIndex() == i){
                    return s;
                }
            }            
            return INVALID_PTR;
        }

        void ElfBinary::findSymbols(){
            EPAXAssert(!foundsymbols, "this function should only be called once per binary");
            if (foundsymbols){
                return;
            }

            symtabs = new std::vector<SymbolTable*>();
            strtabs = new std::vector<StringTable*>();
            foundsymbols = true;

            findSections();
            findSegments();

            // find string tables
            uint32_t cur = 0;
            for (std::vector<SectionHeader*>::const_iterator it = sections->begin(); it != sections->end(); it++){
                SectionHeader* h = (SectionHeader*)(*it);
                if (h->isString()){
                    ElfStringTable* st = new ElfStringTable(this, h->getFileOffset(), h->getSize(), h->getVirtAddr(), h->getSize(), cur, h->getName());
                    //st->print();
                    strtabs->push_back(st);
                }
                cur++;
            }

            ElfStringTable* shdrtab = findStringtable(fileheader->getShdrStringIndex());
            if (IS_VALID_PTR(shdrtab)){
                for (std::vector<SectionHeader*>::const_iterator it = sections->begin(); it != sections->end(); it++){
                    SectionHeader* h = (SectionHeader*)(*it);
                    h->setName(shdrtab->getStringAt(h->getNameIndex()));
                }
            }
            //printSections();

            // find symbol tables
            cur = 0;
            for (std::vector<SectionHeader*>::const_iterator it = sections->begin(); it != sections->end(); it++){
                SectionHeader* h = (SectionHeader*)(*it);
                if (h->isSymbol()){
                    ElfStringTable* st = findStringtable(h->getSectionLink());

                    symtabs->push_back(new ElfSymbolTable(this, h->getFileOffset(), h->getSize(), h->getVirtAddr(), h->getSize(), cur, h->getName(), st));
                    //symtabs->back()->print();
                }
                cur++;
            }
        }

        void ElfBinary::findSections(){
            EPAXAssert(!foundsections, "this function should only be called once per binary");
            if (foundsections){
                return;
            }

            sections = new std::vector<SectionHeader*>();
            foundsections = true;

            uint64_t off = fileheader->getSecTableOffset();
            uint32_t cnt = fileheader->getSectionCount();
            uint64_t sz = fileheader->getShdrSize();
            for (uint32_t i = 0; i < cnt; i++){
                if (is32Bit()){
                    sections->push_back(new SectionHeader32(this, off + (i * sz), sz, i));
                } else {
                    sections->push_back(new SectionHeader64(this, off + (i * sz), sz, i));
                }
            }
        }

        void ElfBinary::findSegments(){
            EPAXAssert(!foundsegments, "this function should only be called once per binary");

            if (foundsegments){
                return;
            }

            segments = new std::vector<ProgramHeader*>();
            foundsegments = true;

            uint64_t off = fileheader->getSegTableOffset();
            uint32_t cnt = fileheader->getSegmentCount();
            uint64_t sz = fileheader->getPhdrSize();
            for (uint32_t i = 0; i < cnt; i++){
                if (is32Bit()){
                    segments->push_back(new ProgramHeader32(this, off + (i * sz), sz, i));
                } else {
                    segments->push_back(new ProgramHeader64(this, off + (i * sz), sz, i));
                }
            }
        }

        FileHeader::FileHeader(BaseBinary* b, uint64_t o, uint64_t s)
            : FileBase(b, 0, s),
              entry(INVALID_PTR)
        {
        }

        FileHeader::~FileHeader(){
            if (IS_VALID_PTR(entry)){
                delete entry;
            }
        }

        uint64_t FileHeader32::getSecTableOffset(){
            return EHDR32_ENTRY->e_shoff;
        }

        uint64_t FileHeader64::getSecTableOffset(){
            return EHDR64_ENTRY->e_shoff;
        }

        uint32_t FileHeader32::getShdrSize(){
            return EHDR32_ENTRY->e_shentsize;
        }

        uint32_t FileHeader64::getShdrSize(){
            return EHDR64_ENTRY->e_shentsize;
        }

        uint64_t FileHeader32::getSegTableOffset(){
            return EHDR32_ENTRY->e_phoff;
        }

        uint64_t FileHeader64::getSegTableOffset(){
            return EHDR64_ENTRY->e_phoff;
        }

        uint32_t FileHeader32::getPhdrSize(){
            return EHDR32_ENTRY->e_phentsize;
        }

        uint32_t FileHeader64::getPhdrSize(){
            return EHDR64_ENTRY->e_phentsize;
        }

        uint32_t FileHeader32::getShdrStringIndex(){
            return EHDR32_ENTRY->e_shstrndx;
        }

        uint32_t FileHeader64::getShdrStringIndex(){
            return EHDR64_ENTRY->e_shstrndx;
        }

        uint32_t FileHeader32::getSectionCount(){
            return EHDR32_ENTRY->e_shnum;
        }

        uint32_t FileHeader64::getSectionCount(){
            return EHDR64_ENTRY->e_shnum;
        }

        uint32_t FileHeader32::getSegmentCount(){
            return EHDR32_ENTRY->e_phnum;
        }

        uint32_t FileHeader64::getSegmentCount(){
            return EHDR64_ENTRY->e_phnum;
        }

        uint32_t FileHeader32::getFileType(){
            return EHDR32_ENTRY->e_type;
        }

        uint32_t FileHeader64::getFileType(){
            return EHDR64_ENTRY->e_type;
        }

        FileHeader32::FileHeader32(BaseBinary* b, uint64_t o)
            : FileHeader(b, o, sizeof(Elf32_Ehdr))
        {
            entry = (rawbyte_t*)new Elf32_Ehdr();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        FileHeader64::FileHeader64(BaseBinary* b, uint64_t o)
            : FileHeader(b, o, sizeof(Elf64_Ehdr))
        {
            entry = (rawbyte_t*)new Elf64_Ehdr();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        uint64_t FileHeader32::getStartAddr(){
            return EHDR32_ENTRY->e_entry;
        }

        uint64_t FileHeader64::getStartAddr(){
            return EHDR64_ENTRY->e_entry;
        }

        bool FileHeader32::verify(){
            if (EHDR32_ENTRY->e_ident[EI_MAG0] == ELFMAG0 &&
                EHDR32_ENTRY->e_ident[EI_MAG1] == ELFMAG1 &&
                EHDR32_ENTRY->e_ident[EI_MAG2] == ELFMAG2 &&
                EHDR32_ENTRY->e_ident[EI_MAG3] == ELFMAG3){

                if (EHDR32_ENTRY->e_ident[EI_CLASS] == ELFCLASS32){
                    return true;
                }
            }

            return false;
        }

        bool FileHeader64::verify(){
            if (EHDR64_ENTRY->e_ident[EI_MAG0] == ELFMAG0 &&
                EHDR64_ENTRY->e_ident[EI_MAG1] == ELFMAG1 &&
                EHDR64_ENTRY->e_ident[EI_MAG2] == ELFMAG2 &&
                EHDR64_ENTRY->e_ident[EI_MAG3] == ELFMAG3){

                if (EHDR64_ENTRY->e_ident[EI_CLASS] == ELFCLASS64){
                    return true;
                }
            }

            return false;
        }

        bool FileHeader32::isARM(){
            return (EHDR32_ENTRY->e_machine == EM_ARM); 
        }

        bool FileHeader64::isARM(){
            // TODO: have a good idea, but not totally sure what ARM64 looks like here
            return false;
        }

        bool ElfBinary::isExecutable(){
            return (fileheader->getFileType() == ET_EXEC);
        }

        void FileHeader::describe(){
            EPAXOut << "format=elf" << TAB;
            std::cout << "bits=" << DEC(getBits()) << TAB;
            switch(getISA()){
#define CASE(__typ__) case EM_ ## __typ__: std::cout << "isa=" << #__typ__ ; break
                CASE(SPARC);
                CASE(386);
                CASE(MIPS);
                CASE(PPC);
                CASE(PPC64);
                CASE(ARM);
                CASE(IA_64);
                CASE(X86_64);
                CASE(VAX);
                CASE(AARCH64);
            default:
                std::cout << "isa=unknown";
                EPAXAssert(false, "Unkown ISA found: " << std::dec << getISA());
            }
            std::cout << ENDL;
        }

        uint32_t FileHeader32::getISA(){
            return EHDR32_ENTRY->e_machine;
        }

        uint32_t FileHeader64::getISA(){
            return EHDR64_ENTRY->e_machine;
        }

        uint32_t FileHeader32::getBits(){
            return 32;
        }

        uint32_t FileHeader64::getBits(){
            return 64;
        }

        ElfSymbol::ElfSymbol(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : Symbol(b, o, s, i),
              entry(INVALID_PTR)
        {
        }

        ElfSymbol::~ElfSymbol(){
            if (IS_VALID_PTR(entry)){
                delete entry;
            }
        }

        void ElfStringTable::print(std::ostream& stream){
            stream << "ElfStringTable scn=" << DEC(getIndex()) << ENDL;
            uint32_t cur = 1;
            while (cur < getFileSize()){
                char* t = getStringAt(cur);
                stream << TAB << "[" << DEC(cur) << "]" << TAB << t << ENDL;
                cur += strlen(t) + 1;
            }
        }

        void ElfSymbolTable::print(std::ostream& stream){
            stream << "ElfSymbolTable scn=" << DEC(getIndex()) << " count=" << DEC(countSymbols()) << ENDL;
            for (std::vector<Symbol*>::const_iterator it = symbols->begin(); it != symbols->end(); it++){
                ElfSymbol* sym = (ElfSymbol*)(*it);
                sym->print(stream);
            }
        }

        void ElfSymbol::print(std::ostream& stream){
            stream << TAB "[" << DEC(getIndex()) << "]";

            if (isFunction()){
                if (isThumbFunction()){
                    stream << "*T";
                } else {
                    stream << "*A";
                }
            }
            stream << TAB "val=" << HEX(getValue())
                      << TAB "size=" << DEC(getSize())
                      << TAB "name=" << getName()
                      << ENDL;
        }

        ElfSymbol32::ElfSymbol32(BaseBinary* b, uint64_t o, uint32_t i)
            : ElfSymbol(b, o, sizeof(Elf32_Sym), i)
        {
            entry = (rawbyte_t*)new Elf32_Sym();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        ElfSymbol64::ElfSymbol64(BaseBinary* b, uint64_t o, uint32_t i)
            : ElfSymbol(b, o, sizeof(Elf64_Sym), i)
        {
            entry = (rawbyte_t*)new Elf64_Sym();
            getInputFile()->getBytes(o, getFileSize(), entry);
        }

        uint64_t ElfSymbol32::getNameIndex(){
            return SYM32_ENTRY->st_name;
        }

        uint64_t ElfSymbol64::getNameIndex(){
            return SYM64_ENTRY->st_name;
        }

        uint64_t ElfSymbol32::getValue(){
            return SYM32_ENTRY->st_value;
        }

        uint64_t ElfSymbol64::getValue(){
            return SYM64_ENTRY->st_value;
        }

        uint32_t ElfSymbol32::getSize(){
            return SYM32_ENTRY->st_size;
        }

        uint32_t ElfSymbol64::getSize(){
            return SYM64_ENTRY->st_size;
        }

        uint32_t ElfSymbol32::getSection(){
            return SYM32_ENTRY->st_shndx;
        }

        uint32_t ElfSymbol64::getSection(){
            return SYM64_ENTRY->st_shndx;
        }

        uint32_t ElfSymbol32::getType(){
            return ELF32_ST_TYPE(SYM32_ENTRY->st_info);
        }

        uint32_t ElfSymbol64::getType(){
            return ELF64_ST_TYPE(SYM64_ENTRY->st_info);
        }

        uint32_t ElfSymbol32::getBinding(){
            return ELF32_ST_BIND(SYM32_ENTRY->st_info);
        }

        uint32_t ElfSymbol64::getBinding(){
            return ELF64_ST_BIND(SYM64_ENTRY->st_info);
        }

        uint64_t ElfSymbol32::getVisibility(){
            return SYM32_ENTRY->st_other;
        }

        uint64_t ElfSymbol64::getVisibility(){
            return SYM64_ENTRY->st_other;
        }

        bool ElfSymbol::isFunction(){
            return (getType() == STT_FUNC);
        }

        bool ElfSymbol::isThumbFunction(){
            return (isFunction() && ADDRESS_IS_THUMB(getValue()));
        }

        uint64_t ElfSymbol::getFunctionAddress(){
            EPAXAssert(isFunction(), "This may only call this for function symbols");
            if (isThumbFunction()){
                return getValue() - 1;
            } else {
                return getValue();
            }
        }

        ElfStringTable::ElfStringTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n)
            : StringTable(b, o, fs, ma, ms, i, n),
              entry(INVALID_PTR)
        {
            entry = new rawbyte_t[getFileSize()];
            bzero(entry, getFileSize());
            getInputFile()->getBytes(getFileOffset(), getFileSize(), entry);
        }

        ElfStringTable::~ElfStringTable(){
            if (IS_VALID_PTR(entry)){
                delete[] entry;
            }
        }

        char* ElfStringTable::getStringAt(uint32_t i){
            EPAXAssert(i < getFileSize(), "StringTable index (" << DEC(i) << ") out of bounds (" << DEC(getFileSize()) << ")");
            return (char*)&(entry[i]);
        }

        ElfSymbolTable::ElfSymbolTable(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n, ElfStringTable* st)
            : SymbolTable(b, o, fs, ma, ms, i, n),
              stringtab(NULL)
        {
            stringtab = st;
            EPAXAssert(IS_VALID_PTR(stringtab), "A symbol table in ELF requires a valid string table");

            uint32_t tablesize = 0;
            symbols = new std::vector<Symbol*>();

            uint32_t cur = 0;
            while (tablesize < getFileSize()){
                ElfSymbol* e;
                if (is32Bit()){
                    e = new ElfSymbol32(getBinary(), getFileOffset() + tablesize, cur);
                } else {
                    e = new ElfSymbol64(getBinary(), getFileOffset() + tablesize, cur);
                }
                tablesize += e->getFileSize();
                e->setName(st->getStringAt(e->getNameIndex()));

                symbols->push_back(e);
                cur++;
            }

            EPAXAssert(tablesize == getFileSize(), "Symbol table size (" << DEC(tablesize) << ") does not match expected size (" << DEC(getFileSize()) << ")");
        }

        SectionHeader::SectionHeader(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : FileBase(b, o, s),
              NameBase(),
              IndexBase(i),
              entry(INVALID_PTR)
        {
        }

        SectionHeader::~SectionHeader(){
            if (IS_VALID_PTR(entry)){
                delete entry;
            }
        }

        ProgramHeader::ProgramHeader(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : FileBase(b, o, s),
              IndexBase(i),
              entry(INVALID_PTR)
        {
        }

        ProgramHeader::~ProgramHeader(){
            if (IS_VALID_PTR(entry)){
                delete entry;
            }
        }

        void ElfBinary::printSections(std::ostream& stream){
            stream << ENDL;
            stream << "Printing all Section Headers in " << getName() << ENDL;
            stream << "ELF_SHDR [" << std::setw(2) << "ID" << "]"
                      << TAB << std::setw(18) << "NAME"
                      << TAB << "VIRTADDR"
                      << TAB << "MEM_SIZE"
                      << TAB << "FLOFFSET"
                      << TAB << "ALGN"
                      << TAB << "FLAGS"
                      << TAB << "SECTYP"
                      << ENDL;
            if (IS_VALID_PTR(sections)){
                for (std::vector<SectionHeader*>::const_iterator it = sections->begin(); it != sections->end(); it++){
                    SectionHeader* shdr = (*it);
                    shdr->print(stream);
                }
            }
        }

        void SectionHeader::print(std::ostream& stream){
            stream << "ELF_SHDR [" << std::setw(2) << DEC(getIndex()) << "]"
                      << TAB << std::setw(18) << getName() 
                      << TAB << HEX(getVirtAddr())
                      << TAB << HEX(getSize())
                      << TAB << HEX(getFileOffset())
                      << TAB << std::setw(4) << DEC(getAlignment())
                      << TAB << (isRead()? "R":" ") << (isWrite()? "W":" ") << (isExec()? "X":" ") << (isAlloc()? "A":" ") << (isMerge()? "M":" ")
                      << TAB << (isText()? "T":" ") << (isData()? "D":" ") << (isBSS()? "B":" ") << (isDebug()? "D":" ") << (isString()? "R":" ") << (isSymbol()? "Y":" ")
                      << ENDL;
        }

        bool SectionHeader::inRange(uint64_t a){
            if (a >= getVirtAddr() && a < getVirtAddr() + getSize()){
                return true;
            }
            return false;
        }

        bool SectionHeader::isText(){
            return (getType() == SHT_PROGBITS && isExec());
        }

        bool SectionHeader::isData(){
            return (getType() == SHT_PROGBITS && isWrite());
        }

        bool SectionHeader::isBSS(){
            return (getType() == SHT_NOBITS && isWrite());
        }

        bool SectionHeader::isDebug(){
            return (getType() == SHT_PROGBITS && !isAlloc() && !isWrite() && !isExec());
        }

        bool SectionHeader::isString(){
            return (getType() == SHT_STRTAB);
        }

        bool SectionHeader::isSymbol(){
            return (getType() == SHT_SYMTAB || getType() == SHT_DYNSYM);
        }

        bool SectionHeader::isRead(){
            return true;
        }

        bool SectionHeader::isWrite(){
            return ((getFlags() & SHF_WRITE) != 0);
        }

        bool SectionHeader::isExec(){
            return ((getFlags() & SHF_EXECINSTR) != 0);
        }

        bool SectionHeader::isAlloc(){
            return ((getFlags() & SHF_ALLOC) != 0);
        }

        bool SectionHeader::isMerge(){
            return ((getFlags() & SHF_MERGE) != 0);
        }

        SectionHeader32::SectionHeader32(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : SectionHeader(b, o, s, i)
        {
            entry = (rawbyte_t*)new Elf32_Shdr();
            getInputFile()->getBytes(o, sizeof(Elf32_Shdr), entry);
        }

        SectionHeader64::SectionHeader64(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : SectionHeader(b, o, s, i)
        {
            entry = (rawbyte_t*)new Elf64_Shdr();
            getInputFile()->getBytes(o, sizeof(Elf64_Shdr), entry);
        }

        uint64_t SectionHeader32::getNameIndex(){
            return SHDR32_ENTRY->sh_name;
        }

        uint64_t SectionHeader64::getNameIndex(){
            return SHDR64_ENTRY->sh_name;
        }

        uint64_t SectionHeader32::getType(){
            return SHDR32_ENTRY->sh_type;
        }

        uint64_t SectionHeader64::getType(){
            return SHDR64_ENTRY->sh_type;
        }

        uint64_t SectionHeader32::getSize(){
            return SHDR32_ENTRY->sh_size;
        }

        uint64_t SectionHeader64::getSize(){
            return SHDR64_ENTRY->sh_size;
        }

        uint64_t SectionHeader32::getVirtAddr(){
            return SHDR32_ENTRY->sh_addr;
        }

        uint64_t SectionHeader64::getVirtAddr(){
            return SHDR64_ENTRY->sh_addr;
        }

        uint64_t SectionHeader32::getFileOffset(){
            return SHDR32_ENTRY->sh_offset;
        }

        uint64_t SectionHeader64::getFileOffset(){
            return SHDR64_ENTRY->sh_offset;
        }

        uint64_t SectionHeader32::getSectionLink(){
            return SHDR32_ENTRY->sh_link;
        }

        uint64_t SectionHeader64::getSectionLink(){
            return SHDR64_ENTRY->sh_link;
        }

        uint64_t SectionHeader32::getAlignment(){
            return SHDR32_ENTRY->sh_addralign;
        }

        uint64_t SectionHeader64::getAlignment(){
            return SHDR64_ENTRY->sh_addralign;
        }

        uint64_t SectionHeader32::getEntrySize(){
            return SHDR32_ENTRY->sh_entsize;
        }

        uint64_t SectionHeader64::getEntrySize(){
            return SHDR64_ENTRY->sh_entsize;
        }

        uint64_t SectionHeader32::getFlags(){
            return SHDR32_ENTRY->sh_flags;
        }

        uint64_t SectionHeader64::getFlags(){
            return SHDR64_ENTRY->sh_flags;
        }        

        bool ProgramHeader::isValidVaddr(uint64_t v){
            return (v >= getVaddr() && v < getVaddr() + getMSize());
        }

        uint64_t ProgramHeader::vaddrToFileaddr(uint64_t v){
            if (!isValidVaddr(v)){
                return 0;
            }
            return v - getVaddr() + getFOffset();
        }

        ProgramHeader32::ProgramHeader32(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : ProgramHeader(b, o, s, i)
        {
            entry = (rawbyte_t*)new Elf32_Phdr();
            getInputFile()->getBytes(o, sizeof(Elf32_Phdr), entry);
        }

        ProgramHeader64::ProgramHeader64(BaseBinary* b, uint64_t o, uint64_t s, uint32_t i)
            : ProgramHeader(b, o, s, i)
        {
            entry = (rawbyte_t*)new Elf64_Phdr();
            getInputFile()->getBytes(o, sizeof(Elf64_Phdr), entry);
        }

        uint64_t ProgramHeader32::getVaddr(){
            return PHDR32_ENTRY->p_vaddr;
        }

        uint64_t ProgramHeader64::getVaddr(){
            return PHDR64_ENTRY->p_vaddr;
        }

        uint64_t ProgramHeader32::getPaddr(){
            return PHDR32_ENTRY->p_paddr;
        }

        uint64_t ProgramHeader64::getPaddr(){
            return PHDR64_ENTRY->p_paddr;
        }

        uint64_t ProgramHeader32::getFSize(){
            return PHDR32_ENTRY->p_filesz;
        }

        uint64_t ProgramHeader64::getFSize(){
            return PHDR64_ENTRY->p_filesz;
        }

        uint64_t ProgramHeader32::getMSize(){
            return PHDR32_ENTRY->p_memsz;
        }

        uint64_t ProgramHeader64::getMSize(){
            return PHDR64_ENTRY->p_memsz;
        }

        uint64_t ProgramHeader32::getFlags(){
            return PHDR32_ENTRY->p_flags;
        }

        uint64_t ProgramHeader64::getFlags(){
            return PHDR64_ENTRY->p_flags;
        }

        uint32_t ProgramHeader32::getAlignment(){
            return PHDR32_ENTRY->p_align;
        }

        uint32_t ProgramHeader64::getAlignment(){
            return PHDR64_ENTRY->p_align;
        }

        uint32_t ProgramHeader32::getSegmentType(){
            return PHDR32_ENTRY->p_type;
        }

        uint32_t ProgramHeader64::getSegmentType(){
            return PHDR64_ENTRY->p_type;
        }

        uint64_t ProgramHeader32::getFOffset(){
            return PHDR32_ENTRY->p_offset;
        }

        uint64_t ProgramHeader64::getFOffset(){
            return PHDR64_ENTRY->p_offset;
        }

    } // namespace Elf

} // namespace EPAX
