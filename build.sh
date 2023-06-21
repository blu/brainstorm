#!/bin/sh

CXX=${CXX:-g++}

# echo "Using CXX=$CXX"
${CXX} --version

${CXX} -Ofast -fstrict-aliasing -fno-rtti -fno-exceptions -march=native -mtune=native main${1}.cpp util_file.cpp -o brinterp -DNDEBUG -DPRINT_ASCII=1 -DENABLE_DIAGNOSTICS=0
