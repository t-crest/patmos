# A linker script for bootable programs
SECTIONS
{
  . = SEGMENT_START(".text", 0x0);
  .text : { *(.text) }
  
  . = SEGMENT_START(".data", 0x20000);
  .data : { *(.data) }
  .bss : { *(.bss) }
  
  . = ALIGN(8);
  PROVIDE (end = .);
  
  . = SEGMENT_START(".rodata", 0xf0008010);
  .rodata : { *(.rodata) }
  
}
