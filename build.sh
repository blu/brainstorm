#!/bin/sh

CXX=${CXX:-g++}

# echo "Using CXX=$CXX"

${CXX} -Ofast -fstrict-aliasing -fno-rtti -fno-exceptions -march=native -mtune=native main${1}.cpp get_file_size.cpp -o brinterp -DNDEBUG -DPRINT_ASCII=1 -DENABLE_DIAGNOSTICS=0
