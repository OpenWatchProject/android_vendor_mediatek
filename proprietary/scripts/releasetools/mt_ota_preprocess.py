#!/usr/bin/env python
#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys

if sys.hexversion < 0x02070000:
  print >> sys.stderr, "Python 2.7 or newer is required."
  sys.exit(1)

import copy
import os
import shutil
import re

upgrade_list_include = ["tee1","tee2"]
upgrade_list_exclude = {
        "bbota" : ["boot","system","android","recovery","bootimg","super"],
        "abota" : ["super"]
}
upgrade_list_specific_sequence = {"preloader", "preloader2"}
upgrade_list_mount_map = {"lk" : "bootloader", "lk2" : "bootloader2", "uboot" : "bootloader", "uboot2" : "bootloader2"}
partition_map = {}
add_to_target_list_exclude = ["vbmeta.img","dtbo.img","super.img","vbmeta_vendor.img","vbmeta_system.img"]
extra_image_list = ["logo.bin", "preloader_SBOOT_DIS.img", "PGPT", "preloader_emmc.img", "preloader_ufs.img"]

def addPartMap(part_map, part_name, file_name, storage_type):
  if not partition_map.has_key(file_name):
     partition_map[file_name] = []
  if part_name.lower() in upgrade_list_mount_map.keys():
    part_name = upgrade_list_mount_map[part_name.lower()]
  if part_name.lower() not in partition_map[file_name]:
    partition_map[file_name].append(part_name.lower())
    # for preloader in EMMC, add preloader2 manually because scatter does not have preloader2 partition
    if part_name == "preloader" and ("EMMC" in storage_type or "HW_STORAGE_UFS" in storage_type):
      if os.getenv("MTK_PRELOADER_OTA_BACKUP") is None or os.getenv("MTK_PRELOADER_OTA_BACKUP") == "yes":
        partition_map[file_name].append(part_name.lower()+"2")
      else:
        print "[OTA Preprocess] Preloader will update without backup!"

def generate_updatelist(scatter_file, update_list):
  if not os.path.exists(scatter_file):
    print "[OTA Preprocess] Scatter file %s not found!" % scatter_file
    sys.exit(1)
  print "[OTA Preprocess] Generate %s from %s" %(update_list, scatter_file)

  # processu default update list include
  if os.getenv("MTK_LOADER_UPDATE") is not None and os.getenv("MTK_LOADER_UPDATE") == "yes":
    if "lk" not in upgrade_list_include:
      upgrade_list_include.append("lk")
    if "preloader" not in upgrade_list_include:
      upgrade_list_include.append("preloader")
    if "uboot" not in upgrade_list_include:
      upgrade_list_include.append("uboot")

  # open input/output files
  inputfile = open(scatter_file)
  outputfile = open(update_list, 'w')

  my_text = inputfile.read() #reads to whole text file, skipping first 4 lines

  split_str = "- partition_index"
  m = re.compile(r'%s.*?\n(.*?)\n\n' % (split_str),re.S)
  result = m.findall(my_text)
  for item in result:
    is_update = re.search(r'is_upgradable: (.*?)\n', item)
    part_name = re.search(r'partition_name: (.*?)\n', item).group(1)
    file_name = re.search(r'file_name: (.*?)\n', item).group(1)
    storage_type = re.search(r'storage: (.*?)\n', item).group(1)
    fs_type = re.search(r'type: (.*?)\n', item).group(1)
    if "preloader_" in file_name:
      file_name  = "preloader.img"
    if is_update is not None:
      if is_update.group(1) == "true" and not part_name.lower() in upgrade_list_exclude["bbota"] and file_name != "NONE":
        if fs_type != "EXT4_IMG" and fs_type != "UBI_IMG":
          addPartMap(partition_map, part_name, file_name, storage_type)
        else:
          print "[OTA Preprocess] %s is an %s, can not be upgraded by this method" % (part_name, fs_type)
    else:
      if part_name.lower() in upgrade_list_include and file_name != "NONE":
        if fs_type != "EXT4_IMG" and fs_type != "UBI_IMG":
          addPartMap(partition_map, part_name, file_name, storage_type)
        else:
          print "[OTA Preprocess] %s is an %s, can not be upgraded by this method" % (part_name, fs_type)

  inputfile.close()
  outputstrs_head=[]
  outputstrs_tail=[]
  for img in partition_map:
    if len([part for part in partition_map[img] if part.lower() not in upgrade_list_specific_sequence]) > 0:
      outputstrs_head.append(img + " " + " ".join(partition_map[img]) + "\n")
    else:
      outputstrs_tail.append(img + " " + " ".join(partition_map[img]) + "\n")

  if len(outputstrs_head) > 0:
    outputfile.writelines(outputstrs_head)
  if len(outputstrs_tail) > 0:
    outputfile.writelines(outputstrs_tail)
  outputfile.close()

def add_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list, type=None):
  print "[OTA Preprocess] add_ab_combo_preloader"

  if type == "combo":
    # Append preloader_emmc and preloader_ufs in ab_partition.txt
    ab_partitions_file = open(zip_root_folder + ab_update_list, "a+")
    print "[OTA Preprocess] Add update partition preloader_emmc and preloader_ufs to ab_partitoin.txt"
    ab_partitions_file.write("preloader_ufs\n")
    ab_partitions_file.write("preloader_emmc\n")
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "preloader_emmc.img")
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "preloader_ufs.img")
    ab_partitions_file.close()

    # Remove preloader from ab_partitionx.txt
    ab_partitions_file = open(zip_root_folder + ab_update_list, "r")
    lines = ab_partitions_file.readlines()
    ab_partitions_file.close()
    ab_partitions_file = open(zip_root_folder + ab_update_list, "w")
    for line in lines:
      if line != "preloader"+"\n":
        ab_partitions_file.write(line)

    ab_partitions_file.close()
  elif type == "emmc":
    # Append preloader_emmc in ab_partition.txt
    ab_partitions_file = open(zip_root_folder + ab_update_list, "a+")
    print "[OTA Preprocess] Add update partition preloader of emmc to ab_partitoin.txt"
    ab_partitions_file.write("preloader\n")
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "preloader_emmc.img", "preloader.img")
    ab_partitions_file.close()
  elif type == "ufs":
    # Append preloader_ufs in ab_partition.txt
    ab_partitions_file = open(zip_root_folder + ab_update_list, "a+")
    print "[OTA Preprocess] Add update partition preloader of ufs to ab_partitoin.txt"
    ab_partitions_file.write("preloader\n")
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "preloader_ufs.img", "preloader.img" )
    ab_partitions_file.close()
  else:
    print "[OTA Preprocess] Update preloader without combo image"
    ab_partitions_file = open(zip_root_folder + ab_update_list, "a+")
    ab_partitions_file.write("preloader\n")
    ab_partitions_file.close()

def handle_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list):
  if os.getenv("MTK_EMMC_SUPPORT", "") == "yes" and os.getenv("MTK_UFS_SUPPORT", "") == "yes":
    if os.getenv("AB_OTA_UPDATER", "") != "true":
      add_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list, "combo")
    elif os.getenv("AB_OTA_UPDATER", "") == "true" and int(os.getenv("PRODUCT_SHIPPING_API_LEVEL", ""), 10) >= 29:
      add_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list, "combo")
    else:
      print "[OTA Preprocess] Update preloader without combo image for cross-version ab update %s" % os.getenv("AB_OTA_UPDATER_DEFAULT_TYPE", "")
      add_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list, os.getenv("AB_OTA_UPDATER_DEFAULT_TYPE", ""))
  else:
    add_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list)

def copy_files(src_dir, dst_dir, src_filename, tgt_filename=None):
  if not os.path.exists(dst_dir):
    os.makedirs(dst_dir)
  verified_filename = os.path.splitext(src_filename)[0] + "-verified" + os.path.splitext(src_filename)[1]
  src = os.path.join(src_dir, verified_filename) if os.path.isfile(os.path.join(src_dir, verified_filename)) else os.path.join(src_dir, src_filename)
  dst = os.path.join(dst_dir, src_filename) if tgt_filename is None else os.path.join(dst_dir, tgt_filename)
  if src_filename not in add_to_target_list_exclude:
    print "[OTA Preprocess] Copy from %s to %s" % (src, dst)
    shutil.copy(src, dst)
  else:
    print "[OTA Preprocess] %s is handle by add_img_to_target_files" % (src_filename)

def copy_ab_ota_filese(out_folder, scatter_file, zip_root_folder, ab_update_list):
  ab_ota_partitoins = os.getenv("AB_OTA_PARTITIONS", "")
  if not os.path.exists(scatter_file):
    print "[OTA Preprocess] Scatter file %s not found!" % scatter_file
    sys.exit(1)
  if ab_update_list is not None:
    print "[OTA Preprocess] copy AB OTA files"

  # open input/output files
  inputfile = open(scatter_file)
  if ab_update_list is not None:
    ab_ota_file = open(zip_root_folder + ab_update_list, "a+")

  my_text = inputfile.read() #reads to whole text file, skipping first 4 lines
  ab_partition = ''
  if ab_update_list is not None:
    ab_partition = ab_ota_file.read()

  split_str = "- partition_index"
  m = re.compile(r'%s.*?\n(.*?)\n\n' % (split_str),re.S)
  result = m.findall(my_text)
  for item in result:
    is_update = re.search(r'is_upgradable: (.*?)\n', item)
    part_name = re.search(r'partition_name: (.*?)\n', item).group(1).split('_')
    part_name = part_name[0] if len(part_name) == 1 else "_".join(part_name[:-1])
    fs_type = re.search(r'type: (.*?)\n', item).group(1)
    if is_update.group(1) == "true" :
      if not part_name.lower() in ab_partition and not part_name in ab_ota_partitoins and part_name not in upgrade_list_exclude["abota"]:
        if ab_update_list is not None:
          print "[OTA Preprocess] Add update partition %s to ab_partitions.txt" % part_name
          ab_ota_file.write(part_name.lower()+"\n")
        else:
          print part_name.lower()
        ab_partition = ab_partition + (part_name.lower()+"\n")
      if (fs_type != "EXT4_IMG") and (out_folder is not None):
        copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), part_name.lower() + ".img")

  # There are no vendor and system partitions if super partition is enabled
  board_super_partition_partition_list = os.getenv("BOARD_SUPER_PARTITION_PARTITION_LIST", "")
  if ab_update_list is not None:
    print "[OTA Preprocess] board_super_partition_partition_list %s" % board_super_partition_partition_list
  result = board_super_partition_partition_list.split()
  for super_part_name in result:
    if not super_part_name.lower() in ab_ota_partitoins :
      if ab_update_list is not None:
        print "[OTA Preprocess] Add update partition %s to ab_partitions.txt" % super_part_name
        ab_ota_file.write(super_part_name.lower()+"\n")
      else:
        print super_part_name.lower()

  inputfile.close()
  if ab_update_list is not None:
    ab_ota_file.close()

  if out_folder is not None and zip_root_folder is not None and ab_update_list is not None:
    handle_ab_combo_preloader(out_folder, zip_root_folder, ab_update_list)

# zip_root product_out update_list
def main(argv):
  if len(argv) != 3:
    print "[OTA Preprocess] Incorrect parameter number, pass %d(need 3)" % len(argv)
    sys.exit(1)

  zip_root_folder = argv[0]
  out_folder = argv[1]
  update_list = argv[2]

  if not os.path.exists(zip_root_folder):
    print "[OTA Preprocess] Target dir:%s is invalid" % zip_root_folder
    sys.exit(1)

  if not os.path.exists(out_folder):
    print "[OTA Preprocess] Source dir:%s is invalid" % out_folder
    sys.exit(1)

  # generate update list
  scatter_file = out_folder + "/" + [filename for filename in os.listdir(out_folder) if filename.endswith("Android_scatter.txt")][0]
  if os.getenv("AB_OTA_UPDATER") != "true":
    generate_updatelist(scatter_file, update_list)
    # copy ota_scatter.txt and ota_update_list.txt to intermediates folder, generate ota_scatter.txt if file not exists
    shutil.copy(update_list, zip_root_folder + "/OTA/" + update_list.split("/")[-1])

  if not os.path.isfile(out_folder + "/ota_scatter.txt"):
    print "[OTA Preprocess] generating %s from %s" % (out_folder + "/ota_scatter.txt", scatter_file)
    os.system("perl ./device/mediatek/build/build/tools/ptgen/ota_scatter.pl %s %s" % (scatter_file, out_folder + "/ota_scatter.txt"))
  shutil.copy(out_folder + "/ota_scatter.txt", zip_root_folder + "/OTA/ota_scatter.txt")

  # copy boot and recovery prebuilt and signed image to target files package
  if os.getenv("MAIN_VBMETA_IN_BOOT") != "yes":
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "boot.img")

  # copy preloader_SBOOT_DIS.img to target files package
  for img_name in extra_image_list:
    if os.path.isfile(os.path.join(out_folder, img_name)):
      print "copy extra_image %s to target files" % (os.path.join(out_folder, img_name))
      copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), img_name)

  if os.getenv("AB_OTA_UPDATER") != "true":
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), "recovery.img")
  else:
    copy_ab_ota_filese(out_folder, scatter_file, zip_root_folder ,"/META/ab_partitions.txt")

  # copy image files needed by update list to IMAGES/ in intermediates folder, note that add_img_to_target_files.py need to be performed with -a option after this step
  for filename in partition_map:
    copy_files(out_folder, os.path.join(zip_root_folder, "IMAGES"), filename)

if __name__ == '__main__':
  main(sys.argv[1:])
