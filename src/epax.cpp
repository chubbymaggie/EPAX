/**
 * @file EPAX.cpp
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

#include "EPAX.hpp"
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <fstream>

void error_out(char* prg, const char* msg){
    std::cerr << "error: " << msg << std::endl << std::endl;
    std::cerr << "usage: " << prg << " <path_to_executable> [<arg1> [<arg2>] ...]" << std::endl;
    exit(1);
}

int main(int argc, char** argv){

    const char* fname;
    char** args = NULL;
    
    if (argc < 2){
        error_out(argv[0], "at least one argument (a path to an executable) is required");
    } else {
        fname = argv[1];
    }

    args = &(argv[1]);

    EPAX::BIN mybin = EPAX::BIN_create(fname);

    std::string sfname(fname);
    sfname.append(".static");

    for (EPAX::FUNC f = BIN_firstFunc(mybin); EPAX::BIN_isLastFunc(mybin, f) == false; f = EPAX::BIN_nextFunc(mybin, f)){
        //FUNC_print(f);
    }

    EPAX::BIN_printStaticFile(mybin, sfname.c_str());
    if (false && EPAX::BIN_isExecutable(mybin)){
        EPAX::BIN_run(mybin, argc - 1, args); // does not return
        assert(false && "should not arrive");
    }
    EPAX::BIN_destroy(mybin);
}
