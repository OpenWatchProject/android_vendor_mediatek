#!/usr/bin/env python
# -*- coding: UTF-8 -*-
"""
This module supports image signing, including single bin and multi bin.
Multi bin means a composite image composed of multiple sub-images.
"""
import sys
import os
import struct
import re
import subprocess
import ntpath
import shutil
import stat
import lib.mkimghdr
import lib.getPublicKey
import lib.cert
import cert_gen

CERT1_REPLACE_TARGET = r"(\s)*pubk2"
CERT2_REPLACE_HASH = r"(\s)*imgHash EXTERNAL_BITSTRING"
CERT2_REPLACE_HEADER_HASH = r"(\s)*imgHdrHash EXTERNAL_BITSTRING"
CERT2_REPLACE_IMG_HASH_MULTI = r"(\s)*imgHash_Multi EXTERNAL_BITSTRING"
CERT2_REPLACE_SOCID = r"(\s)*socid PRINTABLESTRING"
CERT1MD_REPLACE_TARGET = r"(\s)*pubkHash EXTERNAL_BITSTRING"
SW_ID_REPLACE_TARGET = r"(\s)*swID INTEGER"
IMG_VER_REPLACE_TARGET = r"(\s)*imgVer INTEGER"
IMG_GROUP_REPLACE_TARGET = r"(\s)*imgGroup INTEGER"
MKIMAGE_HDR_MAGIC = 0x58881688
MKIMAGE_HDR_FMT = "<2I 32s 2I 8I"
BOOTIMG_HDR_MAGIC = "ANDROID"
BOOTIMG_HDR_FMT = "<8c 10I 16c 512c 8I 1024c I Q I I Q"
CCCI_HDR_FMT = "<8c 10I 16c 512c 8I"
DTBO_HDR_MAGIC = 0xD7B7AB1E
DTBO_HDR_FMT = ">8I"

class Sign(object):
    """
    Sign is used to pass parameter to module 'sign'.
    """
    def __init__(self):
        """
        initialization of arguments
        """
        self.args = {}
        self.context = {}

        self.args['type'] = 0
        self.args['img'] = 0
        self.args['privk'] = 0
        self.args['pubk'] = 0
        self.args['cert1'] = 0
        self.args['swID'] = 0
        self.args['ver'] = 0
        self.args['name'] = ''
        self.args['group'] = 0
        self.args['root_key_padding'] = 0
        self.args['getHashList'] = 0
        self.args['platform'] = 'NOT_SET'
        self.args['project'] = 'NOT_SET'
        self.args['env_cfg'] = os.path.join(os.path.dirname(__file__), \
                                            'env.cfg')
        self.args['socid'] = '0'

        return

    def reset(self):
        """
        reset all arguments
        """
        self.__init__()

    def get_arg(self, field_name):
        """
        get value of field and returns 'NOT_SET' if it's not defined
        """
        try:
            return self.args[field_name]
        except KeyError, error:
            print 'KeyError: ' + str(error)
            return 'NOT_SET'

    def set_context(self):
        """
        Set temporary path for certificate generation.
        """
        img_name = self.args['name']
        package_name = os.path.splitext(os.path.basename(self.args['img']))[0]

        print '=============================='
        print 'env_cfg parsing'
        print '=============================='
        cfg_file = open(self.args['env_cfg'], 'r')
        cfg_file_dir = os.path.dirname(os.path.abspath(self.args['env_cfg']))
        for line in cfg_file.readlines():
            line = line.strip()
            if line:
                elements = line.split('=')
                field = elements[0].strip()
                value = elements[1].strip()
                value = value.replace("${PLATFORM}", self.get_arg('platform'))
                value = value.replace("${PROJECT}", self.get_arg('project'))
                if field == 'out_path':
                    self.context['out'] = os.environ.get('PRODUCT_OUT')
                    if self.context['out'] is None:
                        print 'Use out path in env.cfg'
                        self.context['out'] = os.path.join(cfg_file_dir, value)
                    else:
                        print 'Use out path in Android'
                    self.context['out_path'] = os.path.join(self.context['out'], "resign")
                elif field == 'x509_template_path':
                    x509_template_path = os.path.join(cfg_file_dir, value)
                elif field == 'mkimage_tool_path':
                    mkimage_tool_path = os.path.join(cfg_file_dir, value)
                else:
                    print 'ignored: ' + field
        print '=============================='

        self.context['bin_path'] = os.path.join(self.context['out_path'], "bin")
        self.context['tmpcert_name'] = os.path.join(self.context['out_path'], \
                                                    "cert", \
                                                    package_name, \
                                                    img_name, \
                                                    "tmp.der")

        #tool path
        mkimage_config = os.path.join(mkimage_tool_path, "img_hdr.cfg")
        cert_out = os.path.join(self.context['out_path'], "cert", package_name, img_name)

        self.context['mkimage_config'] = mkimage_config

        mkimage_config_out = ""
        if self.args['type'] == "cert1md":
            cert1md_tmp = os.path.join(cert_out, "cert1md", "intermediate")
            cert1md_name = os.path.join(cert_out, "cert1md", "cert1md.der")
            cert1md_hash_path = os.path.join(cert1md_tmp, "hash")
            cert1md_config = os.path.join(x509_template_path, "cert1md.cfg")
            cert1md_config_out = os.path.join(cert1md_tmp, "cert1md.cfg")
            mkimage_config_out = os.path.join(cert1md_tmp, "img_hdr.cfg")
            self.context['cert1md_name'] = cert1md_name
            self.context['cert1md_config'] = cert1md_config
            self.context['cert1md_config_out'] = cert1md_config_out
            self.context['cert1md_hash_path'] = cert1md_hash_path
        elif self.args['type'] == "cert1":
            cert1_tmp = os.path.join(cert_out, "cert1", "intermediate")
            cert1_name = os.path.join(cert_out, "cert1", "cert1.der")
            cert1_config = os.path.join(x509_template_path, "cert1.cfg")
            cert1_config_out = os.path.join(cert1_tmp, "cert1.cfg")
            mkimage_config_out = os.path.join(cert1_tmp, "img_hdr.cfg")
            self.context['cert1_name'] = cert1_name
            self.context['cert1_config'] = cert1_config
            self.context['cert1_config_out'] = cert1_config_out
        elif self.args['type'] == "cert2":
            cert2_tmp = os.path.join(cert_out, "cert2", "intermediate")
            cert2_name = os.path.join(cert_out, "cert2", "cert2.der")
            cert2_hash_path = os.path.join(cert2_tmp, "hash")
            cert2_config = os.path.join(x509_template_path, "cert2.cfg")
            cert2_config_out = os.path.join(cert2_tmp, "cert2.cfg")
            dm_cert = os.path.join(cert2_tmp, "dm_cert.der")
            bin_tmp_path = os.path.join(cert2_tmp, "tmp_bin")
            sig_path = os.path.join(self.context['out'], "sig", package_name)
            mkimage_config_out = os.path.join(cert2_tmp, "img_hdr.cfg")
            self.context['bin_tmp_path'] = bin_tmp_path
            self.context['cert2_name'] = cert2_name
            self.context['cert2_config'] = cert2_config
            self.context['cert2_config_out'] = cert2_config_out
            self.context['cert2_hash_path'] = cert2_hash_path
            self.context['dm_cert'] = dm_cert
            self.context['sig_path'] = sig_path

        self.context['mkimage_config_out'] = mkimage_config_out
        return

    def __create_out_dir(self, path):
        """
        create output folder
        """
        dir_path = os.path.dirname(path)
        print "Create dir:" + dir_path

        try:
            os.makedirs(dir_path)
        except OSError, error:
            if error.errno != os.errno.EEXIST:
                raise

    def __create_abs_dir(self, path):
        """
        create output folder based on absolute path
        """
        dir_path = os.path.abspath(path)
        print "Create dir:" + dir_path

        try:
            os.makedirs(dir_path)
        except OSError, error:
            if error.errno != os.errno.EEXIST:
                raise

    def __create_folder(self):
        """
        Create output folder for certificate generation.
        """
        mkimage_config_out = self.context['mkimage_config_out']
        bin_path = self.context['bin_path']

        self.__create_out_dir(mkimage_config_out)
        self.__create_abs_dir(bin_path)

        if self.args['type'] == "cert1md":
            cert1md_name = self.context['cert1md_name']
            cert1md_hash_path = self.context['cert1md_hash_path']
            self.__create_out_dir(cert1md_name)
            self.__create_abs_dir(cert1md_hash_path)
        elif self.args['type'] == "cert1":
            cert1_name = self.context['cert1_name']
            cert1_config_out = self.context['cert1_config_out']
            self.__create_out_dir(cert1_name)
            self.__create_out_dir(cert1_config_out)
        elif self.args['type'] == "cert2":
            cert2_name = self.context['cert2_name']
            cert2_hash_path = self.context['cert2_hash_path']
            sig_path = self.context['sig_path']
            bin_tmp_path = self.context['bin_tmp_path']
            self.__create_out_dir(cert2_name)
            self.__create_abs_dir(cert2_hash_path)
            self.__create_abs_dir(sig_path)
            self.__create_abs_dir(bin_tmp_path)
        return

    def gen_cert(self, cert_config, privk_key, cert_der, root_key_padding):
        """
        Generate certificate based on the configuration given.
        """
        cert_gen_args = cert_gen.CertGenArgs()
        cert_gen_args.config_file = cert_config
        cert_gen_args.prvk_file_path = privk_key
        #root key padding will be set to 'legacy' by default
        if root_key_padding != 0:
            cert_gen_args.root_key_padding = root_key_padding
        cert_gen_args.x509cert_file = cert_der
        cert_gen.cert_gen_op(cert_gen_args)

    def gen_cert1(self):
        """
        Generate cert1, which is key certificate for images
        other than modem image.
        """
        cert1_config = self.context['cert1_config']
        cert1_config_out = self.context['cert1_config_out']
        #copy cert1 config
        shutil.copy2(cert1_config, cert1_config_out)
        sw_id = self.args['swID']
        img_ver = self.args['ver']
        img_group = self.args['group']

        cert1_name = self.context['cert1_name']

        if self.args['root_key_padding'] == 0:
            root_key_padding = 'legacy'
        else:
            root_key_padding = self.args['root_key_padding']
        print "root_key_padding = " + root_key_padding

        if int(sw_id) != 0:
            fill_cert_config(cert1_config_out, sw_id, SW_ID_REPLACE_TARGET)
        if int(img_ver) != 0:
            fill_cert_config(cert1_config_out, img_ver, IMG_VER_REPLACE_TARGET)
        if int(img_group) != 0:
            fill_cert_config(cert1_config_out, img_group, IMG_GROUP_REPLACE_TARGET)

        fill_cert_config(cert1_config_out, self.args['pubk'], CERT1_REPLACE_TARGET)
        #gen cert
        self.gen_cert(cert1_config_out, \
                      self.args['privk'], \
                      cert1_name, \
                      root_key_padding)
        cert1_img_type = 0x2 << 24
        add_mkimg_header(cert1_name, 0, cert1_img_type, "cert1", self.context)
        print "output path:" + cert1_name
        return

    def gen_cert1md(self):
        """
        Generate cert1md, which is key certificate for modem image.
        """
        img_file = self.args['img']
        cert_privk = self.args['privk']
        cert1md_hash_path = self.context['cert1md_hash_path']
        cert1md_config = self.context['cert1md_config']
        cert1md_config_out = self.context['cert1md_config_out']

        if self.args['root_key_padding'] == 0:
            root_key_padding = 'legacy'
        else:
            root_key_padding = self.args['root_key_padding']
        print "root_key_padding = " + root_key_padding

        print img_file
        #check is Raw
        is_raw, is_md, hdr_size = check_is_raw(img_file)

        if is_raw == 0:
            print "Is not raw image"
            return

        if is_md < 1:
            print "Wrong MD image type"
            return

        cert1md_name = self.context['cert1md_name']

        #SV5 Image(MD)
        split_header, split_image = img_split(img_file, cert1md_hash_path, hdr_size)
        key_hash_tmp = os.path.join(cert1md_hash_path, "key_tmp.hash")
        key_hash = os.path.join(cert1md_hash_path, "key.hash")
        header_hash = os.path.join(cert1md_hash_path, "header.hash")

        #get md key
        md_key = get_md_key(split_image, is_md, self.context)
        #gen header hash
        lib.cert.hash_gen(split_header, header_hash)
        #gen key hash
        lib.cert.hash_gen(md_key, key_hash_tmp)
        #Endiness conversion
        endiness_convert(key_hash_tmp, key_hash)

        #fill config
        shutil.copy2(cert1md_config, cert1md_config_out)
        fill_cert_config(cert1md_config_out, key_hash, CERT1MD_REPLACE_TARGET)
        fill_cert_config(cert1md_config_out, self.args['pubk'], CERT1_REPLACE_TARGET)

        sw_id = self.args['swID']
        img_ver = self.args['ver']
        img_group = self.args['group']
        if int(sw_id) != 0:
            fill_cert_config(cert1md_config_out, sw_id, SW_ID_REPLACE_TARGET)
        if int(img_ver) != 0:
            fill_cert_config(cert1md_config_out, img_ver, IMG_VER_REPLACE_TARGET)
        if int(img_group) != 0:
            fill_cert_config(cert1md_config_out, img_group, IMG_GROUP_REPLACE_TARGET)
        #gen cert
        self.gen_cert(cert1md_config_out, cert_privk, cert1md_name, root_key_padding)
        #add mkimage header on cert1md

        cert1md_img_type = 0x2 << 24 | 0x1
        add_mkimg_header(cert1md_name, 0, cert1md_img_type, "cert1md", self.context)
        print "output path:" + cert1md_name

    def gen_cert2(self):
        """
        Generate cert2, which is content certificate.
        """
        img_file = self.args['img']
        cert1 = self.args['cert1']
        cert_privk = self.args['privk']
        img_name = self.args['name']
        img_ver = self.args['ver']
        socid = self.args['socid']
        is_img_hash_list = 0
        is_raw = 0
        is_boot = 0
        is_dtbo = 0

        if self.get_arg('getHashList') is '1':
            is_img_hash_list = 1

        img_array = []
        size_array = []
        offset_array = []
        img_name_array = []

        bin_path = self.context['bin_path']
        bin_tmp_path = self.context['bin_tmp_path']
        cert2_name = self.context['cert2_name']
        cert2_config = self.context['cert2_config']
        cert2_config_out = self.context['cert2_config_out']
        cert2_hash_path = self.context['cert2_hash_path']
        dm_cert = self.context['dm_cert']
        sig_path = self.context['sig_path']
        out = self.context['out']

        is_raw, is_md, hdr_size = check_is_raw(img_file)
        if not is_raw:
            is_boot, boot_img_size, hdr_size = check_is_boot(img_file)

        if is_boot:
            has_dm_cert = get_vboot10_cert(img_file, boot_img_size, self.context)

        if (not is_raw) and (not is_boot):
            is_dtbo, dtbo_img_size, hdr_size = check_is_dtbo(img_file)

        if is_raw:
            img_array, \
            size_array, \
            offset_array, \
            img_name_array = parse_multi_bin(img_file, img_name, self.context)
            if img_array is None:
                return
        elif is_boot:
            img_array.append(os.path.join(bin_tmp_path, "tmp.bin"))
            size_array.append(boot_img_size)
            offset_array.append(0)
            img_name_array.append(img_name)
        elif is_dtbo:
            img_array.append(os.path.join(bin_tmp_path, "tmp.bin"))
            size_array.append(dtbo_img_size)
            offset_array.append(0)
            img_name_array.append(img_name)
        elif not is_img_hash_list:
            print "wrong image format"
            return

        if is_img_hash_list:
            print "get pure image hash..."
            self.gen_img_hash_list(img_file, cert1, cert_privk, img_name, img_ver)
            return

        get_pure_img(img_file, img_array, size_array, offset_array)
        print "bin_number:" + str(len(img_array))
        #Get hash from image
        i = 0
        img_list_end = 0
        for img in img_array:
            if i == len(img_array) - 1:
                img_list_end = 1

            if is_raw == 1 or is_boot == 1 or is_md == 1 or is_md == 2 or is_dtbo:
                #Raw Image
                if img_name_array[i] == img_name:
                    split_header, split_image = img_split(img, cert2_hash_path, hdr_size)

                    image_hash = os.path.join(cert2_hash_path, "image.hash")
                    header_hash = os.path.join(cert2_hash_path, "header.hash")

                    #gen header hash
                    lib.cert.hash_gen(split_header, header_hash)
                    #gen image hash
                    lib.cert.hash_gen(split_image, image_hash)

                    #if exist DM cert, append it after hash calculate
                    if is_boot == 1 and has_dm_cert == 1:
                        append_file(img, dm_cert)

                    #cat cert1
                    append_file(img, cert1)

                    #fill cert2 config
                    shutil.copy2(cert2_config, cert2_config_out)
                    fill_cert_config(cert2_config_out, image_hash, CERT2_REPLACE_HASH)
                    fill_cert_config(cert2_config_out, header_hash, CERT2_REPLACE_HEADER_HASH)
                    fill_cert_config(cert2_config_out, socid.upper(), CERT2_REPLACE_SOCID)

                    if int(img_ver) != 0:
                        fill_cert_config(cert2_config_out, img_ver, IMG_VER_REPLACE_TARGET)
                    #gen cert2
                    self.gen_cert(cert2_config_out, cert_privk, cert2_name, 0)
                    #add mkimage header on cert2
                    cert2_img_type = 0x2 << 24 | 0x2
                    add_mkimg_header(cert2_name, \
                                     img_list_end, \
                                     cert2_img_type, \
                                     "cert2", \
                                     self.context)
                    #cat cert2
                    append_file(img, cert2_name)
                    #cat sig file
                    sig_file = os.path.join(sig_path, img_name + ".sig")
                    print img_ver
                    print "sig:" + sig_file
                    shutil.copy2(cert1, sig_file)
                    append_file(sig_file, cert2_name)

            i += 1

        img_name = ntpath.split(img_file)[1]
        img_name = img_name.split(".")[0] + "-verified." + img_name.split(".")[1]
        final_bin = os.path.join(bin_tmp_path, img_name)

        #cat all bin to img_array[0]
        cat_img(img_array, final_bin)

        #Post Process
        shutil.copy2(final_bin, os.path.join(bin_path, img_name))
        shutil.copy2(os.path.join(bin_path, img_name), os.path.join(out, img_name))
        print "output path:" + os.path.join(out, img_name)
        return

    def gen_img_hash_list(self, img_file, cert1, cert_privk, img_name, img_ver):
        """
        Generate image hash list. This is a verified boot mechanism
        for images that are too big to fit into DRAM at once. In this
        case, image is split into 128MB chunks and sent to DA one by one.
        DA will receive a signed image hash list and verifies it before
        flashing. After that, DA will calculate hash on receiving a chunk
        and compares it against the hash in signed image hash list.
        """
        data_size = 128 * 1048576

        img_size = os.path.getsize(img_file)
        print "img_size => "+ str(img_size)
        file1 = open(img_file, 'rb')
        package_num = (img_size + (data_size - 1)) / data_size
        print "package_num => "+ str(package_num)
        print "get " + img_name + " hash list..."

        bin_tmp_path = self.context['bin_tmp_path']
        cert2_name = self.context['cert2_name']
        cert2_config = self.context['cert2_config']
        cert2_config_out = self.context['cert2_config_out']
        cert2_hash_path = self.context['cert2_hash_path']
        sig_path = self.context['sig_path']
        out = self.context['out']

        for i in range(package_num):
            div_out = os.path.join(bin_tmp_path, img_name + "_" + str(i) + ".bin")
            div_hash_out = os.path.join(cert2_hash_path, img_name + "_" + str(i) + ".hash")
            output_file = open(div_out, 'wb')
            content = file1.read(data_size)
            output_file.write(content)
            output_file.close()
            lib.cert.hash_gen(div_out, div_hash_out)
        file1.close()

        img_hash_file = os.path.join(cert2_hash_path, img_name + "_total.hash")
        file2 = open(img_hash_file, 'wb')
        for i in range(package_num):
            hash_file = open(os.path.join(cert2_hash_path, img_name + "_" + str(i) + ".hash"), 'rb')
            img_hash = hash_file.read()
            file2.write(img_hash)
        file2.close()

        shutil.copy2(cert2_config, cert2_config_out)

        fill_cert_config(cert2_config_out, img_hash_file, CERT2_REPLACE_IMG_HASH_MULTI)
        if int(img_ver) != 0:
            fill_cert_config(cert2_config_out, img_ver, IMG_VER_REPLACE_TARGET)
        #gen cert2
        self.gen_cert(cert2_config_out, cert_privk, cert2_name, 0)
        img_list_end = 1
        #add mkimage header on cert2
        cert2_img_type = 0x2 << 24 | 0x2
        add_mkimg_header(cert2_name, img_list_end, cert2_img_type, "cert2", self.context)
        sig_file = os.path.join(sig_path, img_name + ".sig")
        shutil.copy2(cert1, sig_file)
        append_file(sig_file, cert2_name)
        shutil.copy2(sig_file, os.path.join(out, img_name + ".sig"))

        return

    def sign_op(self):
        """
        perform image signing
        """
        self.set_context()

        if self.args['type'] == "cert1":
            self.__create_folder()
            self.gen_cert1()
        elif self.args['type'] == "cert2":
            self.__create_folder()
            self.gen_cert2()
        elif self.args['type'] == "cert1md":
            self.__create_folder()
            self.gen_cert1md()
        else:
            print "wrong cert type !"

        return

    def dump(self):
        """
        dump parameters
        """
        print 'cert_type = ' + self.get_arg(self.args['type'])
        print 'platform = ' + self.get_arg(self.args['platform'])
        print 'project = ' + self.get_arg(self.args['project'])
        print 'img = ' + self.get_arg(self.args['img'])
        print 'name = ' + self.get_arg(self.args['name'])
        print 'cert1_path = ' + self.get_arg(self.args['cert1'])
        print 'root_prvk = ' + self.get_arg(self.args['privk'])
        print 'img_pubk = ' + self.get_arg(self.args['pubk'])
        print 'group = ' + self.get_arg(self.args['group'])
        print 'ver = ' + self.get_arg(self.args['ver'])
        print 'root_padding = ' + self.get_arg(self.args['root_key_padding'])
        print 'getHashList = ' + self.get_arg(self.args['getHashList'])
        print 'env_cfg = ' + self.get_arg(self.args['env_cfg'])
        return

def parse_multi_bin(img_file, target_img_name, context):
    """
    we may concatenate multiple images to form a composite image.
    This function is used to parse the composite image to get
    information of sub-image with name target_img_name.
    """
    #parse bin
    img_size = 0
    index = 0
    file1 = open(img_file, 'rb')
    last_pos = 0
    pre_pos = 0

    img_array = []
    size_array = []
    offset_array = []
    img_name_array = []

    file_size = os.path.getsize(img_file)
    print "file_size: "+ str(hex(file_size))

    first_img = 1
    final_size = 0
    match_target = 0
    img_name = 0

    bin_tmp_path = context['bin_tmp_path']

    while True:
        file1.seek(last_pos)
        header_size = struct.calcsize(MKIMAGE_HDR_FMT)
        fin = file1.read(header_size)
        unpack_array = struct.unpack(MKIMAGE_HDR_FMT, fin)
        file1.seek(last_pos)

        magic_number = unpack_array[0]
        dsize = unpack_array[1]
        hdr_size = unpack_array[6]
        align_size = unpack_array[10]
        img_type = unpack_array[8]

        if ~cmp(magic_number, int(MKIMAGE_HDR_MAGIC)) == 0:
            print "wrong image header magic"
            sys.exit()

        # include header + image + padding size to 16 bytes align
        img_size = (dsize + hdr_size + (align_size - 1)) / align_size * align_size
        print "img-" + str(index) + " size:" + hex(img_size)

        img_type_byte3 = (img_type >> 24) & 0xFF

        if img_type_byte3 != 2:
            # image not cert
            pre_img_name = img_name
            img_name = unpack_array[2].rstrip('\t\r\n\0')
            if target_img_name == img_name:
                print "Target image, remove cert for replace"
                is_target = 1
                match_target = 1
            else:
                print "Not target image, retain cert"
                is_target = 0
            is_raw = 1
        else:
            #image is cert
            is_raw = 0

        if is_raw and first_img == 0:
            print "add index" + str(index) + " image"
            img_str = os.path.join(bin_tmp_path, "tmp_" + str(index) + ".bin")
            img_array.append(img_str)
            size_array.append(final_size)
            offset_array.append(pre_pos)
            img_name_array.append(pre_img_name)
            pre_pos = last_pos
            final_size = 0
            index += 1

        first_img = 0

        if is_target:
            if is_raw:
                final_size = img_size
            else:
                print "is cert, discard it"
        else:
            final_size += img_size

        last_pos += img_size

        if last_pos >= file_size:
            print "add index" + str(index) + " image, this is final image"
            img_str = os.path.join(bin_tmp_path, "tmp_" + str(index) + ".bin")
            img_array.append(img_str)
            size_array.append(final_size)
            offset_array.append(pre_pos)
            img_name_array.append(img_name)
            pre_pos = last_pos
            final_size = 0
            break

    file1.close()
    if match_target == 0:
        print "img name not match,exit!!"
        return None, None, None, None
    return img_array, size_array, offset_array, img_name_array

def check_is_raw(img_file):
    """
    check whether image is raw image format, which includes
    images with mkimage header and modem image.
    """
    is_raw = 0
    is_md = 0
    print img_file
    file1 = open(img_file, 'rb')
    header_size = struct.calcsize(MKIMAGE_HDR_FMT)
    fin = file1.read(header_size)

    unpack_array = struct.unpack(MKIMAGE_HDR_FMT, fin)
    file1.close()

    hdr_size = unpack_array[6]
    img_type = unpack_array[8]

    if cmp(unpack_array[0], int(MKIMAGE_HDR_MAGIC)) == 0:
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
    return is_raw, is_md, hdr_size

def get_vboot10_cert(img_file, img_size, context):
    """
    parse certificate that follows boot image on vboot1.0
    """
    has_dm_cert = 0
    dm_cert = context['dm_cert']
    file_size = os.path.getsize(img_file)
    if file_size <= img_size + 4:
        return has_dm_cert

    file1 = open(img_file, 'rb')
    fin = file1.read(img_size)
    fin = file1.read(4)

    unpack_array = struct.unpack("<4c", fin)

    if ord(unpack_array[0]) == 0x30:
        cert_size = (ord(unpack_array[2]) << 8) + ord(unpack_array[3]) + 4
        print hex(cert_size)
        file2 = open(dm_cert, 'wb')
        file1.seek(img_size)
        file2.write(file1.read(cert_size))
        file2.close()
        has_dm_cert = 1
        padding_file(dm_cert, 16)
    file1.close()
    return has_dm_cert


def check_is_md(img_file):
    """
    check whether it's modem image.
    """
    #parse bin
    file1 = open(img_file, 'rb')
    header_size = struct.calcsize(CCCI_HDR_FMT)
    fin = file1.read(header_size)
    unpack_array = struct.unpack(CCCI_HDR_FMT, fin)
    file1.close()
    magic_str = ""
    for i in range(0, 7):
        magic_str = magic_str + unpack_array[i]
    return 0

def check_is_boot(img_file):
    """
    check whether it's boot/recovery image.
    """
    #parse bin
    img_size = 0
    is_boot = 0
    hdr_size = 0

    file1 = open(img_file, 'rb')
    header_size = struct.calcsize(BOOTIMG_HDR_FMT)

    fin = file1.read(header_size)
    unpack_array = struct.unpack(BOOTIMG_HDR_FMT, fin)
    file1.close()
    magic_str = ""
    for i in range(0, 7):
        magic_str = magic_str + unpack_array[i]

    if cmp(magic_str, BOOTIMG_HDR_MAGIC) == 0:
        print "Is Boot Img"
        page_size = unpack_array[15]
        hdr_size = page_size
        kernel_size = (unpack_array[8] + (page_size - 1)) / page_size * page_size
        ramdisk_size = (unpack_array[10] + (page_size - 1)) / page_size * page_size
        #recovery dtbo image
        dtbo_size = (unpack_array[1578] + (page_size - 1)) / page_size * page_size
        #dtb image
        dtb_size = (unpack_array[1581] + (page_size - 1)) / page_size * page_size
        print "Header size:" + `hex(header_size)`
        print "Kernel size:" + `hex(kernel_size)`
        print "Ramdisk size:" + `hex(ramdisk_size)`
        print "DTBO size:" + `hex(dtbo_size)`
        print "dtb size:" + `hex(dtb_size)`
        print "page size:" + `hex(unpack_array[15])`
        img_size = (kernel_size + ramdisk_size + hdr_size + dtbo_size + dtb_size + (16 - 1)) / 16 * 16
        print "Total Size(include header and padding):" + `hex(img_size)`
        print hex(img_size)
        is_boot = 1
    else:
        print "Not Boot Img"

    return is_boot, img_size, hdr_size

def check_is_dtbo(img_file):
    """
    check whether it's dtbo image.
    """
    #parse bin
    img_size = 0
    is_dtbo = 0
    hdr_size = 0
    file1 = open(img_file, 'rb')
    header_size = struct.calcsize(DTBO_HDR_FMT)

    fin = file1.read(header_size)
    unpack_array = struct.unpack(DTBO_HDR_FMT, fin)
    file1.close()

    if  unpack_array[0] == DTBO_HDR_MAGIC:
        print "Is DTBO Img"
        hdr_size = unpack_array[2]
        img_size = unpack_array[1]
        print "Header size:" + `hex(hdr_size)`
        print "Image size:" + `hex(img_size)`
        print "Total Size" + `hex(unpack_array[1])`
        is_dtbo = 1
    else:
        print "Not dtbo Img"

    return is_dtbo, img_size, hdr_size

def fill_cert_config(cert_path, replace_str, tgt_line_pattern):
    """
    We provide certificate configuration template and could
    generate cerificate based on the configuration. Before that,
    you need to fill up the configuration template so certificate
    generation engine knows how to generate certificate.
    """
    os.chmod(cert_path, stat.S_IWRITE + stat.S_IREAD)

    file1 = open(cert_path, 'r')
    lines = file1.readlines()
    file1.close()

    file2 = open(cert_path, 'w')

    format1 = re.compile(tgt_line_pattern)
    for line in lines:
        if format1.match(line):
            print line
            line2 = line.split("::=")[0] + "::= " + replace_str + "\n"
            print line2
            file2.write(line2)
        else:
            file2.write(line)

def padding_file(input_file, align_num):
    """
    Fill 0 to make input_file size multiple of align_num.
    """
    filesize = os.stat(input_file).st_size
    file1 = open(input_file, 'ab+')
    padding = filesize % align_num
    if padding != 0:
        padding = align_num - padding
        for _ in range(padding):
            file1.write("\x00")
    file1.close()

def append_file(img_file, cert_file):
    """
    Append cert_file to the end of img_file.
    """
    padding_file(img_file, 16)
    file1 = open(img_file, 'ab+')
    file2 = open(cert_file, 'rb')
    file1.write(file2.read())
    file1.close()
    file2.close()

def get_pure_img(img_file, img_array, size_array, offset_array):
    """
    Put sub-image data into array for composite image.
    """
    index = 0
    file2 = open(img_file, 'rb')
    for sub_img in img_array:
        img_size = size_array[index]
        offset = offset_array[index]
        file2.seek(offset)
        file1 = open(sub_img, 'wb+')
        file1.write(file2.read(img_size))
        file1.close()
        index += 1
    file2.close()

def backup_file(tmp_file, target_file, backup_file_path):
    """
    Backup target_file to backup_file_path
    """
    if backup_file_path != "":
        shutil.move(target_file, backup_file_path)
    shutil.move(tmp_file, target_file)

def cat_img(img_array, final_bin):
    """
    Concatenate data in img_array to form final_bin,
    which is a composite image.
    """
    file1 = open(final_bin, 'wb')
    index = 0
    for img in img_array:
        file2 = open(img, 'rb')
        file1.write(file2.read())
        file2.close()
        index += 1
    file1.close()

def add_mkimg_header(cert_name, img_list_end, img_type, img_name, context):
    """
    mkimage header is the header for Mediatek proprietary images.
    This function is used to generate mkimage header for
    certificate files.

    """
    mkimage_config = context['mkimage_config']
    mkimage_config_out = context['mkimage_config_out']
    tmpcert_name = context['tmpcert_name']
    file1 = open(mkimage_config, 'r')
    file2 = open(mkimage_config_out, 'w+')
    format1 = re.compile("IMG_LIST_END")
    format2 = re.compile("IMG_TYPE")
    format3 = re.compile("NAME")
    for line in file1:
        if format1.match(line):
            end = line.split("=")[1]
            line2 = line.replace(end, str(img_list_end))
            file2.write(line2 + "\n")
        elif format2.match(line):
            end = line.split("=")[1]
            line2 = line.replace(end, str(img_type))
            file2.write(line2 + "\n")
        elif format3.match(line):
            end = line.split("=")[1]
            line2 = line.replace(end, str(img_name))
            file2.write(line2 + "\n")
        else:
            file2.write(line)

    file1.close()
    file2.close()

    img_hdr = lib.mkimghdr.mkimage_hdr()
    img_hdr.update_mkimage_hdr(cert_name, mkimage_config_out)
    img_hdr.pack()
    img_hdr.output(cert_name, tmpcert_name)
    shutil.move(tmpcert_name, cert_name)

def get_md_key(md_img, is_md, context):
    """
    Get modem public key from modem image
    """
    found = 0
    cert1md_hash_path = context['cert1md_hash_path']
    md_key_path = os.path.join(cert1md_hash_path, "md_key.bin")

    if is_md == 1:
        md1_handler = lib.getPublicKey.md1_image()
        found = md1_handler.parse(md_img)
        if found:
            md1_handler.output(md_img, md_key_path)
            print "output file done"
    elif is_md == 2:
        md3_handler = lib.getPublicKey.md3_image()
        found = md3_handler.parse(md_img)
        if found:
            md3_handler.output(md_img, md_key_path)
            print "output file done"
    else:
        print "wrong md type!!!"

    return md_key_path

def img_split(img, split_path, hdr_size):
    """
    split image into header and image body
    """
    split_header = os.path.join(split_path, "header.bin")
    split_image = os.path.join(split_path, "image.bin")

    file1 = open(img, 'rb')
    file2 = open(split_header, 'wb')
    file2.write(file1.read(hdr_size))
    file2.close()
    file2 = open(split_image, 'wb')
    file2.write(file1.read())
    file2.close()
    file1.close()
    padding_file(split_image, 16)

    return split_header, split_image

def endiness_convert(in_file, out_file):
    """
    Converts endian of in_file and save result to out_file.
    """
    endian = sys.byteorder
    if endian == "little":
        file1 = open(out_file, "wb")
        file2 = open(in_file, "rb")
        from array import array
        for _ in range(8, 0, -1):
            tmp = array("B", file2.read(4))
            tmp.reverse()
            tmp.tofile(file1)
        file2.close()
        file1.close()

def fill_arg_dict(input_string, key, args):
    """
    Fill up argument dictionary from input parameters
    """
    prefix = input_string.split("=")[0].strip()
    fmt = re.compile(key, re.I)
    if fmt.search(prefix):
        val = input_string.split("=")[1].strip()
        args[key] = val
        print key + ": " + val
        print args[key]
    return args

def parse_arg(argv):
    """
    Parse input arguments and save the result into argument dictionary.
    """
    args = {'type': 0, \
            'img': 0, \
            'privk': 0, \
            'pubk': 0, \
            'cert1': 0, \
            'swID': 0, \
            'ver': 0, \
            'name': '', \
            'group': 0, \
            'root_key_padding': 0, \
            'getHashList': 0, \
            'env_cfg': 0, \
            'platform': 'NOT_SET', \
            'project': 'NOT_SET', \
            'socid': '0'}
    for input_string in argv:
        for key in args:
            args = fill_arg_dict(input_string, key, args)
    input_wrong = 0

    #check input
    if args['type'] == "cert1":
        if args['privk'] == "" or args['pubk'] == "":
            print "wrong cert1 input"
            input_wrong = 1
    elif args['type'] == "cert2":
        if args['privk'] == "" or args['cert1'] == "" or args['img'] == "" or args['name'] == "":
            print "wrong cert2 input"
    elif args['type'] == "cert1md":
        if args['img'] == "" or args['privk'] == "":
            print "wrong cert1md input"
    else:
        print "wrong cert type!"
        input_wrong = 1
    if input_wrong == 1:
        help_menu()
        sys.exit()

    if args['env_cfg'] == 0:
        #env_cfg is not given, we set it to env.cfg in path of this tool
        args['env_cfg'] = os.path.join(os.path.dirname(__file__), 'env.cfg')
    return args

def help_menu():
    """
    Print usage for this tool.
    """
    print "Gen Cert1:"
    print "    usage: python sign.py type=cert1 privk=[cert1_privk.pem] pubk=[cert2_pubk.pem]"
    print "    optional: swID=[number] ver=[number] group=[number] "
    print "    output: cert1"
    print "Gen Cert2 and append cert1,cert2 to the image:"
    print "    usage: python sign.py type=cert2 img=[xxx.bin] name=[img name] cert1=[cert1.der] \
privk=[cert2_privk.pem]"
    print "    optional:ver=number"
    print "    output: image append with cert1 and cert2"
    print "Gen Cert1md:"
    print "    usage: python sign.py type=cert1md img=[xxx.bin] privk=[cert1md_privk.pem] \
pubk=[cert2_pubk.pem]"
    print "    output: cert1md"

def main():
    """
    Main function when this tool is executed from command line.
    """
    if len(sys.argv) < 3:
        help_menu()

    sign = Sign()
    sign.args = parse_arg(sys.argv)
    sign.sign_op()

if __name__ == '__main__':
    main()
