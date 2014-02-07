/**
 * @file InputFile.hpp
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

#ifndef __EPAX_InputFile_hpp__
#define __EPAX_InputFile_hpp__

#include "BaseClass.hpp"

namespace EPAX {

    class InputFile : public NameBase {
    private:
        std::ifstream handle;

    public:
        InputFile(std::string n);
        virtual ~InputFile();

        uint64_t getBytes(uint64_t offset, uint64_t size, rawbyte_t* buffer);
        uint64_t getFileSize();
    }; // class BinaryInputFile

} // namespace EPAX

#endif // __EPAX_InputFile_hpp__
