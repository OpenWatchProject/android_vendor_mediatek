"""
This module is for x509 certificate generation with asn1.der encoding
"""
import os
import shutil
import stat
import argparse
import lib.cert
import lib.asn1_gen
import pbp

class CertGenArgs(object):
    """
    CertGenArgs is used to pass argument to this module: cert_gen
    """
    def __init__(self):
        """
        contructor for CertGenArgs
        """
        self.config_file = None
        self.prvk_file_path = None
        self.root_key_padding = 'legacy'
        self.x509cert_file = None
        self.env = os.path.join(os.path.dirname(__file__), 'env.cfg')
        return

    def reset(self):
        """
        reset all arguments
        """
        self.__init__()
        return

    def dump(self):
        """
        dump parameters
        """
        get_arg = lambda arg: 'Not Set' if arg is None else arg
        print 'config_file = ' + get_arg(self.config_file)
        print 'prvk_file_path = ' + get_arg(self.prvk_file_path)
        print 'root_key_padding = ' + get_arg(self.root_key_padding)
        print 'x509cert_file = ' + get_arg(self.x509cert_file)
        print 'env = ' + get_arg(self.env)
        return

def cert_gen_op(cert_gen_args):
    """
    generate certificate
    """
    env_args = {}
    parse_env_cfg(cert_gen_args.env, env_args)

    config_file = cert_gen_args.config_file
    prvk_file_path = cert_gen_args.prvk_file_path
    root_key_padding = cert_gen_args.root_key_padding
    x509cert_file = cert_gen_args.x509cert_file

    in_file = os.path.basename(os.path.abspath(config_file))
    out_folder = os.path.dirname(os.path.abspath(x509cert_file))
    #core name will be used to create hash/sig and other temporary files
    core_name = os.path.splitext(in_file)[0]

    create_cert_intermediate_folder(out_folder)
    intermediate_folder_path = os.path.join(out_folder, "cert_intermediate")

    hash_file_path = os.path.join(intermediate_folder_path, "tbs_" + core_name + ".hash")
    sig_file_path = os.path.join(intermediate_folder_path, "tbs_" + core_name + ".sig")
    tbs_cert_file_path = os.path.join(intermediate_folder_path, "tbs_" + core_name + ".der")
    pubk_file_path = os.path.join(intermediate_folder_path, core_name + "_pubk.pem")

    #generate public key
    print "Generating public key from private key..."
    if lib.cert.is_prvk(prvk_file_path):
        lib.cert.prvk_to_pubk(os.path.abspath(prvk_file_path), pubk_file_path)
    elif lib.cert.is_pubk(prvk_file_path):
        #will delegate image signing to HSM.
        shutil.copy2(prvk_file_path, pubk_file_path)
    else:
        print 'unknown key format'
        return

    print "Generating tbsCertificate..."
    #update public key path in config file
    tbs_config_file_path = os.path.join(intermediate_folder_path, "tbs_" + core_name + ".cfg")
    #generate tbs config file
    gen_tbs_config_file(config_file, tbs_config_file_path, pubk_file_path, root_key_padding)

    lib.asn1_gen.asn1_gen(tbs_config_file_path, tbs_cert_file_path, False)
    lib.cert.sig_gen(tbs_cert_file_path, prvk_file_path, 'pss', sig_file_path)

    print "Generating x509cert..."
    #update signature file path in x509 config file
    x509cert_config_file_path = os.path.join(intermediate_folder_path, "x509_" + core_name + ".cfg")
    #generate x509 config file
    gen_x509_config_file(os.path.join(env_args['x509_template_path'], 'x509cert_template.cfg'), \
                         x509cert_config_file_path, \
                         tbs_config_file_path, \
                         sig_file_path)
    lib.asn1_gen.asn1_gen(x509cert_config_file_path, x509cert_file, False)
    print "Done"
    return

def pubk_to_gfh(pubk, gfh_file_path, padding):
    """
    convert pem format of root public key to gfh, which is needed for public key authentication.
    """
    pbp_args = pbp.PbpArgs()

    if padding == 'pss':
        pbp_args.op = 'keybin_pss'
    elif padding == 'legacy':
        pbp_args.op = 'keybin_legacy'
    else:
        return -1

    pbp_args.key_path = pubk
    pbp_args.output_path = gfh_file_path
    if pbp.pbp_op(pbp_args):
        print 'public key to gfh conversion fails'
        return -2

    return 0

def update_tbs_config_file(config_file_path, pubk_pem_path, root_key_padding):
    """
    compose configuration for tbsCertificate
    """
    config_file = open(config_file_path, 'r')
    lines = config_file.readlines()
    config_file.close()

    out_config_file = open(config_file_path, 'w')
    for line in lines:
        component = line.split()
        if (component[0] == 'pubk') and (component[1] == 'EXTERNAL_PEM'):
            line_regen = "    pubk EXTERNAL_PEM ::= " + pubk_pem_path + '\n'
            out_config_file.write(line_regen)
        elif (component[0] == 'rootpubk') and (component[1] == 'EXTERNAL_BITSTRING'):
            #this is root cert, must generate gfh and oemkey.h
            #pubk is generated from prvk, and we assuem pubk is in out_folder
            intermediate_folder_path = os.path.dirname(os.path.abspath(pubk_pem_path))
            #generate gfh from pubk
            gfh_file_path = os.path.join(intermediate_folder_path, "gfh_sec_key.bin")
            pubk_to_gfh(pubk_pem_path, gfh_file_path, root_key_padding)
            #generate final path of gfh
            line_regen = "    rootpubk EXTERNAL_BITSTRING ::= " + gfh_file_path + '\n'
            out_config_file.write(line_regen)
        else:
            out_config_file.write(line)

    out_config_file.close()
    return

def gen_tbs_config_file(config_file, tbs_config_file, pubk_file, root_key_padding):
    """
    generate configuration file for tbs(to-be-signed) certificate in x509 certificate.
    """
    shutil.copy2(config_file, tbs_config_file)
    update_tbs_config_file(tbs_config_file, pubk_file, root_key_padding)

def update_x509_config_file(x509cert_cfg, tbs_cert_cfg, sig_path):
    """
    compose x509 certificate with tbsCertificate and signature
    """
    config_file = open(x509cert_cfg, 'r')
    lines = config_file.readlines()
    config_file.close()

    out_config_file = open(x509cert_cfg, 'w')
    for line in lines:
        component = line.split()
        if (component[0] == 'tbsCertificate') and (component[1] == 'EXTERNAL_CFG'):
            line_regen = "    tbsCertificate EXTERNAL_CFG ::= " + tbs_cert_cfg + '\n'
            out_config_file.write(line_regen)
        elif (component[0] == 'sigValue') and (component[1] == 'EXTERNAL_BITSTRING'):
            line_regen = "    sigValue EXTERNAL_BITSTRING ::= " + sig_path + '\n'
            out_config_file.write(line_regen)
        else:
            out_config_file.write(line)

    out_config_file.close()
    return

def gen_x509_config_file(x509cert_template_cfg, x509cert_config_file, tbs_config_file, sig_file):
    """
    generate configuration file for final x509 certificate
    """
    shutil.copy2(x509cert_template_cfg, x509cert_config_file)
    os.chmod(x509cert_config_file, stat.S_IWRITE + stat.S_IREAD)
    update_x509_config_file(x509cert_config_file, tbs_config_file, sig_file)

def create_cert_intermediate_folder(out_folder_path):
    """
    create folder to intermediate data
    """
    intermediate_folder_path = os.path.abspath(os.path.join(out_folder_path, "cert_intermediate"))

    try:
        os.makedirs(intermediate_folder_path)
    except OSError, error:
        if error.errno != os.errno.EEXIST:
            raise

def parse_env_cfg(env_cfg_path, env_args):
    """
    get environment arguments from the given environment configuration file
    """
    print '=============================='
    print 'env_cfg parsing'
    print '=============================='
    cfg_file = open(env_cfg_path, 'r')
    cfg_file_dir = os.path.dirname(os.path.abspath(env_cfg_path))
    for line in cfg_file.readlines():
        line = line.strip()
        if line:
            elements = line.split('=')
            field = elements[0].strip()
            value = elements[1].strip()
            if field == 'x509_template_path':
                env_args['x509_template_path'] = os.path.join(cfg_file_dir, value)
            else:
                print 'ignored: ' + field
    print '=============================='
    return

def main():
    """
    main function which is the entry point when this script is invoked from cmdline.
    """

    parser = argparse.ArgumentParser(description="x509 certificate generation tool")
    parser.add_argument('-cert_cfg',
                        dest='config_file',
                        required=True,
                        help='certificate configuration')
    parser.add_argument('-prvk',
                        dest='prvk_file_path',
                        required=True,
                        help='private key for certificate signature generation.')
    parser.add_argument('-padding',
                        dest='root_key_padding',
                        default='legacy',
                        help='private key for certificate signature generation.')
    parser.add_argument('-out',
                        dest='x509cert_file',
                        required=True,
                        help='output path for certificate')
    parser.add_argument('-env',
                        dest='env',
                        default=os.path.join(os.path.dirname(__file__), 'env.cfg'),
                        help='output path for certificate')
    cert_gen_args = parser.parse_args()

    cert_gen_op(cert_gen_args)

    return

if __name__ == '__main__':
    main()
