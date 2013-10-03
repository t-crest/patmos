# A simple linker script to put code and data into the SPMs
SECTIONS
{
  . = SEGMENT_START(".rodata", 0x10);
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = SEGMENT_START(".text", 0x20000);
  .text : { *(.text .text.*) }

  . = ALIGN(8);
  _end = .; PROVIDE (end = .);
}
