Description
-----------

This is an optimising interpreter for the language [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) with a small twist: brainstorm's output operation can be configured to print only numbers - the numeric value of the memory cell. In other words, brainstorm is oriented toward complex computational problems.

Other than that brainstorm does two very basic optimisations on the code: homogeneous data- and instruction-pointer-move instruction sequences are collapsed to one instruction. Instructions themselves are stored as 16-bit words, containing an optional immediate-operand field.

How to Build
------------

Running the build.sh script in the respective directiory builds the executable, if you happen to have clang++-3.5.

Build Prerequisites
-------------------

clang++-3.5. Or any other sane c++ compiler, i.e. some clang++ or g++ with a Good Scheduler (tm) for the given microarchitecture.


Benchmarks
----------

Erik Bosman's mandelbrot generator (times include printout):

| CPU                                    | compiler      | time (real) |
| -------------------------------------- | ------------- | ----------- |
| AMD C-60 (Bobcat) @ 1.333GHz           | clang++-3.5.2 | 0m37.074s   |
| AMD C-60 (Bobcat) @ 1.333GHz           | g++-4.6.4     | 0m35.630s   |
| Freescale iMX53 (Cortex-A8) @ 1.0GHz   | clang++-3.6.2 | 1m0.941s    |
| Freescale iMX53 (Cortex-A8) @ 1.0GHz   | g++-4.9.2     | 0m55.673s   |
| Intel E3-1270v2 (Ivy Bridge) @ 1.60GHz | clang++-3.5.0 | 0m17.189s   |
| Intel E3-1270v2 (Ivy Bridge) @ 1.60GHz | g++-4.8.2     | 0m17.139s   |
| Intel E3-1270v2 (Ivy Bridge) @ 3.90GHz | g++-4.8.2     | 0m7.033s    |

Musings
-------

Since brainstorm is a classic example of 100%-integer, branchy code, it can be indicative of some fundamental microarchitecture characteristics. For instance, in the mandelbrot test above:

* Bobcat (a dual-issue, out-of-order microarchitecture) is ~1.2x faster per-clock than Cortex-A8 (a dual-issue, in-order microarchitecture). Unfortunately, we cannot tell what part of Bobcat's advantage comes from the out-of-orderness, and what - from a potentially better branch predictor (currently perf does not support very well Cortex-A8's performance counters).
* Ivy Bridge (a quad-issue, aggressively out-of-order microarchitecture) is ~1.7x faster per-clock than Bobcat. IB does not quite 2x the Bobcat in this test, even though IB should have a clear branch-predictor advantage, on top of its twice-wider frontend. Indeed, running the test under perf stat on both CPUs shows that IB achieves an average IPC of 2.28 and no branch mispredictions to speak of (0.49%), versus IPC of 1.41 for Bobcat and slightly worse but still negligible branch mispredictions (1.36%). So we have an apparent case of diminishing returns from the issue width, even when branch mispredictions are not an issue. Actually this is not only in this test as 2.35-2.5 is normal IPC for the late 4-issue Intel microarchitectures under many loads.
