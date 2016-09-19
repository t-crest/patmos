---
format: pml-0.1
triple: patmos-unknown-unknown-elf
machine-functions:
- name: 10
  level: machinecode
  mapsto: main
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
      stack-cache-argument: 8
      address: 133572
    - index: 1
      opcode: SUBi
      size: 4
      address: 133576
    - index: 2
      opcode: MFS
      size: 4
      address: 133580
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133584
    - index: 4
      opcode: MFS
      size: 4
      address: 133588
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133592
    - index: 6
      opcode: LIl
      size: 8
      address: 133596
    - index: 7
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133604
    - index: 8
      opcode: LIl
      size: 8
      address: 133608
    - index: 9
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133616
    - index: 10
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133620
    - index: 11
      opcode: NOP
      size: 4
      address: 133624
    - index: 12
      opcode: MOV
      size: 4
      address: 133628
    - index: 13
      opcode: CALLND
      callees:
      - floatMult
      size: 4
      branch-type: call
      address: 133632
    - index: 14
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 133636
    - index: 15
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133640
    - index: 16
      opcode: MOV
      size: 4
      address: 133644
    - index: 17
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133648
    - index: 18
      opcode: NOP
      size: 4
      address: 133652
    - index: 19
      opcode: MTS
      size: 4
      address: 133656
    - index: 20
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133660
    - index: 21
      opcode: NOP
      size: 4
      address: 133664
    - index: 22
      opcode: MTS
      size: 4
      address: 133668
    - index: 23
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 133672
    - index: 24
      opcode: ADDi
      size: 4
      address: 133676
    - index: 25
      opcode: RETND
      size: 4
      branch-type: return
      address: 133680
    address: 133572
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 8
  level: machinecode
  mapsto: floatMult
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
      stack-cache-argument: 8
      address: 133412
    - index: 1
      opcode: SUBi
      size: 4
      address: 133416
    - index: 2
      opcode: MFS
      size: 4
      address: 133420
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133424
    - index: 4
      opcode: MFS
      size: 4
      address: 133428
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133432
    - index: 6
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133436
    - index: 7
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133440
    - index: 8
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133444
    - index: 9
      opcode: NOP
      size: 4
      address: 133448
    - index: 10
      opcode: CALLND
      callees:
      - __mulsf3
      size: 4
      branch-type: call
      address: 133452
    - index: 11
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 133456
    - index: 12
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133460
    - index: 13
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133464
    - index: 14
      opcode: NOP
      size: 4
      address: 133468
    - index: 15
      opcode: MTS
      size: 4
      address: 133472
    - index: 16
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133476
    - index: 17
      opcode: NOP
      size: 4
      address: 133480
    - index: 18
      opcode: MTS
      size: 4
      address: 133484
    - index: 19
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 133488
    - index: 20
      opcode: ADDi
      size: 4
      address: 133492
    - index: 21
      opcode: RETND
      size: 4
      branch-type: return
      address: 133496
    address: 133412
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 240
  level: machinecode
  mapsto: __mulsf3
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors:
    - 2
    - 1
    instructions:
    - index: 0
      opcode: SRESi
      size: 4
      stack-cache-argument: 24
      address: 324260
    - index: 1
      opcode: SUBi
      size: 4
      address: 324264
    - index: 2
      opcode: MFS
      size: 4
      address: 324268
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324272
    - index: 4
      opcode: MFS
      size: 4
      address: 324276
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324280
    - index: 6
      opcode: MFS
      size: 4
      address: 324284
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324288
    - index: 8
      opcode: MOV
      size: 4
      address: 324292
    - index: 9
      opcode: MOV
      size: 4
      address: 324296
    - index: 10
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324300
    - index: 11
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324304
    - index: 12
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324308
    - index: 13
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324312
    - index: 14
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324316
    - index: 15
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324320
    - index: 16
      opcode: SRi
      size: 4
      address: 324324
    - index: 17
      opcode: ANDi
      size: 4
      address: 324328
    - index: 18
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324332
    - index: 19
      opcode: NOP
      size: 4
      address: 324336
    - index: 20
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324340
    - index: 21
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324344
    - index: 22
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324348
    - index: 23
      opcode: SRi
      size: 4
      address: 324352
    - index: 24
      opcode: ANDi
      size: 4
      address: 324356
    - index: 25
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324360
    - index: 26
      opcode: NOP
      size: 4
      address: 324364
    - index: 27
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324368
    - index: 28
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324372
    - index: 29
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324376
    - index: 30
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324380
    - index: 31
      opcode: NOP
      size: 4
      address: 324384
    - index: 32
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324388
    - index: 33
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324392
    - index: 34
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324396
    - index: 35
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324400
    - index: 36
      opcode: NOP
      size: 4
      address: 324404
    - index: 37
      opcode: XORr
      size: 4
      address: 324408
    - index: 38
      opcode: ANDl
      size: 8
      address: 324412
    - index: 39
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324420
    - index: 40
      opcode: NOP
      size: 4
      address: 324424
    - index: 41
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324428
    - index: 42
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324432
    - index: 43
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324436
    - index: 44
      opcode: ANDl
      size: 8
      address: 324440
    - index: 45
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 324448
    - index: 46
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324452
    - index: 47
      opcode: NOP
      size: 4
      address: 324456
    - index: 48
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324460
    - index: 49
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324464
    - index: 50
      opcode: ANDl
      size: 8
      address: 324468
    - index: 51
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 324476
    - index: 52
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324480
    - index: 53
      opcode: NOP
      size: 4
      address: 324484
    - index: 54
      opcode: SUBi
      size: 4
      address: 324488
    - index: 55
      opcode: LIi
      size: 4
      address: 324492
    - index: 56
      opcode: CMPULT
      size: 4
      address: 324496
    - index: 57
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 2
      address: 324500
    - index: 58
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 1
      address: 324504
    address: 324260
  - name: 1
    mapsto: lor.lhs.false
    predecessors:
    - 0
    successors:
    - 2
    - 23
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324516
    - index: 1
      opcode: NOP
      size: 4
      address: 324520
    - index: 2
      opcode: SUBi
      size: 4
      address: 324524
    - index: 3
      opcode: MOV
      size: 4
      address: 324528
    - index: 4
      opcode: LIi
      size: 4
      address: 324532
    - index: 5
      opcode: CMPULT
      size: 4
      address: 324536
    - index: 6
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324540
    - index: 7
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 23
      address: 324544
    - index: 8
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 2
      address: 324548
    address: 324516
  - name: 2
    mapsto: if.then
    predecessors:
    - 0
    - 1
    successors:
    - 3
    - 4
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324564
    - index: 1
      opcode: NOP
      size: 4
      address: 324568
    - index: 2
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324572
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324576
    - index: 4
      opcode: ANDl
      size: 8
      address: 324580
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324588
    - index: 6
      opcode: NOP
      size: 4
      address: 324592
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324596
    - index: 8
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324600
    - index: 9
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324604
    - index: 10
      opcode: ANDl
      size: 8
      address: 324608
    - index: 11
      opcode: LIl
      size: 8
      address: 324616
    - index: 12
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324624
    - index: 13
      opcode: NOP
      size: 4
      address: 324628
    - index: 14
      opcode: CMPULT
      size: 4
      address: 324632
    - index: 15
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324636
    - index: 16
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 4
      address: 324640
    - index: 17
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 3
      address: 324644
    address: 324564
  - name: 3
    mapsto: if.then1
    predecessors:
    - 2
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324660
    - index: 1
      opcode: NOP
      size: 4
      address: 324664
    - index: 2
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324668
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324672
    - index: 4
      opcode: ORl
      size: 8
      address: 324676
    - index: 5
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 324684
    - index: 6
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324688
    - index: 7
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 324692
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324696
    - index: 9
      opcode: NOP
      size: 4
      address: 324700
    - index: 10
      opcode: NOP
      size: 4
      address: 324704
    address: 324660
  - name: 4
    mapsto: if.end
    predecessors:
    - 2
    successors:
    - 9
    - 5
    instructions:
    - index: 0
      opcode: LIl
      size: 8
      address: 324724
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324732
    - index: 2
      opcode: NOP
      size: 4
      address: 324736
    - index: 3
      opcode: CMPULT
      size: 4
      address: 324740
    - index: 4
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 5
      address: 324744
    - index: 5
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 9
      address: 324748
    address: 324724
  - name: 5
    mapsto: if.end3
    predecessors:
    - 4
    successors:
    - 6
    - 7
    instructions:
    - index: 0
      opcode: LIl
      size: 8
      address: 324752
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324760
    - index: 2
      opcode: NOP
      size: 4
      address: 324764
    - index: 3
      opcode: CMPNEQ
      size: 4
      address: 324768
    - index: 4
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 7
      address: 324772
    - index: 5
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 6
      address: 324776
    address: 324752
  - name: 6
    mapsto: if.then4
    predecessors:
    - 5
    successors:
    - 11
    - 10
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324780
    - index: 1
      opcode: NOP
      size: 4
      address: 324784
    - index: 2
      opcode: CMPIEQ
      size: 4
      address: 324788
    - index: 3
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 11
      address: 324792
    - index: 4
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 10
      address: 324796
    address: 324780
  - name: 7
    mapsto: if.end6
    predecessors:
    - 5
    successors:
    - 8
    - 14
    instructions:
    - index: 0
      opcode: LIl
      size: 8
      address: 324800
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324808
    - index: 2
      opcode: NOP
      size: 4
      address: 324812
    - index: 3
      opcode: CMPNEQ
      size: 4
      address: 324816
    - index: 4
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 14
      address: 324820
    - index: 5
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 8
      address: 324824
    address: 324800
  - name: 8
    mapsto: if.then7
    predecessors:
    - 7
    successors:
    - 13
    - 12
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324828
    - index: 1
      opcode: NOP
      size: 4
      address: 324832
    - index: 2
      opcode: CMPIEQ
      size: 4
      address: 324836
    - index: 3
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 13
      address: 324840
    - index: 4
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 12
      address: 324844
    address: 324828
  - name: 9
    mapsto: if.then2
    predecessors:
    - 4
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324852
    - index: 1
      opcode: NOP
      size: 4
      address: 324856
    - index: 2
      opcode: CALLND
      callees:
      - toRep321
      size: 4
      branch-type: call
      address: 324860
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324864
    - index: 4
      opcode: ORl
      size: 8
      address: 324868
    - index: 5
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 324876
    - index: 6
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324880
    - index: 7
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 324884
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324888
    - index: 9
      opcode: NOP
      size: 4
      address: 324892
    - index: 10
      opcode: NOP
      size: 4
      address: 324896
    address: 324852
  - name: 10
    mapsto: if.then5
    predecessors:
    - 6
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324916
    - index: 1
      opcode: NOP
      size: 4
      address: 324920
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 324924
    - index: 3
      opcode: NOP
      size: 4
      address: 324928
    - index: 4
      opcode: ORr
      size: 4
      address: 324932
    - index: 5
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 324936
    - index: 6
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324940
    - index: 7
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 324944
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324948
    - index: 9
      opcode: NOP
      size: 4
      address: 324952
    - index: 10
      opcode: NOP
      size: 4
      address: 324956
    address: 324916
  - name: 11
    mapsto: if.else
    predecessors:
    - 6
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LIl
      size: 8
      address: 324964
    - index: 1
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 324972
    - index: 2
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 324976
    - index: 3
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 324980
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 324984
    - index: 5
      opcode: NOP
      size: 4
      address: 324988
    - index: 6
      opcode: NOP
      size: 4
      address: 324992
    address: 324964
  - name: 12
    mapsto: if.then8
    predecessors:
    - 8
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325012
    - index: 1
      opcode: NOP
      size: 4
      address: 325016
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325020
    - index: 3
      opcode: NOP
      size: 4
      address: 325024
    - index: 4
      opcode: ORr
      size: 4
      address: 325028
    - index: 5
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 325032
    - index: 6
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325036
    - index: 7
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 325040
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325044
    - index: 9
      opcode: NOP
      size: 4
      address: 325048
    - index: 10
      opcode: NOP
      size: 4
      address: 325052
    address: 325012
  - name: 13
    mapsto: if.else9
    predecessors:
    - 8
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LIl
      size: 8
      address: 325060
    - index: 1
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 325068
    - index: 2
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325072
    - index: 3
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 325076
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325080
    - index: 5
      opcode: NOP
      size: 4
      address: 325084
    - index: 6
      opcode: NOP
      size: 4
      address: 325088
    address: 325060
  - name: 14
    mapsto: if.end10
    predecessors:
    - 7
    successors:
    - 17
    - 15
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325108
    - index: 1
      opcode: NOP
      size: 4
      address: 325112
    - index: 2
      opcode: MOVrp
      size: 4
      address: 325116
    - index: 3
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 15
      address: 325120
    - index: 4
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 17
      address: 325124
    address: 325108
  - name: 15
    mapsto: if.end12
    predecessors:
    - 14
    successors:
    - 18
    - 16
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325128
    - index: 1
      opcode: NOP
      size: 4
      address: 325132
    - index: 2
      opcode: MOVrp
      size: 4
      address: 325136
    - index: 3
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 16
      address: 325140
    - index: 4
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 18
      address: 325144
    address: 325128
  - name: 16
    mapsto: if.end14
    predecessors:
    - 15
    successors:
    - 19
    - 20
    instructions:
    - index: 0
      opcode: MOV
      size: 4
      address: 325148
    - index: 1
      opcode: LIl
      size: 8
      address: 325152
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325160
    - index: 3
      opcode: NOP
      size: 4
      address: 325164
    - index: 4
      opcode: CMPULT
      size: 4
      address: 325168
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325172
    - index: 6
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 20
      address: 325176
    - index: 7
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 19
      address: 325180
    address: 325148
  - name: 17
    mapsto: if.then11
    predecessors:
    - 14
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325188
    - index: 1
      opcode: NOP
      size: 4
      address: 325192
    - index: 2
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 325196
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325200
    - index: 4
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 325204
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325208
    - index: 6
      opcode: NOP
      size: 4
      address: 325212
    - index: 7
      opcode: NOP
      size: 4
      address: 325216
    address: 325188
  - name: 18
    mapsto: if.then13
    predecessors:
    - 15
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325236
    - index: 1
      opcode: NOP
      size: 4
      address: 325240
    - index: 2
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 325244
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325248
    - index: 4
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 325252
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325256
    - index: 6
      opcode: NOP
      size: 4
      address: 325260
    - index: 7
      opcode: NOP
      size: 4
      address: 325264
    address: 325236
  - name: 19
    mapsto: if.then15
    predecessors:
    - 16
    successors:
    - 20
    instructions:
    - index: 0
      opcode: ADDi
      size: 4
      address: 325284
    - index: 1
      opcode: CALLND
      callees:
      - normalize323
      size: 4
      branch-type: call
      address: 325288
    - index: 2
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 20
      address: 325292
    - index: 3
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325296
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325300
    - index: 5
      opcode: NOP
      size: 4
      address: 325304
    address: 325284
  - name: 20
    mapsto: if.end16
    predecessors:
    - 16
    - 19
    successors:
    - 21
    - 22
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325316
    - index: 1
      opcode: NOP
      size: 4
      address: 325320
    - index: 2
      opcode: LIl
      size: 8
      address: 325324
    - index: 3
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325332
    - index: 4
      opcode: NOP
      size: 4
      address: 325336
    - index: 5
      opcode: CMPULT
      size: 4
      address: 325340
    - index: 6
      opcode: MOV
      size: 4
      address: 325344
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325348
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325352
    - index: 9
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 22
      address: 325356
    - index: 10
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 21
      address: 325360
    address: 325316
  - name: 21
    mapsto: if.then17
    predecessors:
    - 20
    successors:
    - 22
    instructions:
    - index: 0
      opcode: ADDi
      size: 4
      address: 325380
    - index: 1
      opcode: CALLND
      callees:
      - normalize323
      size: 4
      branch-type: call
      address: 325384
    - index: 2
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325388
    - index: 3
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325392
    - index: 4
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 22
      address: 325396
    - index: 5
      opcode: NOP
      size: 4
      address: 325400
    - index: 6
      opcode: ADDr
      size: 4
      address: 325404
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325408
    address: 325380
  - name: 22
    mapsto: if.end18
    predecessors:
    - 20
    - 21
    successors:
    - 23
    instructions:
    - index: 0
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 23
      address: 325428
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325432
    - index: 2
      opcode: NOP
      size: 4
      address: 325436
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325440
    address: 325428
  - name: 23
    mapsto: if.end19
    predecessors:
    - 1
    - 22
    successors:
    - 25
    - 24
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325460
    - index: 1
      opcode: NOP
      size: 4
      address: 325464
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325468
    - index: 3
      opcode: NOP
      size: 4
      address: 325472
    - index: 4
      opcode: ORl
      size: 8
      address: 325476
    - index: 5
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325484
    - index: 6
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325488
    - index: 7
      opcode: NOP
      size: 4
      address: 325492
    - index: 8
      opcode: ORl
      size: 8
      address: 325496
    - index: 9
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325504
    - index: 10
      opcode: SLi
      size: 4
      address: 325508
    - index: 11
      opcode: ADDi
      size: 4
      address: 325512
    - index: 12
      opcode: ADDi
      size: 4
      address: 325516
    - index: 13
      opcode: MOV
      size: 4
      address: 325520
    - index: 14
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325524
    - index: 15
      opcode: CALLND
      callees:
      - wideMultiply324
      size: 4
      branch-type: call
      address: 325528
    - index: 16
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325532
    - index: 17
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325536
    - index: 18
      opcode: NOP
      size: 4
      address: 325540
    - index: 19
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325544
    - index: 20
      opcode: NOP
      size: 4
      address: 325548
    - index: 21
      opcode: ADDr
      size: 4
      address: 325552
    - index: 22
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325556
    - index: 23
      opcode: NOP
      size: 4
      address: 325560
    - index: 24
      opcode: ADDr
      size: 4
      address: 325564
    - index: 25
      opcode: SUBi
      size: 4
      address: 325568
    - index: 26
      opcode: LBUC
      size: 4
      memmode: load
      memtype: cache
      address: 325572
    - index: 27
      opcode: NOP
      size: 4
      address: 325576
    - index: 28
      opcode: ANDi
      size: 4
      address: 325580
    - index: 29
      opcode: CMPIEQ
      size: 4
      address: 325584
    - index: 30
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325588
    - index: 31
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 25
      address: 325592
    - index: 32
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 24
      address: 325596
    address: 325460
  - name: 24
    mapsto: if.then20
    predecessors:
    - 23
    successors:
    - 26
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325604
    - index: 1
      opcode: NOP
      size: 4
      address: 325608
    - index: 2
      opcode: ADDi
      size: 4
      address: 325612
    - index: 3
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 26
      address: 325616
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325620
    - index: 5
      opcode: NOP
      size: 4
      address: 325624
    - index: 6
      opcode: NOP
      size: 4
      address: 325628
    address: 325604
  - name: 25
    mapsto: if.else21
    predecessors:
    - 23
    successors:
    - 26
    instructions:
    - index: 0
      opcode: ADDi
      size: 4
      address: 325636
    - index: 1
      opcode: ADDi
      size: 4
      address: 325640
    - index: 2
      opcode: LIi
      size: 4
      address: 325644
    - index: 3
      opcode: CALLND
      callees:
      - wideLeftShift325
      size: 4
      branch-type: call
      address: 325648
    - index: 4
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325652
    - index: 5
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 26
      address: 325656
    - index: 6
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325660
    - index: 7
      opcode: NOP
      size: 4
      address: 325664
    - index: 8
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325668
    address: 325636
  - name: 26
    mapsto: if.end22
    predecessors:
    - 24
    - 25
    successors:
    - 29
    - 27
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325684
    - index: 1
      opcode: NOP
      size: 4
      address: 325688
    - index: 2
      opcode: LIi
      size: 4
      address: 325692
    - index: 3
      opcode: CMPLT
      size: 4
      address: 325696
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325700
    - index: 5
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 27
      address: 325704
    - index: 6
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 29
      address: 325708
    address: 325684
  - name: 27
    mapsto: if.end24
    predecessors:
    - 26
    successors:
    - 30
    - 28
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325712
    - index: 1
      opcode: NOP
      size: 4
      address: 325716
    - index: 2
      opcode: CMPLT
      size: 4
      address: 325720
    - index: 3
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 28
      address: 325724
    - index: 4
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 30
      address: 325728
    address: 325712
  - name: 28
    mapsto: if.else26
    predecessors:
    - 27
    successors:
    - 31
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325732
    - index: 1
      opcode: NOP
      size: 4
      address: 325736
    - index: 2
      opcode: ANDl
      size: 8
      address: 325740
    - index: 3
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325748
    - index: 4
      opcode: NOP
      size: 4
      address: 325752
    - index: 5
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 31
      address: 325756
    - index: 6
      opcode: SLi
      size: 4
      address: 325760
    - index: 7
      opcode: ORr
      size: 4
      address: 325764
    - index: 8
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325768
    address: 325732
  - name: 29
    mapsto: if.then23
    predecessors:
    - 26
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325780
    - index: 1
      opcode: NOP
      size: 4
      address: 325784
    - index: 2
      opcode: ORl
      size: 8
      address: 325788
    - index: 3
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 325796
    - index: 4
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325800
    - index: 5
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 325804
    - index: 6
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325808
    - index: 7
      opcode: NOP
      size: 4
      address: 325812
    - index: 8
      opcode: NOP
      size: 4
      address: 325816
    address: 325780
  - name: 30
    mapsto: if.then25
    predecessors:
    - 27
    successors:
    - 31
    instructions:
    - index: 0
      opcode: LIi
      size: 4
      address: 325828
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325832
    - index: 2
      opcode: NOP
      size: 4
      address: 325836
    - index: 3
      opcode: SUBr
      size: 4
      address: 325840
    - index: 4
      opcode: ADDi
      size: 4
      address: 325844
    - index: 5
      opcode: ADDi
      size: 4
      address: 325848
    - index: 6
      opcode: CALLND
      callees:
      - wideRightShiftWithSticky326
      size: 4
      branch-type: call
      address: 325852
    - index: 7
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 31
      address: 325856
    - index: 8
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 325860
    - index: 9
      opcode: NOP
      size: 4
      address: 325864
    - index: 10
      opcode: NOP
      size: 4
      address: 325868
    address: 325828
  - name: 31
    mapsto: if.end27
    predecessors:
    - 28
    - 30
    successors:
    - 32
    - 33
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325876
    - index: 1
      opcode: NOP
      size: 4
      address: 325880
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325884
    - index: 3
      opcode: NOP
      size: 4
      address: 325888
    - index: 4
      opcode: ORr
      size: 4
      address: 325892
    - index: 5
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325896
    - index: 6
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325900
    - index: 7
      opcode: NOP
      size: 4
      address: 325904
    - index: 8
      opcode: LIl
      size: 8
      address: 325908
    - index: 9
      opcode: CMPULT
      size: 4
      address: 325916
    - index: 10
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 325920
    - index: 11
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 33
      address: 325924
    - index: 12
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 32
      address: 325928
    address: 325876
  - name: 32
    mapsto: if.then28
    predecessors:
    - 31
    successors:
    - 33
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 325932
    - index: 1
      opcode: NOP
      size: 4
      address: 325936
    - index: 2
      opcode: ADDi
      size: 4
      address: 325940
    - index: 3
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325944
    address: 325932
  - name: 33
    mapsto: if.end29
    predecessors:
    - 31
    - 32
    successors:
    - 34
    - 35
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325948
    - index: 1
      opcode: NOP
      size: 4
      address: 325952
    - index: 2
      opcode: LIl
      size: 8
      address: 325956
    - index: 3
      opcode: CMPNEQ
      size: 4
      address: 325964
    - index: 4
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 35
      address: 325968
    - index: 5
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 34
      address: 325972
    address: 325948
  - name: 34
    mapsto: if.then30
    predecessors:
    - 33
    successors:
    - 35
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 325976
    - index: 1
      opcode: NOP
      size: 4
      address: 325980
    - index: 2
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 35
      address: 325984
    - index: 3
      opcode: ANDi
      size: 4
      address: 325988
    - index: 4
      opcode: ADDr
      size: 4
      address: 325992
    - index: 5
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 325996
    address: 325976
  - name: 35
    mapsto: if.end31
    predecessors:
    - 33
    - 34
    successors:
    - 36
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326004
    - index: 1
      opcode: NOP
      size: 4
      address: 326008
    - index: 2
      opcode: CALLND
      callees:
      - fromRep322
      size: 4
      branch-type: call
      address: 326012
    - index: 3
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 36
      address: 326016
    - index: 4
      opcode: SENSi
      size: 4
      stack-cache-argument: 24
      address: 326020
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326024
    - index: 6
      opcode: NOP
      size: 4
      address: 326028
    address: 326004
  - name: 36
    mapsto: return
    predecessors:
    - 35
    - 29
    - 18
    - 17
    - 12
    - 13
    - 10
    - 11
    - 9
    - 3
    successors: []
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326036
    - index: 1
      opcode: NOP
      size: 4
      address: 326040
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326044
    - index: 3
      opcode: NOP
      size: 4
      address: 326048
    - index: 4
      opcode: MTS
      size: 4
      address: 326052
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326056
    - index: 6
      opcode: NOP
      size: 4
      address: 326060
    - index: 7
      opcode: MTS
      size: 4
      address: 326064
    - index: 8
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326068
    - index: 9
      opcode: NOP
      size: 4
      address: 326072
    - index: 10
      opcode: MTS
      size: 4
      address: 326076
    - index: 11
      opcode: SFREEi
      size: 4
      stack-cache-argument: 24
      address: 326080
    - index: 12
      opcode: ADDi
      size: 4
      address: 326084
    - index: 13
      opcode: RETND
      size: 4
      branch-type: return
      address: 326088
    address: 326036
  subfunctions:
  - name: 0
    blocks:
    - 0
  - name: 1
    blocks:
    - 1
  - name: 2
    blocks:
    - 2
  - name: 3
    blocks:
    - 3
  - name: 4
    blocks:
    - 4
    - 5
    - 6
    - 7
    - 8
  - name: 9
    blocks:
    - 9
  - name: 10
    blocks:
    - 10
  - name: 11
    blocks:
    - 11
  - name: 12
    blocks:
    - 12
  - name: 13
    blocks:
    - 13
  - name: 14
    blocks:
    - 14
    - 15
    - 16
  - name: 17
    blocks:
    - 17
  - name: 18
    blocks:
    - 18
  - name: 19
    blocks:
    - 19
  - name: 20
    blocks:
    - 20
  - name: 21
    blocks:
    - 21
  - name: 22
    blocks:
    - 22
  - name: 23
    blocks:
    - 23
  - name: 24
    blocks:
    - 24
  - name: 25
    blocks:
    - 25
  - name: 26
    blocks:
    - 26
    - 27
    - 28
  - name: 29
    blocks:
    - 29
  - name: 30
    blocks:
    - 30
  - name: 31
    blocks:
    - 31
    - 32
    - 33
    - 34
  - name: 35
    blocks:
    - 35
  - name: 36
    blocks:
    - 36
- name: 241
  level: machinecode
  mapsto: toRep321
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: MOV
      size: 4
      address: 326100
    - index: 1
      opcode: RETND
      size: 4
      branch-type: return
      address: 326104
    address: 326100
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 242
  level: machinecode
  mapsto: fromRep322
  arguments:
  - name: ! '%x'
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
      opcode: MOV
      size: 4
      address: 326116
    - index: 1
      opcode: RETND
      size: 4
      branch-type: return
      address: 326120
    address: 326116
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 243
  level: machinecode
  mapsto: normalize323
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
      stack-cache-argument: 8
      address: 326132
    - index: 1
      opcode: MFS
      size: 4
      address: 326136
    - index: 2
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326140
    - index: 3
      opcode: MFS
      size: 4
      address: 326144
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326148
    - index: 5
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326152
    - index: 6
      opcode: NOP
      size: 4
      address: 326156
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326160
    - index: 8
      opcode: MOV
      size: 4
      address: 326164
    - index: 9
      opcode: CALLND
      callees:
      - rep_clz327
      size: 4
      branch-type: call
      address: 326168
    - index: 10
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 326172
    - index: 11
      opcode: LIl
      size: 8
      address: 326176
    - index: 12
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326184
    - index: 13
      opcode: CALLND
      callees:
      - rep_clz327
      size: 4
      branch-type: call
      address: 326188
    - index: 14
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 326192
    - index: 15
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326196
    - index: 16
      opcode: NOP
      size: 4
      address: 326200
    - index: 17
      opcode: SUBr
      size: 4
      address: 326204
    - index: 18
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326208
    - index: 19
      opcode: NOP
      size: 4
      address: 326212
    - index: 20
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326216
    - index: 21
      opcode: NOP
      size: 4
      address: 326220
    - index: 22
      opcode: SLr
      size: 4
      address: 326224
    - index: 23
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326228
    - index: 24
      opcode: LIi
      size: 4
      address: 326232
    - index: 25
      opcode: SUBr
      size: 4
      address: 326236
    - index: 26
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326240
    - index: 27
      opcode: NOP
      size: 4
      address: 326244
    - index: 28
      opcode: MTS
      size: 4
      address: 326248
    - index: 29
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326252
    - index: 30
      opcode: NOP
      size: 4
      address: 326256
    - index: 31
      opcode: MTS
      size: 4
      address: 326260
    - index: 32
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 326264
    - index: 33
      opcode: RETND
      size: 4
      branch-type: return
      address: 326268
    address: 326132
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 244
  level: machinecode
  mapsto: wideMultiply324
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
    successors: []
    instructions:
    - index: 0
      opcode: MULU
      size: 4
      address: 326276
    - index: 1
      opcode: NOP
      size: 4
      address: 326280
    - index: 2
      opcode: MFS
      size: 4
      address: 326284
    - index: 3
      opcode: MFS
      size: 4
      address: 326288
    - index: 4
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326292
    - index: 5
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326296
    - index: 6
      opcode: RETND
      size: 4
      branch-type: return
      address: 326300
    address: 326276
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 245
  level: machinecode
  mapsto: wideLeftShift325
  arguments:
  - name: ! '%count'
    index: 2
    registers:
    - r5
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326308
    - index: 1
      opcode: NOP
      size: 4
      address: 326312
    - index: 2
      opcode: SLr
      size: 4
      address: 326316
    - index: 3
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326320
    - index: 4
      opcode: NOP
      size: 4
      address: 326324
    - index: 5
      opcode: LIi
      size: 4
      address: 326328
    - index: 6
      opcode: SUBr
      size: 4
      address: 326332
    - index: 7
      opcode: SRr
      size: 4
      address: 326336
    - index: 8
      opcode: ORr
      size: 4
      address: 326340
    - index: 9
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326344
    - index: 10
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326348
    - index: 11
      opcode: NOP
      size: 4
      address: 326352
    - index: 12
      opcode: SLr
      size: 4
      address: 326356
    - index: 13
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326360
    - index: 14
      opcode: RETND
      size: 4
      branch-type: return
      address: 326364
    address: 326308
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 246
  level: machinecode
  mapsto: wideRightShiftWithSticky326
  arguments:
  - name: ! '%count'
    index: 2
    registers:
    - r5
  hash: 0
  blocks:
  - name: 0
    mapsto: entry
    predecessors: []
    successors:
    - 1
    - 2
    instructions:
    - index: 0
      opcode: SRESi
      size: 4
      stack-cache-argument: 8
      address: 326372
    - index: 1
      opcode: MFS
      size: 4
      address: 326376
    - index: 2
      opcode: MOV
      size: 4
      address: 326380
    - index: 3
      opcode: LIi
      size: 4
      address: 326384
    - index: 4
      opcode: CMPULT
      size: 4
      address: 326388
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326392
    - index: 6
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326396
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326400
    - index: 8
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 2
      address: 326404
    - index: 9
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 1
      address: 326408
    address: 326372
  - name: 1
    mapsto: if.then
    predecessors:
    - 0
    successors:
    - 6
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326412
    - index: 1
      opcode: NOP
      size: 4
      address: 326416
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326420
    - index: 3
      opcode: NOP
      size: 4
      address: 326424
    - index: 4
      opcode: LIi
      size: 4
      address: 326428
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326432
    - index: 6
      opcode: NOP
      size: 4
      address: 326436
    - index: 7
      opcode: SUBr
      size: 4
      address: 326440
    - index: 8
      opcode: SLr
      size: 4
      address: 326444
    - index: 9
      opcode: MOVrp
      size: 4
      address: 326448
    - index: 10
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326452
    - index: 11
      opcode: NOP
      size: 4
      address: 326456
    - index: 12
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326460
    - index: 13
      opcode: NOP
      size: 4
      address: 326464
    - index: 14
      opcode: SLr
      size: 4
      address: 326468
    - index: 15
      opcode: SRr
      size: 4
      address: 326472
    - index: 16
      opcode: ORr
      size: 4
      address: 326476
    - index: 17
      opcode: MOVpr
      size: 4
      address: 326480
    - index: 18
      opcode: ORr
      size: 4
      address: 326484
    - index: 19
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326488
    - index: 20
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326492
    - index: 21
      opcode: NOP
      size: 4
      address: 326496
    - index: 22
      opcode: SRr
      size: 4
      address: 326500
    - index: 23
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 6
      address: 326504
    - index: 24
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326508
    - index: 25
      opcode: NOP
      size: 4
      address: 326512
    - index: 26
      opcode: NOP
      size: 4
      address: 326516
    address: 326412
  - name: 2
    mapsto: if.else
    predecessors:
    - 0
    successors:
    - 3
    - 4
    instructions:
    - index: 0
      opcode: LIi
      size: 4
      address: 326520
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326524
    - index: 2
      opcode: NOP
      size: 4
      address: 326528
    - index: 3
      opcode: CMPULT
      size: 4
      address: 326532
    - index: 4
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 4
      address: 326536
    - index: 5
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 3
      address: 326540
    address: 326520
  - name: 3
    mapsto: if.then1
    predecessors:
    - 2
    successors:
    - 5
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326548
    - index: 1
      opcode: NOP
      size: 4
      address: 326552
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326556
    - index: 3
      opcode: NOP
      size: 4
      address: 326560
    - index: 4
      opcode: LIi
      size: 4
      address: 326564
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326568
    - index: 6
      opcode: NOP
      size: 4
      address: 326572
    - index: 7
      opcode: SUBr
      size: 4
      address: 326576
    - index: 8
      opcode: SLr
      size: 4
      address: 326580
    - index: 9
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326584
    - index: 10
      opcode: NOP
      size: 4
      address: 326588
    - index: 11
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326592
    - index: 12
      opcode: NOP
      size: 4
      address: 326596
    - index: 13
      opcode: ORr
      size: 4
      address: 326600
    - index: 14
      opcode: MOVrp
      size: 4
      address: 326604
    - index: 15
      opcode: SUBi
      size: 4
      address: 326608
    - index: 16
      opcode: SRr
      size: 4
      address: 326612
    - index: 17
      opcode: MOVpr
      size: 4
      address: 326616
    - index: 18
      opcode: ORr
      size: 4
      address: 326620
    - index: 19
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 5
      address: 326624
    - index: 20
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326628
    - index: 21
      opcode: NOP
      size: 4
      address: 326632
    - index: 22
      opcode: NOP
      size: 4
      address: 326636
    address: 326548
  - name: 4
    mapsto: if.else3
    predecessors:
    - 2
    successors:
    - 5
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326644
    - index: 1
      opcode: NOP
      size: 4
      address: 326648
    - index: 2
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326652
    - index: 3
      opcode: NOP
      size: 4
      address: 326656
    - index: 4
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326660
    - index: 5
      opcode: NOP
      size: 4
      address: 326664
    - index: 6
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 326668
    - index: 7
      opcode: NOP
      size: 4
      address: 326672
    - index: 8
      opcode: ORr
      size: 4
      address: 326676
    - index: 9
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 5
      address: 326680
    - index: 10
      opcode: MOVrp
      size: 4
      address: 326684
    - index: 11
      opcode: MOVpr
      size: 4
      address: 326688
    - index: 12
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326692
    address: 326644
  - name: 5
    mapsto: if.end
    predecessors:
    - 4
    - 3
    successors:
    - 6
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326708
    - index: 1
      opcode: NOP
      size: 4
      address: 326712
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326716
    - index: 3
      opcode: NOP
      size: 4
      address: 326720
    - index: 4
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 6
      address: 326724
    - index: 5
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326728
    - index: 6
      opcode: MOV
      size: 4
      address: 326732
    - index: 7
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 326736
    address: 326708
  - name: 6
    mapsto: if.end5
    predecessors:
    - 5
    - 1
    successors: []
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326756
    - index: 1
      opcode: NOP
      size: 4
      address: 326760
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 326764
    - index: 3
      opcode: NOP
      size: 4
      address: 326768
    - index: 4
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 326772
    - index: 5
      opcode: MTS
      size: 4
      address: 326776
    - index: 6
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 326780
    - index: 7
      opcode: RETND
      size: 4
      branch-type: return
      address: 326784
    address: 326756
  subfunctions:
  - name: 0
    blocks:
    - 0
    - 1
    - 2
  - name: 3
    blocks:
    - 3
  - name: 4
    blocks:
    - 4
  - name: 5
    blocks:
    - 5
  - name: 6
    blocks:
    - 6
- name: 247
  level: machinecode
  mapsto: rep_clz327
  arguments:
  - name: ! '%a'
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
      opcode: SRi
      size: 4
      address: 326804
    - index: 1
      opcode: ORr
      size: 4
      address: 326808
    - index: 2
      opcode: SRi
      size: 4
      address: 326812
    - index: 3
      opcode: ORr
      size: 4
      address: 326816
    - index: 4
      opcode: SRi
      size: 4
      address: 326820
    - index: 5
      opcode: ORr
      size: 4
      address: 326824
    - index: 6
      opcode: SRi
      size: 4
      address: 326828
    - index: 7
      opcode: ORr
      size: 4
      address: 326832
    - index: 8
      opcode: SRi
      size: 4
      address: 326836
    - index: 9
      opcode: NORr
      size: 4
      address: 326840
    - index: 10
      opcode: SRi
      size: 4
      address: 326844
    - index: 11
      opcode: ANDl
      size: 8
      address: 326848
    - index: 12
      opcode: SUBr
      size: 4
      address: 326856
    - index: 13
      opcode: ANDl
      size: 8
      address: 326860
    - index: 14
      opcode: SRi
      size: 4
      address: 326868
    - index: 15
      opcode: ANDl
      size: 8
      address: 326872
    - index: 16
      opcode: ADDr
      size: 4
      address: 326880
    - index: 17
      opcode: SRi
      size: 4
      address: 326884
    - index: 18
      opcode: ADDr
      size: 4
      address: 326888
    - index: 19
      opcode: ANDl
      size: 8
      address: 326892
    - index: 20
      opcode: LIl
      size: 8
      address: 326900
    - index: 21
      opcode: MUL
      size: 4
      address: 326908
    - index: 22
      opcode: NOP
      size: 4
      address: 326912
    - index: 23
      opcode: MFS
      size: 4
      address: 326916
    - index: 24
      opcode: SRi
      size: 4
      address: 326920
    - index: 25
      opcode: RETND
      size: 4
      branch-type: return
      address: 326924
    address: 326804
  subfunctions:
  - name: 0
    blocks:
    - 0
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
      opcode: alloca
    - index: 2
      opcode: alloca
    - index: 3
      opcode: store
      memmode: store
    - index: 4
      opcode: store
      memmode: store
    - index: 5
      opcode: load
      memmode: load
    - index: 6
      opcode: load
      memmode: load
    - index: 7
      opcode: call
      callees:
      - floatMult
    - index: 8
      opcode: store
      memmode: store
    - index: 9
      opcode: ret
- name: floatMult
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
      opcode: alloca
    - index: 2
      opcode: alloca
    - index: 3
      opcode: store
      memmode: store
    - index: 4
      opcode: store
      memmode: store
    - index: 5
      opcode: load
      memmode: load
    - index: 6
      opcode: load
      memmode: load
    - index: 7
      opcode: fmul
    - index: 8
      opcode: store
      memmode: store
    - index: 9
      opcode: load
      memmode: load
    - index: 10
      opcode: ret
- name: __mulsf3
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors:
    - if.then
    - lor.lhs.false
    instructions:
    - index: 0
      opcode: alloca
    - index: 1
      opcode: alloca
    - index: 2
      opcode: alloca
    - index: 3
      opcode: alloca
    - index: 4
      opcode: call
      callees:
      - toRep321
    - index: 5
      opcode: lshr
    - index: 6
      opcode: and
    - index: 7
      opcode: call
      callees:
      - toRep321
    - index: 8
      opcode: lshr
    - index: 9
      opcode: and
    - index: 10
      opcode: call
      callees:
      - toRep321
    - index: 11
      opcode: call
      callees:
      - toRep321
    - index: 12
      opcode: xor
    - index: 13
      opcode: and
    - index: 14
      opcode: call
      callees:
      - toRep321
    - index: 15
      opcode: and
    - index: 16
      opcode: store
      memmode: store
    - index: 17
      opcode: call
      callees:
      - toRep321
    - index: 18
      opcode: and
    - index: 19
      opcode: store
      memmode: store
    - index: 20
      opcode: add
    - index: 21
      opcode: icmp
    - index: 22
      opcode: br
  - name: lor.lhs.false
    predecessors:
    - entry
    successors:
    - if.then
    - if.end19
    instructions:
    - index: 0
      opcode: add
    - index: 1
      opcode: icmp
    - index: 2
      opcode: br
  - name: if.then
    predecessors:
    - lor.lhs.false
    - entry
    successors:
    - if.then1
    - if.end
    instructions:
    - index: 0
      opcode: call
      callees:
      - toRep321
    - index: 1
      opcode: and
    - index: 2
      opcode: call
      callees:
      - toRep321
    - index: 3
      opcode: and
    - index: 4
      opcode: icmp
    - index: 5
      opcode: br
  - name: if.then1
    predecessors:
    - if.then
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - toRep321
    - index: 1
      opcode: or
    - index: 2
      opcode: call
      callees:
      - fromRep322
    - index: 3
      opcode: br
  - name: if.end
    predecessors:
    - if.then
    successors:
    - if.then2
    - if.end3
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then2
    predecessors:
    - if.end
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - toRep321
    - index: 1
      opcode: or
    - index: 2
      opcode: call
      callees:
      - fromRep322
    - index: 3
      opcode: br
  - name: if.end3
    predecessors:
    - if.end
    successors:
    - if.then4
    - if.end6
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then4
    predecessors:
    - if.end3
    successors:
    - if.else
    - if.then5
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then5
    predecessors:
    - if.then4
    successors:
    - return
    instructions:
    - index: 0
      opcode: or
    - index: 1
      opcode: call
      callees:
      - fromRep322
    - index: 2
      opcode: br
  - name: if.else
    predecessors:
    - if.then4
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - fromRep322
    - index: 1
      opcode: br
  - name: if.end6
    predecessors:
    - if.end3
    successors:
    - if.then7
    - if.end10
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then7
    predecessors:
    - if.end6
    successors:
    - if.else9
    - if.then8
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then8
    predecessors:
    - if.then7
    successors:
    - return
    instructions:
    - index: 0
      opcode: or
    - index: 1
      opcode: call
      callees:
      - fromRep322
    - index: 2
      opcode: br
  - name: if.else9
    predecessors:
    - if.then7
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - fromRep322
    - index: 1
      opcode: br
  - name: if.end10
    predecessors:
    - if.end6
    successors:
    - if.then11
    - if.end12
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then11
    predecessors:
    - if.end10
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - fromRep322
    - index: 1
      opcode: br
  - name: if.end12
    predecessors:
    - if.end10
    successors:
    - if.then13
    - if.end14
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then13
    predecessors:
    - if.end12
    successors:
    - return
    instructions:
    - index: 0
      opcode: call
      callees:
      - fromRep322
    - index: 1
      opcode: br
  - name: if.end14
    predecessors:
    - if.end12
    successors:
    - if.then15
    - if.end16
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then15
    predecessors:
    - if.end14
    successors:
    - if.end16
    instructions:
    - index: 0
      opcode: call
      callees:
      - normalize323
    - index: 1
      opcode: br
  - name: if.end16
    predecessors:
    - if.then15
    - if.end14
    successors:
    - if.then17
    - if.end18
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: icmp
    - index: 2
      opcode: br
  - name: if.then17
    predecessors:
    - if.end16
    successors:
    - if.end18
    instructions:
    - index: 0
      opcode: call
      callees:
      - normalize323
    - index: 1
      opcode: add
    - index: 2
      opcode: br
  - name: if.end18
    predecessors:
    - if.then17
    - if.end16
    successors:
    - if.end19
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: br
  - name: if.end19
    predecessors:
    - if.end18
    - lor.lhs.false
    successors:
    - if.else21
    - if.then20
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: load
      memmode: load
    - index: 2
      opcode: or
    - index: 3
      opcode: store
      memmode: store
    - index: 4
      opcode: load
      memmode: load
    - index: 5
      opcode: or
    - index: 6
      opcode: store
      memmode: store
    - index: 7
      opcode: shl
    - index: 8
      opcode: call
      callees:
      - wideMultiply324
    - index: 9
      opcode: add
    - index: 10
      opcode: add
    - index: 11
      opcode: add
    - index: 12
      opcode: load
      memmode: load
    - index: 13
      opcode: and
    - index: 14
      opcode: icmp
    - index: 15
      opcode: br
  - name: if.then20
    predecessors:
    - if.end19
    successors:
    - if.end22
    instructions:
    - index: 0
      opcode: add
    - index: 1
      opcode: br
  - name: if.else21
    predecessors:
    - if.end19
    successors:
    - if.end22
    instructions:
    - index: 0
      opcode: call
      callees:
      - wideLeftShift325
    - index: 1
      opcode: br
  - name: if.end22
    predecessors:
    - if.else21
    - if.then20
    successors:
    - if.then23
    - if.end24
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: icmp
    - index: 2
      opcode: br
  - name: if.then23
    predecessors:
    - if.end22
    successors:
    - return
    instructions:
    - index: 0
      opcode: or
    - index: 1
      opcode: call
      callees:
      - fromRep322
    - index: 2
      opcode: br
  - name: if.end24
    predecessors:
    - if.end22
    successors:
    - if.then25
    - if.else26
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then25
    predecessors:
    - if.end24
    successors:
    - if.end27
    instructions:
    - index: 0
      opcode: sub
    - index: 1
      opcode: call
      callees:
      - wideRightShiftWithSticky326
    - index: 2
      opcode: br
  - name: if.else26
    predecessors:
    - if.end24
    successors:
    - if.end27
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: and
    - index: 2
      opcode: shl
    - index: 3
      opcode: or
    - index: 4
      opcode: store
      memmode: store
    - index: 5
      opcode: br
  - name: if.end27
    predecessors:
    - if.else26
    - if.then25
    successors:
    - if.then28
    - if.end29
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: or
    - index: 2
      opcode: store
      memmode: store
    - index: 3
      opcode: load
      memmode: load
    - index: 4
      opcode: icmp
    - index: 5
      opcode: br
  - name: if.then28
    predecessors:
    - if.end27
    successors:
    - if.end29
    instructions:
    - index: 0
      opcode: add
    - index: 1
      opcode: store
      memmode: store
    - index: 2
      opcode: br
  - name: if.end29
    predecessors:
    - if.then28
    - if.end27
    successors:
    - if.then30
    - if.end31
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: icmp
    - index: 2
      opcode: br
  - name: if.then30
    predecessors:
    - if.end29
    successors:
    - if.end31
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: and
    - index: 2
      opcode: add
    - index: 3
      opcode: store
      memmode: store
    - index: 4
      opcode: br
  - name: if.end31
    predecessors:
    - if.then30
    - if.end29
    successors:
    - return
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: call
      callees:
      - fromRep322
    - index: 2
      opcode: br
  - name: return
    predecessors:
    - if.end31
    - if.then23
    - if.then13
    - if.then11
    - if.else9
    - if.then8
    - if.else
    - if.then5
    - if.then2
    - if.then1
    successors: []
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: ret
- name: toRep321
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: bitcast
    - index: 1
      opcode: ret
- name: fromRep322
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: bitcast
    - index: 1
      opcode: ret
- name: normalize323
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: call
      callees:
      - rep_clz327
    - index: 2
      opcode: call
      callees:
      - rep_clz327
    - index: 3
      opcode: sub
    - index: 4
      opcode: load
      memmode: load
    - index: 5
      opcode: shl
    - index: 6
      opcode: store
      memmode: store
    - index: 7
      opcode: sub
    - index: 8
      opcode: ret
- name: wideMultiply324
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: zext
    - index: 1
      opcode: zext
    - index: 2
      opcode: mul
    - index: 3
      opcode: lshr
    - index: 4
      opcode: trunc
    - index: 5
      opcode: store
      memmode: store
    - index: 6
      opcode: trunc
    - index: 7
      opcode: store
      memmode: store
    - index: 8
      opcode: ret
- name: wideLeftShift325
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: shl
    - index: 2
      opcode: load
      memmode: load
    - index: 3
      opcode: sub
    - index: 4
      opcode: lshr
    - index: 5
      opcode: or
    - index: 6
      opcode: store
      memmode: store
    - index: 7
      opcode: load
      memmode: load
    - index: 8
      opcode: shl
    - index: 9
      opcode: store
      memmode: store
    - index: 10
      opcode: ret
- name: wideRightShiftWithSticky326
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors:
    - if.then
    - if.else
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then
    predecessors:
    - entry
    successors:
    - if.end5
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: sub
    - index: 2
      opcode: shl
    - index: 3
      opcode: icmp
    - index: 4
      opcode: load
      memmode: load
    - index: 5
      opcode: shl
    - index: 6
      opcode: lshr
    - index: 7
      opcode: or
    - index: 8
      opcode: zext
    - index: 9
      opcode: or
    - index: 10
      opcode: store
      memmode: store
    - index: 11
      opcode: load
      memmode: load
    - index: 12
      opcode: lshr
    - index: 13
      opcode: br
  - name: if.else
    predecessors:
    - entry
    successors:
    - if.then1
    - if.else3
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then1
    predecessors:
    - if.else
    successors:
    - if.end
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: sub
    - index: 2
      opcode: shl
    - index: 3
      opcode: load
      memmode: load
    - index: 4
      opcode: or
    - index: 5
      opcode: icmp
    - index: 6
      opcode: add
    - index: 7
      opcode: lshr
    - index: 8
      opcode: zext
    - index: 9
      opcode: or
    - index: 10
      opcode: br
  - name: if.else3
    predecessors:
    - if.else
    successors:
    - if.end
    instructions:
    - index: 0
      opcode: load
      memmode: load
    - index: 1
      opcode: load
      memmode: load
    - index: 2
      opcode: or
    - index: 3
      opcode: icmp
    - index: 4
      opcode: zext
    - index: 5
      opcode: br
  - name: if.end
    predecessors:
    - if.else3
    - if.then1
    successors:
    - if.end5
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: store
      memmode: store
    - index: 2
      opcode: br
  - name: if.end5
    predecessors:
    - if.end
    - if.then
    successors: []
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: store
      memmode: store
    - index: 2
      opcode: ret
- name: rep_clz327
  level: bitcode
  hash: 0
  blocks:
  - name: entry
    predecessors: []
    successors: []
    instructions:
    - index: 0
      opcode: call
    - index: 1
      opcode: ret
relation-graphs:
- src:
    function: main
    level: bitcode
  dst:
    function: 10
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
    function: floatMult
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
    function: __mulsf3
    level: bitcode
  dst:
    function: 240
    level: machinecode
  nodes:
  - name: 0
    type: entry
    src-block: entry
    dst-block: 0
    src-successors:
    - 2
    - 3
    dst-successors:
    - 2
    - 3
  - name: 1
    type: exit
  - name: 2
    type: progress
    src-block: if.then
    dst-block: 2
    src-successors:
    - 18
    - 19
    dst-successors:
    - 18
    - 19
  - name: 3
    type: progress
    src-block: lor.lhs.false
    dst-block: 1
    src-successors:
    - 4
    - 2
    dst-successors:
    - 4
    - 2
  - name: 4
    type: progress
    src-block: if.end19
    dst-block: 23
    src-successors:
    - 5
    - 6
    dst-successors:
    - 5
    - 6
  - name: 5
    type: progress
    src-block: if.else21
    dst-block: 25
    src-successors:
    - 7
    dst-successors:
    - 7
  - name: 6
    type: progress
    src-block: if.then20
    dst-block: 24
    src-successors:
    - 7
    dst-successors:
    - 7
  - name: 7
    type: progress
    src-block: if.end22
    dst-block: 26
    src-successors:
    - 8
    - 9
    dst-successors:
    - 8
    - 9
  - name: 8
    type: progress
    src-block: if.end24
    dst-block: 27
    src-successors:
    - 11
    - 12
    dst-successors:
    - 11
    - 12
  - name: 9
    type: progress
    src-block: if.then23
    dst-block: 29
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 10
    type: progress
    src-block: return
    dst-block: 36
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 11
    type: progress
    src-block: if.else26
    dst-block: 28
    src-successors:
    - 13
    dst-successors:
    - 13
  - name: 12
    type: progress
    src-block: if.then25
    dst-block: 30
    src-successors:
    - 13
    dst-successors:
    - 13
  - name: 13
    type: progress
    src-block: if.end27
    dst-block: 31
    src-successors:
    - 14
    - 15
    dst-successors:
    - 14
    - 15
  - name: 14
    type: progress
    src-block: if.end29
    dst-block: 33
    src-successors:
    - 16
    - 17
    dst-successors:
    - 16
    - 17
  - name: 15
    type: progress
    src-block: if.then28
    dst-block: 32
    src-successors:
    - 14
    dst-successors:
    - 14
  - name: 16
    type: progress
    src-block: if.end31
    dst-block: 35
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 17
    type: progress
    src-block: if.then30
    dst-block: 34
    src-successors:
    - 16
    dst-successors:
    - 16
  - name: 18
    type: progress
    src-block: if.end
    dst-block: 4
    src-successors:
    - 20
    - 21
    dst-successors:
    - 20
    - 21
  - name: 19
    type: progress
    src-block: if.then1
    dst-block: 3
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 20
    type: progress
    src-block: if.end3
    dst-block: 5
    src-successors:
    - 22
    - 23
    dst-successors:
    - 22
    - 23
  - name: 21
    type: progress
    src-block: if.then2
    dst-block: 9
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 22
    type: progress
    src-block: if.end6
    dst-block: 7
    src-successors:
    - 26
    - 27
    dst-successors:
    - 26
    - 27
  - name: 23
    type: progress
    src-block: if.then4
    dst-block: 6
    src-successors:
    - 24
    - 25
    dst-successors:
    - 24
    - 25
  - name: 24
    type: progress
    src-block: if.else
    dst-block: 11
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 25
    type: progress
    src-block: if.then5
    dst-block: 10
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 26
    type: progress
    src-block: if.end10
    dst-block: 14
    src-successors:
    - 30
    - 31
    dst-successors:
    - 30
    - 31
  - name: 27
    type: progress
    src-block: if.then7
    dst-block: 8
    src-successors:
    - 28
    - 29
    dst-successors:
    - 28
    - 29
  - name: 28
    type: progress
    src-block: if.else9
    dst-block: 13
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 29
    type: progress
    src-block: if.then8
    dst-block: 12
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 30
    type: progress
    src-block: if.end12
    dst-block: 15
    src-successors:
    - 32
    - 33
    dst-successors:
    - 32
    - 33
  - name: 31
    type: progress
    src-block: if.then11
    dst-block: 17
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 32
    type: progress
    src-block: if.end14
    dst-block: 16
    src-successors:
    - 34
    - 35
    dst-successors:
    - 34
    - 35
  - name: 33
    type: progress
    src-block: if.then13
    dst-block: 18
    src-successors:
    - 10
    dst-successors:
    - 10
  - name: 34
    type: progress
    src-block: if.end16
    dst-block: 20
    src-successors:
    - 36
    - 37
    dst-successors:
    - 36
    - 37
  - name: 35
    type: progress
    src-block: if.then15
    dst-block: 19
    src-successors:
    - 34
    dst-successors:
    - 34
  - name: 36
    type: progress
    src-block: if.end18
    dst-block: 22
    src-successors:
    - 4
    dst-successors:
    - 4
  - name: 37
    type: progress
    src-block: if.then17
    dst-block: 21
    src-successors:
    - 36
    dst-successors:
    - 36
  status: valid
- src:
    function: toRep321
    level: bitcode
  dst:
    function: 241
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
    function: fromRep322
    level: bitcode
  dst:
    function: 242
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
    function: normalize323
    level: bitcode
  dst:
    function: 243
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
    function: wideMultiply324
    level: bitcode
  dst:
    function: 244
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
    function: wideLeftShift325
    level: bitcode
  dst:
    function: 245
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
    function: wideRightShiftWithSticky326
    level: bitcode
  dst:
    function: 246
    level: machinecode
  nodes:
  - name: 0
    type: entry
    src-block: entry
    dst-block: 0
    src-successors:
    - 2
    - 3
    dst-successors:
    - 2
    - 3
  - name: 1
    type: exit
  - name: 2
    type: progress
    src-block: if.else
    dst-block: 2
    src-successors:
    - 5
    - 6
    dst-successors:
    - 5
    - 6
  - name: 3
    type: progress
    src-block: if.then
    dst-block: 1
    src-successors:
    - 4
    dst-successors:
    - 4
  - name: 4
    type: progress
    src-block: if.end5
    dst-block: 6
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 5
    type: progress
    src-block: if.else3
    dst-block: 4
    src-successors:
    - 7
    dst-successors:
    - 7
  - name: 6
    type: progress
    src-block: if.then1
    dst-block: 3
    src-successors:
    - 7
    dst-successors:
    - 7
  - name: 7
    type: progress
    src-block: if.end
    dst-block: 5
    src-successors:
    - 4
    dst-successors:
    - 4
  status: valid
- src:
    function: rep_clz327
    level: bitcode
  dst:
    function: 247
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
timing:
- scope:
    function: 10
  cycles: 4833
  level: machinecode
  origin: platin
  cache-max-cycles-instr: 3255
  cache-min-hits-instr: 28
  cache-max-misses-instr: 30
  cache-max-cycles-stack: 0
  cache-max-misses-stack: 0
  cache-max-cycles-data: 840
  cache-min-hits-data: 0
  cache-max-misses-data: 19
  cache-max-stores-data: 21
  cache-unknown-address-data: 40
  cache-max-cycles: 4095
  profile:
  - reference:
      function: 10
      edgesource: 0
    cycles: 284
    wcet-frequency: 1
    wcet-contribution: 284
  - reference:
      function: 8
      edgesource: 0
    cycles: 364
    wcet-frequency: 1
    wcet-contribution: 364
  - reference:
      function: 240
      edgesource: 0
      edgetarget: 1
    cycles: 458
    wcet-frequency: 1
    wcet-contribution: 458
  - reference:
      function: 240
      edgesource: 1
      edgetarget: 2
    cycles: 75
    wcet-frequency: 1
    wcet-contribution: 75
  - reference:
      function: 240
      edgesource: 2
      edgetarget: 4
    cycles: 152
    wcet-frequency: 1
    wcet-contribution: 152
  - reference:
      function: 240
      edgesource: 4
      edgetarget: 5
    cycles: 175
    wcet-frequency: 1
    wcet-contribution: 175
  - reference:
      function: 240
      edgesource: 5
      edgetarget: 7
    cycles: 7
    wcet-frequency: 1
    wcet-contribution: 7
  - reference:
      function: 240
      edgesource: 7
      edgetarget: 14
    cycles: 8
    wcet-frequency: 1
    wcet-contribution: 8
  - reference:
      function: 240
      edgesource: 14
      edgetarget: 15
    cycles: 111
    wcet-frequency: 1
    wcet-contribution: 111
  - reference:
      function: 240
      edgesource: 15
      edgetarget: 16
    cycles: 6
    wcet-frequency: 1
    wcet-contribution: 6
  - reference:
      function: 240
      edgesource: 16
      edgetarget: 19
    cycles: 11
    wcet-frequency: 1
    wcet-contribution: 11
  - reference:
      function: 240
      edgesource: 19
      edgetarget: 20
    cycles: 51
    wcet-frequency: 1
    wcet-contribution: 51
  - reference:
      function: 240
      edgesource: 20
      edgetarget: 21
    cycles: 98
    wcet-frequency: 1
    wcet-contribution: 98
  - reference:
      function: 240
      edgesource: 21
      edgetarget: 22
    cycles: 74
    wcet-frequency: 1
    wcet-contribution: 74
  - reference:
      function: 240
      edgesource: 22
      edgetarget: 23
    cycles: 46
    wcet-frequency: 1
    wcet-contribution: 46
  - reference:
      function: 240
      edgesource: 23
      edgetarget: 25
    cycles: 332
    wcet-frequency: 1
    wcet-contribution: 332
  - reference:
      function: 240
      edgesource: 25
      edgetarget: 26
    cycles: 138
    wcet-frequency: 1
    wcet-contribution: 138
  - reference:
      function: 240
      edgesource: 26
      edgetarget: 27
    cycles: 134
    wcet-frequency: 1
    wcet-contribution: 134
  - reference:
      function: 240
      edgesource: 27
      edgetarget: 30
    cycles: 8
    wcet-frequency: 1
    wcet-contribution: 8
  - reference:
      function: 240
      edgesource: 30
      edgetarget: 31
    cycles: 77
    wcet-frequency: 1
    wcet-contribution: 77
  - reference:
      function: 240
      edgesource: 31
      edgetarget: 32
    cycles: 246
    wcet-frequency: 1
    wcet-contribution: 246
  - reference:
      function: 240
      edgesource: 32
      edgetarget: 33
    cycles: 25
    wcet-frequency: 1
    wcet-contribution: 25
  - reference:
      function: 240
      edgesource: 33
      edgetarget: 34
    cycles: 29
    wcet-frequency: 1
    wcet-contribution: 29
  - reference:
      function: 240
      edgesource: 34
      edgetarget: 35
    cycles: 48
    wcet-frequency: 1
    wcet-contribution: 48
  - reference:
      function: 240
      edgesource: 35
      edgetarget: 36
    cycles: 73
    wcet-frequency: 1
    wcet-contribution: 73
  - reference:
      function: 240
      edgesource: 36
    cycles: 101
    wcet-frequency: 1
    wcet-contribution: 101
  - reference:
      function: 246
      edgesource: 0
      edgetarget: 2
    cycles: 242
    wcet-frequency: 1
    wcet-contribution: 242
  - reference:
      function: 246
      edgesource: 2
      edgetarget: 3
    cycles: 9
    wcet-frequency: 1
    wcet-contribution: 9
  - reference:
      function: 246
      edgesource: 3
      edgetarget: 5
    cycles: 191
    wcet-frequency: 1
    wcet-contribution: 191
  - reference:
      function: 246
      edgesource: 5
      edgetarget: 6
    cycles: 92
    wcet-frequency: 1
    wcet-contribution: 92
  - reference:
      function: 246
      edgesource: 6
    cycles: 95
    wcet-frequency: 1
    wcet-contribution: 95
  - reference:
      function: 245
      edgesource: 0
    cycles: 207
    wcet-frequency: 1
    wcet-contribution: 207
  - reference:
      function: 244
      edgesource: 0
    cycles: 94
    wcet-frequency: 1
    wcet-contribution: 94
  - reference:
      function: 243
      edgesource: 0
    cycles: 295
    wcet-frequency: 2
    wcet-contribution: 401
  - reference:
      function: 247
      edgesource: 0
    cycles: 197
    wcet-frequency: 4
    wcet-contribution: 284
  - reference:
      function: 242
      edgesource: 0
    cycles: 26
    wcet-frequency: 1
    wcet-contribution: 26
  - reference:
      function: 241
      edgesource: 0
    cycles: 26
    wcet-frequency: 8
    wcet-contribution: 61
