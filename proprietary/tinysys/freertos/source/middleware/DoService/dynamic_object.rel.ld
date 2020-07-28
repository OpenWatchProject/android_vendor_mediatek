FORCE_COMMON_ALLOCATION

SECTIONS
{
  . = ALIGN(4);
  .do_header :
  {
    /* magic : "DynamicObject", 0x00, 0xff, 0xff */
    LONG(0x616e7944)
    LONG(0x4f63696d)
    LONG(0x63656a62)
    LONG(0xffff0074)

    /* Used to retrieve init/deinit functions of each feature */
    LONG(__do_features_start)
    LONG(__do_features_end)

    /* Used to locate ARM_EXIDX section in SRAM */
    LONG(__do_exidx_start)
    LONG(__do_exidx_end)

    /* Used to allocate BSS section in SRAM */
    LONG(__do_bss_start)
    LONG(__do_bss_end)

    /* reserved for future use */
    LONG(0x65736572)
    LONG(0x64657672)

    . = ALIGN(4);
  }

  . = ALIGN(4);
  .text :
  {
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.commands))
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  }

  . = ALIGN(4);
  .do_features :
  {
    KEEP (*(.do_features.startup))
    __do_features_start = .;
    KEEP (*(.do_features))
    __do_features_end = .;
    . = ALIGN(4);
  }

  . = ALIGN(4);
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  }

  . = ALIGN(4);
  .ARM :
  {
    __do_exidx_start = .;
    *(.ARM.exidx*)
    __do_exidx_end = .;
  }

  . = ALIGN(4);
  .data :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
    . = ALIGN(4);
  }

  . = ALIGN(4);
  .bss :
  {
    __do_bss_start = .;
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(4);
    __do_bss_end = .;
  }

  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
