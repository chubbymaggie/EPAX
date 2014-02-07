/**
 * @file Binary.hpp
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


#ifndef __EPAX_Binary_hpp__
#define __EPAX_Binary_hpp__

namespace EPAX {

    /**
     * Binary format
     */
    typedef enum {
        BinaryFormat_undefined = 0,
        BinaryFormat_Elf32,
        BinaryFormat_Elf64,
        BinaryFormat_MachO32,
        BinaryFormat_MachO64,
        BinaryFormat_PE,
        BinaryFormat_total
    } BinaryFormat;

    class BaseBinary;
    class Function;

    /**
     * A thin wrapper around the classes which will hold all of the useful information about program binaryies.
     * The idea is that this class will provide a single interface on top of any number of different formats.
     */
    class Binary : public EPAXExport {
    private:

        BinaryFormat format;
        BaseBinary* binary;

        void construct(std::string n, BinaryFormat f);

        /**
         * Emits an Binary instance to disk.
         *
         * @param n  The name of the file to write.
         * @return none
         */
        void emit(std::string n);

    public:

        /**
         * Constructs an Binary object.
         *
         * @param n  The name of a file. Format will be set based on the file's contents.
         */
        Binary(std::string n);

        /**
         * Constructs an Binary object.
         *
         * @param n  The name of a file.
         * @param f  The format of the file.
         */
        Binary(std::string n, BinaryFormat f);

        /**
         * Destroys an Binary instance. Should not be called directly.
         */
        virtual ~Binary();

        void runBasic(int argc, char* argv[]);

        /**
         * Attempts to guess the format of an binary file
         *
         * @return the format of the binary file, or BinaryFormat_undefined(0) if the format cannot be found
         */
        static BinaryFormat detectFormat(std::string n);

        uint64_t getStartAddr();
        std::string getName();

        /**
         * Gets the format of the binary.
         *
         * @return the format of this binary
         */
        BinaryFormat getFormat() { return format; }

        /**
         * Gets a string representation of the format of this binary.
         *
         * @ return a string representation of the format of this binary
         */
        const char* getFormatName();


        /**
         * Counts the functions in the binary
         *
         * @return the number of functions in the binary
         */
        uint32_t countFunctions();

        /**
         * Gets the first function in the binary
         *
         * @return the first function in the binary
         */
        Function* getFirstFunction();

        /**
         * Gets the next function in the binary
         * 
         * @param f a function in the binary
         *
         * @return the function following f
         */
        Function* getNextFunction(Function* f);

        /**
         * Tells whether this is the last function in the binary
         *
         * @param f a function in the binary
         *
         * @return true iff f is the last function in the binary
         */
        bool isLastFunction(Function* f);

        Function* findFunction(uint64_t addr);

        bool isExecutable();

        void printStaticFile(std::string& fname);
        void printStaticFile(const char* fname);

        uint32_t getFileSize();

    }; // class Binary

} // namespace EPAX

#endif // __EPAX_Binary_hpp__

