OUTPUT_FORMAT("elf64-littleaarch64", "elf64-bigaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(_start)
PHDRS
{
  text PT_LOAD ;
}
SECTIONS
{
    . = 0x000000000 + 0x38000000;

    /* text/read-only data */
    /* set the load address to physical MEMBASE */
    .text : AT(0x00000000 + 0x38000000) {
        _stext = .;
        KEEP(*(.text.boot))
        KEEP(*(.text.boot.vectab))
        . = ALIGN(4096);
        _el1_vectab_start = .;
        KEEP(*(.text.el1.vectab))
        _el1_vectab_end = .;
        *(.text* .sram.text.glue_7* .gnu.linkonce.t.*)
        _etext = .;
    }

    .init.text : ALIGN(4096) {
        _sinittext = .;
        *(.init.text)
        _einittext = .;
    } : text

    .interp : { *(.interp) }
    .hash : { *(.hash) }
    .dynsym : { *(.dynsym) }
    .dynstr : { *(.dynstr) }
    .rel.text : { *(.rel.text) *(.rel.gnu.linkonce.t*) }
    .rela.text : { *(.rela.text) *(.rela.gnu.linkonce.t*) }
    .rel.data : { *(.rel.data) *(.rel.gnu.linkonce.d*) }
    .rela.data : { *(.rela.data) *(.rela.gnu.linkonce.d*) }
    .rel.rodata : { *(.rel.rodata) *(.rel.gnu.linkonce.r*) }
    .rela.rodata : { *(.rela.rodata) *(.rela.gnu.linkonce.r*) }
    .rel.got : { *(.rel.got) }
    .rela.got : { *(.rela.got) }
    .rel.ctors : { *(.rel.ctors) }
    .rela.ctors : { *(.rela.ctors) }
    .rel.dtors : { *(.rel.dtors) }
    .rela.dtors : { *(.rela.dtors) }
    .rel.init : { *(.rel.init) }
    .rela.init : { *(.rela.init) }
    .rel.fini : { *(.rel.fini) }
    .rela.fini : { *(.rela.fini) }
    .rel.bss : { *(.rel.bss) }
    .rela.bss : { *(.rela.bss) }
    .rel.plt : { *(.rel.plt) }
    .rela.plt : { *(.rela.plt) }
    .init : { *(.init) } =0x9090
    .plt : { *(.plt) }

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
    __exidx_end = .;

    .rodata : ALIGN(4096) {
        __rodata_start = .;
        __fault_handler_table_start = .;
        KEEP(*(.rodata.fault_handler_table))
        __fault_handler_table_end = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
INCLUDE "arch/shared_rodata_sections.ld"
        . = ALIGN(8);
        __rodata_end = .;
    }

/* for criterion unittest framework */
INCLUDE "arch/criterion.ld"

    .data : ALIGN(4096) {
        /* writable data  */
        __data_start_rom = .;
        /* in one segment binaries, the rom data address is on top of the ram data address */
        __data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
INCLUDE "arch/shared_data_sections.ld"
    }

    .init.data : ALIGN(4096) {
       *(.init.rodata)
       *(.init.rodata.str*)
       *(.init.data)
       *(.init.data.rel)
       *(.init.data.rel.*)
    } : text

    .ctors : ALIGN(8) {
        __ctor_list = .;
        KEEP(*(.ctors .init_array))
        __ctor_end = .;
    }
    .dtors : ALIGN(8) {
        __dtor_list = .;
        KEEP(*(.dtors .fini_array))
        __dtor_end = .;
    }
    .got : { *(.got.plt) *(.got) }
    .dynamic : { *(.dynamic) }

    __data_end = .;

    /* unintialized data (in same segment as writable data) */
    .bss : ALIGN(8) {
        KEEP(*(.bss.prebss.*))
        . = ALIGN(8);
        __bss_start = .;
        *(.bss)
        . = ALIGN(8);
        __per_cpu_start = .;
        *(.bss.percpu)
        . = ALIGN(8);
        *(.bss.percpu.read_mostly)
        . = ALIGN(8);
        __per_cpu_data_end = .;
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }

    . = ALIGN(8);
    _end = .;

    . = 0x000000000 + 0x4000000 ;
    _end_of_ram = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : {
       *(.exit.text)
       *(.exit.data)
       *(.exitcall.exit)
        *(.comment .note .eh_frame)
    }
}
