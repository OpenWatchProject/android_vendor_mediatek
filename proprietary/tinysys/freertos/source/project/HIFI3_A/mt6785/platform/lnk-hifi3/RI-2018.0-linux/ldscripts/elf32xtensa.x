/* This linker script generated from xt-genldscripts.tpp for LSP RI-2018.0-linux */
/* Linker Script for default link */
MEMORY
{
  dram_0_seg :                        	org = 0x4FFB0000, len = 0x7800
  dram_reserved_seg :                 	org = 0x4FFB7800, len = 0x800
  iram0_0_seg :                       	org = 0x4FFE0000, len = 0x400
  iram0_1_seg :                       	org = 0x4FFE0400, len = 0x17C
  iram0_2_seg :                       	org = 0x4FFE057C, len = 0x24
  iram0_3_seg :                       	org = 0x4FFE05A0, len = 0x40
  iram0_4_seg :                       	org = 0x4FFE05E0, len = 0x40
  iram0_5_seg :                       	org = 0x4FFE0620, len = 0x40
  iram0_6_seg :                       	org = 0x4FFE0660, len = 0x3C
  iram0_7_seg :                       	org = 0x4FFE069C, len = 0x20
  iram0_8_seg :                       	org = 0x4FFE06BC, len = 0x20
  iram0_9_seg :                       	org = 0x4FFE06DC, len = 0x20
  iram0_10_seg :                      	org = 0x4FFE06FC, len = 0x8904
  sram_0_seg :                        	org = 0x56000400, len = 0x6FFC00
}

PHDRS
{
  dram_0_phdr PT_LOAD;
  dram_0_bss_phdr PT_LOAD;
  dram_reserved_phdr PT_LOAD;
  iram0_0_phdr PT_LOAD;
  iram0_1_phdr PT_LOAD;
  iram0_2_phdr PT_LOAD;
  iram0_3_phdr PT_LOAD;
  iram0_4_phdr PT_LOAD;
  iram0_5_phdr PT_LOAD;
  iram0_6_phdr PT_LOAD;
  iram0_7_phdr PT_LOAD;
  iram0_8_phdr PT_LOAD;
  iram0_9_phdr PT_LOAD;
  iram0_10_phdr PT_LOAD;
  sram_0_phdr PT_LOAD;
  sram_0_bss_phdr PT_LOAD;
}


/*  Default entry point:  */
ENTRY(_ResetVector)


/*  Memory boundary addresses:  */
_memmap_mem_dram0_start = 0x4ffb0000;
_memmap_mem_dram0_end   = 0x4ffc0000;
_memmap_mem_dram1_start = 0x4ffc0000;
_memmap_mem_dram1_end   = 0x4ffe0000;
_memmap_mem_iram0_start = 0x4ffe0000;
_memmap_mem_iram0_end   = 0x4fff0000;
_memmap_mem_sram_start = 0x56000000;
_memmap_mem_sram_end   = 0x56700000;
_memmap_mem_confreg_start = 0x0;
_memmap_mem_confreg_end   = 0x40000000;
_memmap_mem_audiosys_start = 0xd1210000;
_memmap_mem_audiosys_end   = 0xd1211000;
confreg = 0x0;
audiosys = 0xd1210000;

/*  Memory segment boundary addresses:  */
_memmap_seg_dram_0_start = 0x4ffb0000;
_memmap_seg_dram_0_max   = 0x4ffb7800;
_memmap_seg_dram_reserved_start = 0x4ffb7800;
_memmap_seg_dram_reserved_max   = 0x4ffb8000;
_memmap_seg_iram0_0_start = 0x4ffe0000;
_memmap_seg_iram0_0_max   = 0x4ffe0400;
_memmap_seg_iram0_1_start = 0x4ffe0400;
_memmap_seg_iram0_1_max   = 0x4ffe057c;
_memmap_seg_iram0_2_start = 0x4ffe057c;
_memmap_seg_iram0_2_max   = 0x4ffe05a0;
_memmap_seg_iram0_3_start = 0x4ffe05a0;
_memmap_seg_iram0_3_max   = 0x4ffe05e0;
_memmap_seg_iram0_4_start = 0x4ffe05e0;
_memmap_seg_iram0_4_max   = 0x4ffe0620;
_memmap_seg_iram0_5_start = 0x4ffe0620;
_memmap_seg_iram0_5_max   = 0x4ffe0660;
_memmap_seg_iram0_6_start = 0x4ffe0660;
_memmap_seg_iram0_6_max   = 0x4ffe069c;
_memmap_seg_iram0_7_start = 0x4ffe069c;
_memmap_seg_iram0_7_max   = 0x4ffe06bc;
_memmap_seg_iram0_8_start = 0x4ffe06bc;
_memmap_seg_iram0_8_max   = 0x4ffe06dc;
_memmap_seg_iram0_9_start = 0x4ffe06dc;
_memmap_seg_iram0_9_max   = 0x4ffe06fc;
_memmap_seg_iram0_10_start = 0x4ffe06fc;
_memmap_seg_iram0_10_max   = 0x4ffe9000;
_memmap_seg_sram_0_start = 0x56000400;
_memmap_seg_sram_0_max   = 0x56700000;

_rom_store_table = 0;
PROVIDE(_memmap_reset_vector = 0x4ffe0000);
PROVIDE(_memmap_vecbase_reset = 0x4ffe0400);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x04000144;
_memmap_cacheattr_wt_base = 0x04000344;
_memmap_cacheattr_bp_base = 0x04000444;
_memmap_cacheattr_unused_mask = 0xF0FFF000;
_memmap_cacheattr_wb_trapnull = 0x44444144;
_memmap_cacheattr_wba_trapnull = 0x44444144;
_memmap_cacheattr_wbna_trapnull = 0x44444244;
_memmap_cacheattr_wt_trapnull = 0x44444344;
_memmap_cacheattr_bp_trapnull = 0x44444444;
_memmap_cacheattr_wb_strict = 0x04000144;
_memmap_cacheattr_wt_strict = 0x04000344;
_memmap_cacheattr_bp_strict = 0x04000444;
_memmap_cacheattr_wb_allvalid = 0x44444144;
_memmap_cacheattr_wt_allvalid = 0x44444344;
_memmap_cacheattr_bp_allvalid = 0x44444444;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .UserExceptionVector.literal : ALIGN(4)
  {
    _UserExceptionVector_literal_start = ABSOLUTE(.);
    *(.UserExceptionVector.literal)
    . = ALIGN (4);
    _UserExceptionVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .ResetVector.literal : ALIGN(4)
  {
    _ResetVector_literal_start = ABSOLUTE(.);
    *(.ResetVector.literal)
    . = ALIGN (4);
    _ResetVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .NMIExceptionVector.literal : ALIGN(4)
  {
    _NMIExceptionVector_literal_start = ABSOLUTE(.);
    *(.NMIExceptionVector.literal)
    . = ALIGN (4);
    _NMIExceptionVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .Level4InterruptVector.literal : ALIGN(4)
  {
    _Level4InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level4InterruptVector.literal)
    . = ALIGN (4);
    _Level4InterruptVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .Level3InterruptVector.literal : ALIGN(4)
  {
    _Level3InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level3InterruptVector.literal)
    . = ALIGN (4);
    _Level3InterruptVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .Level2InterruptVector.literal : ALIGN(4)
  {
    _Level2InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level2InterruptVector.literal)
    . = ALIGN (4);
    _Level2InterruptVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .KernelExceptionVector.literal : ALIGN(4)
  {
    _KernelExceptionVector_literal_start = ABSOLUTE(.);
    *(.KernelExceptionVector.literal)
    . = ALIGN (4);
    _KernelExceptionVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .DoubleExceptionVector.literal : ALIGN(4)
  {
    _DoubleExceptionVector_literal_start = ABSOLUTE(.);
    *(.DoubleExceptionVector.literal)
    . = ALIGN (4);
    _DoubleExceptionVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .DebugExceptionVector.literal : ALIGN(4)
  {
    _DebugExceptionVector_literal_start = ABSOLUTE(.);
    *(.DebugExceptionVector.literal)
    . = ALIGN (4);
    _DebugExceptionVector_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .dram.rodata : ALIGN(4)
  {
    _dram_rodata_start = ABSOLUTE(.);
    *(.dram.rodata)
    . = ALIGN (4);
    _dram_rodata_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .dram.literal : ALIGN(4)
  {
    _dram_literal_start = ABSOLUTE(.);
    *(.dram.literal)
    . = ALIGN (4);
    _dram_literal_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .dram.data : ALIGN(4)
  {
    _dram_data_start = ABSOLUTE(.);
    *(.dram.data)
    . = ALIGN (4);
    _dram_data_end = ABSOLUTE(.);
  } >dram_0_seg :dram_0_phdr

  .dram.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _dram_bss_start = ABSOLUTE(.);
    *(.dram.bss)
    . = ALIGN (8);
    _dram_bss_end = ABSOLUTE(.);
    _stack_sentry = ALIGN(0x8);
    _memmap_seg_dram_0_end = ALIGN(0x8);
  } >dram_0_seg :dram_0_bss_phdr

  PROVIDE(__stack = 0x4ffb7800);

  .dram_reserved : ALIGN(4)
  {
    _dram_reserved_start = ABSOLUTE(.);
    KEEP (*(.dram_reserved))
    . = ALIGN (4);
    _dram_reserved_end = ABSOLUTE(.);
    _memmap_seg_dram_reserved_end = ALIGN(0x8);
  } >dram_reserved_seg :dram_reserved_phdr


  .ResetVector.text : ALIGN(4)
  {
    _ResetVector_text_start = ABSOLUTE(.);
    KEEP (*(.ResetVector.text))
    . = ALIGN (4);
    _ResetVector_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .ResetHandler.text : ALIGN(4)
  {
    _ResetHandler_text_start = ABSOLUTE(.);
    *(.ResetHandler.literal .ResetHandler.text)
    . = ALIGN (4);
    _ResetHandler_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_0_end = ALIGN(0x8);
  } >iram0_0_seg :iram0_0_phdr


  .WindowVectors.text : ALIGN(4)
  {
    _WindowVectors_text_start = ABSOLUTE(.);
    KEEP (*(.WindowVectors.text))
    . = ALIGN (4);
    _WindowVectors_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_1_end = ALIGN(0x8);
  } >iram0_1_seg :iram0_1_phdr


  .Level2InterruptVector.text : ALIGN(4)
  {
    _Level2InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level2InterruptVector.text))
    . = ALIGN (4);
    _Level2InterruptVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_2_end = ALIGN(0x8);
  } >iram0_2_seg :iram0_2_phdr


  .Level3InterruptVector.text : ALIGN(4)
  {
    _Level3InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level3InterruptVector.text))
    . = ALIGN (4);
    _Level3InterruptVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_3_end = ALIGN(0x8);
  } >iram0_3_seg :iram0_3_phdr


  .Level4InterruptVector.text : ALIGN(4)
  {
    _Level4InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level4InterruptVector.text))
    . = ALIGN (4);
    _Level4InterruptVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_4_end = ALIGN(0x8);
  } >iram0_4_seg :iram0_4_phdr


  .DebugExceptionVector.text : ALIGN(4)
  {
    _DebugExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DebugExceptionVector.text))
    . = ALIGN (4);
    _DebugExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_5_end = ALIGN(0x8);
  } >iram0_5_seg :iram0_5_phdr


  .NMIExceptionVector.text : ALIGN(4)
  {
    _NMIExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.NMIExceptionVector.text))
    . = ALIGN (4);
    _NMIExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_6_end = ALIGN(0x8);
  } >iram0_6_seg :iram0_6_phdr


  .KernelExceptionVector.text : ALIGN(4)
  {
    _KernelExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.KernelExceptionVector.text))
    . = ALIGN (4);
    _KernelExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_7_end = ALIGN(0x8);
  } >iram0_7_seg :iram0_7_phdr


  .UserExceptionVector.text : ALIGN(4)
  {
    _UserExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.UserExceptionVector.text))
    . = ALIGN (4);
    _UserExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_8_end = ALIGN(0x8);
  } >iram0_8_seg :iram0_8_phdr


  .DoubleExceptionVector.text : ALIGN(4)
  {
    _DoubleExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DoubleExceptionVector.text))
    . = ALIGN (4);
    _DoubleExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_9_end = ALIGN(0x8);
  } >iram0_9_seg :iram0_9_phdr


  .iram.text : ALIGN(4)
  {
    _iram_text_start = ABSOLUTE(.);
    *(.iram.literal .iram.text)
    . = ALIGN (4);
    _iram_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_10_end = ALIGN(0x8);
  } >iram0_10_seg :iram0_10_phdr


  .sram.text : ALIGN(4)
  {
    _sram_text_start = ABSOLUTE(.);
    *(.sram.literal .sram.text)
    . = ALIGN (4);
    _sram_text_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    *(.entry.text)
    *(.init.literal)
    KEEP(*(.init))
    *(.literal.sort.* SORT(.text.sort.*))
    KEEP (*(.literal.keepsort.* SORT(.text.keepsort.*) .literal.keep.* .text.keep.* .literal.*personality* .text.*personality*))
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.fini.literal)
    KEEP(*(.fini))
    *(.gnu.version)
    . = ALIGN (4);
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >sram_0_seg :sram_0_phdr

  .clib.rodata : ALIGN(4)
  {
    _clib_rodata_start = ABSOLUTE(.);
    *(.clib.rodata)
    . = ALIGN (4);
    _clib_rodata_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .rtos.rodata : ALIGN(4)
  {
    _rtos_rodata_start = ABSOLUTE(.);
    *(.rtos.rodata)
    . = ALIGN (4);
    _rtos_rodata_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .sram.rodata : ALIGN(4)
  {
    _sram_rodata_start = ABSOLUTE(.);
    *(.sram.rodata)
    . = ALIGN (4);
    _sram_rodata_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    *(.rodata)
    *(SORT(.rodata.sort.*))
    KEEP (*(SORT(.rodata.keepsort.*) .rodata.keep.*))
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    KEEP (*(.xt_except_table))
    KEEP (*(.gcc_except_table))
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    KEEP (*(.eh_frame))
    /*  C++ constructor and destructor tables, properly ordered:  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_dram_bss_start)
    LONG(_dram_bss_end)
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);
    . = ALIGN (4);
    _rodata_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .mpu_ro_separator : ALIGN(4)
  {
    _mpu_ro_separator_start = ABSOLUTE(.);
    *(.mpu_ro_separator)
    . = ALIGN (4);
    _mpu_ro_separator_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .clib.percpu.data : ALIGN(4)
  {
    _clib_percpu_data_start = ABSOLUTE(.);
    *(.clib.percpu.data)
    . = ALIGN (4);
    _clib_percpu_data_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .rtos.percpu.data : ALIGN(4)
  {
    _rtos_percpu_data_start = ABSOLUTE(.);
    *(.rtos.percpu.data)
    . = ALIGN (4);
    _rtos_percpu_data_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .clib.data : ALIGN(4)
  {
    _clib_data_start = ABSOLUTE(.);
    *(.clib.data)
    . = ALIGN (4);
    _clib_data_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .rtos.data : ALIGN(4)
  {
    _rtos_data_start = ABSOLUTE(.);
    *(.rtos.data)
    . = ALIGN (4);
    _rtos_data_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.data)
    *(SORT(.data.sort.*))
    KEEP (*(SORT(.data.keepsort.*) .data.keep.*))
    *(.data.*)
    *(.gnu.linkonce.d.*)
    KEEP(*(.gnu.linkonce.d.*personality*))
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    KEEP(*(.jcr))
    . = ALIGN (4);
    _data_end = ABSOLUTE(.);
  } >sram_0_seg :sram_0_phdr

  .bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.bss)
    *(SORT(.bss.sort.*))
    KEEP (*(SORT(.bss.keepsort.*) .bss.keep.*))
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    *(.clib.percpu.bss)
    *(.rtos.percpu.bss)
    *(.clib.bss)
    *(.rtos.bss)
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _end = ALIGN(0x8);
    PROVIDE(end = ALIGN(0x8));
    _memmap_seg_sram_0_end = ALIGN(0x8);
  } >sram_0_seg :sram_0_bss_phdr

  _heap_sentry = 0x56700000;
  .debug  0 :  { *(.debug) }
  .line  0 :  { *(.line) }
  .debug_srcinfo  0 :  { *(.debug_srcinfo) }
  .debug_sfnames  0 :  { *(.debug_sfnames) }
  .debug_aranges  0 :  { *(.debug_aranges) }
  .debug_pubnames  0 :  { *(.debug_pubnames) }
  .debug_info  0 :  { *(.debug_info) }
  .debug_abbrev  0 :  { *(.debug_abbrev) }
  .debug_line  0 :  { *(.debug_line) }
  .debug_frame  0 :  { *(.debug_frame) }
  .debug_str  0 :  { *(.debug_str) }
  .debug_loc  0 :  { *(.debug_loc) }
  .debug_macinfo  0 :  { *(.debug_macinfo) }
  .debug_weaknames  0 :  { *(.debug_weaknames) }
  .debug_funcnames  0 :  { *(.debug_funcnames) }
  .debug_typenames  0 :  { *(.debug_typenames) }
  .debug_varnames  0 :  { *(.debug_varnames) }
  .xt.insn 0 :
  {
    KEEP (*(.xt.insn))
    KEEP (*(.gnu.linkonce.x.*))
  }
  .xt.prop 0 :
  {
    KEEP (*(.xt.prop))
    KEEP (*(.xt.prop.*))
    KEEP (*(.gnu.linkonce.prop.*))
  }
  .xt.lit 0 :
  {
    KEEP (*(.xt.lit))
    KEEP (*(.xt.lit.*))
    KEEP (*(.gnu.linkonce.p.*))
  }
  .debug.xt.callgraph 0 :
  {
    KEEP (*(.debug.xt.callgraph .debug.xt.callgraph.* .gnu.linkonce.xt.callgraph.*))
  }
}

