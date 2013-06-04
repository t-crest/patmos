# A simple linker script to put code and data into the SPMs
SECTIONS
{
  . = SEGMENT_START(".rodata", 0x0);
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = SEGMENT_START(".text", 0x800000);
  .text : { *(.text .text.*) }
}
