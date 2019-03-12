---
format: pml-0.1
triple: patmos-unknown-unknown-elf
machine-configuration:
  memories:
  - name: "main"
    size: 0x200000
    transfer-size: 16
    read-latency: 0
    read-transfer-time: 21
    write-latency: 0
    write-transfer-time: 21
  - name: "local"
    size: 2048
    transfer-size: 4
    read-latency: 0
    read-transfer-time: 0
    write-latency: 0
    write-transfer-time: 0
  caches:
  - name: "data-cache"
    block-size: 16
    associativity: 1
    size: 2048
    policy: "lru"
    type: "set-associative"
  - name: "method-cache"
    block-size: 8
    associativity: 16
    size: 4096
    policy: "fifo"
    type: "method-cache"
  - name: "stack-cache"
    block-size: 4
    size: 2048
    type: "stack-cache"
  memory-areas:
  - name: "code"
    type: "code"
    memory: "main"
    cache: "method-cache"
    address-range:
      min: 0
      max: 0x200000
  - name: "data"
    type: "data"
    memory: "main"
    cache: "data-cache"
    address-range:
      min: 0
      max: 0x200000
    attributes:
    - key: "heap-end"
      value: 0x100000
    - key: "stack-base"
      value: 0x200000
    - key: "shadow-stack-base"
      value: 0x1f8000
