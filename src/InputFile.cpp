/**
 * @file InputFile.cpp
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
#include "InputFile.hpp"

namespace EPAX {

    InputFile::InputFile(std::string n)
        : NameBase(n)
    {
        handle.open(getName().c_str(), std::ios::in | std::ios::binary);

        EPAXAssert(handle.is_open(), getName() << " is not a valid file.");
    }

    InputFile::~InputFile(){
        if (handle.is_open()){
            handle.close();
        }
    }

    uint64_t InputFile::getFileSize(){
        handle.seekg(0, std::ios::beg);
        uint64_t b = handle.tellg();

        handle.seekg(0, std::ios::end);
        uint64_t e = handle.tellg();

        return (e - b);
    }

    uint64_t InputFile::getBytes(uint64_t offset, uint64_t size, rawbyte_t* buffer){
        std::streampos pos(offset);
        if (handle.tellg() != pos){
            handle.seekg(offset);
        }
        handle.read(buffer, size);
        EPAXAssert(!handle.fail(), "Cannot read byte range [" << std::dec << offset << "," << (offset + size) << ") in " << getName() << ".");
        return size;
    }

} // namespace EPAX
