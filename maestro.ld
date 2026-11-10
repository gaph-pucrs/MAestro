ENTRY(_start)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0)); . = SEGMENT_START("text-segment", 0);
  .text           :
  {
    KEEP (*(SORT_NONE(.init)))
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(SORT(.text.sorted.*))
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf.em.  */
    *(.gnu.warning)
    . = ALIGN(16);    /* The sizes reported to the MMU must add up with no gaps */
  }
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  .rodata         :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata .srodata.*)
    . = ALIGN(16);
  }
  .data           :
  {
    __DATA_BEGIN__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
    __SDATA_BEGIN__ = .;
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    /* . = ALIGN(4); */
  }
  _edata = .; PROVIDE (edata = .);
  __bss_start = .;
  .bss            :
  {
   *(.dynsbss)
   *(.sbss .sbss.* .gnu.linkonce.sb.*)
   *(.scommon)
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we do not
      pad the .data section.  */
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  . = ALIGN(32 / 8);
  . = SEGMENT_START("ldata-segment", .);
  . = ALIGN(32 / 8);
  __BSS_END__ = .;
    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
		            MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));
  _end = .; PROVIDE (end = .);

  __text_size   = SIZEOF(.text);
  __rodata_size = SIZEOF(.rodata);
  __data_size   = SIZEOF(.data);
  __bss_size    = _end - __bss_start;
  ASSERT(ADDR(.rodata) == ADDR(.text) + SIZEOF(.text), "gap between .text and .rodata")
  ASSERT(ADDR(.data) == ADDR(.rodata) + SIZEOF(.rodata), "gap between .rodata and .data")
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
