#!/usr/bin/env python

import os, sys
from optparse import OptionParser
import string
import shutil

# For running from development directory. It should take precedence over the
# installed pyelftools.
sys.path.insert(0, '.')

from elftools import __version__
from elftools.common.exceptions import ELFError
from elftools.common.utils import do_log_fatal, do_log_w, do_log_i
from elftools.elf.dynamic_object import DynamicObject
from elftools.elf.tinysys_scp import TinysysScp

#-------------------------------------------------------------------------------
VERSION_STRING = '%%prog: based on pyelftools %s' % __version__
#-------------------------------------------------------------------------------

def main(stream=None):
    optparser = OptionParser(
            usage='usage: %prog [options]',
            add_help_option=False,
            prog='resolve_tinysys_scp_symbols.py',
            version=VERSION_STRING)

    optparser.add_option('-d', '--do-file', dest='in_do_file')
    optparser.add_option('-t', '--tinysys-file', dest='in_tinysys_file')
    optparser.add_option('-o', '--output-file', dest='out_file')

    options, args = optparser.parse_args()

    if not options.in_do_file:
        do_log_fatal('No DO file as input\n')

    if not options.in_tinysys_file:
        do_log_fatal('No tinysys_scp file as input\n')

    if not options.out_file:
        do_log_fatal('No output file is specified\n')

    do_file_und_symbols_map = None
    do_file_weak_und_symbols_map = None
    tinysys_resolved_symbols_map = None

    with open(options.in_do_file, 'rb') as do_file, open(options.in_tinysys_file, 'rb') as tinysys_file:
        try:
            elf_do = DynamicObject(do_file)
            do_file_und_symbols_map = elf_do.get_und_symbols_map()
            if (do_file_und_symbols_map is None or
                len(do_file_und_symbols_map) == 0):
                # It's possible that there is no UND symbol in DO files.
                # just dup the file and exit with 'success'
                shutil.copy(options.in_do_file, options.out_file)
                do_log_w('No undefined symbols found in DO file\n')
                sys.exit(0)

            # According to SPEC, undefined weak symbols won't be resolved!
            # And they have a zero value.
            do_file_weak_und_symbols_map = elf_do.get_weak_und_symbols_map()

            # Filter out weak undefined symbols
            for weak_und in do_file_weak_und_symbols_map.keys():
                do_file_und_symbols_map.pop(weak_und, None)

            elf_tinysys = TinysysScp(tinysys_file)
            tinysys_resolved_symbols_map = elf_tinysys.search_global_symbols_for(
                do_file_und_symbols_map.keys())

            # TODO: is this checking necessary?
            # If no UND symbols in DO file: already bail out
            # So there must be some symbols waiting to be resolved.
            if (tinysys_resolved_symbols_map is None or
                len(tinysys_resolved_symbols_map) == 0):
                do_log_fatal('No symbols found in tinysys_scp file\n')

            if len(do_file_und_symbols_map) != len(tinysys_resolved_symbols_map):
                for und_sym in do_file_und_symbols_map.keys():
                    if und_sym not in tinysys_resolved_symbols_map:
                        do_log_w('%s is not found in tinysys_scp\n' % (und_sym))

                do_log_fatal('Number of resolved symbols are not matched\n')

            do_log_i('Rewrite DO __exidx_* symbols\n')
            resolved_symbols_map = elf_do.rewrite_exidx_symbols(
                elf_tinysys.get_do_ws_start_addr(), tinysys_resolved_symbols_map)

            elf_do.update_und_symbols(
                do_file_und_symbols_map, resolved_symbols_map, options.out_file)

        except ELFError as ex:
            do_log_fatal('ELF error: %s\n' % ex)

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
