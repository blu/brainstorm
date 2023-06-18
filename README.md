Description
-----------

This is an optimising interpreter for the language [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) with a small twist: brainstorm's output operation can be configured to print only numbers - the numeric value of the memory cell. In other words, brainstorm is oriented toward complex computational problems.

Other than that brainstorm does two very basic optimisations on the code: homogeneous data- and instruction-pointer-move instruction sequences are collapsed to one instruction. Instructions themselves are stored as 16-bit words, containing an optional immediate-operand field.

How to Build
------------

Running the build.sh script in the source directiory builds the executable, if you happen to have clang++-3.6 (hardcoded in the script -- substitute for your preferred compiler).

There are three versions of the interpreter -- vanilla, alt and alt-alt. They differ by minor tweaks to the interpeter loop, which can benefit some combinations of microarchitectures and compilers. The vanilla version is built by default; to build the alt version, pass `_alt` as an arg to the build script, and to build the alt-alt version, pass `_alt_alt`, respectively.

Benchmarks
----------

Erik Bosman's mandelbrot generator (times include printout; 'alt' = alt version, 'alt^2' = alt-alt version):

| CPU                                                        | compiler            | time (real)    |
| ---------------------------------------------------------- | ------------------- | -------------- |
| AMD C-60 (Bobcat) @ 1.33GHz                                | clang++-3.5.2       | 0m36.497s [^1] |
| AMD C-60 (Bobcat) @ 1.33GHz                                | g++-4.8.4           | 0m34.668s [^1] |
| AMD A8-7600 (Steamroller) @ 2.4GHz                         | clang++-3.5.2       | 0m16.720s      |
| AMD Ryzen 1700 (Zen) @ 3.0GHz                              | g++-6.3.0           | 0m9.54s        |
| AMD Ryzen 1700X (Zen) @ 2.2GHz                             | g++-4.9.2           | 0m11.81s  [^2] |
| Intel Core2 Quad Q6600 (Kentsfield) @ 3.2GHz               | clang++-3.7.x       | 0m12.650s      |
| Intel Core2 Quad Q6600 (Kentsfield) @ 3.2GHz               | g++-4.9.2           | 0m16.430s      |
| Intel Core2 Duo P8600 (Penryn) @ 2.4GHz (alt^2)            | apple clang++-8.1.0 | 0m13.383s      |
| Intel i7-2600 (Sandy Bridge) @ 3.8GHz                      | clang++-3.7.1       | 0m9.200s       |
| Intel i7-2600 (Sandy Bridge) @ 3.8GHz                      | g++-5.3.0           | 0m8.350s       |
| Intel E5-2687W (Sandy Bridge) @ 3.1GHz                     | clang++-3.6.2       | 0m9.390s       |
| Intel E5-2687W (Sandy Bridge) @ 3.1GHz                     | g++-4.7.2           | 0m9.946s       |
| Intel i5-3470S (Ivy Bridge) @ 1.6GHz                       | g++-4.8.2           | 0m17.087s      |
| Intel i5-3570 (Ivy Bridge) @ 1.6GHz                        | g++-4.8.4           | 0m17.086s      |
| Intel i5-3570 (Ivy Bridge) @ 3.8GHz                        | g++-4.8.4           | 0m7.200s       |
| Intel E3-1270v2 (Ivy Bridge) @ 1.6GHz                      | clang++-3.5.0       | 0m17.189s      |
| Intel E3-1270v2 (Ivy Bridge) @ 1.6GHz                      | g++-4.8.2           | 0m17.088s      |
| Intel E3-1270v2 (Ivy Bridge) @ 3.9GHz                      | g++-4.8.2           | 0m7.033s       |
| Intel i7-5820K (Haswell) @ 3.6GHz                          | clang++-3.6.x       | 0m7.276s       |
| Intel i7-5820K (Haswell) @ 3.6GHz                          | g++-4.8.x           | 0m6.905s       |
| Freescale iMX53 (Cortex-A8 r2p5) @ 1.0GHz                  | clang++-3.6.2       | 1m0.941s       |
| Freescale iMX53 (Cortex-A8 r2p5) @ 1.0GHz                  | g++-4.9.2           | 0m55.673s      |
| Samsung Exynos 5422 (Cortex-A15 r2p3) @ 2.0GHz             | clang++-3.6.2       | 0m28.754s      |
| Samsung Exynos 5422 (Cortex-A15 r2p3) @ 2.0GHz             | g++-6.3.0           | 0m20.007s      |
| Allwinner A64 (Cortex-A53 r0p4) @ 1.152GHz (alt)           | clang++-3.6.2       | 0m40.832s      |
| AppliedMicro X-Gene 1 @ 2.4GHz (alt)                       | clang++-3.5.0       | 0m19.623s      |
| AppliedMicro X-Gene 1 @ 2.4GHz (alt)                       | g++-4.9.1           | 0m19.608s      |
| Rockchip RK3368 (Cortex-A53 r0p3) A32 @ 1.51GHz (alt^2)    | g++-4.9.3           | 0m32.623s      |
| Rockchip RK3368 (Cortex-A53 r0p3) A64 @ 1.51GHz (alt^2)    | clang++-3.6.0       | 0m30.224s      |
| Rockchip RK3399 (Cortex-A72 r0p2) A64 @ 1.8GHz (alt^2)     | g++-7.4.0           | 0m16.394s [^3] |
| MediaTek MT8163A (Cortex-A53 r0p3) A32 @ 1.5GHz (alt^2)    | g++-4.9.2           | 0m33.430s [^4] |
| MediaTek MT8163A (Cortex-A53 r0p3) A64 @ 1.5GHz (alt^2)    | clang++-3.6.2       | 0m30.587s [^4] |
| MediaTek MT8173C (Cortex-A72 r0p0) A64 @ 2.1GHz            | clang++-5.0.1       | 0m19.083s [^3] |
| MediaTek MT8173C (Cortex-A72 r0p0) A64 @ 2.1GHz (alt^2)    | g++-7.2.1           | 0m14.011s [^3] |
| MediaTek MT8173C (Cortex-A72 r0p0) A64 @ 2.1GHz (alt^2)    | g++-8.3.0           | 0m13.433s      |
| Marvell ARMADA 8040 (Cortex-A72 r0p1) A64 @ 1.3GHz (alt^2) | g++-7.2.1           | 0m22.703s [^3] |
| Marvell ARMADA 8040 (Cortex-A72 r0p1) A64 @ 1.3GHz         | clang++-3.5.2       | 0m30.836s [^3] |
| Marvell ARMADA 8040 (Cortex-A72 r0p1) A64 @ 2.0GHz         | clang++-3.5.2       | 0m20.035s [^3] |
| NXP LX2160A (Cortex-A72 r0p3) A64 @ 2.0GHz (alt^2)         | g++-7.4.0           | 0m14.748s      |
| NXP LX2160A (Cortex-A72 r0p3) A64 @ 2.0GHz (alt^2)         | clang++-8.0.0       | 0m19.590s [^3] |
| AWS Graviton (Cortex-A72 r0p3) A64 @ 2.28GHz (alt^2)       | g++-7.3.0           | 0m12.881s      |
| AWS Graviton (Cortex-A72 r0p3) A64 @ 2.28GHz               | clang++-6.0.0       | 0m16.924s      |
| AWS Graviton2 (Cortex-A76 r3p0) A64 @ 2.5GHz (alt^2)       | g++-7.5.0           | 0m8.115s       |
| AWS Graviton2 (Cortex-A76 r3p0) A64 @ 2.5GHz (alt^2)       | clang++-9.0.1       | 0m7.740s       |
| Baikal-T1 (MIPS P5600) @ 1.2GHz (alt)                      | g++-6.3.0           | 0m37.212s      |
| Baikal-T1 (MIPS P5600) @ 1.2GHz (alt^2)                    | g++-7.3.0           | 0m31.822s      |
| Amlogic S922X (Cortex-A73 r0p2) A64 @ 1.8GHz (alt^2)       | g++-7.3.0           | 0m15.459s      |
| NVIDIA armv8.2 (Carmel) A64 @ 1.907GHz (alt^2)             | g++-8.4.0           | 0m14.680s      |
| NVIDIA armv8.2 (Carmel) A64 @ 2.265GHz (alt^2)             | g++-8.4.0           | 0m10.990s      |
| Fujitsu armv8.2 (A64fx) A64 @ 2.2GHz (alt^2)               | clang++-7.0.0       | 0m14.652s      |
| Snapdragon 835 (Cortex-A73 r?p?) A64 @ 2.55GHz (alt^2)     | clang++-9.0.0       | 0m12.117s      |
| Snapdragon 835 (Cortex-A73 r?p?) A64 @ 2.55GHz (alt^2)     | g++-7.5.0           | 0m11.208s      |
| Snapdragon 835 (Cortex-A73 r?p?) A64 @ 2.55GHz (alt^2)     | g++-8.3.0           | 0m10.915s      |
| Snapdragon SQ1 (Cortex-A76 r?p?) A64 @ 3.0GHz (alt^2)      | clang++-9.0.1       | 0m7.043s       |
| Snapdragon SQ1 (Cortex-A76 r?p?) A64 @ 3.0GHz (alt^2)      | clang++-10.0.0      | 0m6.690s       |
| Apple armv8.4 (Firestorm) A64 @ 3.2GHz (alt^2)             | apple clang++ 12.0.0| 0m6.510s       |
| Apple armv8.4 (Firestorm) A64 @ 3.2GHz (alt^2)             | g++-11.0.0          | 0m4.350s       |
| Apple armv8.5 (Avalanche) A64 @ 3.49GHz (alt^2)            | apple clang++ 14.0.0| 0m5.880s       |
| Intel i5-1035G4 (Ice Lake) @ 3.5 GHz (alt^2)               | clang++-9.0.0       | 0m4.930s       |
| AMD Ryzen 9 7950x (Zen4) @ 4.5 GHz                         | g++-11.2.1          | 0m03.90        |
| AMD Ryzen 9 7950x (Zen4) @ full boost (~5.7 GHz)           | g++-11.2.1          | 0m03.11        |
| Intel i5-5257U @ 3.1 GHz (MacBook Pro 13 early 2015)       | apple clang++ 11.0.3| 0m06.70        |
| Intel i5-5257U @ 3.1 GHz (MacBook Pro 13 e 2015) (alt^2)   | apple clang++ 11.0.3| 0m06.00        |

[^1]: Generic compiler tuning; native tuning does not pose any speed advantage
[^2]: Non-native compiler tuning -march=corei7
[^3]: Non-native compiler tuning -mcpu=cortex-a57
[^4]: This MT8163A resides in a BQ M10 Ubuntu tablet, and as such is subject to the following performance detriments:

    (1) Power management causes cores to pop in and out of existence, rather than just scaling them by frequency.  
    (2) There is an entire (albeit minimal) Android running in a lxc container on that tablet.

Note: There are two compiler snafus in all A64 alt-alt entries built by clang. First, the interpereter loop does not get aligned to a multiple-of-16 address, so one has to inject nops before the loop to get optimal loop alignment. Second, the code generated for the loop could be better:

```
400f00:       787a7aa8        ldrh    w8, [x21,x26,lsl #1]
400f04:       12000909        and     w9, w8, #0x7
400f08:       71001d3f        cmp     w9, #0x7            // since we just AND'd its source register with 7, w9 could not be larger than 7,
400f0c:       54000468        b.hi    400f98              // so this pair of ops is essentially nops in the innermost loop
...
400f98:       f94007e8        ldr     x8, [sp,#8]
400f9c:       9100075a        add     x26, x26, #0x1
400fa0:       eb08035f        cmp     x26, x8
400fa4:       54fffae3        b.cc    400f00
```

Normalized performance from the above as `ticks = duration x CPU_GHz` (lower is better):

| CPU                                                   | compiler            | ticks       |
|------------------------------------------------------ | ------------------- | ----------- |
| Freescale iMX53 (Cortex-A8 r2p5)                      | g++-4.9.2           | 55.67       |
| AppliedMicro X-Gene 1 (alt)                           | g++-4.9.1           | 47.06       |
| Allwinner A64 (Cortex-A53 r0p4) (alt)                 | clang++-3.6.2       | 47.04       |
| AMD C-60 (Bobcat)                                     | g++-4.8.4           | 46.11       |
| MediaTek MT8163A (Cortex-A53 r0p3) (alt^2)            | clang++-3.6.2       | 45.88       |
| Rockchip RK3368 (Cortex-A53 r0p3) (alt^2)             | clang++-3.6.0       | 45.64       |
| Intel Core2 Quad Q6600 (Kentsfield)                   | clang++-3.7.x       | 40.48       |
| AMD A8-7600 (Steamroller)                             | clang++-3.5.2       | 40.13       |
| Samsung Exynos 5422 (Cortex-A15 r2p3)                 | g++-6.3.0           | 40.01       |
| Baikal-T1 (MIPS P5600) (alt^2)                        | g++-7.3.0           | 38.19       |
| Fujitsu armv8.2 (A64fx) (alt^2)                       | clang++-7.0.0       | 32.23       |
| Intel Core2 Duo P8600 (Penryn) (alt^2)                | apple clang++-8.1.0 | 32.12       |
| Intel i7-2600 (Sandy Bridge)                          | g++-5.3.0           | 31.73       |
| Rockchip RK3399 (Cortex-A72 r0p2) (alt^2)             | g++-7.4.0           | 29.51       |
| Marvell ARMADA 8040 (Cortex-A72 r0p1) (alt^2)         | g++-7.2.1           | 29.51       |
| NXP LX2160A (Cortex-A72 r0p3) (alt^2)                 | g++-7.4.0           | 29.50       |
| MediaTek MT8173C (Cortex-A72 r0p0) (alt^2)            | g++-7.2.1           | 29.42       |
| AWS Graviton (Cortex-A72 r0p3) (alt^2)                | g++-7.3.0           | 29.37       |
| Intel E5-2687W (Sandy Bridge)                         | clang++-3.6.2       | 29.11       |
| MediaTek MT8173C (Cortex-A72 r0p0) (alt^2)            | g++-8.3.0           | 28.21       |
| Snapdragon 835 (Cortex-A73 r?p?) (alt^2)              | g++-8.3.0           | 27.83       |
| Amlogic S922X (Cortex-A73 r0p2) (alt^2)               | g++-7.3.0           | 27.83       |
| Intel E3-1270v2 (Ivy Bridge)                          | g++-4.8.2           | 27.34       |
| AMD Ryzen 1700X (Zen)                                 | g++-4.9.2           | 25.98       |
| NVIDIA armv8.2 (Carmel) A64 @ 2.265GHz (alt^2)        | g++-8.4.0           | 24.89       |
| Intel i7-5820K (Haswell)                              | g++-4.8.x           | 24.86       |
| Snapdragon SQ1 (Cortex-A76 r?p?) (alt^2)              | clang++-9.0.1       | 21.13       |
| Intel i5-5257U @ 3.1 GHz (MacBook Pro 13 early 2015)  | apple clang++ 11.0.3| 20.77       |
| Apple armv8.5 (Avalanche) A64 @ 3.49GHz (alt^2)       | clang++-14.0.0      | 20.52       |
| Snapdragon SQ1 (Cortex-A76 r?p?) (alt^2)              | clang++-10.0.0      | 20.07       |
| AWS Graviton2 (Cortex-A76 r3p0) (alt^2)               | clang++-9.0.1       | 19.35       |
| Intel i5-5257U @ 3.1 GHz (MacBook Pro 13 2015) (alt^2)| apple clang++ 11.0.3| 18.60       |
| AMD Ryzen 9 7950x (Zen4) @ 4.5 GHz                    | g++-11.2.1          | 17.55       |
| Intel i5-1035G4 (Ice Lake) (alt^2)                    | clang++-9.0.0       | 17.26       |
| Apple armv8.4 (Firestorm) (alt^2)                     | g++-11.0.0          | 13.92       |

Musings
-------

Since brainstorm is a classic example of 100%-integer, branchy code, it can be indicative of some fundamental microarchitecture characteristics. For instance, in the mandelbrot test above:

* Bobcat (a dual-decode/dual-issue/six-dispatch, out-of-order microarchitecture) is ~1.2x faster per-clock than Cortex-A8 (a dual-decode/dual-issue, in-order microarchitecture). Unfortunately, we cannot tell what part of Bobcat's advantage comes from the out-of-orderness, and what - from a potentially better branch predictor (currently perf does not support very well Cortex-A8's performance counters).
* Ivy Bridge (a quad-decode/quad-issue/six-dispatch, aggressively out-of-order microarchitecture) is ~1.7x faster per-clock than Bobcat. IVB does not quite outperform Bobcat twice in this test, even though IVB should have a clear branch-predictor advantage, on top of its twice-wider decoder. Indeed, running the test under perf stat shows that IVB achieves an average IPC of 2.46 and no branch mispredictions to speak of (0.34%), versus IPC of 1.42 for Bobcat and slightly worse but still negligible branch mispredictions (1.36%). So we have an apparent case of diminishing returns from the decode width, even when branch mispredictions are not an issue. Actually this is true not only for this test as 2.35-2.5 is normal IPC for the late 4-decode Intel microarchitectures across many loads.
