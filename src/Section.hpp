/**
 * @file Section.hpp
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


#ifndef __EPAX_Section_hpp__
#define __EPAX_Section_hpp__

#include "BaseClass.hpp"

namespace EPAX {

    class BinaryInputFile;

    class Section : public FileBase, public MemoryBase, public IndexBase, public NameBase, public EPAXExport {
    public:
        Section(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n);
        virtual ~Section() {}

        virtual void print(std::ostream& stream = std::cout);

        virtual bool isText() { return false; }
        virtual bool isData() { return false; }
        virtual bool isBSS() { return false; }
        virtual bool isDebug() { return false; }
        virtual bool isString() { return false; }
        virtual bool isSymbol() { return false; }

    }; // class Section

} // namespace EPAX

#endif // __EPAX_Section_hpp__

