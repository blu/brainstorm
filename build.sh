#!/bin/sh

clang++ -O3 -fno-rtti -fno-exceptions -march=native -mtune=native main.cpp get_file_size.cpp -o brinterp -DBAD_CASE=1
