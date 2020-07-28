#-------------------------------------------------------------------------------
# Name:        cert_gen
# Purpose:
#
# Author:      MTK02464
#
# Created:     08/06/2015
# Copyright:   (c) MTK02464 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------
import base64
import os
import sys
import subprocess

def pem_to_der(pem_file_path, der_file_path):
    in_file = open(pem_file_path, 'r')
    in_data = ""
    while True:
        line = in_file.readline()
        if line == '':
            break
        if line[0] != '-':
            in_data = in_data + line

    out_data = base64.standard_b64decode(in_data)

    out_file = open(der_file_path, 'wb')
    out_file.write(out_data)
    out_file.close()
    return

def main():
    in_file = sys.argv[1]
    out_file = sys.argv[2]

    pem_to_der(in_file, out_file)

if __name__ == '__main__':
    main()
