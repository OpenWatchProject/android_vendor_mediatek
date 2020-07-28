#!/usr/bin/python

# Copyright (c) 2016 TRUSTONIC LIMITED, All rights reserved

import os
import sys
import re
import getopt
import struct

version='1.0'
# Hardcode some known structure sizes that should not change
# or if they change, it will be because of a new config version that needs
# updates anyway
marker_size = 8
config_version_size = 4
config_static_version_size = 4
placeholder_item_size = 4
supported_config_version = 1

def help():
    """Display minimal usage information"""
    print 'Supported arguments: '
    print '-t <TA location> : The path to the unsigned TA with placeholder keys'
    print '-d : Display debug information during execution'
    print '-v : Display version'
    print '--hexkeypath=<transport key data location>: Path to file with transport key information in hex'
    return
    
def parse_options(argv):
    """Parse command-line arguments"""
    ta_path = ''
    debug = False
    keydata_defs = {}

    try:
        opts, args = getopt.getopt(argv,"vdt:k:", ['trustedapplicationpath=', 'hexkeypath='])
    except getopt.GetoptError as err:
        print 'Failure in parsing the options: ' + str(err)
        help()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            help()
            sys.exit()
        elif opt in ('-t', '--trusted-application-path'):
            if arg == '':
                print "ERROR: Please provide the trusted application path"
                sys.exit(0)
            else:
                ta_path = arg
        elif opt in ('-d', '--debug'):
            debug = True
        elif opt in ('-v', '--version'):
            print 'Keymaster transport key replacement tool version ' + version
        elif opt in ('-k','--hexkeypath'):
            keydata_defs['keydata'] = dict([('path', arg), ('type', 'h')])        

    return (ta_path, debug, keydata_defs)


def is_replacement_data_ok(length, placeholder_length, name):
    """ Check that the replacement data is OK to replace the placeholder"""

    if placeholder_length != length:
        print 'ERROR: ' + name + ' size does not match placeholder size'
        print 'Expected    : ' + str(placeholder_length)
        print 'Got instead : ' + str(length)
        return False

    # TODO: Figure out additional checks, do them
    return True


def get_asciihex_item(file_data, item_name):
    """Get the bytes for a given item in ascii hexadecimal format"""
    match_obj = re.search( r'' + item_name + '\s?\{\s?([^\}]+)?\}', file_data, re.I | re.DOTALL)
    if not match_obj:
        print "ERROR: Could not find " + item_name + " placeholder"
        sys.exit(0)

    item_data = re.sub(r'[^0-9a-fA-Fx,]\n\s{2,}', '', match_obj.group(1), re.MULTILINE | re.DOTALL | re.I)
    if debug:
        print 'The item ' + item_name + ' data as hex: '
        print item_data
    # cleanup and split the data for calculating the number of hex bytes
    item_octets = re.split(', ', item_data.strip())

    # Create actual bytes
    return re.sub(r'(0x)|([\s,])', '', item_data.strip()).decode('hex')

def is_placeholder_ekkb_pub_size_ok(size, item_name):
    if size < 128:
        print 'ERROR: key placeholder length can not be less than 128 byte!'
        return False
    if size > 256:
       print 'WARNING: ' + item_name + ' key placeholder length is more than 256 bytes (now ' + size + ')'

    return True

def is_placeholder_pkb_size_ok(size, item_name):
    if size < 128:
        print 'ERROR: ' + item_name + ' key placeholder length is less than 128 bytes! (' + size + ')'
        return False
    if size > 256:
        print 'ERROR: ' + item_name + ' key placeholder length is more than 256 bytes! (' + size + ')'
        return False
    return True


keydata_defs = {}
ta_path = ''
debug = False

ta_path, debug, keydata_defs = parse_options(sys.argv[1:])

# Ensure that we have the actual key data sources configured
if not keydata_defs.has_key('keydata') or keydata_defs['keydata'].get('path', '') == '':
    print "ERROR: No key data source specified"
    help()
    sys.exit(0)

try:
    tafile = open(ta_path, "r+b")
except IOError:
    print "Could not open the TA file at path: " + ta_path
    sys.exit(0)

ta_data = tafile.read()

# Seek to the marker in TA file
found = ta_data.find('$KEYKM$')

if found:
    print "Key configuration block found from offset " + str(found)
else:
    print "ERROR: Key configuration block not found"
    exit

# Seek the file position into the beginning of the key data
tafile.seek(found)
tafile.seek(marker_size + marker_size, 1)
config_version = struct.unpack('<L', tafile.read(placeholder_item_size))[0]
if config_version != supported_config_version:
    print 'ERROR: Unsupported config format version ' + config_version
    sys.exit(0)

tafile.seek(config_static_version_size, 1)

# Get key placeholder exponent size
key_ekkb_pub_ph_size = struct.unpack('<L', tafile.read(placeholder_item_size))[0]
if not is_placeholder_ekkb_pub_size_ok(key_ekkb_pub_ph_size, 'EKKB_PUB'):
   sys.exit(0)

# Get key placeholder modulus size
key_pkb_ph_size = struct.unpack('<L', tafile.read(placeholder_item_size))[0]
if not is_placeholder_pkb_size_ok(key_pkb_ph_size, 'PKB'):
    sys.exit(0)

print "Loading key information..."
try:
    key_file = open(keydata_defs['keydata'].get('path', ''))
    key_data = key_file.read()
except IOError:
    print "ERROR: Could not open/read Key data from path: " + keydata_defs['keydata'].get('path', '')
    sys.exit(0)

key_file.close()

# Get the actual Key exponent/modulus bytes
key_ekkb_pub = get_asciihex_item(key_data, 'EKKB_PUB')
if debug:
    print "New Key exponent will be:\n" + key_ekkb_pub.encode('hex')
key_pkb = get_asciihex_item(key_data, 'PKB')
if debug:
    print "New Key modulus will be:\n" + key_pkb.encode('hex')

# Replace Key exponent
if not is_replacement_data_ok(len(key_ekkb_pub), key_ekkb_pub_ph_size, 'Key exponent'):
    sys.exit(0)
tafile.write(key_ekkb_pub);

# Replace Key modulus
if not is_replacement_data_ok(len(key_pkb), key_pkb_ph_size, 'Key placeholder modulus'):
    sys.exit(0)
tafile.write(key_pkb)

tafile.close();
print "TA keys updated successfully!"
