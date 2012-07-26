# macro to simplify test definition
macro (test_asm num expr)
  ADD_TEST(asm-test-${num} ${CMAKE_BINARY_DIR}/src/paasm ${PROJECT_SOURCE_DIR}/tests/test${num}.s ${CMAKE_CURRENT_BINARY_DIR}/test${num}.bin)
  SET_TESTS_PROPERTIES(asm-test-${num} PROPERTIES PASS_REGULAR_EXPRESSION ${expr})
  SET_PROPERTY(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES test${num}.bin)
endmacro (test_asm)

test_asm(01 "Emitted: 16 words
Errors : 0")

test_asm(02 "Emitted: 16 words
Errors : 0")

test_asm(03 "Emitted: 24 words
Errors : 0")

test_asm(04 "Emitted: 16 words
Errors : 0")

test_asm(05 "Emitted: 32 words
Errors : 0")

test_asm(06 "Emitted: 36 words
Errors : 0")

test_asm(07 "Emitted: 20 words
Errors : 0")

test_asm(08 "Emitted: 32 words
Errors : 0")

test_asm(09 "Emitted: 28 words
Errors : 0")

test_asm(10 "Emitted: 44 words
Errors : 0")

test_asm(11 "Emitted: 56 words
Errors : 0")

test_asm(12 "Emitted: 56 words
Errors : 0")

test_asm(13 "Emitted: 64 words
Errors : 0")

test_asm(14 "Emitted: 52 words
Errors : 0")

test_asm(15 "Emitted: 12 words
Errors : 0")

test_asm(16 "Emitted: 16 words
Errors : 0")

test_asm(17 "Emitted: 16 words
Errors : 0")

test_asm(18 "Emitted: 16 words
Errors : 0")

test_asm(19 "Emitted: 16 words
Errors : 0")

test_asm(20 "Emitted: 28 words
Errors : 0")

test_asm(21 "Emitted: 68 words
Errors : 0")

test_asm(22 "Emitted: 44 words
Errors : 0")

test_asm(23 "Emitted: 48 words
Errors : 0")

test_asm(25 "Emitted: 24 words
Errors : 0")

test_asm(26 "Emitted: 8 words
Errors : 0")

test_asm(27 "Emitted: 8 words
Errors : 0")

test_asm(28 "Emitted: 60 words
Errors : 0")

test_asm(29 "Emitted: 20 words
Errors : 0")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# SIMULATOR TESTS
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

macro (test_sim num expr)
  ADD_TEST(sim-test-${num} ${CMAKE_BINARY_DIR}/src/pasim --maxc 20000 ${CMAKE_CURRENT_BINARY_DIR}/test${num}.bin)
  SET_TESTS_PROPERTIES(sim-test-${num} PROPERTIES PASS_REGULAR_EXPRESSION ${expr} DEPENDS asm-test-${num})
endmacro (test_sim)

macro (test_io_sim num expr)
  ADD_TEST(sim-test-${num} ${CMAKE_BINARY_DIR}/src/pasim --maxc 20000 --in ${PROJECT_SOURCE_DIR}/tests/test${num}.in ${CMAKE_CURRENT_BINARY_DIR}/test${num}.bin)
  SET_TESTS_PROPERTIES(sim-test-${num} PROPERTIES PASS_REGULAR_EXPRESSION ${expr} DEPENDS asm-test-${num})
endmacro (test_io_sim)

macro (test_elf_io_sim num expr)
  ADD_TEST(sim-test-${num} ${CMAKE_BINARY_DIR}/src/pasim --maxc 20000 --in ${PROJECT_SOURCE_DIR}/tests/test${num}.in ${PROJECT_SOURCE_DIR}/tests/test${num}.elf)
  SET_TESTS_PROPERTIES(sim-test-${num} PROPERTIES PASS_REGULAR_EXPRESSION ${expr})
endmacro (test_elf_io_sim)

test_sim(01 "r1 : 00000005   r2 : 0000000a")

test_sim(02 "r1 : 00000005   r2 : 00000005")

test_sim(03 "r1 : 00a00000   r2 : 00000000")

test_sim(04 "r1 : 00000000   r2 : 00000005")

test_sim(05 "PRR: 00000011.*
.*r1 : 00000005   r2 : 00000007   r3 : 0000000c   r4 : 00000000")

test_sim(06 "PRR: 00010011.*
.*r1 : 00000005   r2 : 00000007   r3 : 0000000c")

test_sim(07 "s2 : 00000023   s3 : 00000000")

test_sim(08 "r1 : 00000005   r2 : 00000007   r3 : 00000000   r4 : 00000023   r5 : 00000023")

test_sim(09 "r0 : 00000000   r1 : 0283f180   r2 : fffff180   r3 : fffffff1   r4 : 0000f180   r5 : 000000f1")

test_sim(10 "r0 : 00000000   r1 : 0283f180   r2 : 000000f0   r3 : 0283f180   r4 : f1800000   r5 : 80000000")

test_sim(11 "BASE: 00000000   PC : 0000003c.*
.*r1 : 00000026")

test_sim(13 "BASE: 00000020   PC : 00000048.*
.*r1 : 00000026   r2 : 00000000   r3 : 0000000c")

test_sim(14 "r0 : 00000000   r1 : 0283f180   r2 : 0283f180   r3 : fffff180   r4 : fffffff1   r5 : 0000f180   r6 : 000000f1.*
 0000000c:  00
 0000000b:  00
 0000000a:  00
 00000009:  00
 00000008:  00
 00000007:  00
 00000006:  00
 00000005:  00
 00000004:  00
 00000003:  80
 00000002:  f1
 00000001:  80")

test_sim(15 "Unmapped memory access: 0000000c: fffffffc")

test_sim(16 "Stack size exceeded: 00000010")

test_sim(17 "Stack size exceeded: 00000010")

test_sim(18 "Stack size exceeded: 00000010")

test_sim(19 "r1 : 00000007")

test_sim(20 "r1 : 0281fa80   r2 : 8281fa81")

test_sim(21 "BASE: 00000000   PC : 00000048.*
.*r1 : 0000003c")

test_io_sim(22 "r1 : 000000fb")

test_sim(23 "!
.*
.*r2 : 00000021")

test_elf_io_sim(24 "1234567890")

test_sim(25 "s2 : 00000d00   s3 : 00000034")

test_sim(26 "Unaligned")

test_sim(27 "Unaligned")

test_sim(28 "BASE: 00000000   PC : 00000040.*
.*r1 : 00000026")

test_sim(29 "PRR: 00000001.*
.*r1 : 00000009")

