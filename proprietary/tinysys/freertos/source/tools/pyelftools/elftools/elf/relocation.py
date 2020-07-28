#-------------------------------------------------------------------------------
# elftools: elf/relocation.py
#
# ELF relocations
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import os, sys
import struct
from collections import namedtuple

from ..common.exceptions import ELFRelocationError
from ..common.utils import elf_assert, struct_parse
from ..common.utils import do_log_fatal, do_log_w, do_log_i
from .sections import Section
from .enums import ENUM_RELOC_TYPE_i386, ENUM_RELOC_TYPE_x64, ENUM_RELOC_TYPE_MIPS
from .enums import ENUM_RELOC_TYPE_ARM


class Relocation(object):
    """ Relocation object - representing a single relocation entry. Allows
        dictionary-like access to the entry's fields.

        Can be either a REL or RELA relocation.
    """
    def __init__(self, entry, elffile):
        self.entry = entry
        self.elffile = elffile

    def is_RELA(self):
        """ Is this a RELA relocation? If not, it's REL.
        """
        return 'r_addend' in self.entry

    def __getitem__(self, name):
        """ Dict-like access to entries
        """
        return self.entry[name]

    def __repr__(self):
        return '<Relocation (%s): %s>' % (
                'RELA' if self.is_RELA() else 'REL',
                self.entry)

    def __str__(self):
        return self.__repr__()


class RelocationSection(Section):
    """ ELF relocation section. Serves as a collection of Relocation entries.
    """
    def __init__(self, header, name, stream, elffile):
        super(RelocationSection, self).__init__(header, name, stream)
        self.elffile = elffile
        self.elfstructs = self.elffile.structs
        if self.header['sh_type'] == 'SHT_REL':
            expected_size = self.elfstructs.Elf_Rel.sizeof()
            self.entry_struct = self.elfstructs.Elf_Rel
        elif self.header['sh_type'] == 'SHT_RELA':
            expected_size = self.elfstructs.Elf_Rela.sizeof()
            self.entry_struct = self.elfstructs.Elf_Rela
        else:
            elf_assert(False, 'Unknown relocation type section')

        elf_assert(
            self.header['sh_entsize'] == expected_size,
            'Expected sh_entsize of SHT_REL section to be %s' % expected_size)

    def is_RELA(self):
        """ Is this a RELA relocation section? If not, it's REL.
        """
        return self.header['sh_type'] == 'SHT_RELA'

    def num_relocations(self):
        """ Number of relocations in the section
        """
        return self['sh_size'] // self['sh_entsize']

    def get_relocation(self, n):
        """ Get the relocation at index #n from the section (Relocation object)
        """
        entry_offset = self['sh_offset'] + n * self['sh_entsize']
        entry = struct_parse(
            self.entry_struct,
            self.stream,
            stream_pos=entry_offset)
        return Relocation(entry, self.elffile)

    def iter_relocations(self):
        """ Yield all the relocations in the section
        """
        for i in range(self.num_relocations()):
            yield self.get_relocation(i)


class RelocationHandler(object):
    """ Handles the logic of relocations in ELF files.
    """
    def __init__(self, elffile):
        self.elffile = elffile

    def find_relocations_for_section(self, section):
        """ Given a section, find the relocation section for it in the ELF
            file. Return a RelocationSection object, or None if none was
            found.
        """
        reloc_section_names = (
                '.rel' + section.name,
                '.rela' + section.name)
        # Find the relocation section aimed at this one. Currently assume
        # that either .rel or .rela section exists for this section, but
        # not both.
        for relsection in self.elffile.iter_sections():
            if (    isinstance(relsection, RelocationSection) and
                    relsection.name in reloc_section_names):
                return relsection
        return None

    def apply_section_relocations(self, stream, reloc_section):
        """ Apply all relocations in reloc_section (a RelocationSection object)
            to the given stream, that contains the data of the section that is
            being relocated. The stream is modified as a result.
        """
        # The symbol table associated with this relocation section
        symtab = self.elffile.get_section(reloc_section['sh_link'])
        for reloc in reloc_section.iter_relocations():
            self._do_apply_relocation(stream, reloc, symtab)

    def _do_apply_relocation(self, stream, reloc, symtab):
        # Preparations for performing the relocation: obtain the value of
        # the symbol mentioned in the relocation, as well as the relocation
        # recipe which tells us how to actually perform it.
        # All peppered with some sanity checking.
        if reloc['r_info_sym'] >= symtab.num_symbols():
            raise ELFRelocationError(
                'Invalid symbol reference in relocation: index %s' % (
                    reloc['r_info_sym']))
        sym_value = symtab.get_symbol(reloc['r_info_sym'])['st_value']

        reloc_type = reloc['r_info_type']
        recipe = None

        if self.elffile.get_machine_arch() == 'x86':
            if reloc.is_RELA():
                raise ELFRelocationError(
                    'Unexpected RELA relocation for x86: %s' % reloc)
            recipe = self._RELOCATION_RECIPES_X86.get(reloc_type, None)
        elif self.elffile.get_machine_arch() == 'x64':
            if not reloc.is_RELA():
                raise ELFRelocationError(
                    'Unexpected REL relocation for x64: %s' % reloc)
            recipe = self._RELOCATION_RECIPES_X64.get(reloc_type, None)
        elif self.elffile.get_machine_arch() == 'MIPS':
            if reloc.is_RELA():
                raise ELFRelocationError(
                    'Unexpected RELA relocation for MIPS: %s' % reloc)
            recipe = self._RELOCATION_RECIPES_MIPS.get(reloc_type, None)

        if recipe is None:
            raise ELFRelocationError(
                    'Unsupported relocation type: %s' % reloc_type)

        # So now we have everything we need to actually perform the relocation.
        # Let's get to it:

        # 0. Find out which struct we're going to be using to read this value
        #    from the stream and write it back.
        if recipe.bytesize == 4:
            value_struct = self.elffile.structs.Elf_word('')
        elif recipe.bytesize == 8:
            value_struct = self.elffile.structs.Elf_word64('')
        else:
            raise ELFRelocationError('Invalid bytesize %s for relocation' %
                    recipe_bytesize)

        # 1. Read the value from the stream (with correct size and endianness)
        original_value = struct_parse(
            value_struct,
            stream,
            stream_pos=reloc['r_offset'])
        # 2. Apply the relocation to the value, acting according to the recipe
        relocated_value = recipe.calc_func(
            value=original_value,
            sym_value=sym_value,
            offset=reloc['r_offset'],
            addend=reloc['r_addend'] if recipe.has_addend else 0)
        # 3. Write the relocated value back into the stream
        stream.seek(reloc['r_offset'])

        # Make sure the relocated value fits back by wrapping it around. This
        # looks like a problem, but it seems to be the way this is done in
        # binutils too.
        relocated_value = relocated_value % (2 ** (recipe.bytesize * 8))
        value_struct.build_stream(relocated_value, stream)

    # Relocations are represented by "recipes". Each recipe specifies:
    #  bytesize: The number of bytes to read (and write back) to the section.
    #            This is the unit of data on which relocation is performed.
    #  has_addend: Does this relocation have an extra addend?
    #  calc_func: A function that performs the relocation on an extracted
    #             value, and returns the updated value.
    #
    _RELOCATION_RECIPE_TYPE = namedtuple('_RELOCATION_RECIPE_TYPE',
        'bytesize has_addend calc_func')

    def _reloc_calc_identity(value, sym_value, offset, addend=0):
        return value

    def _reloc_calc_sym_plus_value(value, sym_value, offset, addend=0):
        return sym_value + value

    def _reloc_calc_sym_plus_value_pcrel(value, sym_value, offset, addend=0):
        return sym_value + value - offset

    def _reloc_calc_sym_plus_addend(value, sym_value, offset, addend=0):
        return sym_value + addend

    def _reloc_calc_sym_plus_addend_pcrel(value, sym_value, offset, addend=0):
        return sym_value + addend - offset

    # https://dmz-portal.mips.com/wiki/MIPS_relocation_types
    _RELOCATION_RECIPES_MIPS = {
        ENUM_RELOC_TYPE_MIPS['R_MIPS_NONE']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=False, calc_func=_reloc_calc_identity),
        ENUM_RELOC_TYPE_MIPS['R_MIPS_32']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=False,
            calc_func=_reloc_calc_sym_plus_value),
    }

    _RELOCATION_RECIPES_X86 = {
        ENUM_RELOC_TYPE_i386['R_386_NONE']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=False, calc_func=_reloc_calc_identity),
        ENUM_RELOC_TYPE_i386['R_386_32']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=False,
            calc_func=_reloc_calc_sym_plus_value),
        ENUM_RELOC_TYPE_i386['R_386_PC32']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=False,
            calc_func=_reloc_calc_sym_plus_value_pcrel),
    }

    _RELOCATION_RECIPES_X64 = {
        ENUM_RELOC_TYPE_x64['R_X86_64_NONE']: _RELOCATION_RECIPE_TYPE(
            bytesize=8, has_addend=True, calc_func=_reloc_calc_identity),
        ENUM_RELOC_TYPE_x64['R_X86_64_64']: _RELOCATION_RECIPE_TYPE(
            bytesize=8, has_addend=True, calc_func=_reloc_calc_sym_plus_addend),
        ENUM_RELOC_TYPE_x64['R_X86_64_PC32']: _RELOCATION_RECIPE_TYPE(
            bytesize=8, has_addend=True,
            calc_func=_reloc_calc_sym_plus_addend_pcrel),
        ENUM_RELOC_TYPE_x64['R_X86_64_32']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=True, calc_func=_reloc_calc_sym_plus_addend),
        ENUM_RELOC_TYPE_x64['R_X86_64_32S']: _RELOCATION_RECIPE_TYPE(
            bytesize=4, has_addend=True, calc_func=_reloc_calc_sym_plus_addend),
    }

#------------------------------------------ MTK ---------------------------------------

class ArmRelocationHandler(object):
    def __init__(self, elffile, do_ws_start_sram_addr):
        self.elffile = elffile
        self.do_ws_start_sram_addr = do_ws_start_sram_addr
        """
        According to relocation type, we have following categories:
        (1) ARM32: read 4-bytes, and write 4-bytes
        (2) Thumb32: read 2 2-bytes, first for "upper", second for "lower"; write 2 2-bytes
        (3) Thumb16: read 2-bytes, write 2-bytes
        """
        self.handler_arm32 = Arm32RelocationHandler(elffile, do_ws_start_sram_addr)
        self.handler_thumb32 = Thumb32RelocationHandler(elffile, do_ws_start_sram_addr)
        self.handler_thumb16 = Thumb16RelocationHandler(elffile, do_ws_start_sram_addr)

        self._RELOCATION_HANDLER_TYPE_ARM = {
            ENUM_RELOC_TYPE_ARM['R_ARM_NONE']: self.handler_arm32,
            ENUM_RELOC_TYPE_ARM['R_ARM_ABS32']: self.handler_arm32,
            ENUM_RELOC_TYPE_ARM['R_ARM_TARGET1']: self.handler_arm32,
            ENUM_RELOC_TYPE_ARM['R_ARM_PREL31']: self.handler_arm32,
            ENUM_RELOC_TYPE_ARM['R_ARM_THM_CALL']: self.handler_thumb32,
            ENUM_RELOC_TYPE_ARM['R_ARM_THM_JUMP24']: self.handler_thumb32,
            ENUM_RELOC_TYPE_ARM['R_ARM_THM_JUMP11']: self.handler_thumb16,
        }

    def _reloc_calc_identity(self, value, sym_value, offset, addend=0):
        return value

    def _reloc_calc_sym_plus_value(self, value, sym_value, offset, addend=0):
        return sym_value + value

    def _reloc_calc_sym_plus_value_pcrel(self, value, sym_value, offset, addend=0):
        return sym_value + value - offset

    def getRelEntrySymbolName(self, reloc, symtab):
        symbol = symtab.get_symbol(reloc['r_info_sym'])
        if symbol['st_name'] == 0:
            symsec = self.elffile.get_section(symbol['st_shndx'])
            symbol_name = '[SEC] ' + symsec.name
        else:
            symbol_name = symbol.name
        return symbol_name


    def _do_apply_relocation(self, stream, reloc, symtab, target_sec):
        reloc_type = reloc['r_info_type']
        handler = self._RELOCATION_HANDLER_TYPE_ARM.get(reloc_type, None)
        if handler != None:
            return handler._do_apply_relocation(stream, reloc, symtab, target_sec)
        else:
            raise ELFRelocationError(
                'This REL relocation type (%d) is not support!' % (reloc_type))
            sys.exit(1)

#-------------------------------------------------------------------------------
class Arm32RelocationHandler(ArmRelocationHandler):
    def __init__(self, elffile, do_ws_start_sram_addr):
        self.elffile = elffile
        self.do_ws_start_sram_addr = do_ws_start_sram_addr

    def read_arm32(self, stream, reloc):
        value_struct = self.elffile.structs.Elf_word('')
        return struct_parse(value_struct, stream, stream_pos=reloc['r_offset'])

    def write_arm32(self, relocated_value):
        return struct.pack('<I', relocated_value)

    # for R_ARM_PREL31
    # offset should be the 'final SRAM address'
    def _reloc_calc_sym_plus_value_pcrel31(self, value, sym_value, offset, addend=0):
        return (value + (sym_value - offset)) & 0x7fffffff

    def _do_apply_relocation(self, stream, reloc, symtab, target_sec):
        original_value_arm32 = self.read_arm32(stream, reloc)
        sym_value = symtab.get_symbol(reloc['r_info_sym'])['st_value']
        rel_target_sram_addr = self.do_ws_start_sram_addr + target_sec['sh_addr'] + reloc['r_offset']

        reloc_type = reloc['r_info_type']
        if reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_NONE']:
            relocated_value_arm32 = self._reloc_calc_identity(
                original_value_arm32, sym_value, rel_target_sram_addr, 0)
        elif reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_ABS32'] or reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_TARGET1']:
            relocated_value_arm32 = self._reloc_calc_sym_plus_value(
                original_value_arm32, sym_value, rel_target_sram_addr, 0)
        elif reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_PREL31']:
            relocated_value_arm32 = self._reloc_calc_sym_plus_value_pcrel31(
                original_value_arm32, sym_value, rel_target_sram_addr, 0)
        else:
            raise ELFRelocationError(
                'This REL relocation type (%d) is not support!' % (reloc_type))
            sys.exit(1)

        # Is this necessary?
        inst_bytesize = 4
        relocated_value_arm32 = relocated_value_arm32 % (2 ** (inst_bytesize * 8))

        target_file_offset = target_sec['sh_offset'] + reloc['r_offset']
        do_log_i("[0x%08x]: 0x%08x -> 0x%08x (%s)\n" % (
            target_file_offset,
            original_value_arm32, relocated_value_arm32,
            self.getRelEntrySymbolName(reloc, symtab)))

        return self.write_arm32(relocated_value_arm32)

#-------------------------------------------------------------------------------
class Thumb32RelocationHandler(ArmRelocationHandler):
    def __init__(self, elffile, do_ws_start_sram_addr):
        self.elffile = elffile
        self.do_ws_start_sram_addr = do_ws_start_sram_addr

    def read_thumb32(self, stream, reloc):
        value_struct = self.elffile.structs.Elf_half('')
        # read first 16 bits
        original_value_upper16 = struct_parse(value_struct, stream, stream_pos=reloc['r_offset'])
        # read second 16 bits
        original_value_lower16 = struct_parse(value_struct, stream, stream_pos=(reloc['r_offset']+2))
        return {'ori_upper16': original_value_upper16, 'ori_lower16': original_value_lower16}

    def write_thumb32(self, relocated_value_upper16, relocated_value_lower16):
        return struct.pack('<HH', relocated_value_upper16, relocated_value_lower16)

    def _decode_R_ARM_THM_CALL(self, upper, lower):
        sign = (upper >> 10) & 0x1
        j1 = (lower >> 13) & 0x1
        j2 = (lower >> 11) & 0x1
        original_offset_value = (sign << 24) | ((~(j1 ^ sign) & 0x1) << 23) | ((~(j2 ^ sign) & 0x1) << 22) | ((upper & 0x03ff) << 12) | ((lower & 0x07ff) << 1)
        if original_offset_value & 0x01000000 != 0:
            original_offset_value -= 0x02000000

        do_log_i("[R_ARM_THM_CALL] original_offset_value :%d\n" % (original_offset_value))
        return original_offset_value


    def _encode_R_ARM_THM_CALL(self, upper, lower, calculated_offset_value):
        do_log_i("[R_ARM_THM_CALL] calculated_offset_value :%d\n" % (calculated_offset_value))
        sign = (calculated_offset_value >> 24) & 0x1
        j1 = sign ^ (~(calculated_offset_value >> 23) & 0x1)
        j2 = sign ^ (~(calculated_offset_value >> 22) & 0x1)
        upper16 = (upper & 0xf800) | (sign << 10) | ((calculated_offset_value >> 12) & 0x03ff)
        lower16 = (lower & 0xd000) | (j1 << 13) | (j2 << 11) | ((calculated_offset_value >> 1) & 0x07ff)
        return {'relocated_upper16': upper16, 'relocated_lower16': lower16}


    def _do_apply_relocation(self, stream, reloc, symtab, target_sec):
        original_value = self.read_thumb32(stream, reloc)
        ori_upper16 = original_value['ori_upper16']
        ori_lower16 = original_value['ori_lower16']

        sym_value = symtab.get_symbol(reloc['r_info_sym'])['st_value']
        rel_target_sram_addr = self.do_ws_start_sram_addr + target_sec['sh_addr'] + reloc['r_offset']

        reloc_type = reloc['r_info_type']
        if reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_THM_CALL'] or reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_THM_JUMP24']:
            # TODO: additional check for unsupported internetworking call?

            # decode the original offset value
            original_offset_value = self._decode_R_ARM_THM_CALL(ori_upper16, ori_lower16)

            # do the relocation math
            calculated_offset_value = self._reloc_calc_sym_plus_value_pcrel(
                original_offset_value, sym_value, rel_target_sram_addr, 0)

            # encode back to instruction format
            relocated_value = self._encode_R_ARM_THM_CALL(ori_upper16, ori_lower16, calculated_offset_value)
            relocated_upper16 = relocated_value['relocated_upper16']
            relocated_lower16 = relocated_value['relocated_lower16']
        else:
            raise ELFRelocationError(
                'This REL relocation type (%d) is not support!' % (reloc_type))
            sys.exit(1)

        target_file_offset = target_sec['sh_offset'] + reloc['r_offset']
        do_log_i("[0x%08x]: 0x%04x 0x%04x -> 0x%04x 0x%04x (%s)\n" % (
            target_file_offset, ori_upper16, ori_lower16,
            relocated_upper16, relocated_lower16,
            self.getRelEntrySymbolName(reloc, symtab)))

        # return the final result
        return self.write_thumb32(relocated_upper16, relocated_lower16)


#-------------------------------------------------------------------------------
class Thumb16RelocationHandler(ArmRelocationHandler):
    def __init__(self, elffile, do_ws_start_sram_addr):
        self.elffile = elffile
        self.do_ws_start_sram_addr = do_ws_start_sram_addr

    def read_thumb16(self, stream, reloc):
        value_struct = self.elffile.structs.Elf_half('')
        return struct_parse(value_struct, stream, stream_pos=reloc['r_offset'])

    def write_thumb16(self, relocated_value):
        return struct.pack('<H', relocated_value)

    def decode_R_ARM_THM_JUMP11(self, ori_val):
        original_offset_value = (ori_val & 0x07ff) << 1
        do_log_i("[R_ARM_THM_JUMP11] original_offset_value :%d\n" % (original_offset_value))
        return original_offset_value

    def _encode_R_ARM_THM_JUMP11(self, ori_val, calculated_offset_value):
        do_log_i("[R_ARM_THM_JUMP11] calculated_offset_value :%d\n" % (calculated_offset_value))
        return (ori_val & 0xf800) | ((calculated_offset_value >> 1) & 0x07ff)

    def _do_apply_relocation(self, stream, reloc, symtab, target_sec):
        original_value_thumb16 = self.read_thumb16(stream, reloc)
        sym_value = symtab.get_symbol(reloc['r_info_sym'])['st_value']
        rel_target_sram_addr = self.do_ws_start_sram_addr + target_sec['sh_addr'] + reloc['r_offset']

        reloc_type = reloc['r_info_type']
        if reloc_type == ENUM_RELOC_TYPE_ARM['R_ARM_THM_JUMP11']:
            # decode the original offset value
            original_offset_value = self.decode_R_ARM_THM_JUMP11(original_value_thumb16)

            # do the relocation math
            calculated_offset_value = self._reloc_calc_sym_plus_value_pcrel(
                original_offset_value, sym_value, rel_target_sram_addr, 0)

            # encode back to instruction format
            relocated_value_thumb16 = self._encode_R_ARM_THM_JUMP11(
                original_value_thumb16, calculated_offset_value)
        else:
            raise ELFRelocationError(
                'This REL relocation type (%d) is not support!' % (reloc_type))
            sys.exit(1)

        # Is this necessary?
        inst_bytesize = 2
        relocated_value_thumb16 = relocated_value_thumb16 % (2 ** (inst_bytesize * 8))

        target_file_offset = target_sec['sh_offset'] + reloc['r_offset']
        do_log_i("[0x%08x]: 0x%08x -> 0x%08x (%s)\n" % (
            target_file_offset,
            original_value_thumb16, relocated_value_thumb16,
            self.getRelEntrySymbolName(reloc, symtab)))

        return self.write_thumb16(relocated_value_thumb16)
