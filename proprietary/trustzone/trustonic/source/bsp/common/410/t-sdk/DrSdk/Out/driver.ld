/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_drEntry)

PHDRS
{
   phdr1  PT_LOAD FLAGS(0x80000007);
}


SECTIONS
{
   . = 0x1000;

   .text :
   {
      KEEP(*(stext))
      *(.text)
      *(.text.*)
      *(.conststring)
      *(text)
      *(.rodata .rodata.* .gnu.linkonce.r.*) *(.constdata)
      *(etext)
   } :phdr1

   /*In the virtual address space we leave one page unused between code and data*/
   . = . + 0x1000;
   .data ALIGN(0x1000):
   {
      *(sdata)
      KEEP(*(.data .data.* .gnu.linkonce.d.*))
      *(edata)
      . = ALIGN(0x8);
   }

   .bss :
   {
      _sbss = .;
      *(COMMON)
      *(stack)
      *(heap)
      *(.bss.*)
      *(.bss)
      . = ALIGN(0x8); /* prevent external libs from unaligning TCI buffer */
      /*Reserved location for tciBuffer and tciBufferLen*/
      . = . + 0x8;
   }

   /DISCARD/ :
   {
      *(.comment)
      *(.ARM.attributes)
/* --strip-debug has been added when MODE=Release
      *(.debug_info)
      *(.debug_line)
      *(.debug_abbrev)
      *(.debug_frame)
      *(.debug_loc)
      *(.debug_macinfo)
      *(.debug_pubnames)
*/
      *(.arm_vfe_header)
      *(.note.gnu.gold-version)
      *(.symtab)
      *(.strtab)
   }
}
