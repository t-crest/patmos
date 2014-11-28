/*
	Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>

#define MINADDR (512*1024)
#define TEST_START ((volatile _UNCACHED unsigned int *) MINADDR + 0)

void test_ld(int l0, int l1, int l2, int l3) {
    int res0, res1, res2, res3;
    int lda0, lda1, lda2, lda3;

    lda0 = (int)(TEST_START + l0);
    lda1 = (int)(TEST_START + l1);
    lda2 = (int)(TEST_START + l2);
    lda3 = (int)(TEST_START + l3);

    asm volatile("\
lwc %0 = [%4];\
lwc %1 = [%5];\
lwc %2 = [%6];\
lwc %3 = [%7];\
nop;\
"
: "=&r" (res0), "=&r" (res1), "=&r" (res2), "=&r" (res3)
:   "r" (lda0),   "r" (lda1),   "r" (lda2),   "r" (lda3));

    if (res0 != lda0) exit(1);
    if (res1 != lda1) exit(2);
    if (res2 != lda2) exit(3);
    if (res3 != lda3) exit(4);
}

void test_st(int s0, int s1, int s2, int s3) {
    int res0, res1, res2, res3;
    int sta0, sta1, sta2, sta3;

    sta0 = (int)(TEST_START + s0);
    sta1 = (int)(TEST_START + s1);
    sta2 = (int)(TEST_START + s2);
    sta3 = (int)(TEST_START + s3);

    asm volatile("\
swc [%0] = %4;\
swc [%1] = %5;\
swc [%2] = %6;\
swc [%3] = %7;\
"
: 
: "r" (sta0), "r" (sta1), "r" (sta2), "r" (sta3),
  "r" (sta0), "r" (sta1), "r" (sta2), "r" (sta3));
}

int main() {

    test_st(0, 1, 2, 3);
    test_st(0x1000+0, 0x1000+1, 0x1000+2, 0x1000+3);

    fputc('A', stderr);
    test_ld(0, 1, 2, 3);
    fputc('B', stderr);
    test_ld(0x1000+0, 0x1000+1, 0x1000+2, 0x1000+3);
    fputc('C', stderr);
    test_ld(0, 3, 1, 2);
    fputc('D', stderr);

    return 0;

}
