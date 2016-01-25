#!/usr/bin/env python

import getopt
import sys
import os
import string

objs = ['BIN', 'SECT', 'FUNC', 'CFG', 'LOOP', 'BBL', 'INSN', 'SYM', 'FLOW']
other = {}
other['std::vector<std::string>&'] = 'char**'
other['std::vector<const char*>&'] = 'char**'
other['std::string'] = 'const char*'
other['std::vector<uint64_t>&'] = 'uint64_t*'
other['std::vector<EPAX_func>&'] = 'EPAX_func*'
other['std::vector<EPAX_bbl>&'] = 'EPAX_bbl*'
other['std::vector<EPAX_insn>&'] = 'EPAX_insn*'
other['Interface.hpp'] = 'Interface.h'
other['Interface_hpp'] = 'Interface_h'
other['bool'] = 'uint32_t'
other['namespace EPAX {'] = '#ifdef __cplusplus\nextern "C"\n{\n#endif'
other['} // namespace EPAX'] = '#ifdef __cplusplus\n}\n#endif'
other['const const'] = 'const'

remove = {}
remove['#include <string>'] = True
remove['#include <iostream>'] = True
remove['#include <vector>'] = True

def file_exists(f):
    if os.path.isfile(f):
        return True
    return False

def error_die(msg=''):
    if msg != '':
        print 'error: ' + str(msg)
    sys.exit(1)

def print_usage(msg=''):
    print 'usage: ' + str(sys.argv[0]) + ' <EPAX_C++_interface> <EPAX_C_interface>'
    error_die(msg)

def char_range(c1, c2):
    for c in xrange(ord(c1), ord(c2)+1):
        yield chr(c)

def main():
    if len(sys.argv) != 3:
        print_usage('exactly 2 arguments required')

    if not file_exists(sys.argv[1]):
        print_usage('file cannot be found: ' + str(sys.argv[1]))

    inp = open(sys.argv[1], 'r')
    outp = open(sys.argv[2], 'w')
    outp.write('/* automatically generated via `' + string.join(sys.argv) + '\' */\n')
    for line in inp:
        line = line.strip()
        toks = line.split()
        if len(toks) == 0:
            continue
        if len(toks) >= 1:
            if toks[0] == 'class':
                continue
            if toks[0] == 'typedef':
                if len(toks) == 3:
                    outp.write('typedef void* ' + 'EPAX_' + toks[2].lower() + '\n')
                continue

        for o in objs:
            line = line.replace(o, 'EPAX_' + o.lower())
        for k in other.keys():
            line = line.replace(k, other[k])

        if remove.has_key(line):
           continue

        if toks[0] == '//':
            outp.write('\n')
            continue

        if toks[0] == '/**':
            outp.write('\n')

        if len(toks) > 1 and toks[1] == '//':
            outp.write(line + '\n')
            continue

        if line.startswith('*'):
            line = ' ' + line
        outp.write(line + '\n')
        
    inp.close()
    outp.close()


if __name__ == '__main__':
    main()
