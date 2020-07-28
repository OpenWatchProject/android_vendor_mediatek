"""
This module is for debug control certificate generation
"""
import os
import sys
from lib import gfh
from lib import cert


def get_file_sizeb(file_path):
    """
    get binary file size
    """
    if not os.path.isfile(file_path):
        return 0
    file_handle = open(file_path, "rb")
    file_handle.seek(0, 2)
    file_size = file_handle.tell()
    file_handle.close()
    return file_size


def concatb(file1_path, file2_path):
    """
    concatenate two binary files
    """
    file2_size = get_file_sizeb(file2_path)
    file1 = open(file1_path, "ab+")
    file2 = open(file2_path, "rb")
    file1.write(file2.read(file2_size))
    file2.close()
    file1.close()


class SctrlCert(object):
    """
    class for debug control certificate
    """
    def __init__(self, out_path, sctrl_cert_path):
        self.m_out_path = out_path
        if not os.path.exists(self.m_out_path):
            os.makedirs(self.m_out_path)
        self.m_sctrl_cert_path = sctrl_cert_path
        self.m_gfh = gfh.ImageGFH()
        self.m_key_path = ""
        self.m_out_path = out_path
        self.m_sig_handler = None

    def create_gfh(self, gfh_config):
        """
        create GFH(generic file header) for debug control certificate
        """
        self.m_gfh.load_ini(gfh_config)
        self.m_gfh.dump()
        return

    def sign(self, key_ini_path, key_cert_path, primary_dbg_config_ini_path, primary_dbg_path,
             secondary_config_file_path):
        """
        generate signature for debug control certificate
        """
        # tool auth contains only gfh and signature, no extra content
        self.m_gfh.finalize(0, key_ini_path)
        # create tbs_sctrl_cert.bin
        tbs_sctrl_cert_file_path = os.path.join(self.m_out_path, "tbs_sctrl_cert.bin")
        tbs_sctrl_cert_file = open(tbs_sctrl_cert_file_path, "wb")
        tbs_sctrl_cert_file.write(self.m_gfh.pack())
        tbs_sctrl_cert_file.close()
        print "===sctrl_cert sign==="
        if self.m_gfh.get_sig_type() == "CERT_CHAIN":
            self.m_sig_handler = cert.CertChainV5()
            # create key cert
            if key_cert_path == "":
                key_cert_path = os.path.join(self.m_out_path, "key_cert.bin")
            if os.path.isfile(key_ini_path):
                key_cert_file_name = os.path.basename(os.path.abspath(key_cert_path))
                self.m_sig_handler.create_key_cert(key_ini_path,
                                                   self.m_out_path,
                                                   key_cert_file_name)
                key_cert_path = os.path.join(self.m_out_path, key_cert_file_name)
            else:
                self.m_sig_handler.set_key_cert(key_cert_path)
            # create primary debug cert
            if primary_dbg_path == "":
                primary_dbg_path = "primary_dbg_cert.bin"
            if os.path.isfile(primary_dbg_config_ini_path):
                primary_dbg_cert_file_name = os.path.basename(os.path.abspath(primary_dbg_path))
                self.m_sig_handler.create_primary_dbg_cert(primary_dbg_config_ini_path,
                                                           tbs_sctrl_cert_file_path,
                                                           self.m_out_path,
                                                           primary_dbg_cert_file_name)
            else:
                self.m_sig_handler.set_primary_dbg_cert(primary_dbg_path)
            # create secondary debug cert
            secondary_dbg_cert_file_name = "secondary_dbg_cert.bin"
            secondary_dbg_cert_file_path = os.path.join(self.m_out_path,
                                                        secondary_dbg_cert_file_name)
            self.m_sig_handler.create_secondary_dbg_cert(secondary_config_file_path,
                                                         self.m_out_path,
                                                         secondary_dbg_cert_file_name)
            # create final cert chain
            sig_name = "sctrl_cert.sig"
            sig_file_path = os.path.join(self.m_out_path, sig_name)
            self.m_sig_handler.output(self.m_out_path, sig_name)
            # create final sctrl cert
            if os.path.isfile(self.m_sctrl_cert_path):
                os.remove(self.m_sctrl_cert_path)
            concatb(self.m_sctrl_cert_path, tbs_sctrl_cert_file_path)
            concatb(self.m_sctrl_cert_path, sig_file_path)
            os.remove(secondary_dbg_cert_file_path)
        elif self.m_gfh.get_sig_type() == "SINGLE":
            self.m_sig_handler = cert.SigSingle(self.m_gfh.get_pad_type())
            self.m_sig_handler.set_out_path(self.m_out_path)
            self.m_sig_handler.create(key_ini_path, tbs_sctrl_cert_file_path)
            self.m_sig_handler.sign()
            sig_name = "sctrl_cert.sig"
            sig_file_path = os.path.join(self.m_out_path, sig_name)
            self.m_sig_handler.output(self.m_out_path, sig_name)
            # create final toolauth file
            if os.path.isfile(self.m_sctrl_cert_path):
                os.remove(self.m_sctrl_cert_path)
            concatb(self.m_sctrl_cert_path, tbs_sctrl_cert_file_path)
            concatb(self.m_sctrl_cert_path, sig_file_path)
        else:
            print "unknown signature type"

        # clean up
        os.remove(tbs_sctrl_cert_file_path)
        os.remove(sig_file_path)
        return


def main():
    """
    entry point if this module is executed from cmdline.
    """
    # parameter parsing
    idx = 1
    key_ini_path = ""
    key_cert_path = ""
    gfh_config_ini_path = ""
    primary_dbg_path = ""
    primary_dbg_config_ini_path = ""
    secondary_dbg_config_ini_path = ""
    sctrl_cert_path = ""

    while idx < len(sys.argv):
        if sys.argv[idx][0] == '-':
            if sys.argv[idx][1] == 'i':
                print "key: " + sys.argv[idx + 1]
                key_ini_path = sys.argv[idx + 1]
                idx += 2
            elif sys.argv[idx][1] == 'g':
                print "gfh config: " + sys.argv[idx + 1]
                gfh_config_ini_path = sys.argv[idx + 1]
                idx += 2
            elif sys.argv[idx][1] == 'p':
                print "primary dbg cert: " + sys.argv[idx + 1]
                primary_dbg_path = sys.argv[idx + 1]
                idx += 2
            elif sys.argv[idx][1] == 'q':
                print "primary dbg cert config: " + sys.argv[idx + 1]
                primary_dbg_config_ini_path = sys.argv[idx + 1]
                idx += 2
            elif sys.argv[idx][1] == 's':
                print "secondary dbg cert config: " + sys.argv[idx + 1]
                secondary_dbg_config_ini_path = sys.argv[idx + 1]
                idx += 2
            elif sys.argv[idx][1] == 'k':
                print "key cert: " + sys.argv[idx + 1]
                key_cert_path = sys.argv[idx + 1]
                idx += 2
            else:
                print "unknown input"
                idx += 2
        else:
            sctrl_cert_path = sys.argv[idx]
            print "sctrl_cert_path: " + sctrl_cert_path
            idx += 1

    if not key_cert_path and not key_ini_path:
        print "key path is not given!"
        return -1
    if not gfh_config_ini_path:
        print "sctrl_cert_config_path is not given!"
        return -1
    if not sctrl_cert_path:
        print "sctrl_cert is not given!"
        return -1

    out_path = os.path.dirname(os.path.abspath(sctrl_cert_path))

    sctrl_cert_obj = SctrlCert(out_path, sctrl_cert_path)
    sctrl_cert_obj.create_gfh(gfh_config_ini_path)
    sctrl_cert_obj.sign(key_ini_path, key_cert_path, primary_dbg_config_ini_path, primary_dbg_path,
                        secondary_dbg_config_ini_path)

    return 0


if __name__ == '__main__':
    main()
