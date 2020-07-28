"""
This module converts pem format data to Mediatek proprietary configuration file and C header
"""
import sys
from lib import cert


def usage():
    """
    show usage of this module
    """
    print "========================================="
    print "[usage]"
    print "========================================="
    print "python key_pem_to_ini.py prvk.pem CHIP_TEST_KEY.ini BL_SIGN"
    print "python key_pem_to_ini.py prvk.pem CHIP_TEST_KEY.ini BL_SBC"
    print "python key_pem_to_ini.py prvk.pem oemkey.h AND_SBC"
    print "python key_pem_to_ini.py prvk.pem oemkey.h DA_SBC"


def ini_gen_bl_sign(key, output):
    """
    generate configuration file for preloader signing
    """
    out_file = open(output, 'w')
    out_file.write("; ##############################\n")
    out_file.write("; Code Signature Key\n")
    out_file.write("; ##############################\n\n")
    out_file.write("[Code Sig Key]\n")
    out_file.write("; Hex format\n")
    out_file.write("private_key_d = \"" + key.m_d_str.upper() + "\"\n")
    out_file.write("private_key_n = \"" + key.m_n_str.upper() + "\"\n")
    out_file.write("public_key_e = \"010001\"\n")
    out_file.write("public_key_n = \"" + key.m_n_str.upper() + "\"\n")
    out_file.write("[Install Sig Key]\n")
    out_file.write("public_key_e = \"010001\"\n")
    out_file.write("public_key_n = \"" + key.m_n_str.upper() + "\"\n\n")
    out_file.write("; ##############################\n")
    out_file.write("; Anticlone Key\n")
    out_file.write("; ##############################\n\n")
    out_file.write("[Anticlone key]\n")
    out_file.write("; Hex format\n")
    out_file.write("anticlone_key = \"0x11111111, 0x22222222, 0x33333333, 0x44444444\"\n")
    out_file.close()


def ini_gen_sla_sign(key, output):
    """
    generate header file for sla public key
    """
    out_file = open(output, 'w')
    out_file.write("#ifndef __SLAKEY__\n")
    out_file.write("#define __SLAKEY__\n")
    out_file.write("#define SLA_KEY_SZ 256\n")
    out_file.write("#define SLA_KEY_N   ")
    for i in range(0, 255):
        out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + ", ")
        if (i + 1) % 16 == 0:
            out_file.write("\\\n                   ")
    i = 255
    out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + "\n")

    out_file.write("\\\n")

    out_file.write("#define SLA_KEY_D   ")
    for i in range(0, 255):
        out_file.write("0x" + key.m_d_str[2 * i].upper() + key.m_d_str[2 * i + 1].upper() + ", ")
        if (i + 1) % 16 == 0:
            out_file.write("\\\n                   ")
    i = 255
    out_file.write("0x" + key.m_d_str[2 * i].upper() + key.m_d_str[2 * i + 1].upper() + "\n")
    out_file.write("#endif /* __SLAKEY__ */")
    out_file.close()


def ini_gen_bl_sbc(key, output):
    """
    generate configuration file for only public key part of root key.
    """
    out_file = open(output, 'w')
    out_file.write("[Install Sig Key]\n")
    out_file.write("public_key_e = \"010001\"\n")
    out_file.write("public_key_n = \"" + key.m_n_str.upper() + "\"\n")
    out_file.close()


def ini_gen_and_sbc(key, output):
    """
    generate header for android verified boot key
    """
    out_file = open(output, 'w')
    out_file.write("#ifndef __OEMKEY__\n")
    out_file.write("#define __OEMKEY__\n")
    out_file.write("#define OEM_PUBK_SZ 256\n")
    out_file.write("#define OEM_PUBK   ")
    for i in range(0, 255):
        out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + ", ")
        if (i + 1) % 16 == 0:
            out_file.write("\\\n                   ")
    i = 255
    out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + "\n")
    out_file.write("#endif /* __OEM_KEY__ */")
    out_file.close()


def ini_gen_da_sbc(key, output):
    """
    generate header for DA(download agent) authentication.
    """
    out_file = open(output, 'w')
    out_file.write("#ifndef __DAKEY__\n")
    out_file.write("#define __DAKEY__\n")
    out_file.write("#define DA_PUBK_SZ 256\n")
    out_file.write("#define DA_PUBK   ")
    for i in range(0, 255):
        out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + ", ")
        if (i + 1) % 16 == 0:
            out_file.write("\\\n                   ")
    i = 255
    out_file.write("0x" + key.m_n_str[2 * i].upper() + key.m_n_str[2 * i + 1].upper() + "\n")
    out_file.write("#endif /* __DAKEY__ */")
    out_file.close()


def main():
    """
    main function for pem to configuration/header conversion.
    """
    if len(sys.argv) != 4:
        usage()
        return -1

    key_path = sys.argv[1]
    output = sys.argv[2]
    op = sys.argv[3]
    print "========================================="
    print "key_path  = " + key_path
    print "output    = " + output
    print "op        = " + op
    print "========================================="

    key = cert.prvk_converter(key_path, 0)
    if op == "BL_SIGN":
        ini_gen_bl_sign(key, output)
    elif op == "SLA_SIGN":
        ini_gen_sla_sign(key, output)
    elif op == "BL_SBC":
        ini_gen_bl_sbc(key, output)
    elif op == "AND_SBC":
        ini_gen_and_sbc(key, output)
    elif op == "DA_SBC":
        ini_gen_da_sbc(key, output)
    else:
        print "Unknown operation"
        return -1

    return 0


if __name__ == '__main__':
    main()
