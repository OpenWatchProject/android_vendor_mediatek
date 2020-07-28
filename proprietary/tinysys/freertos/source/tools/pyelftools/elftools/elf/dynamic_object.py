#!/usr/bin/env python

import os, sys
import struct
from collections import namedtuple

from ..common.py3compat import BytesIO
from ..common.utils import do_log_fatal, do_log_w, do_log_i
from .constants import SH_FLAGS
from .descriptions import describe_symbol_type, _DESCR_ST_INFO_TYPE
from .elffile import ELFFile
from .enums import ENUM_ST_SHNDX
from .relocation import RelocationSection, ArmRelocationHandler
from .sections import SymbolTableSection

#-------------------------------------------------------------------------------
DO_HEADER_SECTION_NAME = ".do_header"
DO_FEATURES_SECTION_NAME = ".do_features"
BSS_SECTION_NAME = ".bss"
ARM_EXIDX_SECTION_NAME = ".ARM"

REL_SECTION_NAME_PREFIX = ".rel"

EXIDX_START_SYMBOL_NAME = "__exidx_start"
EXIDX_END_SYMBOL_NAME = "__exidx_end"

# First 16 bytes of .do_header are used for magic hex
DO_HEADER_MAGIC_LEN = 16

# Size used for storing DO dummy entry point
DO_PSEUDO_ENTRY_LEN = 4

ST_VALUE_OFFSET_IN_BYTES = 4
ST_SHNDX_OFFSET_IN_BYTES = 14
#-------------------------------------------------------------------------------
class SectionContents(object):
    def __init__(self, sec):
        self.name = sec.name
        self.flags = sec['sh_flags']
        self.addr = sec['sh_addr']
        self.size = sec['sh_size']
        self.addralign = sec['sh_addralign']

    def __repr__(self):
        return repr((self.name, self.flags, self.addr, self.size, self.addralign))

class DynamicObject(ELFFile):
    def __init__(self, stream):
        super(DynamicObject, self).__init__(stream)

    def calculate_required_sram_size(self):
        all_alloc_sections = []
        for nsec, section in enumerate(self.iter_sections()):
            if section['sh_flags'] & SH_FLAGS.SHF_ALLOC == SH_FLAGS.SHF_ALLOC:
                all_alloc_sections.append(SectionContents(section))

        if len(all_alloc_sections) <= 0:
            do_log_fatal('No any section with SHF_ALLOC flag\n')

        all_alloc_sections.sort(key=lambda s: s.addr)

        """ To detect a known issue:
            If a user-defined section is declared, sh_addr of this section in the linked
            dynamic object ELF file will set as '0', which will affect all sections' sh_addr
            after this section, and the final step (OBJCOPY) of DO generation.
        """
        for idx in xrange(0, len(all_alloc_sections)-1):
            addr_range_end = all_alloc_sections[idx].addr + all_alloc_sections[idx].size
            if all_alloc_sections[idx+1].addr >= addr_range_end:
                idx += 1
            else:
                do_log_fatal('Invalid section: %s - sh_addr: 0x%x\n' % (all_alloc_sections[idx+1].name, all_alloc_sections[idx+1].addr))

        section_highest_addr = all_alloc_sections[-1].addr
        section_size = all_alloc_sections[-1].size
        section_addr_alignment = all_alloc_sections[-1].addralign

        if section_highest_addr <= 0:
            do_log_fatal('Invalid sh_addr: 0x%x\n' % (section_highest_addr))

        '''do_log_i('required SRAM size is calculated from addr: 0x%x, size: 0x%x, addr_align:0x%x\n' % (
            section_highest_addr, section_size, section_addr_alignment))'''

        required_sram_size = 0x0
        if section_addr_alignment == 0 or section_addr_alignment == 1:
            # this means no address alignment requirement
            required_sram_size = section_highest_addr + section_size
        else:
            # TODO: check if section_addr_alignment is power of 2
            _r = (section_highest_addr + section_size) % section_addr_alignment
            if _r > 0:
                required_sram_size = section_highest_addr + section_size - _r + section_addr_alignment
            else:
                required_sram_size = section_highest_addr + section_size

        return required_sram_size

    def get_allocated_sections_mapping(self, do_ws_start_sram_addr):
        do_bin_sections_index_addr_map = {}
        for nsec, section in enumerate(self.iter_sections()):
            if section['sh_flags'] & SH_FLAGS.SHF_ALLOC != SH_FLAGS.SHF_ALLOC:
                continue
            else:
                do_bin_sections_index_addr_map[nsec] = do_ws_start_sram_addr + section['sh_addr']

        return do_bin_sections_index_addr_map

    def get_rel_sections(self):
        rel_sections_list = []
        for nsec, section in enumerate(self.iter_sections()):
            if section['sh_type'] == 'SHT_RELA':
                do_log_fatal('Not support RELA now!\n')
            elif section['sh_type'] == 'SHT_REL':
                rel_sections_list.append(nsec)

        return rel_sections_list

    def resolve_local_symbols(self, do_bin_index_addr_map, output_filename):
        fd_out = open(output_filename, 'wb')
        self._dup_elf_file_contents(fd_out)

        section = self.get_section_by_name('.symtab')
        if not isinstance(section, SymbolTableSection):
            do_log_fatal("Invalid .symtab section (class not match)\n")

        if section['sh_type'] != 'SHT_SYMTAB':
            do_log_fatal("Invalid .symtab section (wrong section type)\n")

        do_log_i('Update local symbols in DO file\n\n')
        for nsym, symbol in enumerate(section.iter_symbols()):
            if (symbol['st_shndx'] == 'SHN_UNDEF' or
                symbol['st_shndx'] == 'SHN_ABS' or
                symbol['st_shndx'] == 'SHN_COMMON'):
                do_log_i("#%d: %s\n" % (nsym, symbol['st_shndx']))
                continue

            sec_sram_addr_start = do_bin_index_addr_map.get(symbol['st_shndx'], None)
            if sec_sram_addr_start is None:
                if describe_symbol_type(symbol['st_info']['type']) == _DESCR_ST_INFO_TYPE['STT_SECTION']:
                    do_log_i("#%d: [SEC] %s: Not included in DO.bin\n" % (
                        nsym, self.get_section(symbol['st_shndx']).name))
                    continue
                else:
                    do_log_fatal("#%d: [Not Section] Not included in DO.bin\n" % (nsym))
            else:
                sym_sram_addr = sec_sram_addr_start + symbol['st_value']
                if (symbol.name == "" and
                    describe_symbol_type(symbol['st_info']['type']) == _DESCR_ST_INFO_TYPE['STT_SECTION']):
                    sym_sec = self.get_section(symbol['st_shndx'])
                    sym_name = '[SEC] ' + sym_sec.name
                else:
                    sym_name = symbol.name

                do_log_i("#%d: %s = %s;\n" % (nsym, sym_name,
                    self._format_hex(sym_sram_addr, fullhex=True, lead0x=True)))

                sym_sram_addr_b = struct.pack('<I', sym_sram_addr)
                fd_out.seek(section['sh_offset'] + nsym * section['sh_entsize'] + ST_VALUE_OFFSET_IN_BYTES, 0)
                fd_out.write(sym_sram_addr_b)

        do_log_i('\n')
        fd_out.close()

    def get_exidx_start_sram_addr(self, do_ws_start_sram_addr):
        arm_exidx_section = self.get_section_by_name(ARM_EXIDX_SECTION_NAME)
        if arm_exidx_section is None:
            do_log_i('No ARM_EXIDX section is found\n')
            return None

        return do_ws_start_sram_addr + arm_exidx_section['sh_addr']

    def get_exidx_end_sram_addr(self, do_ws_start_sram_addr):
        arm_exidx_section = self.get_section_by_name(ARM_EXIDX_SECTION_NAME)
        if arm_exidx_section is None:
            do_log_i('No ARM_EXIDX section is found\n')
            return None

        return do_ws_start_sram_addr + arm_exidx_section['sh_addr'] + arm_exidx_section['sh_size']

    def rewrite_exidx_symbols(self, do_ws_start_sram_addr, tinysys_resolved_symbols_map):
        if tinysys_resolved_symbols_map.get(EXIDX_START_SYMBOL_NAME, None) is not None:
            exidx_start_sram_addr = self.get_exidx_start_sram_addr(do_ws_start_sram_addr)
            if exidx_start_sram_addr is not None:
                tinysys_resolved_symbols_map[EXIDX_START_SYMBOL_NAME] = exidx_start_sram_addr
                do_log_i('%s = 0x%x\n' % (EXIDX_START_SYMBOL_NAME, exidx_start_sram_addr))

        if tinysys_resolved_symbols_map.get(EXIDX_END_SYMBOL_NAME, None) is not None:
            exidx_end_sram_addr = self.get_exidx_end_sram_addr(do_ws_start_sram_addr)
            if exidx_end_sram_addr is not None:
                tinysys_resolved_symbols_map[EXIDX_END_SYMBOL_NAME] = exidx_end_sram_addr
                do_log_i('%s = 0x%x\n' % (EXIDX_END_SYMBOL_NAME, exidx_end_sram_addr))

        return tinysys_resolved_symbols_map

    def update_und_symbols(self, und_symbols_map, resolved_symbols_map, output_filename):
        fd_out = open(output_filename, 'wb')
        self._dup_elf_file_contents(fd_out)

        symtab = self.get_section_by_name('.symtab')
        self._check_if_valid_symtab_section(symtab)

        do_log_i('Update undefined symbols in DO file\n\n')
        for k in und_symbols_map.keys():
            und_sym_idx = und_symbols_map.get(k, None)
            if und_sym_idx is None:
                do_log_fatal('UND symbol index is missing\n')

            und_sym_val = resolved_symbols_map.get(k, None)
            if und_sym_val is None:
                do_log_fatal('Corresponding symbol value is not found\n')

            do_log_i("Update #%d %s = %s\n" % (und_sym_idx, k,
                self._format_hex(und_sym_val, fullhex=True, lead0x=True)))

            und_sym_val_b = struct.pack('<I', und_sym_val)
            fd_out.seek(symtab['sh_offset'] + und_sym_idx * symtab['sh_entsize'] + ST_VALUE_OFFSET_IN_BYTES, 0)
            fd_out.write(und_sym_val_b)

            und_sym_shndx_b = struct.pack('<H', ENUM_ST_SHNDX['SHN_ABS'])
            fd_out.seek(symtab['sh_offset'] + und_sym_idx * symtab['sh_entsize'] + ST_SHNDX_OFFSET_IN_BYTES, 0)
            fd_out.write(und_sym_shndx_b)

        do_log_i('\n')
        fd_out.close()

    def apply_relocations(self, do_ws_start_sram_addr, rel_sections_list, output_filename):
        fd_out = open(output_filename, 'wb')
        self._dup_elf_file_contents(fd_out)

        reloc_handler = ArmRelocationHandler(self, do_ws_start_sram_addr)

        do_log_i('Fix up relocations in DO file\n\n')
        for i in xrange(0, len(rel_sections_list)):
            rel_sec = self.get_section(rel_sections_list[i])
            if not isinstance(rel_sec, RelocationSection):
                do_log_fatal("#%2d is not REL section\n" % (i))

            target_sec = self.get_section(rel_sec['sh_info'])
            expected_rel_sec_name = REL_SECTION_NAME_PREFIX + target_sec.name
            if not rel_sec.name in expected_rel_sec_name:
                do_log_fatal("REL: [%s] vs. target: [%s] - section name are not matched!\n" % (
                    rel_sec.name, target_sec.name))

            symtab = self.get_section(rel_sec['sh_link'])
            if symtab.name != '.symtab':
                do_log_fatal("#%2d is not .symtab section\n" % (rel_sec['sh_link']))

            if self.get_machine_arch() != 'ARM':
                do_log_fatal('Only suppoprt ARM relocation for now\n')

            do_log_i('\n')
            do_log_i("Applying relocation from %s to %s, sh_off = 0x%08x\n" % (
                rel_sec.name, target_sec.name, target_sec['sh_offset']))

            self.stream.seek(target_sec['sh_offset'], 0)
            target_section_stream = BytesIO()
            target_section_stream.write(self.stream.read(target_sec['sh_size']))

            for rel_entry in rel_sec.iter_relocations():
                if rel_entry.is_RELA():
                    do_log_fatal('Not suppoprt RELA relocation for now!\n')

                if rel_entry['r_info_sym'] >= symtab.num_symbols():
                    do_log_fatal('Invalid symbol reference in relocation: index %s\n' % (
                            rel_entry['r_info_sym']))

                # target_section_stream is the stream containing whole target section contents
                result_b = reloc_handler._do_apply_relocation(
                    target_section_stream, rel_entry, symtab, target_sec)

                # prepare destination position to write
                target_file_offset = target_sec['sh_offset'] + rel_entry['r_offset']
                fd_out.seek(target_file_offset, 0)
                fd_out.write(result_b)
                fd_out.flush()

        do_log_i('\n')
        fd_out.close()

    def update_do_header_section(self, do_ws_start_sram_addr, output_filename):
        fd_out = open(output_filename, 'wb')
        self._dup_elf_file_contents(fd_out)

        do_header_sec = self.get_section_by_name(DO_HEADER_SECTION_NAME)
        if not do_header_sec:
            do_log_fatal('Section \'%s\' is not found\n' % (DO_HEADER_SECTION_NAME))

        self.stream.seek(do_header_sec['sh_offset'], 0)

        # parsed into pre-defined structure
        dynamic_object_header = self.structs.dynamic_object_header_t.parse_stream(self.stream)

        # won't change the magic hex in first 16 bytes of .do_header section
        do_features_sec = self.get_section_by_name(DO_FEATURES_SECTION_NAME)
        if do_features_sec is None:
            do_log_fatal('.do_features section is not found\n')

        dynamic_object_header['do_features_start'] = do_ws_start_sram_addr + do_features_sec['sh_addr'] + DO_PSEUDO_ENTRY_LEN
        dynamic_object_header['do_features_end'] = do_ws_start_sram_addr + do_features_sec['sh_addr'] + do_features_sec['sh_size']

        exidx_start_sram_addr = self.get_exidx_start_sram_addr(do_ws_start_sram_addr)
        if exidx_start_sram_addr is None:
            exidx_start_sram_addr = 0
        dynamic_object_header['do_exidx_start'] = exidx_start_sram_addr

        exidx_end_sram_addr = self.get_exidx_end_sram_addr(do_ws_start_sram_addr)
        if exidx_end_sram_addr is None:
            exidx_end_sram_addr = 0
        dynamic_object_header['do_exidx_end'] = exidx_end_sram_addr

        do_bss_sec = self.get_section_by_name(BSS_SECTION_NAME)
        if do_bss_sec is None:
            dynamic_object_header['do_bss_start'] = 0
            dynamic_object_header['do_bss_end'] = 0
        else:
            dynamic_object_header['do_bss_start'] = do_ws_start_sram_addr + do_bss_sec['sh_addr']
            dynamic_object_header['do_bss_end'] = dynamic_object_header['do_bss_start'] + do_bss_sec['sh_size']

        do_log_i('Update .do_header section contents\n\n')
        for k in dynamic_object_header.keys():
            do_log_i("%s - 0x%08x\n" % (k, dynamic_object_header.get(k)))

        do_log_i('\n')

        # skip first 16 bytes magic hex
        fd_out.seek(do_header_sec['sh_offset'] + DO_HEADER_MAGIC_LEN, 0)

        do_header_b = struct.pack('<IIIIII',
            dynamic_object_header['do_features_start'],
            dynamic_object_header['do_features_end'],
            dynamic_object_header['do_exidx_start'],
            dynamic_object_header['do_exidx_end'],
            dynamic_object_header['do_bss_start'],
            dynamic_object_header['do_bss_end'])

        fd_out.write(do_header_b)
        fd_out.close()

    def _dup_elf_file_contents(self, fd_out):
        self.stream.seek(0, 0)
        fd_out.seek(0, 0)
        fd_out.write(self.stream.read())
        fd_out.flush()
