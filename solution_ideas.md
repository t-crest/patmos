# Comments
* SPM memory blocks must be able to do dual-reads (in same cycle), or a write & read into same cycle. dual writes in same cycle is not needed - the only time where this is needed is when a node requests to write to its own address range, as well as the NI has an external write request. This can **only** happen in the scheduele slot *L*, where a value is written into its memory. For these cases, the local read (from the node into its own memory) can be stalled for 1 clock cycle. Alternatively, the NI could be clocked at double the frequency of the core and NoC. With this, we can do two writes in 1 NoC cycle, and thus do both external writes into the SPM in the NI, as well as internal write from the node.


# Solution approach
* By inversing the schedule, we can respond to read request in the same order as they were received.
* To do this, we must offset the two schedules, to allow for memory reading. Probably one clock cycle.

* We add one more NoC for the return channel. NoC1 supports write and read requests while NoC2 returns the read data. Thereby, NoC1 and NoC2 has the same but inversed and time shifted schedule.

* This, by introducing read and a shared distributed memory, following things must be implemented.

# ToDo
* NI:
    * Handles write/read request. For this, we must extend the package by a flag bit, which indicates a read or write. On a read request, the data packet is empty.
    * Processor, memory and network interfacing.
    * Address to node translation.
    * Choosing time slot for sending message.
* Dummy node:
    * write a dummy node that can use the RwChannel (try read, wait for read return, try write, wait for write return)

# Tests:
* Simultanious local node write and external node write
* Simultanious local node read and external node read-request
* Speed/stress test
    * Some small kind of processing in the node (ie. read a value, change it, and write it back)
    * "beneficial test": the memories are laid out in a nice order, where each node mostly/only reads/writes to local memory
    * "bad test": The memories are laid out such that each node almost nenver/never reads/writes to its local memory
* Congestion
* Reverse-schedule validation
    * Our intuition tells us that reversing the schedule still yields a valid schedule that has no congestion. Formal proof of this?
    * 


![alt tex][logo]


[logo]: https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 2"
