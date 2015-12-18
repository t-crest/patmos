---
format: pml-0.1
triple: patmos-unknown-unknown-elf
machine-configuration:
  memories:
  - name: main
    size: 2097152
    transfer-size: 16
    read-latency: 0
    read-transfer-time: 21
    write-latency: 0
    write-transfer-time: 21
  caches:
  - name: method-cache
    type: method-cache
    policy: fifo
    associativity: 8
    block-size: 8
    size: 2048
  - name: stack-cache
    type: stack-cache
    policy: block
    associativity: 512
    block-size: 4
    size: 2048
  - name: data-cache
    type: set-associative
    policy: dm
    associativity: 1
    block-size: 16
    size: 2048
  memory-areas:
  - name: code
    type: code
    cache: method-cache
    memory: main
    address-range:
      min: 0
      max: 4294967295
  - name: data
    type: data
    cache: data-cache
    memory: main
    address-range:
      min: 0
      max: 4294967295
