#!/bin/sh

clang++-3.6 -Ofast -fstrict-aliasing -fno-rtti -fno-exceptions -mcpu=native -mtune=native main${1}.cpp get_file_size.cpp -o brinterp -DPRINT_ASCII=1 -DENABLE_DIAGNOSTICS=0
