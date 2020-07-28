import os
import shutil
import sys
from lib import dainfo


def print_usage():
    print "python resign_da.py [in_da_file_path] [chip_name] [bbchip_config_path] [load_region_idx] [out_da_file_path]"
    print "example to sign all load regions in MT6755:"
    print "python resign_da.py in/da.bin MT6755 bbchips.ini all out/da-resign.bin"
    print "example to sign only load region 0 in MT6755:"
    print "python resign_da.py in/da.bin MT6755 bbchips.ini 0 out/da-resign.bin"
    print "note: da.bin may contain multiple das, one for one chip"


def main():
    load_region_idx = "all"
    if len(sys.argv) != 6:
        print_usage()
        return -1

    found = 0

    in_da_file_path = sys.argv[1]
    chip_name = sys.argv[2]
    bbchip_config_path = sys.argv[3]
    load_region_idx = sys.argv[4]
    out_da_file_path = sys.argv[5]
    chip_configs = dainfo.BBChips()
    print "in_da_file_path = " + in_da_file_path
    print "chip_name = " + chip_name
    print "bbchip_config_path = " + bbchip_config_path
    print "load_region_idx = " + load_region_idx
    print "out_da_file_path = " + out_da_file_path

    out_path = os.path.dirname(os.path.abspath(out_da_file_path))
    if not os.path.exists(out_path):
        os.makedirs(out_path)
    shutil.copyfile(in_da_file_path, out_da_file_path)

    chip_configs.load_config(bbchip_config_path)
    da_info_obj = dainfo.DaInfo()
    da_info_obj.parse(out_da_file_path, 0)

    # extract da load regions for resign
    for chip_config in chip_configs.chiplist:
        if chip_name == chip_config.name:
            for da_info_entry in da_info_obj.m_da_info_entries:
                if dainfo.chip_match(chip_config, da_info_entry) is True:
                    print "chip match!: " + chip_name
                    found = 1
                    if load_region_idx == 'all':
                        dainfo.resign_load_regions(out_da_file_path, chip_name, da_info_entry, chip_config)
                    else:
                        dainfo.resign_load_region_with_idx(load_region_idx, out_da_file_path, chip_name, da_info_entry,
                                                           chip_config)
                    break
            break

    if found == 0:
        os.remove(out_da_file_path)
	da_info_obj.dump(False)
        print("=============================================================")
        print(chip_name + " not found!")
        print("please refer to the above info and fill bbchips.ini correctly")
        print("re-sign failed")
        print("=============================================================")


if __name__ == '__main__':
    main()
