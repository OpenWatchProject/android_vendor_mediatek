#!/usr/bin/env python
# -*- coding: UTF-8 -*-
"""
This module is used to deploy key certificate and image key for verified boot
"""
import sys
import os
import struct
import re
import shutil
from collections import namedtuple
import sign
import lib.cert

IMG_HDR_MAGIC = 0x58881688
MKIMG_HDR_FMT = "<2I 32s 2I 8I"

TmpPath = namedtuple('TmpPath', ' \
tmp_out_path, \
out_cert1_dir, \
out_cert1md_dir, \
cert1_dir, \
cert2_key_dir, \
out_cert1_path, \
out_cert1md_path \
')

Env = namedtuple('Env', ' \
in_path, \
out, \
img_ver_path, \
')

def create_out_dir(path):
    """
    create output directory
    """
    out_dir = os.path.abspath(path)
    print "Create dir:" + out_dir

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

def create_folder(tmp_path):
    """
    create temporary folders for output
    """
    create_out_dir(tmp_path.tmp_out_path)
    create_out_dir(tmp_path.out_cert1_dir)
    create_out_dir(tmp_path.out_cert1md_dir)
    create_out_dir(tmp_path.cert1_dir)
    create_out_dir(tmp_path.cert2_key_dir)
    return

def set_path(args):
    """
    initialize paths needed in deployment process
    """

    tmp_path = TmpPath(tmp_out_path='', \
                       out_cert1_dir='', \
                       out_cert1md_dir='', \
                       cert1_dir='', \
                       cert2_key_dir='', \
                       out_cert1_path='', \
                       out_cert1md_path='' \
                       )

    env = Env(in_path=None, out=None, img_ver_path=None)

    #load environment settings from env.cfg
    cfg_file = open(args['env_cfg'], 'r')
    cfg_file_dir = os.path.dirname(os.path.abspath(args['env_cfg']))
    for line in cfg_file.readlines():
        line = line.strip()
        if line:
            elements = line.split('=')
            field = elements[0].strip()
            value = elements[1].strip()
            value = value.replace("${PLATFORM}", args['platform'])
            value = value.replace("${PROJECT}", args['project'])
            if field == 'out_path':
                out = os.environ.get('PRODUCT_OUT')
                if out is None:
                    out = os.path.join(cfg_file_dir, value)
                tmp_out_path = os.path.join(out, 'resign')
                env = env._replace(out=out)
                tmp_path = tmp_path._replace(tmp_out_path=tmp_out_path)
                # in_path is optional, hence we give it default value here
                if env.in_path is None:
                    env = env._replace(in_path=out)
            elif field == 'in_path':
                env = env._replace(in_path=os.environ.get('PRODUCT_OUT'))
                if env.in_path is None:
                    env = env._replace(in_path=os.path.join(cfg_file_dir, value))
            elif field == 'cert1_dir':
                tmp_path = tmp_path._replace(cert1_dir=os.path.join(cfg_file_dir, value))
            elif field == 'cert2_key_dir':
                tmp_path = tmp_path._replace(cert2_key_dir=os.path.join(cfg_file_dir, value))
            elif field == 'img_ver_path':
                env = env._replace(img_ver_path=os.path.join(cfg_file_dir, value))
            else:
                print "ignore " + field

    out_cert1_dir = os.path.join(tmp_out_path, "cert", "cert1")
    out_cert1_path = os.path.join(out_cert1_dir, "cert1.der")
    out_cert1md_dir = os.path.join(tmp_out_path, "cert", "${PACKAGE}", "cert1md")
    out_cert1md_path = os.path.join(out_cert1md_dir, "cert1md.der")

    #tmp path
    tmp_path = tmp_path._replace(out_cert1_dir=out_cert1_dir)
    tmp_path = tmp_path._replace(out_cert1md_dir=out_cert1md_dir)
    tmp_path = tmp_path._replace(out_cert1_path=out_cert1_path)
    tmp_path = tmp_path._replace(out_cert1md_path=out_cert1md_path)

    return env, tmp_path

def fill_arg_dict(input_str, key, args):
    """
    parse one input argument into input argument dictionary
    """
    prefix = input_str.split("=")[0]
    fmt = re.compile(key, re.I)
    if fmt.search(prefix):
        val = input_str.split("=")[1]
        args[key] = val
        print key + ": " + val
    return args

def parse_arg(argv):
    """
    parse one input arguments
    """
    args = {'cert2_key_path': 0, 'cert1_key_path': 0, 'root_key_padding': 0, 'env_cfg': 0}

    for input_str in argv:
        for key in args:
            args = fill_arg_dict(input_str, key, args)

    args['platform'] = argv[1]
    args['project'] = argv[2]

    if args['env_cfg'] == 0:
        #env_cfg is not given, we set it to env.cfg in path of this tool
        args['env_cfg'] = os.path.join(os.path.dirname(__file__), 'env.cfg')

    return args


def get_img_list(env):
    """
    get image list from image version configuration file
    """
    pattern1 = r"\["
    format1 = re.compile(pattern1)
    file1 = open(env.img_ver_path, 'r')
    img_list = []
    for line in file1:
        if not line.strip():
            continue

        if format1.match(line):
            bin_name = line[line.index("[")+1:line.index("]")]
            img_list.append(bin_name)

    file1.close()
    return img_list

def get_img_ver_group(img_name, env):
    """
    get image version group
    """
    img_ver = 0
    img_group = 0
    target_line = 0

    file1 = open(env.img_ver_path, 'r')

    pattern0 = r"\["
    format0 = re.compile(pattern0)
    pattern1 = r"\[" + img_name + r"\]"
    format1 = re.compile(pattern1)
    pattern2 = r"img_ver[\w]*"
    format2 = re.compile(pattern2)
    pattern3 = r"img_group[\w]*"
    format3 = re.compile(pattern3)
    for line in file1:
        if not line.strip():
            continue
        if format0.match(line):
            if format1.match(line):
                target_line = 1
            else:
                target_line = 0
        elif format2.match(line):
            if target_line == 1:
                img_ver = line.split("=")[1].strip()
        elif format3.match(line):
            if target_line == 1:
                img_group = line.split("=")[1].strip()
    file1.close()

    return img_ver, img_group


def check_img_type(img, env):
    """
    get image type
    """
    is_raw = 0
    is_md = 0
    img_file = os.path.join(env.in_path, img + ".img")
    if not os.path.exists(img_file):
        img_file = os.path.join(env.in_path, img + ".bin")
    if not os.path.exists(img_file):
        is_raw = 1
        is_md = 0
        return is_raw, is_md
    print img_file

    file1 = open(img_file, "rb")
    header_size = struct.calcsize(MKIMG_HDR_FMT)
    fin = file1.read(header_size)

    unpack_array = struct.unpack(MKIMG_HDR_FMT, fin)
    file1.close()

    img_type = unpack_array[8]

    if cmp(unpack_array[0], int(IMG_HDR_MAGIC)) == 0:
        img_type_byte0 = img_type & 0xFF
        img_type_byte3 = (img_type >> 24) & 0xFF

        if img_type == 0:
            print "Raw IMG"
            is_raw = 1
        elif img_type_byte3 == 1:
            if img_type_byte0 == 0:
                print "MD IMG:LTE"
                is_raw = 1
                is_md = 1
            elif img_type_byte0 == 1:
                print "MD IMG:C2K"
                is_raw = 1
                is_md = 2
    else:
        print "Not Raw Img"
        is_raw = 0
    return is_raw, is_md


def gen_cert2_key(img_list, args, tmp_path):
    """
    generate private key for image signing
    """
    print "Start gen cert2 key to " + tmp_path.cert2_key_dir

    print "clean up cert2_key_dir"
    for old_file in os.listdir(tmp_path.cert2_key_dir):
        os.remove(os.path.join(tmp_path.cert2_key_dir, old_file))

    for img in img_list:
        prvk = img + '_privk2.pem'
        pubk = img + '_pubk2.pem'
        final_prvk = os.path.join(tmp_path.cert2_key_dir, prvk)
        final_pubk = os.path.join(tmp_path.cert2_key_dir, pubk)
        if args['cert2_key_path'] == 'auto':
            print "Gen(auto) " + prvk
            lib.cert.rsa_key_gen(final_prvk, 2048)
        else:
            if lib.cert.is_prvk(args["cert2_key_path"]):
                print "Gen " + prvk
                shutil.copy2(args["cert2_key_path"], final_prvk)
            elif lib.cert.is_pubk(args["cert2_key_path"]):
                print "Gen " + pubk
                shutil.copy2(args["cert2_key_path"], final_pubk)
            else:
                print 'Unknown key type: ignored'

def img_name_to_part_name(img_name):
    """
    map image name to partition name
    """
    if re.match('md1rom', img_name):
        part_name = 'md1img'
    elif re.match('md3rom', img_name):
        part_name = 'md3img'
    else:
        part_name = img_name

    return part_name

def gen_cert1(img_list, args, env, tmp_path):
    """
    generate key certificate
    """

    is_raw = 0
    is_md = 0

    print "clean up cert1_dir"
    for old_file in os.listdir(tmp_path.cert1_dir):
        os.remove(os.path.join(tmp_path.cert1_dir, old_file))

    for img in img_list:
        need_to_clear_pubk = False
        cert2_prvk_path = os.path.join(tmp_path.cert2_key_dir, img + "_privk2.pem")
        cert2_pubk_path = os.path.join(tmp_path.cert2_key_dir, img + "_pubk2.pem")

        if not os.path.isfile(cert2_pubk_path):
            need_to_clear_pubk = True
            lib.cert.prvk_to_pubk(cert2_prvk_path, cert2_pubk_path)

        img_ver, img_group = get_img_ver_group(img, env)
        part_name = img_name_to_part_name(img)
        is_raw, is_md = check_img_type(part_name, env)
        print "img: " + str(img)
        print "img_ver: " + str(img_ver)
        print "img_group: " + str(img_group)
        print "is_raw: " + str(is_raw)
        print "is_md: " + str(is_md)
        if is_md == 0:
            if args["root_key_padding"]:
                sign_obj = sign.Sign()
                sign_obj.args['platform'] = args['platform']
                sign_obj.args['project'] = args['project']
                sign_obj.args['type'] = 'cert1'
                sign_obj.args['img'] = ''
                sign_obj.args['privk'] = args['cert1_key_path']
                sign_obj.args['pubk'] = cert2_pubk_path
                sign_obj.args['ver'] = str(img_ver)
                sign_obj.args['group'] = str(img_group)
                sign_obj.args['root_key_padding'] = str(args['root_key_padding'])
                sign_obj.sign_op()
            else:
                sign_obj = sign.Sign()
                sign_obj.args['platform'] = args['platform']
                sign_obj.args['project'] = args['project']
                sign_obj.args['type'] = 'cert1'
                sign_obj.args['img'] = ''
                sign_obj.args['privk'] = args['cert1_key_path']
                sign_obj.args['pubk'] = cert2_pubk_path
                sign_obj.args['ver'] = str(img_ver)
                sign_obj.args['group'] = str(img_group)
                sign_obj.sign_op()
            tmp_cert1_out_path = tmp_path.out_cert1_path
        else:
            md_img = os.path.join(env.in_path, part_name + ".img")
            if os.path.isfile(md_img):
                print md_img + " exist"
                if args["root_key_padding"]:
                    sign_obj = sign.Sign()
                    sign_obj.args['platform'] = args['platform']
                    sign_obj.args['project'] = args['project']
                    sign_obj.args['type'] = 'cert1md'
                    sign_obj.args['img'] = md_img
                    sign_obj.args['privk'] = args['cert1_key_path']
                    sign_obj.args['pubk'] = cert2_pubk_path
                    sign_obj.args['ver'] = str(img_ver)
                    sign_obj.args['group'] = str(img_group)
                    sign_obj.args['root_key_padding'] = str(args['root_key_padding'])
                    sign_obj.sign_op()
                else:
                    sign_obj = sign.Sign()
                    sign_obj.args['platform'] = args['platform']
                    sign_obj.args['project'] = args['project']
                    sign_obj.args['type'] = 'cert1md'
                    sign_obj.args['img'] = md_img
                    sign_obj.args['privk'] = args['cert1_key_path']
                    sign_obj.args['pubk'] = cert2_pubk_path
                    sign_obj.args['ver'] = str(img_ver)
                    sign_obj.args['group'] = str(img_group)
                    sign_obj.sign_op()
                tmp_cert1_out_path = tmp_path.out_cert1md_path
                tmp_cert1_out_path = tmp_cert1_out_path.replace("${PACKAGE}", part_name)
            else:
                print md_img + " Not exist"
                print"Bypass md image cert1 Gen!"
        print img
        cert1_path = os.path.join(tmp_path.cert1_dir, img + '_cert1.der')
        print cert1_path
        shutil.copy2(tmp_cert1_out_path, cert1_path)
        if need_to_clear_pubk is True:
            os.remove(cert2_pubk_path)
        print "--------"

def main():
    """
    main function, which is executed when this is executed from cmdline.
    """
    if len(sys.argv) < 4:
        print "img_key_deploy.py <platform> <project>\
cert1_key_path=<key_path> \
cert2_key_path=<key_path> \
root_key_padding=<padding_type>"
        print "For security2.0:"
        print "         img_key_deploy.py mt6797 k97v1_64 \
cert1_key_path=privk1.pem \
cert2_key_path=privk2.pem"
        print "For security2.1:"
        print "         img_key_deploy.py mt6799 k99v1_64 \
cert1_key_path=privk1.pem \
cert2_key_path=privk2.pem \
root_key_padding=pss"
        sys.exit()

    args = parse_arg(sys.argv)

    env, tmp_path = set_path(args)
    create_folder(tmp_path)
    img_list = get_img_list(env)

    if args["cert2_key_path"] == 0:
        print "cert2_key_path is not given!"
        sys.exit()
    else:
        if os.path.isfile(args["cert2_key_path"]):
            gen_cert2_key(img_list, args, tmp_path)
        elif args['cert2_key_path'] == 'auto':
            gen_cert2_key(img_list, args, tmp_path)
        else:
            print "cert2_key not exists, please check cert2 key path!"
            sys.exit()

    if args["cert1_key_path"] == 0:
        print "cert1_key_path is not given!"
        sys.exit()
    else:
        if os.path.isfile(args["cert1_key_path"]):
            gen_cert1(img_list, args, env, tmp_path)
        else:
            print "cert1_key not exists, please check cert1 key path!"
            sys.exit()

    print "Gen pass!"
    sys.exit()


if __name__ == '__main__':
    main()
