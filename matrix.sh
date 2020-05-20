#!/bin/bash

set -euo pipefail

cxxs=(g++-7 g++-8 g++-9 clang++-7 clang++-8 clang++-9)

names=(vanilla _alt alt_alt)
suffixes=('' _alt _alt_alt)

for s in cxx "${names[@]}"; do
	printf "%-9s  " "$s"
done

echo

for cxx in "${cxxs[@]}"; do
	printf "%-9s  " "$cxx"
	for suffix in "${suffixes[@]}"; do
		CXX=$cxx ./build.sh $suffix
		/bin/time -f%E ./brinterp mandelbrot.bf 2>&1 >/dev/null | tr -d '\n'
		echo -n "    "
	done
	echo
done

