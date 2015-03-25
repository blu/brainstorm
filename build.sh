#!/bin/sh

clang++-3.5 -O3 -fno-rtti -fno-exceptions -march=native -mtune=native main.cpp get_file_size.cpp -o brinterp
