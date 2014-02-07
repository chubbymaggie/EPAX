/**
 * @file DataStruct.hpp
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

#ifndef __EPAX_DataStruct_hpp__
#define __EPAX_DataStruct_hpp__

#include "EPAXCommonInternal.hpp"

namespace EPAX {

    class dyn_bitset {
    public:
        uint8_t* _elements;
        uint32_t _size;

    private:
        static const uint32_t div = 3;     // log(sizeof(uint8_t))
        static const uint8_t mask = 7;     // sizeof(uint8_t)-1
        static const uint8_t empty = 0x00; // b00000000
        static const uint8_t full = 0xff;  // b11111111

#define __get_index(__idx) (__idx >> div)
#define __has_bit(__idx) ((_elements[__get_index(__idx)] >> (__idx & mask)) & 1)
#define __set_bit(__idx) _elements[__get_index(__idx)] |= (1 << (__idx & mask))
#define __internal_size (__get_index(_size) + 1)

    public:
        dyn_bitset(uint32_t s):_elements(INVALID_PTR),_size(s)
        {
            _elements = new uint8_t[__internal_size];
        }

        ~dyn_bitset(){
            if (IS_VALID_PTR(_elements)){
                delete[] _elements;
            }
        }

        uint32_t size(){
            return _size;
        }

        void clear(){
            for (uint32_t i = 0; i < __internal_size; i++){
                _elements[i] = empty;
            }
        }

        void set(uint32_t idx){
            EPAXAssert(idx < _size, "Invalid dyn_bitset index given");
            __set_bit(idx);
        }

        void set(){
            for (uint32_t i = 0; i < __internal_size; i++){
                _elements[i] = full;
            }
        }

        bool has(uint32_t idx){
            EPAXAssert(idx < _size, "Invalid dyn_bitset index given");
            return __has_bit(idx);
        }

        const dyn_bitset& operator&=(const dyn_bitset& a){
            EPAXAssert(_size == a._size, "Cannot compare dyn_bitsets of different size");
            for (uint32_t i = 0; i < __internal_size; i++){
                _elements[i] &= a._elements[i];
            }
            return *this;
        }

        const dyn_bitset& operator|=(const dyn_bitset& a){
            EPAXAssert(_size == a._size, "Cannot compare dyn_bitsets of different size");
            for (uint32_t i = 0; i < __internal_size; i++){
                _elements[i] |= a._elements[i];
            }
            return *this;
        }

        const dyn_bitset& operator=(const dyn_bitset& a){
            EPAXAssert(_size == a._size, "Cannot compare dyn_bitsets of different size");
            for (uint32_t i = 0; i < __internal_size; i++){
                _elements[i] = a._elements[i];
            }
            return *this;            
        }

        bool operator==(const dyn_bitset& a){
            EPAXAssert(_size == a._size, "Cannot compare dyn_bitsets of different size");
            for (uint32_t i = 0; i < __internal_size; i++){
                if (_elements[i] != a._elements[i]){
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const dyn_bitset& a){
            return !(*(this) == a);
        }

        void print(){
            std::cout << "DynBitset(" << DEC(_size) << ")" << TAB;
            for (uint32_t i = 0; i < _size; i++){
                std::cout << (has(i)? "1":"0");
            }
            std::cout << ENDL;
        }
    };

} // namespace EPAX

#endif // __EPAX_Loop_hpp__



