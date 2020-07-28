#! /usr/bin/env python
__author__ = 'mtk'

import os
import sys

def clone_file(src_file, dst_file, old, new):
	print('clone ' + src_file + ' to ' + dst_file)
	dst_path = os.path.dirname(dst_file)
	if not os.path.exists(dst_path):
		os.makedirs(dst_path)
	with open(dst_file, 'w') as o:
		with open(src_file, 'r') as i:
			for r in i.readlines():
				w = r.replace(old, new, -1)
				o.write(w)
	return 0

def clone_mssi(src_path, dst_path):
	src_path = src_path.rstrip('/')
	src_name = os.path.basename(src_path)
	dst_path = dst_path.rstrip('/')
	dst_name = os.path.basename(dst_path)
	prune = ['.git']
	for root, dirs, files in os.walk(src_path):
		for d in prune:
			if d in dirs:
				dirs.remove(d)
		for f in files:
			src_file = os.path.join(root, f)
			src_stem = os.path.relpath(src_file, src_path)
			dst_stem = src_stem.replace(src_name, dst_name, 1)
			dst_file = os.path.join(dst_path, dst_stem)
			res = clone_file(src_file, dst_file, src_name, dst_name)
			if res != 0:
				return res
	return 0

def main(argv):
	if len(argv) == 3:
		sys.exit(clone_mssi(argv[1], argv[2]))
	else:
		print(argv[0] + ' device/mediatek/system/mssi_old device/mediatek/system/mssi_new')
		sys.exit(1)

if __name__ == "__main__":
	main(sys.argv)
