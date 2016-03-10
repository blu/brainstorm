Description
-----------

This is an optimising interpreter for the language [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) with a small twist: brainstorm's output operation can be configured to print only numbers - the numeric value of the memory cell. In other words, brainstorm is oriented toward complex computational problems.

Other than that brainstorm does two very basic optimisations on the code: homogeneous data- and instruction-pointer-move instruction sequences are collapsed to one instruction. Instructions themselves are stored as 16-bit words, containing an optional immediate-operand field.

How to Build
------------

Running the build.sh script in the respective directiory builds the executable, if you happen to have clang++-3.5 (hardcoded in the script -- substitute for your preferred compiler).
There are three versions of the interpreter -- vanilla, alt and alt-alt. They differ by minor tweaks to the interpeter loop, which can benefit some combinations of microarchitectures and compilers. The vanilla version is built by default; to build the alt version, pass '\_alt' as an arg to the build script, and to build the alt-alt version, pass '\_alt\_alt', respectively.

Benchmarks
----------

Erik Bosman's mandelbrot generator (times include printout; 'alt' = alt version, 'alt^2' = alt\_alt version):

| CPU                                                     | compiler      | time (real) |
| ---------------------------------------------------     | ------------- | ----------- |
| AMD C-60 (Bobcat) @ 1.333GHz                            | clang++-3.5.2 | 0m37.074s   |
| AMD C-60 (Bobcat) @ 1.333GHz                            | g++-4.6.4     | 0m35.630s   |
| Freescale iMX53 (Cortex-A8 r2p5) @ 1.0GHz               | clang++-3.6.2 | 1m0.941s    |
| Freescale iMX53 (Cortex-A8 r2p5) @ 1.0GHz               | g++-4.9.2     | 0m55.673s   |
| Intel Core2 Quad Q6600 (Kentsfield) @ 3.2GHz            | clang++-3.7.x | 0m12.650s   |
| Intel Core2 Quad Q6600 (Kentsfield) @ 3.2GHz            | g++-4.9.2     | 0m16.430s   |
| Intel i7-2600 (Sandy Bridge) @ 3.8GHz                   | clang++-3.7.1 | 0m9.200s    |
| Intel i7-2600 (Sandy Bridge) @ 3.8GHz                   | g++-5.3.0     | 0m8.350s    |
| Intel E5-2687W (Sandy Bridge) @ 3.1GHz                  | clang++-3.6.2 | 0m9.390s    |
| Intel E5-2687W (Sandy Bridge) @ 3.1GHz                  | g++-4.7.2     | 0m9.946s    |
| Intel i5-3470S (Ivy Bridge) @ 1.6GHz                    | g++-4.8.2     | 0m17.087s   |
| Intel E3-1270v2 (Ivy Bridge) @ 1.6GHz                   | clang++-3.5.0 | 0m17.189s   |
| Intel E3-1270v2 (Ivy Bridge) @ 1.6GHz                   | g++-4.8.2     | 0m17.116s   |
| Intel E3-1270v2 (Ivy Bridge) @ 3.9GHz                   | g++-4.8.2     | 0m7.033s    |
| Intel i7-5820K (Haswell) @ 3.6GHz                       | clang++-3.6.x | 0m7.276s    |
| Intel i7-5820K (Haswell) @ 3.6GHz                       | g++-4.8.x     | 0m6.905s    |
| AppliedMicro X-Gene 1 @ 2.4GHz (alt)                    | clang++-3.5.0 | 0m19.623s   |
| AppliedMicro X-Gene 1 @ 2.4GHz (alt)                    | g++-4.9.1     | 0m19.608s   |
| Rockchip RK3368 (Cortex-A53 r0p3) A32 @ 1.51GHz (alt^2) | g++-4.9.3     | 0m32.623s   |
| Rockchip RK3368 (Cortex-A53 r0p3) A64 @ 1.51GHz (alt^2) | clang++-3.5.0 | 0m35.121s   |

Normalized performance from the above (ticks = duration x CPU\_GHz; lower is better)

| CPU                                                 | compiler      | ticks       |
|---------------------------------------------------- | ------------- | ----------- |
| AMD C-60 (Bobcat)                                   | g++-4.6.4     | 47.49       |
| Freescale iMX53 (Cortex-A8 r2p5)                    | g++-4.9.2     | 55.67       |
| Intel Core2 Quad Q6600 (Kentsfield)                 | clang++-3.7.x | 40.48       |
| Intel i7-2600 (Sandy Bridge)                        | g++-5.3.0     | 31.73       |
| Intel E5-2687W (Sandy Bridge)                       | clang++-3.6.2 | 29.11       |
| Intel E3-1270v2 (Ivy Bridge)                        | g++-4.8.2     | 27.42       |
| Intel i7-5820K (Haswell)                            | g++-4.8.x     | 24.86       |
| AppliedMicro X-Gene 1 (alt)                         | g++-4.9.1     | 47.06       |
| Rockchip RK3368 (Cortex-A53 r0p3) (alt^2)           | g++-4.9.3     | 49.26       |

Musings
-------

Since brainstorm is a classic example of 100%-integer, branchy code, it can be indicative of some fundamental microarchitecture characteristics. For instance, in the mandelbrot test above:

* Bobcat (a dual-decode/dual-issue, out-of-order microarchitecture) is ~1.2x faster per-clock than Cortex-A8 (a dual-decode/dual-issue, in-order microarchitecture). Unfortunately, we cannot tell what part of Bobcat's advantage comes from the out-of-orderness, and what - from a potentially better branch predictor (currently perf does not support very well Cortex-A8's performance counters).
* Ivy Bridge (a quad-decode/six-issue, aggressively out-of-order microarchitecture) is ~1.7x faster per-clock than Bobcat. IVB does not quite outperform Bobcat twice in this test, even though IVB should have a clear branch-predictor advantage, on top of its twice-wider decoder. Indeed, running the test under perf stat shows that IVB achieves an average IPC of 2.46 and no branch mispredictions to speak of (0.34%), versus IPC of 1.42 for Bobcat and slightly worse but still negligible branch mispredictions (1.36%). So we have an apparent case of diminishing returns from the decode width, even when branch mispredictions are not an issue. Actually this is true not only for this test as 2.35-2.5 is normal IPC for the late 4-decode Intel microarchitectures across many loads.
* The performance degradation of the A64 build for RK3368 vs the A32 build could potentially be attributed to the different compilers. I plan to investigate the issue once I get perf running on that machine (currently blocked by an outdated kernel).
