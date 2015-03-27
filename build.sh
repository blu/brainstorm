#!/bin/sh

clang++-3.5 -Ofast -fstrict-aliasing -fno-rtti -fno-exceptions -march=native -mtune=native main.cpp get_file_size.cpp -o brinterp -DPRINT_ASCII -DBAD_CASE=1
