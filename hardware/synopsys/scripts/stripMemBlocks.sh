#!/bin/bash
# remove the memory blocks from the patmos processor to replace them with SRAMs during synthesis
cat Patmos.v | sed /module\ MemBlock_\[0-3\]/,/endmodule/d > Patmos.strippedMemBlocks.v
