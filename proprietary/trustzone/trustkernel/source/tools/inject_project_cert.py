#!/usr/bin/env python
#
# Copyright (c) 2016, Trustkernel co.Ltd
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

import sys

CERT_SIZE_LIMIT = 1024

def get_args():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument('--cert', required=True, help='Project certificate')
    parser.add_argument('--in', required=True, dest='inf', \
            help='Name of raw tee binary')
    parser.add_argument('--out', required=True, help='Name of injected tee binary')
    return parser.parse_args()

def main():
    args = get_args()

    f = open(args.cert, 'rb')
    if f is None:
        print "Failed to open certificate: " + args.cert
        return -1
    cert = f.read()
    f.close()

    if len(cert) != CERT_SIZE_LIMIT:
        print "Invalid cert. bad cert size: %d" % (len(cert))
        return -1

    f = open(args.inf, 'rb')
    if f is None:
        print "Failed to open input tee binary: " + args.inf
        return -1
    img = f.read()
    f.close()

    pos = img.find("FAKE_CERT", 0)
    if pos == -1:
        print "Cannot spot certificate injection point"
        return -1

    pos2 = img.find("FAKE_CERT", pos + 1)
    if pos2 != -1:
        print "More than one occurence of certificate injection point 0x%x" % (pos2)
        return -1

    if pos + CERT_SIZE_LIMIT >= len(img):
        print "Invalid injection position 0x%x" % (pos)
        return -1

    print "Ceritificate injected at 0x%x" % (pos)

    img = img[:pos] + cert + img[pos + CERT_SIZE_LIMIT:]

    f = open(args.out, 'wb')
    if f is None:
        print "Failed to open output tee binary: " + args.out
        return -1
    f.write(img)
    f.close()

if __name__ == "__main__":
    sys.exit(main())
