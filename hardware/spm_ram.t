# A linker script to put .text.spm into the I-SPM and the rest of the code into RAM
SECTIONS
{
  . = SEGMENT_START(".text.spm", 0x10000);
  .text.spm : { *(.text.spm) }
  . = SEGMENT_START(".text", 0x20000);
  .text : { *(.text .text.* ) }
  #. = SEGMENT_START(".rodata", 0x400);
  .rodata : { *(.rodata .rodata.*) }
  .init_array : { *(SORT(.init_array.*) .init_array) }
  .fini_array : { *(SORT(.fini_array.*) .fini_array) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = ALIGN(8);
  _end = .; PROVIDE (end = .);

}
