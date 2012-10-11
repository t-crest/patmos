#
# Expected Result: r2 = 1
#
                addi    r1 = r0, 1;
                btest   p1 = r1, r0;
                por     p2 = !p0, p1;
          ( p2) br      x;
                addi    r2 = r0, 0;
                nop     0;
                addi    r2 = r2, 1;
x:              addi    r2 = r2, 1;
                halt;
