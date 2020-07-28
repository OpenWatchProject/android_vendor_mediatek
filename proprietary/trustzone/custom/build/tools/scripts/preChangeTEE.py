#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2015 - 2016 MediaTek Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

import argparse
import os
import sys
import xml.etree.ElementTree as ET

DEBUG=False

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = "\033[1m"

MANIFEST1 = '/.repo/manifests/sub.xml'
MANIFEST2 = '/.repo/manifest.xml'

def infog(msg):
    print OKGREEN + msg + ENDC

def info(msg):
    print OKBLUE + msg + ENDC

def warn(msg):
    print WARNING + msg + ENDC

def err(msg):
    print FAIL + msg + ENDC

def upgrade_preloader_project_makefile(project, base_project):
	print "Upgrade Preloader Project Makefile ..."

	output = ""
	path = "./../../../../../../../.."

	if os.path.exists(path + MANIFEST1):
		sub_xml = path + MANIFEST1
	elif os.path.exists(path + MANIFEST2):
		sub_xml = path + MANIFEST2
	else:
		print("error: Cannot find manifest.xml\n")

	tree = ET.parse(sub_xml)
	root = tree.getroot()

	for child in root.iterfind('project'):
		preloader_path = child.get('path')
		if 'preloader' in preloader_path:
			print("get preloader path: " + str(preloader_path))
			break

	makefile = "%s/%s/custom/%s/%s.mk" % (path, preloader_path, project, project)
	if (not os.path.exists(makefile)) and (base_project != ""):
		makefile = "%s/%s/custom/%s/%s.mk" % (path, preloader_path, base_project, base_project)

	if DEBUG:
		infog("PreloaderConfig=%s exists=%s" % (makefile, os.path.exists(makefile)))

	with open(makefile, 'r') as f:
		for line in f:
			#print line
			if line.find("CFG_TEE_SUPPORT") != -1:
				line = line.replace("CFG_TEE_SUPPORT", "MTK_TEE_SUPPORT").replace("0", "no").replace("1", "yes")
				print line
			elif line.find("CFG_TRUSTONIC_TEE_SUPPORT") != -1:
				line = line.replace("CFG_TRUSTONIC_TEE_SUPPORT", "TRUSTONIC_TEE_SUPPORT")
				print line
			elif line.find("CFG_MICROTRUST_TEE_SUPPORT") != -1:
				line = line.replace("CFG_MICROTRUST_TEE_SUPPORT", "MICROTRUST_TEE_SUPPORT")
				print line
			elif line.find("CFG_MICROTRUST_TEE_LITE_SUPPORT") != -1:
				line = line.replace("CFG_MICROTRUST_TEE_LITE_SUPPORT", "MICROTRUST_TEE_LITE_SUPPORT")
				print line
			elif line.find("CFG_WATCHDATA_TEE_SUPPORT") != -1:
				line = line.replace("CFG_WATCHDATA_TEE_SUPPORT", "WATCHDATA_TEE_SUPPORT")
				print line
			elif line.find("CFG_GOOGLE_TRUSTY_SUPPORT") != -1:
				line = line.replace("CFG_GOOGLE_TRUSTY_SUPPORT", "MTK_GOOGLE_TRUSTY_SUPPORT")
				print line
			elif line.find("CFG_TRUSTKERNEL_TEE_SUPPORT") != -1:
				line = line.replace("CFG_TRUSTKERNEL_TEE_SUPPORT", "TRUSTKERNEL_TEE_SUPPORT")
				print line

			output += line

	if DEBUG:
		infog(output)

	with open(makefile, 'w') as f:
		f.write(output)

	print "Upgrade Preloader Project Makefile ... DONE"

def main(args):
	upgrade_preloader_project_makefile(args.project, args.base_project)
	pass

if __name__ == "__main__":

	parser = argparse.ArgumentParser()
	parser.add_argument("--project", help="MTK Project Name")
	parser.add_argument("--base_project", help="MTK Base Project Name")
	args = parser.parse_args()

	if args.project:
		main(args)
	else:
		print "Usage: %s -h or --help." % sys.argv[0]
