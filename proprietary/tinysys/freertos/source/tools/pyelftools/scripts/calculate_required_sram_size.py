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

#-------------------------------------------------------------------------------
VERSION_STRING = '%%prog: based on pyelftools %s' % __version__
#-------------------------------------------------------------------------------

def main(stream=None):
    optparser = OptionParser(
            usage='usage: %prog [options]',
            add_help_option=False,
            prog='calculate_required_sram_size.py',
            version=VERSION_STRING)

    optparser.add_option('-d', '--do-file', dest='in_do_file')

    options, args = optparser.parse_args()

    if not options.in_do_file:
        do_log_fatal('No DO file as input\n')

    with open(options.in_do_file, 'rb') as do_file:
        try:
            elf_do = DynamicObject(do_file)
            print '%d' % elf_do.calculate_required_sram_size()

        except ELFError as ex:
            do_log_fatal('ELF error: %s\n' % ex)

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
