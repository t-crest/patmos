# A linker script to set virtual addresses
PHDRS
{
  text   PT_LOAD;
  rodata PT_LOAD;
  data   PT_LOAD;
}

SECTIONS
{
  .text.spm  0x10000 : { *(.text.spm) }

  .text   0x20000000 : { *(.text .text.* ) } :text

  .rodata 0x40000000 : { *(.rodata .rodata.*) } :rodata

  .data   0x60000000 : { *(SORT(.init_array.*) .init_array)
                         *(SORT(.fini_array.*) .fini_array) 
                         *(.data .data.*) *(.bss) } :data

  . = ALIGN(8);
  _end = .; PROVIDE (end = .);
}
