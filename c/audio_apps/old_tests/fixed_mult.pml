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
      address: 133588
    - index: 1
      opcode: SUBi
      size: 4
      address: 133592
    - index: 2
      opcode: MFS
      size: 4
      address: 133596
    - index: 3
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133600
    - index: 4
      opcode: MFS
      size: 4
      address: 133604
    - index: 5
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133608
    - index: 6
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133612
    - index: 7
      opcode: LIl
      size: 8
      address: 133616
    - index: 8
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133624
    - index: 9
      opcode: LIl
      size: 8
      address: 133628
    - index: 10
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133636
    - index: 11
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133640
    - index: 12
      opcode: NOP
      size: 4
      address: 133644
    - index: 13
      opcode: CALLND
      callees:
      - __fixsfsi
      size: 4
      branch-type: call
      address: 133648
    - index: 14
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 133652
    - index: 15
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133656
    - index: 16
      opcode: NOP
      size: 4
      address: 133660
    - index: 17
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 133664
    - index: 18
      opcode: CALLND
      callees:
      - __fixsfsi
      size: 4
      branch-type: call
      address: 133668
    - index: 19
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 133672
    - index: 20
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133676
    - index: 21
      opcode: NOP
      size: 4
      address: 133680
    - index: 22
      opcode: MOV
      size: 4
      address: 133684
    - index: 23
      opcode: CALLND
      callees:
      - fixedMult
      size: 4
      branch-type: call
      address: 133688
    - index: 24
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 133692
    - index: 25
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133696
    - index: 26
      opcode: MOV
      size: 4
      address: 133700
    - index: 27
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133704
    - index: 28
      opcode: NOP
      size: 4
      address: 133708
    - index: 29
      opcode: MTS
      size: 4
      address: 133712
    - index: 30
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 133716
    - index: 31
      opcode: NOP
      size: 4
      address: 133720
    - index: 32
      opcode: MTS
      size: 4
      address: 133724
    - index: 33
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 133728
    - index: 34
      opcode: ADDi
      size: 4
      address: 133732
    - index: 35
      opcode: RETND
      size: 4
      branch-type: return
      address: 133736
    address: 133588
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 214
  level: machinecode
  mapsto: __fixsfsi
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
      address: 313188
    - index: 1
      opcode: MFS
      size: 4
      address: 313192
    - index: 2
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313196
    - index: 3
      opcode: MFS
      size: 4
      address: 313200
    - index: 4
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313204
    - index: 5
      opcode: MFS
      size: 4
      address: 313208
    - index: 6
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313212
    - index: 7
      opcode: CALLND
      callees:
      - toRep282
      size: 4
      branch-type: call
      address: 313216
    - index: 8
      opcode: SENSi
      size: 4
      stack-cache-argument: 8
      address: 313220
    - index: 9
      opcode: SRAi
      size: 4
      address: 313224
    - index: 10
      opcode: ORi
      size: 4
      address: 313228
    - index: 11
      opcode: SRi
      size: 4
      address: 313232
    - index: 12
      opcode: ANDi
      size: 4
      address: 313236
    - index: 13
      opcode: SUBi
      size: 4
      address: 313240
    - index: 14
      opcode: ANDl
      size: 8
      address: 313244
    - index: 15
      opcode: ORl
      size: 8
      address: 313252
    - index: 16
      opcode: LIi
      size: 4
      address: 313260
    - index: 17
      opcode: CMPULT
      size: 4
      address: 313264
    - index: 18
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313268
    - index: 19
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313272
    - index: 20
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313276
    - index: 21
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313280
    - index: 22
      opcode: BRCFND
      size: 4
      branch-type: conditional
      branch-targets:
      - 2
      address: 313284
    - index: 23
      opcode: BRCFNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 1
      address: 313288
    address: 313188
  - name: 1
    mapsto: if.then
    predecessors:
    - 0
    successors:
    - 5
    instructions:
    - index: 0
      opcode: LIi
      size: 4
      address: 313300
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313304
    - index: 2
      opcode: NOP
      size: 4
      address: 313308
    - index: 3
      opcode: SUBr
      size: 4
      address: 313312
    - index: 4
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313316
    - index: 5
      opcode: NOP
      size: 4
      address: 313320
    - index: 6
      opcode: SRr
      size: 4
      address: 313324
    - index: 7
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313328
    - index: 8
      opcode: NOP
      size: 4
      address: 313332
    - index: 9
      opcode: MUL
      size: 4
      address: 313336
    - index: 10
      opcode: NOP
      size: 4
      address: 313340
    - index: 11
      opcode: MFS
      size: 4
      address: 313344
    - index: 12
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 5
      address: 313348
    - index: 13
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313352
    - index: 14
      opcode: NOP
      size: 4
      address: 313356
    - index: 15
      opcode: NOP
      size: 4
      address: 313360
    address: 313300
  - name: 2
    mapsto: if.else
    predecessors:
    - 0
    successors:
    - 3
    - 4
    instructions:
    - index: 0
      opcode: LIin
      size: 4
      address: 313380
    - index: 1
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313384
    - index: 2
      opcode: NOP
      size: 4
      address: 313388
    - index: 3
      opcode: CMPLT
      size: 4
      address: 313392
    - index: 4
      opcode: BRND
      size: 4
      branch-type: conditional
      branch-targets:
      - 4
      address: 313396
    - index: 5
      opcode: BRNDu
      size: 4
      branch-type: unconditional
      branch-targets:
      - 3
      address: 313400
    address: 313380
  - name: 3
    mapsto: if.then1
    predecessors:
    - 2
    successors:
    - 5
    instructions:
    - index: 0
      opcode: MOV
      size: 4
      address: 313404
    - index: 1
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 5
      address: 313408
    - index: 2
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313412
    - index: 3
      opcode: NOP
      size: 4
      address: 313416
    - index: 4
      opcode: NOP
      size: 4
      address: 313420
    address: 313404
  - name: 4
    mapsto: if.else2
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
      address: 313424
    - index: 1
      opcode: NOP
      size: 4
      address: 313428
    - index: 2
      opcode: SUBi
      size: 4
      address: 313432
    - index: 3
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313436
    - index: 4
      opcode: NOP
      size: 4
      address: 313440
    - index: 5
      opcode: SLr
      size: 4
      address: 313444
    - index: 6
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313448
    - index: 7
      opcode: NOP
      size: 4
      address: 313452
    - index: 8
      opcode: MUL
      size: 4
      address: 313456
    - index: 9
      opcode: BRCFu
      size: 4
      branch-type: unconditional
      branch-delay-slots: 3
      branch-targets:
      - 5
      address: 313460
    - index: 10
      opcode: NOP
      size: 4
      address: 313464
    - index: 11
      opcode: MFS
      size: 4
      address: 313468
    - index: 12
      opcode: SWS
      size: 4
      memmode: store
      memtype: stack
      address: 313472
    address: 313424
  - name: 5
    mapsto: return
    predecessors:
    - 4
    - 3
    - 1
    successors: []
    instructions:
    - index: 0
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313492
    - index: 1
      opcode: NOP
      size: 4
      address: 313496
    - index: 2
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313500
    - index: 3
      opcode: NOP
      size: 4
      address: 313504
    - index: 4
      opcode: MTS
      size: 4
      address: 313508
    - index: 5
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313512
    - index: 6
      opcode: NOP
      size: 4
      address: 313516
    - index: 7
      opcode: MTS
      size: 4
      address: 313520
    - index: 8
      opcode: LWS
      size: 4
      memmode: load
      memtype: stack
      address: 313524
    - index: 9
      opcode: NOP
      size: 4
      address: 313528
    - index: 10
      opcode: MTS
      size: 4
      address: 313532
    - index: 11
      opcode: SFREEi
      size: 4
      stack-cache-argument: 8
      address: 313536
    - index: 12
      opcode: RETND
      size: 4
      branch-type: return
      address: 313540
    address: 313492
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
    - 3
    - 4
  - name: 5
    blocks:
    - 5
- name: 9
  level: machinecode
  mapsto: fixedMult
  arguments:
  - name: ! '%x'
    index: 0
    registers:
    - r3
  - name: ! '%y'
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
      opcode: SUBi
      size: 4
      address: 133524
    - index: 1
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133528
    - index: 2
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133532
    - index: 3
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133536
    - index: 4
      opcode: NOP
      size: 4
      address: 133540
    - index: 5
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133544
    - index: 6
      opcode: NOP
      size: 4
      address: 133548
    - index: 7
      opcode: MUL
      size: 4
      address: 133552
    - index: 8
      opcode: NOP
      size: 4
      address: 133556
    - index: 9
      opcode: MFS
      size: 4
      address: 133560
    - index: 10
      opcode: SWC
      size: 4
      memmode: store
      memtype: cache
      address: 133564
    - index: 11
      opcode: LWC
      size: 4
      memmode: load
      memtype: cache
      address: 133568
    - index: 12
      opcode: NOP
      size: 4
      address: 133572
    - index: 13
      opcode: ADDi
      size: 4
      address: 133576
    - index: 14
      opcode: RETND
      size: 4
      branch-type: return
      address: 133580
    address: 133524
  subfunctions:
  - name: 0
    blocks:
    - 0
- name: 215
  level: machinecode
  mapsto: toRep282
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
      address: 313556
    - index: 1
      opcode: RETND
      size: 4
      branch-type: return
      address: 313560
    address: 313556
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
      opcode: alloca
    - index: 4
      opcode: store
      memmode: store
    - index: 5
      opcode: store
      memmode: store
    - index: 6
      opcode: store
      memmode: store
    - index: 7
      opcode: load
      memmode: load
    - index: 8
      opcode: fptosi
    - index: 9
      opcode: load
      memmode: load
    - index: 10
      opcode: fptosi
    - index: 11
      opcode: call
      callees:
      - fixedMult
    - index: 12
      opcode: store
      memmode: store
    - index: 13
      opcode: ret
- name: __fixsfsi
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
      opcode: call
      callees:
      - toRep282
    - index: 1
      opcode: ashr
    - index: 2
      opcode: or
    - index: 3
      opcode: lshr
    - index: 4
      opcode: and
    - index: 5
      opcode: add
    - index: 6
      opcode: and
    - index: 7
      opcode: or
    - index: 8
      opcode: icmp
    - index: 9
      opcode: br
  - name: if.then
    predecessors:
    - entry
    successors:
    - return
    instructions:
    - index: 0
      opcode: sub
    - index: 1
      opcode: lshr
    - index: 2
      opcode: mul
    - index: 3
      opcode: br
  - name: if.else
    predecessors:
    - entry
    successors:
    - if.then1
    - if.else2
    instructions:
    - index: 0
      opcode: icmp
    - index: 1
      opcode: br
  - name: if.then1
    predecessors:
    - if.else
    successors:
    - return
    instructions:
    - index: 0
      opcode: br
  - name: if.else2
    predecessors:
    - if.else
    successors:
    - return
    instructions:
    - index: 0
      opcode: add
    - index: 1
      opcode: shl
    - index: 2
      opcode: mul
    - index: 3
      opcode: br
  - name: return
    predecessors:
    - if.else2
    - if.then1
    - if.then
    successors: []
    instructions:
    - index: 0
      opcode: phi
    - index: 1
      opcode: ret
- name: fixedMult
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
      opcode: mul
    - index: 8
      opcode: store
      memmode: store
    - index: 9
      opcode: load
      memmode: load
    - index: 10
      opcode: ret
- name: toRep282
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
    function: __fixsfsi
    level: bitcode
  dst:
    function: 214
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
    src-block: return
    dst-block: 5
    src-successors:
    - 1
    dst-successors:
    - 1
  - name: 5
    type: progress
    src-block: if.else2
    dst-block: 4
    src-successors:
    - 4
    dst-successors:
    - 4
  - name: 6
    type: progress
    src-block: if.then1
    dst-block: 3
    src-successors:
    - 4
    dst-successors:
    - 4
  status: valid
- src:
    function: fixedMult
    level: bitcode
  dst:
    function: 9
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
    function: toRep282
    level: bitcode
  dst:
    function: 215
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
  cycles: 1253
  level: machinecode
  origin: platin
  cache-max-cycles-instr: 798
  cache-min-hits-instr: 7
  cache-max-misses-instr: 7
  cache-max-cycles-stack: 0
  cache-max-misses-stack: 0
  cache-max-cycles-data: 252
  cache-min-hits-data: 0
  cache-max-misses-data: 5
  cache-max-stores-data: 7
  cache-unknown-address-data: 12
  cache-max-cycles: 1050
  profile:
  - reference:
      function: 10
      edgesource: 0
    cycles: 384
    wcet-frequency: 1
    wcet-contribution: 384
  - reference:
      function: 9
      edgesource: 0
    cycles: 228
    wcet-frequency: 1
    wcet-contribution: 228
  - reference:
      function: 214
      edgesource: 0
      edgetarget: 1
    cycles: 30
    wcet-frequency: 1
    wcet-contribution: 30
  - reference:
      function: 214
      edgesource: 0
      edgetarget: 2
    cycles: 176
    wcet-frequency: 1
    wcet-contribution: 176
  - reference:
      function: 214
      edgesource: 1
      edgetarget: 5
    cycles: 121
    wcet-frequency: 1
    wcet-contribution: 121
  - reference:
      function: 214
      edgesource: 2
      edgetarget: 4
    cycles: 154
    wcet-frequency: 1
    wcet-contribution: 154
  - reference:
      function: 214
      edgesource: 4
      edgetarget: 5
    cycles: 13
    wcet-frequency: 1
    wcet-contribution: 13
  - reference:
      function: 214
      edgesource: 5
    cycles: 100
    wcet-frequency: 2
    wcet-contribution: 116
  - reference:
      function: 215
      edgesource: 0
    cycles: 26
    wcet-frequency: 2
    wcet-contribution: 31
