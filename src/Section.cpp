/**
 * @file Section.cpp
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

#include "Section.hpp"

namespace EPAX {

    Section::Section(BaseBinary* b, uint64_t o, uint64_t fs, uint64_t ma, uint64_t ms, uint32_t i, std::string n)
        : FileBase(b, o, fs),
          MemoryBase(ma, ms),
          IndexBase(i),
          NameBase(n),
          EPAXExport(EPAXExportClass_SECT)
    {
    }
    
    void Section::print(std::ostream& stream){
        __do_not_call__;
    }

} // namespace EPAX
