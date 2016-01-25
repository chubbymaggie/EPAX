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
    
    if (argc != 2){
        error_out(argv[0], "exactly one argument (a path to an executable/library) is required");
    } else {
        fname = argv[1];
    }

    // create a BIN
    EPAX::BIN mybin = EPAX::BIN_create(fname);

    // print out static analysis of the BIN to a file
    std::string sfname(fname);
    sfname.append(".static");
    EPAX::BIN_printStaticFile(mybin, sfname.c_str());

    // destroy the BIN
    EPAX::BIN_destroy(mybin);
}
