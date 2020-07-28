#!/usr/bin/env python

import os, sys
import struct
from collections import namedtuple

from ..common.utils import elf_assert, struct_parse
from ..common.utils import do_log_fatal, do_log_w, do_log_i
from .elffile import ELFFile

#-------------------------------------------------------------------------------
DO_WS_START_SYMBOL_NAME = "__do_ws_start"
#-------------------------------------------------------------------------------

class TinysysScp(ELFFile):
    def __init__(self, stream):
        super(TinysysScp, self).__init__(stream)

    def get_do_ws_start_addr(self):
        symtab = self.get_section_by_name('.symtab')
        for nsym, symbol in enumerate(symtab.iter_symbols()):
            if symbol.name == DO_WS_START_SYMBOL_NAME:
                do_log_i("#%d: %s = %s\n" % (nsym, symbol.name,
                    self._format_hex(symbol['st_value'], fullhex=True, lead0x=True)))
                return symbol['st_value']

    def search_global_symbols_for(self, target_symbols_to_seach):
        """ This API will skip tinysys global UND symbols
        """
        found_symbols = {}
        symtab = self.get_section_by_name('.symtab')
        self._check_if_valid_symtab_section(symtab)

        index_first_non_local_symbol = symtab['sh_info']

        do_log_i('Seach for global symbols in tinysys_scp\n\n')
        for i in xrange(index_first_non_local_symbol, symtab.num_symbols()):
            global_symbol = symtab.get_symbol(i)
            if global_symbol['st_shndx'] == 'SHN_UNDEF':
                continue

            if global_symbol.name in target_symbols_to_seach:
                do_log_i("#%d: %s = %s\n" % (i, global_symbol.name,
                    self._format_hex(global_symbol['st_value'], fullhex=True, lead0x=True)))
                found_symbols[global_symbol.name] = global_symbol['st_value']

        do_log_i('\n')
        return found_symbols
