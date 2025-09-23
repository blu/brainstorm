#!/bin/bash

CXX=${CXX:-g++}
# echo "Using CXX=$CXX"

source cxx_util.sh

UNAME_MACHINE=`uname -m`
CXXFLAGS=(
	-Ofast
	-fstrict-aliasing
	-fno-rtti
	-fno-exceptions
	-DNDEBUG
	-DPRINT_ASCII=1
	-DENABLE_DIAGNOSTICS=0
)
if [[ $UNAME_MACHINE == "aarch64" ]] ; then

		cxx_uarch_arm

elif [[ $UNAME_MACHINE == "arm64" ]] ; then
		CXXFLAGS+=(
			-march=armv8.4-a # baseline for macos/arm64
			-mtune=native
		)
else
		CXXFLAGS+=(
			-march=native
			-mtune=native
		)
fi

# set -x
${CXX} ${CXXFLAGS[@]} main${1}.cpp util_file.cpp -o brinterp
