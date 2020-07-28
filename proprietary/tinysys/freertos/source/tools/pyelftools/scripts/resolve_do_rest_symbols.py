#!/usr/bin/env python

import os, sys
from optparse import OptionParser
import string

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
            prog='resolve_do_rest_symbols.py',
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

    with open(options.in_do_file, 'rb') as do_file, open(options.in_tinysys_file, 'rb') as tinysys_file:
        try:
            elf_tinysys = TinysysScp(tinysys_file)
            elf_do = DynamicObject(do_file)

            do_bin_index_addr_map = elf_do.get_allocated_sections_mapping(
                elf_tinysys.get_do_ws_start_addr())

            if len(do_bin_index_addr_map) == 0:
                do_log_fatal('No valid sections info found in DO file\n')

            elf_do.resolve_local_symbols(do_bin_index_addr_map, options.out_file)

        except ELFError as ex:
            do_log_fatal('ELF error: %s\n' % ex)

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
