# A linker script to put .text.spm into the I-SPM and the rest of the code into RAM
SECTIONS
{
  . = SEGMENT_START(".text.spm", 0x10000);
  .text.spm : { *(.text.spm) }
  
  . = SEGMENT_START(".rodata", 0x20000);
  .rodata : { *(.rodata .rodata.*) }
  
  . += 0x1000;
  .text : { *(.text .text.*) }

  . += 0x1000;
  .fini_array : { *(SORT(.fini_array.*) .fini_array) }
  .init_array : { *(SORT(.init_array.*) .init_array) }
  
  . += 0x1000;
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = ALIGN(8);
  PROVIDE (end = .);
}
