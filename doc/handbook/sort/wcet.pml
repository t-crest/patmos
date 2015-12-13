---
format: pml-0.1
triple: patmos-unknown-unknown-elf
machine-functions:
- name: 8
  level: machinecode
  mapsto: main
  arguments:
  - name: ! '%argc'
    index: 0
    registers:
    - r3
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: SRESi
      size: 4
      stack-cache-argument: 3
      address: 133156
    - index: 1
      opcode: MFS
      size: 4
      address: 133160
    - index: 2
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133164
    - index: 3
      opcode: LIl
      size: 8
      address: 133168
    - index: 4
      opcode: MFS
      size: 4
      address: 133176
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133180
    - index: 6
      opcode: MFS
      size: 4
      address: 133184
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133188
    - index: 8
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133192
    - index: 9
      opcode: NOP
      size: 4
      address: 133196
    - index: 10
      opcode: SHADD2l
      size: 8
      address: 133200
    - index: 11
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133208
    - index: 12
      opcode: NOP
      size: 4
      address: 133212
    - index: 13
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133216
    - index: 14
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133220
    - index: 15
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133224
    - index: 16
      opcode: NOP
      size: 4
      address: 133228
    - index: 17
      opcode: SHADD2l
      size: 8
      address: 133232
    - index: 18
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133240
    - index: 19
      opcode: NOP
      size: 4
      address: 133244
    - index: 20
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133248
    - index: 21
      opcode: LIl
      size: 8
      address: 133252
    - index: 22
      opcode: MULU
      size: 4
      address: 133260
    - index: 23
      opcode: NOP
      size: 4
      address: 133264
    - index: 24
      opcode: MFS
      size: 4
      address: 133268
    - index: 25
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133272
    - index: 26
      opcode: MFS
      size: 4
      address: 133276
    - index: 27
      opcode: MUL
      size: 4
      address: 133280
    - index: 28
      opcode: LIi
      size: 4
      address: 133284
    - index: 29
      opcode: MFS
      size: 4
      address: 133288
    - index: 30
      opcode: LIl
      size: 8
      address: 133292
    - index: 31
      opcode: MUL
      size: 4
      address: 133300
    - index: 32
      opcode: ADDi
      size: 4
      address: 133304
    - index: 33
      opcode: MFS
      size: 4
      address: 133308
    - index: 34
      opcode: ADDr
      size: 4
      address: 133312
    - index: 35
      opcode: ADDr
      size: 4
      address: 133316
    - index: 36
      opcode: CMPULT
      size: 4
      address: 133320
    - index: 37
      opcode: MOVpr
      size: 4
      address: 133324
    - index: 38
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133328
    - index: 39
      opcode: CMPIEQ
      size: 4
      address: 133332
    - index: 40
      opcode: CMOV
      size: 4
      address: 133336
    - index: 41
      opcode: SHADD2l
      size: 8
      address: 133340
    - index: 42
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133348
    - index: 43
      opcode: ADDr
      size: 4
      address: 133352
    - index: 44
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133356
    - index: 45
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133360
    - index: 46
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133364
    - index: 47
      opcode: NOP
      size: 4
      address: 133368
    - index: 48
      opcode: SHADD2l
      size: 8
      address: 133372
    - index: 49
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133380
    - index: 50
      opcode: NOP
      size: 4
      address: 133384
    - index: 51
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133388
    - index: 52
      opcode: NOP
      size: 4
      address: 133392
    - index: 53
      opcode: ANDl
      size: 8
      address: 133396
    - index: 54
      opcode: LIl
      size: 8
      address: 133404
    - index: 55
      opcode: MULU
      size: 4
      address: 133412
    - index: 56
      opcode: NOP
      size: 4
      address: 133416
    - index: 57
      opcode: MFS
      size: 4
      address: 133420
    - index: 58
      opcode: SRi
      size: 4
      address: 133424
    - index: 59
      opcode: LIi
      size: 4
      address: 133428
    - index: 60
      opcode: MUL
      size: 4
      address: 133432
    - index: 61
      opcode: NOP
      size: 4
      address: 133436
    - index: 62
      opcode: MFS
      size: 4
      address: 133440
    - index: 63
      opcode: SUBr
      size: 4
      address: 133444
    - index: 64
      opcode: CALL
      callees:
      - gen_sort
      size: 4
      branch-type: call
      branch-delay-slots: 3
      address: 133448
    - index: 65
      opcode: ADDi
      size: 4
      address: 133452
    - index: 66
      opcode: SUBi
      size: 4
      address: 133456
    - index: 67
      opcode: ADDi
      size: 4
      address: 133460
    - index: 68
      opcode: SENSi
      size: 4
      stack-cache-argument: 3
      address: 133464
    - index: 69
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133468
    - index: 70
      opcode: NOP
      size: 4
      address: 133472
    - index: 71
      opcode: MTS
      size: 4
      address: 133476
    - index: 72
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133480
    - index: 73
      opcode: MOV
      size: 4
      address: 133484
    - index: 74
      opcode: MTS
      size: 4
      address: 133488
    - index: 75
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133492
    - index: 76
      opcode: RET
      size: 4
      branch-type: return
      branch-delay-slots: 3
      address: 133496
    - index: 77
      opcode: ADDi
      size: 4
      address: 133500
    - index: 78
      opcode: MTS
      size: 4
      address: 133504
    - index: 79
      opcode: SFREEi
      size: 4
      stack-cache-argument: 3
      address: 133508
    address: 133156
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 7
  level: machinecode
  mapsto: gen_sort
  arguments:
  - name: ! '%N'
    index: 1
    registers:
    - r4
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors:
    - 1
    instructions:
    - index: 0
      opcode: SRESi
      size: 4
      stack-cache-argument: 10
      address: 132644
    - index: 1
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132648
    - index: 2
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132652
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132656
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132660
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132664
    - index: 6
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132668
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132672
    - index: 8
      opcode: MFS
      size: 4
      address: 132676
    - index: 9
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132680
    - index: 10
      opcode: LIi
      size: 4
      address: 132684
    - index: 11
      opcode: LIl
      size: 8
      address: 132688
    - index: 12
      opcode: LIl
      size: 8
      address: 132696
    - index: 13
      opcode: MOV
      size: 4
      address: 132704
    - index: 14
      opcode: MOV
      size: 4
      address: 132708
    - index: 15
      opcode: LIl
      size: 8
      address: 132712
    - index: 16
      opcode: MOV
      size: 4
      address: 132720
    - index: 17
      opcode: MFS
      size: 4
      address: 132724
    - index: 18
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 1
      address: 132728
    - index: 19
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132732
    - index: 20
      opcode: MFS
      size: 4
      address: 132736
    - index: 21
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 132740
    address: 132644
  - name: 1
    mapsto: for.cond
    predecessors:
    - 0
    - 9
    successors:
    - 7
    - 9
    loops:
    - 1
    instructions:
    - index: 0
      opcode: CMPEQ
      size: 4
      address: 132756
    - index: 1
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 9
      address: 132760
    - index: 2
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 7
      address: 132764
    address: 132756
  - name: 2
    mapsto: while.body.i
    predecessors:
    - 4
    successors:
    - 5
    loops:
    - 5
    - 7
    instructions:
    - index: 0
      opcode: BRu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 2
      branch-targets:
      - 5
      address: 132768
    - index: 1
      opcode: SUBi
      size: 4
      address: 132772
    - index: 2
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 132776
    address: 132768
  - name: 3
    mapsto: while.end.i
    predecessors:
    - 5
    - 4
    successors:
    - 7
    loops:
    - 7
    instructions:
    - index: 0
      opcode: ADDi
      size: 4
      address: 132780
    - index: 1
      opcode: BRu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 2
      branch-targets:
      - 7
      address: 132784
    - index: 2
      opcode: SHADD2r
      size: 4
      address: 132788
    - index: 3
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 132792
    address: 132780
  - name: 4
    mapsto: land.rhs.i
    predecessors:
    - 5
    successors:
    - 3
    - 2
    loops:
    - 5
    - 7
    instructions:
    - index: 0
      opcode: SHADD2r
      size: 4
      address: 132796
    - index: 1
      opcode: SUBi
      size: 4
      address: 132800
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 132804
    - index: 3
      opcode: NOP
      size: 4
      address: 132808
    - index: 4
      opcode: CMPLT
      size: 4
      address: 132812
    - index: 5
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 2
      address: 132816
    - index: 6
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 3
      address: 132820
    address: 132796
  - name: 5
    mapsto: while.cond.i
    predecessors:
    - 6
    - 2
    successors:
    - 4
    - 3
    loops:
    - 5
    - 7
    instructions:
    - index: 0
      opcode: CMPILT
      size: 4
      address: 132824
    - index: 1
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 3
      address: 132828
    - index: 2
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 4
      address: 132832
    address: 132824
  - name: 6
    mapsto: for.body.i
    predecessors:
    - 7
    successors:
    - 5
    loops:
    - 7
    instructions:
    - index: 0
      opcode: SHADD2r
      size: 4
      address: 132836
    - index: 1
      opcode: BRu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 2
      branch-targets:
      - 5
      address: 132840
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 132844
    - index: 3
      opcode: MOV
      size: 4
      address: 132848
    address: 132836
  - name: 7
    mapsto: for.cond.i
    predecessors:
    - 3
    - 1
    successors:
    - 6
    - 8
    loops:
    - 7
    instructions:
    - index: 0
      opcode: CMPULE
      size: 4
      address: 132852
    - index: 1
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 8
      address: 132856
    - index: 2
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 6
      address: 132860
    address: 132852
  - name: 8
    mapsto: sort.exit
    predecessors:
    - 7
    successors: []
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132864
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132868
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132872
    - index: 3
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132876
    - index: 4
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132880
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132884
    - index: 6
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132888
    - index: 7
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132892
    - index: 8
      opcode: MTS
      size: 4
      address: 132896
    - index: 9
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132900
    - index: 10
      opcode: NOP
      size: 4
      address: 132904
    - index: 11
      opcode: MTS
      size: 4
      address: 132908
    - index: 12
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 132912
    - index: 13
      opcode: RET
      size: 4
      branch-type: return
      branch-delay-slots: 3
      address: 132916
    - index: 14
      opcode: NOP
      size: 4
      address: 132920
    - index: 15
      opcode: MTS
      size: 4
      address: 132924
    - index: 16
      opcode: SFREEi
      size: 4
      stack-cache-argument: 10
      address: 132928
    address: 132864
  - name: 9
    mapsto: for.body
    predecessors:
    - 1
    successors:
    - 1
    loops:
    - 1
    instructions:
    - index: 0
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 132948
    - index: 1
      opcode: NOP
      size: 4
      address: 132952
    - index: 2
      opcode: SHADD2l
      size: 8
      address: 132956
    - index: 3
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 132964
    - index: 4
      opcode: NOP
      size: 4
      address: 132968
    - index: 5
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 132972
    - index: 6
      opcode: NOP
      size: 4
      address: 132976
    - index: 7
      opcode: MULU
      size: 4
      address: 132980
    - index: 8
      opcode: NOP
      size: 4
      address: 132984
    - index: 9
      opcode: MFS
      size: 4
      address: 132988
    - index: 10
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 132992
    - index: 11
      opcode: MFS
      size: 4
      address: 132996
    - index: 12
      opcode: MUL
      size: 4
      address: 133000
    - index: 13
      opcode: NOP
      size: 4
      address: 133004
    - index: 14
      opcode: MFS
      size: 4
      address: 133008
    - index: 15
      opcode: MUL
      size: 4
      address: 133012
    - index: 16
      opcode: ADDi
      size: 4
      address: 133016
    - index: 17
      opcode: MFS
      size: 4
      address: 133020
    - index: 18
      opcode: ADDr
      size: 4
      address: 133024
    - index: 19
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133028
    - index: 20
      opcode: ADDr
      size: 4
      address: 133032
    - index: 21
      opcode: SHADD2l
      size: 8
      address: 133036
    - index: 22
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133044
    - index: 23
      opcode: CMPULT
      size: 4
      address: 133048
    - index: 24
      opcode: MOVpr
      size: 4
      address: 133052
    - index: 25
      opcode: CMPIEQ
      size: 4
      address: 133056
    - index: 26
      opcode: CMOV
      size: 4
      address: 133060
    - index: 27
      opcode: ADDr
      size: 4
      address: 133064
    - index: 28
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133068
    - index: 29
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133072
    - index: 30
      opcode: LWL
      size: 4
      memmode: load
      memtype: local
      address: 133076
    - index: 31
      opcode: NOP
      size: 4
      address: 133080
    - index: 32
      opcode: SHADD2l
      size: 8
      address: 133084
    - index: 33
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133092
    - index: 34
      opcode: NOP
      size: 4
      address: 133096
    - index: 35
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133100
    - index: 36
      opcode: CALL
      callees:
      - __umodsi3
      size: 4
      branch-type: call
      branch-delay-slots: 3
      address: 133104
    - index: 37
      opcode: NOP
      size: 4
      address: 133108
    - index: 38
      opcode: ANDl
      size: 8
      address: 133112
    - index: 39
      opcode: MOV
      size: 4
      address: 133120
    - index: 40
      opcode: SENSi
      size: 4
      stack-cache-argument: 10
      address: 133124
    - index: 41
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 1
      address: 133128
    - index: 42
      opcode: SHADD2r
      size: 4
      address: 133132
    - index: 43
      opcode: ADDi
      size: 4
      address: 133136
    - index: 44
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133140
    address: 132948
  subfunctions:
  - name: 0
    blocks:
    - 0
  - name: 1
    blocks:
    - 1
    - 2
    - 3
    - 4
    - 5
    - 6
    - 7
    - 8
  - name: 9
    blocks:
    - 9
- name: 165
  level: machinecode
  mapsto: __umodsi3
  arguments:
  - name: ! '%a'
    index: 0
    registers:
    - r3
  - name: ! '%b'
    index: 1
    registers:
    - r4
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors:
    - 1
    instructions:
    - index: 0
      opcode: MOV
      size: 4
      address: 200084
    - index: 1
      opcode: LIi
      size: 4
      address: 200088
    - index: 2
      opcode: LIi
      size: 4
      address: 200092
    - index: 3
      opcode: BRu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 2
      branch-targets:
      - 1
      address: 200096
    - index: 4
      opcode: MOV
      size: 4
      address: 200100
    - index: 5
      opcode: MFS
      size: 4
      address: 200104
    address: 200084
  - name: 2
    mapsto: for.body.i
    predecessors:
    - 1
    successors:
    - 1
    loops:
    - 1
    instructions:
    - index: 0
      opcode: SRr
      size: 4
      address: 200108
    - index: 1
      opcode: SLi
      size: 4
      address: 200112
    - index: 2
      opcode: ANDi
      size: 4
      address: 200116
    - index: 3
      opcode: ORr
      size: 4
      address: 200120
    - index: 4
      opcode: CMPULT
      size: 4
      address: 200124
    - index: 5
      opcode: SLr
      size: 4
      address: 200128
    - index: 6
      opcode: SUBr
      size: 4
      address: 200132
    - index: 7
      opcode: ORr
      size: 4
      address: 200136
    - index: 8
      opcode: SUBi
      size: 4
      address: 200140
    address: 200108
  - name: 1
    mapsto: for.cond.i
    predecessors:
    - 0
    - 2
    successors:
    - 2
    - 3
    loops:
    - 1
    instructions:
    - index: 0
      opcode: CMPILT
      size: 4
      address: 200144
    - index: 1
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 2
      address: 200148
    address: 200144
  - name: 3
    mapsto: __udivsi3.exit
    predecessors:
    - 1
    successors: []
    instructions:
    - index: 0
      opcode: MUL
      size: 4
      address: 200152
    - index: 1
      opcode: RET
      size: 4
      branch-type: return
      branch-delay-slots: 3
      address: 200156
    - index: 2
      opcode: MTS
      size: 4
      address: 200160
    - index: 3
      opcode: MFS
      size: 4
      address: 200164
    - index: 4
      opcode: SUBr
      size: 4
      address: 200168
    address: 200152
  subfunctions:
  - name: 0
    blocks:
    - 0
    - 2
    - 1
    - 3
valuefacts:
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 8
    block: 0
    instruction: 11
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 8
    block: 0
    instruction: 18
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 8
    block: 0
    instruction: 42
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 8
    block: 0
    instruction: 49
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 7
    block: 9
    instruction: 3
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 7
    block: 9
    instruction: 22
- level: machinecode
  origin: llvm.mc
  variable: mem-address-read
  values:
  - symbol: _reent_ptr
  program-point:
    function: 7
    block: 9
    instruction: 33
bitcode-functions:
- name: main
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: alloca
    - index: 1
      opcode: load
      memmode: load
    - index: 2
      opcode: getelementptr
    - index: 3
      opcode: load
      memmode: load
    - index: 4
      opcode: getelementptr
    - index: 5
      opcode: bitcast
    - index: 6
      opcode: store
      memmode: store
    - index: 7
      opcode: bitcast
    - index: 8
      opcode: call
    - index: 9
      opcode: load
      memmode: load
    - index: 10
      opcode: getelementptr
    - index: 11
      opcode: load
      memmode: load
    - index: 12
      opcode: getelementptr
    - index: 13
      opcode: bitcast
    - index: 14
      opcode: load
      memmode: load
    - index: 15
      opcode: mul
    - index: 16
      opcode: add
    - index: 17
      opcode: load
      memmode: load
    - index: 18
      opcode: getelementptr
    - index: 19
      opcode: load
      memmode: load
    - index: 20
      opcode: getelementptr
    - index: 21
      opcode: bitcast
    - index: 22
      opcode: store
      memmode: store
    - index: 23
      opcode: load
      memmode: load
    - index: 24
      opcode: getelementptr
    - index: 25
      opcode: load
      memmode: load
    - index: 26
      opcode: getelementptr
    - index: 27
      opcode: bitcast
    - index: 28
      opcode: load
      memmode: load
    - index: 29
      opcode: lshr
    - index: 30
      opcode: trunc
    - index: 31
      opcode: and
    - index: 32
      opcode: urem
    - index: 33
      opcode: add
    - index: 34
      opcode: bitcast
    - index: 35
      opcode: call
      callees:
      - gen_sort
    - index: 36
      opcode: call
    - index: 37
      opcode: ret
- name: gen_sort
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors:
    - for.cond
    instructions:
    - index: 0
      opcode: br
  - name: for.cond
    predecessors:
    - for.body
    - entry
    successors:
    - for.cond.i
    - for.body
    loops:
    - for.cond
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: call
    - index: 2
      opcode: icmp
    - index: 3
      opcode: br
  - name: for.body
    predecessors:
    - for.cond
    successors:
    - for.cond
    loops:
    - for.cond
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: getelementptr
    - index: 2
      opcode: load
      memmode: load
    - index: 3
      opcode: getelementptr
    - index: 4
      opcode: bitcast
    - index: 5
      opcode: load
      memmode: load
    - index: 6
      opcode: mul
    - index: 7
      opcode: add
    - index: 8
      opcode: load
      memmode: load
    - index: 9
      opcode: getelementptr
    - index: 10
      opcode: load
      memmode: load
    - index: 11
      opcode: getelementptr
    - index: 12
      opcode: bitcast
    - index: 13
      opcode: store
      memmode: store
    - index: 14
      opcode: load
      memmode: load
    - index: 15
      opcode: getelementptr
    - index: 16
      opcode: load
      memmode: load
    - index: 17
      opcode: getelementptr
    - index: 18
      opcode: bitcast
    - index: 19
      opcode: load
      memmode: load
    - index: 20
      opcode: lshr
    - index: 21
      opcode: trunc
    - index: 22
      opcode: and
    - index: 23
      opcode: urem
    - index: 24
      opcode: getelementptr
    - index: 25
      opcode: store
      memmode: store
    - index: 26
      opcode: add
    - index: 27
      opcode: br
  - name: for.cond.i
    predecessors:
    - for.cond
    - while.end.i
    successors:
    - for.body.i
    - sort.exit
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: call
    - index: 2
      opcode: icmp
    - index: 3
      opcode: br
  - name: for.body.i
    predecessors:
    - for.cond.i
    successors:
    - while.cond.i
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: getelementptr
    - index: 1
      opcode: load
      memmode: load
    - index: 2
      opcode: br
  - name: while.cond.i
    predecessors:
    - while.body.i
    - for.body.i
    successors:
    - land.rhs.i
    - while.end.i
    loops:
    - while.cond.i
    - for.cond.i
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: add
    - index: 2
      opcode: call
    - index: 3
      opcode: icmp
    - index: 4
      opcode: br
  - name: land.rhs.i
    predecessors:
    - while.cond.i
    successors:
    - while.end.i
    - while.body.i
    loops:
    - while.cond.i
    - for.cond.i
    instructions:
    - index: 0
      opcode: getelementptr
    - index: 1
      opcode: getelementptr
    - index: 2
      opcode: load
      memmode: load
    - index: 3
      opcode: icmp
    - index: 4
      opcode: br
  - name: while.body.i
    predecessors:
    - land.rhs.i
    successors:
    - while.cond.i
    loops:
    - while.cond.i
    - for.cond.i
    instructions:
    - index: 0
      opcode: getelementptr
    - index: 1
      opcode: store
      memmode: store
    - index: 2
      opcode: br
  - name: while.end.i
    predecessors:
    - land.rhs.i
    - while.cond.i
    successors:
    - for.cond.i
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: getelementptr
    - index: 1
      opcode: store
      memmode: store
    - index: 2
      opcode: add
    - index: 3
      opcode: br
  - name: sort.exit
    predecessors:
    - for.cond.i
    successors: []
    instructions:
    - index: 0
      opcode: ret
- name: __umodsi3
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors:
    - for.cond.i
    instructions:
    - index: 0
      opcode: br
  - name: for.cond.i
    predecessors:
    - for.inc.i
    - entry
    successors:
    - for.body.i
    - __udivsi3.exit
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: phi
    - index: 2
      opcode: phi
    - index: 3
      opcode: call
    - index: 4
      opcode: icmp
    - index: 5
      opcode: br
  - name: for.body.i
    predecessors:
    - for.cond.i
    successors:
    - for.inc.i
    - if.then.i
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: shl
    - index: 1
      opcode: lshr
    - index: 2
      opcode: and
    - index: 3
      opcode: or
    - index: 4
      opcode: icmp
    - index: 5
      opcode: br
  - name: if.then.i
    predecessors:
    - for.body.i
    successors:
    - for.inc.i
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: sub
    - index: 1
      opcode: shl
    - index: 2
      opcode: or
    - index: 3
      opcode: br
  - name: for.inc.i
    predecessors:
    - if.then.i
    - for.body.i
    successors:
    - for.cond.i
    loops:
    - for.cond.i
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: phi
    - index: 2
      opcode: add
    - index: 3
      opcode: br
  - name: __udivsi3.exit
    predecessors:
    - for.cond.i
    successors: []
    instructions:
    - index: 0
      opcode: mul
    - index: 1
      opcode: sub
    - index: 2
      opcode: ret
flowfacts:
- scope:
    function: gen_sort
    loop: for.cond
  lhs:
  - factor: 1
    program-point:
      function: gen_sort
      block: for.cond
  op: less-equal
  rhs: (1 + %N)
  level: bitcode
  origin: llvm.bc
  classification: loop-global
- scope:
    function: gen_sort
    loop: for.cond
  lhs:
  - factor: 1
    program-point:
      function: gen_sort
      block: for.cond
  op: less-equal
  rhs: 101
  level: bitcode
  origin: user.bc
  classification: loop-global
- scope:
    function: gen_sort
    loop: for.cond.i
  lhs:
  - factor: 1
    program-point:
      function: gen_sort
      block: for.cond.i
  op: less-equal
  rhs: (1 umax %N)
  level: bitcode
  origin: llvm.bc
  classification: loop-global
- scope:
    function: gen_sort
    loop: for.cond.i
  lhs:
  - factor: 1
    program-point:
      function: gen_sort
      block: for.cond.i
  op: less-equal
  rhs: 100
  level: bitcode
  origin: user.bc
  classification: loop-global
- scope:
    function: gen_sort
    loop: while.cond.i
  lhs:
  - factor: 1
    program-point:
      function: gen_sort
      block: while.cond.i
  op: less-equal
  rhs: 100
  level: bitcode
  origin: user.bc
  classification: loop-global
- scope:
    function: __umodsi3
    loop: for.cond.i
  lhs:
  - factor: 1
    program-point:
      function: __umodsi3
      block: for.cond.i
  op: less-equal
  rhs: 33
  level: bitcode
  origin: llvm.bc
  classification: loop-global
- scope:
    function: __umodsi3
    loop: for.cond.i
  lhs:
  - factor: 1
    program-point:
      function: __umodsi3
      block: for.cond.i
  op: less-equal
  rhs: 33
  level: bitcode
  origin: llvm.bc
  classification: loop-global
- scope:
    function: __umodsi3
    loop: for.cond.i
  lhs:
  - factor: 1
    program-point:
      function: __umodsi3
      block: for.cond.i
  op: less-equal
  rhs: 33
  level: bitcode
  origin: user.bc
  classification: loop-global
- scope:
    function: 165
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  level: machinecode
  origin: trace
- scope:
    function: 7
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 1
  op: less-equal
  rhs: '51'
  level: machinecode
  origin: trace
- scope:
    function: 7
    loop: 5
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 5
  op: less-equal
  rhs: '41'
  level: machinecode
  origin: trace
- scope:
    function: 7
    loop: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 7
  op: less-equal
  rhs: '50'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 0
  op: less-equal
  rhs: '1'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 1
  op: less-equal
  rhs: '51'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 2
  op: less-equal
  rhs: '505'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 3
  op: less-equal
  rhs: '49'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 4
  op: less-equal
  rhs: '552'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 5
  op: less-equal
  rhs: '554'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 6
  op: less-equal
  rhs: '49'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 7
  op: less-equal
  rhs: '50'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 8
  op: less-equal
  rhs: '1'
  level: machinecode
  origin: trace
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 9
  op: less-equal
  rhs: '50'
  level: machinecode
  origin: trace
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 0
  op: less-equal
  rhs: '1'
  level: machinecode
  origin: trace
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 2
  op: less-equal
  rhs: '32'
  level: machinecode
  origin: trace
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  level: machinecode
  origin: trace
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 3
  op: less-equal
  rhs: '1'
  level: machinecode
  origin: trace
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  origin: llvm
  level: machinecode
- scope:
    function: 7
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 1
  op: less-equal
  rhs: (1 + r4)
  origin: llvm
  level: machinecode
- scope:
    function: 7
    loop: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 7
  op: less-equal
  rhs: (1 umax r4)
  origin: llvm
  level: machinecode
- scope:
    function: 165
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  origin: llvm
  level: machinecode
- scope:
    function: 165
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  origin: llvm
  level: machinecode
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 0
  - factor: -1
    program-point:
      function: 7
      block: 9
  op: less-equal
  rhs: '0'
  origin: user
  level: machinecode
- scope:
    function: 7
  lhs:
  - factor: -99
    program-point:
      function: 7
      block: 3
  - factor: 1
    program-point:
      function: 7
      block: 2
  op: less-equal
  rhs: '0'
  origin: user
  level: machinecode
- scope:
    function: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 3
  - factor: -99
    program-point:
      function: 7
      block: 8
  op: less-equal
  rhs: '0'
  origin: user
  level: machinecode
- scope:
    function: 7
  lhs:
  - factor: -100
    program-point:
      function: 7
      block: 8
  - factor: 1
    program-point:
      function: 7
      block: 9
  op: less-equal
  rhs: '0'
  origin: user
  level: machinecode
- scope:
    function: 165
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  origin: user
  level: machinecode
- scope:
    function: 7
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 1
  op: less-equal
  rhs: '101'
  origin: user
  level: machinecode
- scope:
    function: 7
    loop: 7
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 7
  op: less-equal
  rhs: '100'
  origin: user
  level: machinecode
- scope:
    function: 7
    loop: 5
  lhs:
  - factor: 1
    program-point:
      function: 7
      block: 5
  op: less-equal
  rhs: '100'
  origin: user
  level: machinecode
- scope:
    function: 165
    loop: 1
  lhs:
  - factor: 1
    program-point:
      function: 165
      block: 1
  op: less-equal
  rhs: '33'
  origin: user
  level: machinecode
relation-graphs:
- src:
    function: main
    level: bitcode
  dst:
    function: 8
    level: machinecode
  nodes:
  - name: 0
    type: entry
    src-block: entry
    dst-block: 0
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 1
    type: exit
  status: valid
- src:
    function: gen_sort
    level: bitcode
  dst:
    function: 7
    level: machinecode
  nodes:
  - name: 0
    type: entry
    src-block: entry
    dst-block: 0
    src-successors:
    - 2
    dst-successors:
    - 2
  - name: 1
    type: exit
  - name: 2
    type: progress
    src-block: for.cond
    dst-block: 1
    src-successors:
    - 3
    - 4
    dst-successors:
    - 3
    - 4
  - name: 3
    type: progress
    src-block: for.body
    dst-block: 9
    src-successors:
    - 2
    dst-successors:
    - 2
  - name: 4
    type: progress
    src-block: for.cond.i
    dst-block: 7
    src-successors:
    - 5
    - 6
    dst-successors:
    - 5
    - 6
  - name: 5
    type: progress
    src-block: for.body.i
    dst-block: 6
    src-successors:
    - 7
    dst-successors:
    - 7
  - name: 6
    type: progress
    src-block: sort.exit
    dst-block: 8
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 7
    type: progress
    src-block: while.cond.i
    dst-block: 5
    src-successors:
    - 8
    - 9
    dst-successors:
    - 8
    - 9
  - name: 8
    type: progress
    src-block: land.rhs.i
    dst-block: 4
    src-successors:
    - 10
    - 9
    dst-successors:
    - 10
    - 9
  - name: 9
    type: progress
    src-block: while.end.i
    dst-block: 3
    src-successors:
    - 4
    dst-successors:
    - 4
  - name: 10
    type: progress
    src-block: while.body.i
    dst-block: 2
    src-successors:
    - 7
    dst-successors:
    - 7
  status: valid
- src:
    function: __umodsi3
    level: bitcode
  dst:
    function: 165
    level: machinecode
  nodes:
  - name: 0
    type: entry
    src-block: entry
    dst-block: 0
    src-successors:
    - 2
    dst-successors:
    - 2
  - name: 1
    type: exit
  - name: 2
    type: progress
    src-block: for.cond.i
    dst-block: 1
    src-successors:
    - 3
    - 4
    dst-successors:
    - 3
    - 4
  - name: 3
    type: progress
    src-block: __udivsi3.exit
    dst-block: 3
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 4
    type: progress
    src-block: for.body.i
    dst-block: 2
    src-successors:
    - 5
    - 6
    dst-successors:
    - 2
  - name: 5
    type: src
    src-block: for.inc.i
    src-successors:
    - 2
  - name: 6
    type: src
    src-block: if.then.i
    src-successors:
    - 5
  status: valid
timing:
- scope:
    function: 7
  cycles: 49089
  level: machinecode
  origin: trace
- scope:
    function: 7
  cycles: 644867
  level: machinecode
  origin: platin
  cache-max-cycles-instr: 651
  cache-min-hits-instr: 398
  cache-max-misses-instr: 3
  cache-max-cycles-stack: 0
  cache-max-misses-stack: 0
  cache-max-cycles-data: 436779
  cache-min-hits-data: 0
  cache-max-misses-data: 10599
  cache-max-stores-data: 10200
  cache-unknown-address-data: 20799
  cache-max-cycles: 437430
  profile:
  - reference:
      function: 7
      edgesource: 0
      edgetarget: 1
    cycles: 22
    wcet-frequency: 1
    wcet-contribution: 22
  - reference:
      function: 7
      edgesource: 1
      edgetarget: 7
    cycles: 5
    wcet-frequency: 1
    wcet-contribution: 5
  - reference:
      function: 7
      edgesource: 1
      edgetarget: 9
    cycles: 257
    wcet-frequency: 100
    wcet-contribution: 752
  - reference:
      function: 7
      edgesource: 2
      edgetarget: 5
    cycles: 24
    wcet-frequency: 9801
    wcet-contribution: 235224
  - reference:
      function: 7
      edgesource: 3
      edgetarget: 7
    cycles: 25
    wcet-frequency: 99
    wcet-contribution: 2475
  - reference:
      function: 7
      edgesource: 4
      edgetarget: 3
    cycles: 30
    wcet-frequency: 99
    wcet-contribution: 2970
  - reference:
      function: 7
      edgesource: 4
      edgetarget: 2
    cycles: 29
    wcet-frequency: 9801
    wcet-contribution: 284229
  - reference:
      function: 7
      edgesource: 5
      edgetarget: 4
    cycles: 5
    wcet-frequency: 9900
    wcet-contribution: 49500
  - reference:
      function: 7
      edgesource: 6
      edgetarget: 5
    cycles: 25
    wcet-frequency: 99
    wcet-contribution: 2475
  - reference:
      function: 7
      edgesource: 7
      edgetarget: 6
    cycles: 5
    wcet-frequency: 99
    wcet-contribution: 495
  - reference:
      function: 7
      edgesource: 7
      edgetarget: 8
    cycles: 4
    wcet-frequency: 1
    wcet-contribution: 4
  - reference:
      function: 7
      edgesource: 8
    cycles: 17
    wcet-frequency: 1
    wcet-contribution: 17
  - reference:
      function: 7
      edgesource: 9
      edgetarget: 1
    cycles: 507
    wcet-frequency: 100
    wcet-contribution: 23673
  - reference:
      function: 165
      edgesource: 0
      edgetarget: 1
    cycles: 132
    wcet-frequency: 100
    wcet-contribution: 726
  - reference:
      function: 165
      edgesource: 2
      edgetarget: 1
    cycles: 9
    wcet-frequency: 3200
    wcet-contribution: 28800
  - reference:
      function: 165
      edgesource: 1
      edgetarget: 2
    cycles: 4
    wcet-frequency: 3200
    wcet-contribution: 12800
  - reference:
      function: 165
      edgesource: 1
      edgetarget: 3
    cycles: 2
    wcet-frequency: 100
    wcet-contribution: 200
  - reference:
      function: 165
      edgesource: 3
    cycles: 5
    wcet-frequency: 100
    wcet-contribution: 500
