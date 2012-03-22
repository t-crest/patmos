#
# Expected Result: r1 = 0x0281fa80 & r2 = 0x8281fa81
#
                dlwm    sm  = [r31 + 0];
                dlwm    sm  = [r31 + 1] ||      mfs    r1  = s1;
                waitm                   ||      mfs    r2  = s1;
                halt;
