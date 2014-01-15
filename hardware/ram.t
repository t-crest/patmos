# A simple linker script to put code and data into the external memory
SECTIONS
{
  . = SEGMENT_START(".rodata", 0x400);
  .rodata : { *(.rodata .rodata.*) }
  .init_array : { *(SORT(.init_array.*) .init_array) }
  .fini_array : { *(SORT(.fini_array.*) .fini_array) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = SEGMENT_START(".text", 0x20000);
  .text : { *(.text .text.*) }

  . = ALIGN(8);
  _end = .; PROVIDE (end = .);
}
