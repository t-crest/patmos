
    // ALUi:
    MK_NINSTR(addil , addi , alui, 0)
    MK_NINSTR(subil , subi , alui, 1)

    // remember the ID of the SUBi instruction to detect NOPs
    NOP_ID = Instructions.size() - 1;

    MK_NINSTR(xoril , xori , alui, 2)
    MK_NINSTR(slil  , sli  , alui, 3)
    MK_NINSTR(sril  , sri  , alui, 4)
    MK_NINSTR(srail , srai , alui, 5)
    MK_NINSTR(oril  , ori  , alui, 6)
    MK_NINSTR(andil , andi , alui, 7)

    // ALUl:
    MK_NINSTR(addil  , addl  , alul,  0)
    MK_NINSTR(subil  , subl  , alul,  1)
    MK_NINSTR(xoril  , xorl  , alul,  2)
    MK_NINSTR(slil   , sll   , alul,  3)
    MK_NINSTR(sril   , srl   , alul,  4)
    MK_NINSTR(srail  , sral  , alul,  5)
    MK_NINSTR(oril   , orl   , alul,  6)
    MK_NINSTR(andil  , andl  , alul,  7)
    MK_INSTR (norl           , alul, 11)
    MK_INSTR (shaddl         , alul, 12)
    MK_INSTR (shadd2l        , alul, 13)

    MK_NINSTR_ALIAS(addil  , add  , alul,  0)
    MK_NINSTR_ALIAS(subil  , sub  , alul,  1)
    MK_NINSTR_ALIAS(xoril  , xor  , alul,  2)
    MK_NINSTR_ALIAS(slil   , sl   , alul,  3)
    MK_NINSTR_ALIAS(sril   , sr   , alul,  4)
    MK_NINSTR_ALIAS(srail  , sra  , alul,  5)
    MK_NINSTR_ALIAS(oril   , or   , alul,  6)
    MK_NINSTR_ALIAS(andil  , and  , alul,  7)
    MK_NINSTR_ALIAS(norl   , nor  , alul, 11)
    MK_NINSTR_ALIAS(shaddl , shadd, alul, 12)
    MK_NINSTR_ALIAS(shadd2l, shadd2, alul, 13)

    // ALUr:
    MK_INSTR(add   , alur,  0)
    MK_INSTR(sub   , alur,  1)
    MK_INSTR(xor   , alur,  2)
    MK_INSTR(sl    , alur,  3)
    MK_INSTR(sr    , alur,  4)
    MK_INSTR(sra   , alur,  5)
    MK_INSTR(or    , alur,  6)
    MK_INSTR(and   , alur,  7)
    MK_INSTR(nor   , alur, 11)
    MK_INSTR(shadd , alur, 12)
    MK_INSTR(shadd2, alur, 13)

    // ALUm
    MK_INSTR(mul , alum, 0)
    MK_INSTR(mulu, alum, 1)

    // ALUc
    MK_INSTR(cmpeq  , aluc, 0)
    MK_INSTR(cmpneq , aluc, 1)
    MK_INSTR(cmplt  , aluc, 2)
    MK_INSTR(cmple  , aluc, 3)
    MK_INSTR(cmpult , aluc, 4)
    MK_INSTR(cmpule , aluc, 5)
    MK_INSTR(btest  , aluc, 6)

    // ALUci
    MK_INSTR(cmpieq  , aluci, 0)
    MK_INSTR(cmpineq , aluci, 1)
    MK_INSTR(cmpilt  , aluci, 2)
    MK_INSTR(cmpile  , aluci, 3)
    MK_INSTR(cmpiult , aluci, 4)
    MK_INSTR(cmpiule , aluci, 5)
    MK_INSTR(btesti  , aluci, 6)

    // ALUp
    MK_INSTR(por , alup,  6)
    MK_INSTR(pand, alup,  7)
    MK_INSTR(pxor, alup, 10)

    // ALUb
    MK_INSTR(bcopy, alub, -1)

    // SPC
    MK_NINSTR(spct, mts , spct, 0)
    MK_NINSTR(spcf, mfs , spcf, 0)

    // LDT
    MK_SINSTR(lws , ldt,  0)
    MK_INSTR(lwl  , ldt,  1)
    MK_INSTR(lwc  , ldt,  2)
    MK_INSTR(lwm  , ldt,  3)
    MK_SINSTR(lhs , ldt,  4)
    MK_INSTR(lhl  , ldt,  5)
    MK_INSTR(lhc  , ldt,  6)
    MK_INSTR(lhm  , ldt,  7)
    MK_SINSTR(lbs , ldt,  8)
    MK_INSTR(lbl  , ldt,  9)
    MK_INSTR(lbc  , ldt, 10)
    MK_INSTR(lbm  , ldt, 11)
    MK_SINSTR(lhus, ldt, 12)
    MK_INSTR(lhul , ldt, 13)
    MK_INSTR(lhuc , ldt, 14)
    MK_INSTR(lhum , ldt, 15)
    MK_SINSTR(lbus, ldt, 16)
    MK_INSTR(lbul , ldt, 17)
    MK_INSTR(lbuc , ldt, 18)
    MK_INSTR(lbum , ldt, 19)

    // STT
    MK_SINSTR(sws, stt,  0)
    MK_INSTR(swl , stt,  1)
    MK_INSTR(swc , stt,  2)
    MK_INSTR(swm , stt,  3)
    MK_SINSTR(shs, stt,  4)
    MK_INSTR(shl , stt,  5)
    MK_INSTR(shc , stt,  6)
    MK_INSTR(shm , stt,  7)
    MK_SINSTR(sbs, stt,  8)
    MK_INSTR(sbl , stt,  9)
    MK_INSTR(sbc , stt, 10)
    MK_INSTR(sbm , stt, 11)

    // STCi
    MK_INSTR(sres  , stci, 0)
    MK_INSTR(sens  , stci, 1)
    MK_INSTR(sfree , stci, 2)
    MK_INSTR(sspill, stci, 3)

    // STCr
    MK_INSTR(sensr  , stcr, 1)
    MK_INSTR(sspillr, stcr, 3)

    // CFLi
    MK_FINSTR(call, callnd, cfli, 0, 0)
    MK_FINSTR(br  , brnd  , cfli, 1, 0)
    MK_FINSTR(brcf, brcfnd, cfli, 2, 0)
    MK_FINSTR(trap, trap  , cfli, 3, 0)
    MK_FINSTR(call, call  , cfli, 0, 1)
    MK_FINSTR(br  , br    , cfli, 1, 1)
    MK_FINSTR(brcf, brcf  , cfli, 2, 1)

    // CFLri
    MK_FINSTR(ret,  retnd , cflri, 0, 0)
    MK_FINSTR(ret,  ret   , cflri, 0, 1)
    MK_FINSTR(xret, xretnd, cflri, 1, 0)
    MK_FINSTR(xret, xret  , cflri, 1, 1)

    // CFLrs
    MK_FINSTR(callr, callrnd, cflrs, 0, 0)
    MK_FINSTR(brr  , brrnd  , cflrs, 1, 0)
    MK_FINSTR(callr, callr  , cflrs, 0, 1)
    MK_FINSTR(brr  , brr    , cflrs, 1, 1)

    // CFLrt
    MK_FINSTR(brcfr, brcfrnd, cflrt, 2, 0)
    MK_FINSTR(brcfr, brcfr  , cflrt, 2, 1)
