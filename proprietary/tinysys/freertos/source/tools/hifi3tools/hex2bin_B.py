#!/usr/bin/env python
""" Read 8 byte hexadecimal format text file"""

import sys

import binascii

h = open(sys.argv[1]) # .txt file
b = open(sys.argv[2],'wb') # .bin file

line = h.readline()

while line:
    line = line.strip()
    my_bin = binascii.a2b_hex(line)
    b.write(my_bin)
    line = h.readline()

h.close()
b.close()
