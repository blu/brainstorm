Description
-----------

This is an optimising interpreter for the language [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) with a small twist: brainstorm's output operation can be configured to print only numbers - the numeric value of the memory cell. In other words, brainstorm is oriented toward complex computational problems.

Other than that brainstorm does two very basic optimisations on the code: homogeneous arithmetic and pointer-move instruction sequences are collapsed to one instruction. Instruction themselves are stored as 16-bit words, with an optional immediate-operand field.

How to Build
------------

Running the build.sh script in the respective directiory builds the executable, if you happen to have clang++-3.5.

Build Prerequisites
-------------------

clang++-3.5. Or any other sane c++ compiler, i.e. some clang++ or g++ with a Good Scheduler (tm) for the given microarchitecture.


Benchmarks
----------

Erik Bosman's mandelbrot generator:

| CPU | compiler | time (real) |
| AMD C-60 (Bobcat) @ 1.333GHz | clang++-3.5.2 | 0m37.074s |
| AMD C-60 (Bobcat) @ 1.333GHz | g++-4.6.4 | 0m35.630s |
| Freescale iMX53 (Cortex-A8) @ 1.0GHz | clang++-3.6.2 | 1m1.011s |
| Intel E3-1270v2 (Ivy Bridge) @ 1.60GHz | clang++-3.5.0 | 0m17.189s |
| Intel E3-1270v2 (Ivy Bridge) @ 1.60GHz | g++-4.8.2 | 0m17.139s |
