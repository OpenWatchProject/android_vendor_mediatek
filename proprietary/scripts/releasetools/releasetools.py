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

import os
import zipfile
import common
import edify_generator
import os
from common import BlockDifference, EmptyImage, GetUserImage

# The joint list of user image partitions of source and target builds.
# - Items should be added to the list if new dynamic partitions are added.
# - Items should not be removed from the list even if dynamic partitions are
#   deleted. When generating an incremental OTA package, this script needs to
#   know that an image is present in source build but not in target build.
USERIMAGE_PARTITIONS = [
    "product",
]

mtStageFile="/cache/recovery/last_mtupdate_stage"
part_dev_map = {
  "preloader"  : "/dev/block/mmcblk0boot0"
                 ,
  "preloader2" : "/dev/block/mmcblk0boot1"
}

force_ro_dev_map = {
  "preloader"  : "/sys/block/mmcblk0boot0/force_ro"
                 ,
  "preloader2" : "/sys/block/mmcblk0boot1/force_ro"
}

post_update_img_list = ["vbmeta"]
incremental_update_raw_img_list = ["md1img" , "md1dsp"]
patch_info_list = []

def GetUserImages(input_tmp, input_zip):
  return {partition: GetUserImage(partition, input_tmp, input_zip)
          for partition in USERIMAGE_PARTITIONS
          if os.path.exists(os.path.join(input_tmp,
                                         "IMAGES", partition + ".img"))}
def FullOTA_GetBlockDifferences(info):
  images = GetUserImages(info.input_tmp, info.input_zip)
  return [BlockDifference(partition, image)
          for partition, image in images.items()]

def IncrementalOTA_GetBlockDifferences(info):
  source_images = GetUserImages(info.source_tmp, info.source_zip)
  target_images = GetUserImages(info.target_tmp, info.target_zip)
  # Use EmptyImage() as a placeholder for partitions that will be deleted.
  for partition in source_images:
    target_images.setdefault(partition, EmptyImage())
  # Use source_images.get() because new partitions are not in source_images.
  return [BlockDifference(partition, target_image, source_images.get(partition))
          for partition, target_image in target_images.items()]

def check_bootloader_path(input_zip):
  """check bootloader path by prop.default."""
  prop_default = input_zip.read("RECOVERY/RAMDISK/prop.default")
  if "ro.vendor.mtk_ufs_support=1" in prop_default:
    print "device type is ufs, modify bootloader path"
    part_dev_map["preloader"] = "/dev/block/sda"
    part_dev_map["preloader2"] ="/dev/block/sdb"

def AddOTA_Items(input_zip, output_zip, isFullOTA):
  common.ZipWriteStr(output_zip, "type.txt", str(isFullOTA))
  ota_scatter = input_zip.read("OTA/ota_scatter.txt")
  common.ZipWriteStr(output_zip, "scatter.txt", ota_scatter)

def SwitchActive(script, from_part, to_part):
  """switch current active partition."""
  partition_type = "EMMC"
  if script.fstab:
    try:
      p = script.fstab["/boot"]
    except:
      print "%s not exists in fstab, try fstab of info_dict" % mount_point
      p = info_dict["fstab"]["/boot"]
    partition_type = common.PARTITION_TYPES[p.fs_type]
  if partition_type == "EMMC":
    script.AppendExtra(('switch_active("%(partition)s", "%(to_part)s");')
        % {'partition':from_part.replace("bootloader","lk"), 'to_part':to_part.replace("bootloader","lk")})
  if partition_type == "MTD":
    script.AppendExtra(('switch_active("%(partition)s", "%(to_part)s");')
        % {'partition':from_part.replace("bootloader","uboot"), 'to_part':to_part.replace("bootloader","uboot")})

def SwitchStage(script, stage_str):
  script.AppendExtra('set_mtupdate_stage("%(stagefile)s", "%(stage_str)s");' % {'stagefile':mtStageFile, 'stage_str':stage_str})

def WriteRawImage2(script, partition, fn, info_dict, mapfn=None):
  """Write the given package file into the given MTD partition."""
  if partition in part_dev_map.keys():
    partition_type = "EMMC"
    if script.fstab:
      try:
        p = script.fstab["/boot"]
      except:
        print "%s not exists in fstab, try fstab of info_dict" % mount_point
        p = info_dict["fstab"]["/boot"]
      partition_type = common.PARTITION_TYPES[p.fs_type]
      args = {'device': p.device, 'fn': fn}
    if partition_type == "MTD":
      script.AppendExtra(
          'write_raw_image(package_extract_file("%(fn)s"), "/dev/preloader");'
          % args)
    else:
      if p.fs_type.upper() == "EMMC":
        if partition == 'preloader':
          script.AppendExtra(
            ('if get_storage_type() then \n'
             ' assert(package_extract_file("preloader_ufs.img", "%(partition)s"));\n'
             'else \n'
             ' assert(set_emmc_writable("%(force_ro)s"),\n'
             '        package_extract_file("preloader_emmc.img", "/dev/block/mmcblk0boot0"));\n'
             'endif;')
            % {'partition': part_dev_map[partition], 'force_ro': force_ro_dev_map[partition]})
        else:
          script.AppendExtra(
            ('if get_storage_type() then \n'
             ' assert(package_extract_file("preloader_ufs.img", "%(partition)s"));\n'
             'else \n'
             ' assert(set_emmc_writable("%(force_ro)s"),\n'
             '        package_extract_file("preloader_emmc.img", "/dev/block/mmcblk0boot1"));\n'
             'endif;')
            % {'partition': part_dev_map[partition], 'force_ro': force_ro_dev_map[partition]})
      else:
        raise ValueError(
            "Preloader don't know how to write \"%s\" partitions" % p.fs_type)
  else:
    mount_point = "/"+partition
    if script.fstab:
      try:
        p = script.fstab[mount_point]
      except:
        print "%s not exists in fstab, try fstab of info_dict" % mount_point
        p = info_dict["fstab"][mount_point]
      partition_type = common.PARTITION_TYPES[p.fs_type]
      args = {'device': p.device, 'fn': fn}
      if partition_type == "EMMC" or partition_type == "MTD":
        if mapfn:
          args["map"] = mapfn
          script.AppendExtra(
              'package_extract_file("%(fn)s", "%(device)s", "%(map)s");' % args)
        else:
          script.AppendExtra(
              'package_extract_file("%(fn)s", "%(device)s");' % args)
      else:
        raise ValueError(
            "don't know how to write \"%s\" partitions" % p.fs_type)

def AddComboPreloader(input_zip, output_zip):
  """add combo preloader"""
  try:
    image = input_zip.read("IMAGES/preloader_emmc.img")
    common.ZipWriteStr(output_zip, "preloader_emmc.img", image)
  except:
    print "no preloader_emmc.img"

  try:
    image = input_zip.read("IMAGES/preloader_ufs.img")
    common.ZipWriteStr(output_zip, "preloader_ufs.img", image)
  except:
    print "no preloader_ufs.img"

def AddOTAImage_Items(source_zip, target_zip, output_zip, info_dict, script):
  try:
    output = target_zip.read("OTA/ota_update_list.txt")
  except:
    print "update_img_list not found"
    return
  storage_type="EMMC"
  td_pair = common.GetTypeAndDevice("/boot", info_dict)
  if not td_pair:
    return
  storage_type = td_pair[0]

  isBackupImgExist = 0
  isFirstRun = 0
  part_list = []

  general_img_list = []
  loader_img_list = []
  last_update_img_list = []
  for line in output.split("\n"):
    if not line: continue
    columns = line.split()
    try:
      img_read = target_zip.read("IMAGES/%s" % columns[0])
    except:
      try:
          img_read = target_zip.read("RADIO/%s" % columns[0])
      except:
          print "read image %s fail, remove from update list" % columns[0]
          continue
    if len(columns) == 2:
      if columns[1] in post_update_img_list:
        print "Move update %s.img to the last" % columns[1]
        common.ZipWriteStr(output_zip, columns[0], img_read)
        last_update_img_list.append(columns[1])
      elif source_zip is not None and columns[1] in incremental_update_raw_img_list:
        print "%s uses incremental update" % columns[1]
        general_img_list.append(columns[:2])
      else:
        general_img_list.append(columns[:2])
        common.ZipWriteStr(output_zip, columns[0], img_read)
    elif len(columns) == 3:
      loader_img_list.append(columns[:3])
      common.ZipWriteStr(output_zip, columns[0], img_read)
    else:
      print "incorrect format in ota_update_list.txt"
      return

  script.AppendExtra('show_mtupdate_stage("%s");' % mtStageFile)

  for img_name, mount_point in general_img_list:
    if general_img_list.index([img_name, mount_point]) == 0:
      script.AppendExtra('ifelse (\nless_than_int(get_mtupdate_stage("%s"), "1") ,\n(' % mtStageFile)
      script.AppendExtra('ui_print("start to update general image");');
    if source_zip is not None and mount_point in incremental_update_raw_img_list:
      print ("Handle update incremental raw %s.img img_name %s" % (mount_point,img_name))
      Inremental_raw_img(source_zip, target_zip,output_zip, mount_point, img_name, info_dict, script)
    else:
      WriteRawImage2(script, mount_point, img_name, info_dict)
  if len(general_img_list) > 0:
    SwitchStage(script, "1")
    script.AppendExtra('),\nui_print("general images are already updated");\n);')

  if len(loader_img_list) > 0:
    for img_name, mount_point, backup_mount_point in loader_img_list:
      if loader_img_list.index([img_name, mount_point, backup_mount_point]) == 0:
        script.AppendExtra('ifelse (\nless_than_int(get_mtupdate_stage("%s"), "3") ,\n(' % mtStageFile)
        script.AppendExtra('if less_than_int(get_mtupdate_stage("%s"), "2") then\n' % mtStageFile)
        script.AppendExtra('ui_print("start to update alt loader image");');
      WriteRawImage2(script, backup_mount_point, img_name, info_dict)
    SwitchStage(script, "2")
    script.AppendExtra('endif;\n')
    for img_name, mount_point, backup_mount_point in loader_img_list:
      SwitchActive(script, mount_point, backup_mount_point)
    SwitchStage(script, "3")
    script.AppendExtra('),\nui_print("alt loder images are already updated");\n);')

    for img_name, mount_point, backup_mount_point in loader_img_list:
      if loader_img_list.index([img_name, mount_point, backup_mount_point]) == 0:
        script.AppendExtra('ifelse (\nless_than_int(get_mtupdate_stage("%s"), "5") ,\n(' % mtStageFile)
        script.AppendExtra('if less_than_int(get_mtupdate_stage("%s"), "4") then\n' % mtStageFile)
        script.AppendExtra('ui_print("start to update main loader image");');
      WriteRawImage2(script, mount_point, img_name, info_dict)
    SwitchStage(script, "4")
    script.AppendExtra('endif;\n')
    for img_name, mount_point, backup_mount_point in loader_img_list:
      SwitchActive(script, backup_mount_point, mount_point)
    script.AppendExtra('),\nui_print("main loader images are already updated");\n);')

  script.AppendExtra('delete("%s");' % mtStageFile)
  for mount_point in last_update_img_list:
    WriteRawImage2(script, mount_point, mount_point+".img", info_dict)
  script.AppendExtra('post_ota_action();')

def get_raw_img(zip_file,img_name):
  raw_img_name_path = "IMAGES/"+ img_name
  namelist = zip_file.namelist()
  if raw_img_name_path in namelist:
    return common.File(raw_img_name_path,zip_file.read(raw_img_name_path))
  else:
    raw_img_name_path = "RADIO/"+ img_name
    if raw_img_name_path in namelist:
      return common.File(raw_img_name_path,zip_file.read(raw_img_name_path))
    else:
      print("get raw image %s fail"%img_name)
      return None

class Patch_info(object):
  def __init__(self, name,source_raw_img, target_raw_img, raw_img_type,raw_img_device, updating_raw_img, include_full_raw_img, by_id):
    self.name = name
    self.source_raw_img = source_raw_img
    self.target_raw_img = target_raw_img
    self.raw_img_type = raw_img_type
    self.raw_img_device = raw_img_device
    self.updating_raw_img = updating_raw_img
    self.include_full_raw_img = include_full_raw_img
    by_id.append(self)

def _Verify_raw_img(source_zip, target_zip,output_zip, mount_point, img_name, info_dict, script):
  # Check the required cache size (i.e. stashed blocks).
  size = []

  include_full_raw_img = False
  source_raw_img = get_raw_img(source_zip,img_name)
  target_raw_img = get_raw_img(target_zip,img_name)
  if not target_raw_img:
    print("Warning: cannot find %s in target_zip, skip update %s" %(img_name, img_name))
    return
  if not source_raw_img :
    print("Warning: cannot find %s in source_zip, full rawdata" %(img_name))
    include_full_raw_img = True
    updating_raw_img = True
  else:
    updating_raw_img = (source_raw_img.data != target_raw_img.data)

  if updating_raw_img:
    raw_img_type, raw_img_device = common.GetTypeAndDevice(
        "/"+ mount_point, info_dict)
    if not include_full_raw_img:
      d = common.Difference(target_raw_img, source_raw_img, "bsdiff")
      _, _, d = d.ComputePatch()
      if d is None:
        include_full_raw_img = True
        common.ZipWriteStr(output_zip, img_name, target_raw_img.data)
      else:
        include_full_raw_img = False

        print("raw img  target: %d  source: %d  diff: %d" % (
            target_raw_img.size, source_raw_img.size, len(d)))

        common.ZipWriteStr(output_zip, "patch/"+img_name+".p", d)

        script.PatchCheck("%s:%s:%d:%s:%d:%s" %
                          (raw_img_type, raw_img_device,
                           source_raw_img.size, source_raw_img.sha1,
                           target_raw_img.size, target_raw_img.sha1),
                           source_raw_img.sha1, target_raw_img.sha1)
        size.append(target_raw_img.size)
    global patch_info_list
    Patch_info(img_name,source_raw_img, target_raw_img, raw_img_type,raw_img_device, updating_raw_img, include_full_raw_img,patch_info_list)
  if size:
    script.CacheFreeSpaceCheck(max(size))

def Verify_raw_img(source_zip, target_zip, output_zip, info_dict, script):
  try:
    output = target_zip.read("OTA/ota_update_list.txt")
  except:
    print "update_img_list not found"
    return
  storage_type="EMMC"
  td_pair = common.GetTypeAndDevice("/boot", info_dict)
  if not td_pair:
    return

  general_img_list = []
  for line in output.split("\n"):
    if not line: continue
    columns = line.split()
    try:
      img_read = target_zip.read("IMAGES/%s" % columns[0])
    except:
      try:
        img_read = target_zip.read("RADIO/%s" % columns[0])
      except:
        print "read image %s fail, remove from update list" % columns[0]
        continue
    if len(columns) == 2:
      general_img_list.append(columns[:2])

  for mount_point in incremental_update_raw_img_list:
    if source_zip is not None and mount_point not in post_update_img_list:
      print "Verify incremental raw %s.img " % (mount_point)
      _Verify_raw_img(source_zip, target_zip,output_zip, mount_point, mount_point+".img", info_dict, script)

def Inremental_raw_img(source_zip, target_zip,output_zip, mount_point, img_name, info_dict, script):
  for patch_info in patch_info_list:
    #print "patch_info.name is %s len is %d" %(patch_info.name,len(patch_info_list))
    if patch_info.name == img_name:
      if patch_info.updating_raw_img:
        if patch_info.include_full_raw_img:
          print("%s image changed; including full." % (mount_point))
          script.Print("Installing {} image...".format(mount_point))
          script.WriteRawImage("/"+mount_point, img_name)
        else:
          # Produce the boot image by applying a patch to the current
          # contents of the boot partition, and write it back to the
          # partition.
          print("%s image changed; including patch." % (mount_point))
          script.Print("Patching {} image...".format(mount_point))
          script.ShowProgress(0.1, 10)
          script.ApplyPatch("%s:%s:%d:%s:%d:%s"
                            % (patch_info.raw_img_type, patch_info.raw_img_device,
                               patch_info.source_raw_img.size, patch_info.source_raw_img.sha1,
                               patch_info.target_raw_img.size, patch_info.target_raw_img.sha1),
                              "-",
                              patch_info.target_raw_img.size, patch_info.target_raw_img.sha1,
                              patch_info.source_raw_img.sha1, "patch/"+img_name+".p")
      else:
        print("%s image unchanged; skipping.", mount_point)
    else:
	  continue

def FullOTA_InstallEnd(self):
  input_zip = self.input_zip
  input_version= self.input_version
  output_zip = self.output_zip
  script = self.script
  input_tmp = self.input_tmp
  metadata = self.metadata
  info_dict = self.info_dict
  source_zip = None

  # add OTA information
  AddOTA_Items(input_zip, output_zip, 1)

  # Check bootloader path by property in porp.defaut
  check_bootloader_path(input_zip)

  # add extra images to upgrade
  AddOTAImage_Items(source_zip, input_zip, output_zip, info_dict, script)

  # add combo preloader
  AddComboPreloader(input_zip, output_zip)

def IncrementalOTA_InstallEnd(self):
  script = self.script
  source_version = self.source_version
  source_zip = self.source_zip
  target_zip = self.target_zip
  target_version = self.target_version
  output_zip = self.output_zip
  script = self.script
  metadata = self.metadata
  info_dict= self.info_dict

  tgt_info_dict = common.LoadInfoDict(target_zip)

  # add OTA information
  AddOTA_Items(target_zip, output_zip, 0)

  # Check bootloader path by property in porp.defaut
  check_bootloader_path(target_zip)

  # add extra images to upgrade
  AddOTAImage_Items(source_zip, target_zip, output_zip, tgt_info_dict, script)

  # add combo preloader
  AddComboPreloader(target_zip, output_zip)

def IncrementalOTA_VerifyEnd(self):
  script = self.script
  source_version = self.source_version
  source_zip = self.source_zip
  target_zip = self.target_zip
  target_version = self.target_version
  output_zip = self.output_zip
  script = self.script
  metadata = self.metadata
  info_dict= self.info_dict

  tgt_info_dict = common.LoadInfoDict(target_zip)
  Verify_raw_img(source_zip, target_zip, output_zip, tgt_info_dict, script)
