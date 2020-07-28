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

def der_to_pem(der_file_path, pem_file_path):
    in_file = open(der_file_path, 'rb')
    in_data = ""
    while True:
        byte = in_file.read(1)
        if byte == "":
            break;
        in_data = in_data + byte

    out_data = base64.standard_b64encode(in_data)

    out_file = open(pem_file_path, 'w')
    out_file.write(out_data)
    out_file.close()
    return

def main():
    in_file = sys.argv[1]
    out_file = sys.argv[2]

    der_to_pem(in_file, out_file)

if __name__ == '__main__':
    main()
