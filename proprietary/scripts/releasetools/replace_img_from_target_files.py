#!/usr/bin/env python
#
# Copyright (C) 2014 The Android Open Source Project
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

"""
Given a target-files zipfile that does contain images (ie, does
have an IMAGES/ top-level subdirectory), replace the images to
the output dir.

Usage:  replace_img_from_target_files target_files output
"""

import sys

if sys.hexversion < 0x02070000:
  print >> sys.stderr, "Python 2.7 or newer is required."
  sys.exit(1)

import errno
import os
import re
import shutil
import subprocess
import tempfile
import zipfile

image_replace_list = ["boot.img","system.img","recovery.img",
                      "userdata.img","custom.img","vendor.img","super.img",
                      "odm.img","vbmeta.img","dtbo.img","odmdtbo.img",
                      "vbmeta_vendor.img", "vbmeta_system.img", "product.img"]

# missing in Python 2.4 and before
if not hasattr(os, "SEEK_SET"):
  os.SEEK_SET = 0

def main(argv):

  if len(argv) != 2:
    sys.exit(1)

  if not os.path.exists(argv[0]):
    print "Target file:%s is invalid" % argv[0]
    sys.exit(1)

  if not os.path.exists(argv[1]):
    print "Output dir:%s is invalid" % argv[1]
    sys.exit(1)

  zf = zipfile.ZipFile(argv[0], 'r')

  for img in zf.namelist():
    if img.find("IMAGES/") != -1:
      if img.find(".img") != -1:
        data = zf.read(img)
        name = img.replace("IMAGES/", '')
        if name == "android.fixup.img":
          name = "system.img"
        if name in image_replace_list:
          verified_name = os.path.splitext(name)[0] + "-verified" + os.path.splitext(name)[1]
          if os.path.isfile(os.path.join(argv[1], verified_name)):
            name = os.path.splitext(name)[0] + "-verified" + os.path.splitext(name)[1]
          print "Replace %s" % name
          name = '/'.join((argv[1], name))
          file = open(name, "w")
          file.write(data)
          file.close()

if __name__ == '__main__':
  main(sys.argv[1:])
