#
# Expected Result: r2 = 1
#
                btest   p1 = r0, r0;
                pand    p2 = p0, !p1;
          ( p2) bc      x;
                addi    r2 = r0, 0;
                nop     0;
                addi    r2 = r2, 1;
x:              addi    r2 = r2, 1;
                halt;
