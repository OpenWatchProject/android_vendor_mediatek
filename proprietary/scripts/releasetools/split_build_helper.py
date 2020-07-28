#!/usr/bin/env python
__author__ = 'mtk'

import os
import sys
import argparse
import subprocess

def perform_parse(filename, option):
	value = ''
	with open(filename, 'r') as f:
		for l in f.readlines():
			line = l.strip()
			if line.startswith(option + ' := '):
				value = line[len(option + ' := '):]
			elif line.startswith(option + ' = '):
				value = line[len(option + ' = '):]
	return value

def perform_find(target_product, target_build_variant, py_args, make_args, make_goals, single_out):
	dirlist = ['device']
	prune = ['.repo', '.git']
	pruneleaves = set(map(lambda x: os.path.split(x)[1], prune))
	filenames = ['AndroidProducts.mk', 'device.mk', 'BoardConfig.mk']
	product_makefile = ''
	system_project = ''
	vendor_project = ''
	system_product = ''
	vendor_product = ''
	support_split_build = True
	for rootdir in dirlist:
		for root, dirs, files in os.walk(rootdir):
			check_prune = False
			for d in dirs:
				if d in pruneleaves:
					check_prune = True
					break
			if check_prune:
				i = 0
				while i < len(dirs):
					if dirs[i] in prune:
						del dirs[i]
					else:
						i += 1
			for filename in filenames:
				if filename in files:
					del dirs[:]
			if root in ['device/mediatek/common', \
				'device/mediatek/system/common', \
				'device/mediatek/vendor/common', \
				'device/mediatek/build', \
				'device/mediatek/config', \
				'device/mediatek/security', \
				'device/mediatek/sepolicy']:
				del dirs[:]
			if (target_product + '.mk' in files) and target_product.startswith('full_'):
				product_makefile = os.path.join(root, target_product + '.mk')
				system_project = perform_parse(product_makefile, 'SYS_TARGET_PROJECT')
				vendor_project = perform_parse(product_makefile, 'MTK_TARGET_PROJECT')
				support = perform_parse(product_makefile, 'support_split_build')
				if support == 'false':
					support_split_build = False
				if system_project != '':
					system_product = 'sys_' + system_project
					vendor_product = 'vnd_' + vendor_project

	commands = []
	if (system_product != '') and (vendor_product != '') and support_split_build:
		command1  = 'source build/envsetup.sh'
		if single_out:
			out_sys = 'out'
		else:
			out_sys = 'out_sys'
			command1 += ' && ' + 'export OUT_DIR=' + out_sys
		command1 += ' && ' + 'lunch ' + system_product + '-' + target_build_variant
		if make_goals != '':
			command1 += ' && ' + 'make' + make_goals
		command1 += ' && ' + 'make' + make_args + ' sys_images'
		if target_product.endswith('_uasan'):
			command1 += ' && ' + 'make' + make_args + ' SANITIZE_TARGET=address' + ' sys_images'
		commands.append(command1)
		command2  = 'source build/envsetup.sh'
		if single_out:
			out_vnd = 'out'
		else:
			out_vnd = 'out'
			command2 += ' && ' + 'export OUT_DIR=' + out_vnd
		command2 += ' && ' + 'lunch ' + vendor_product + '-' + target_build_variant
		if make_goals != '':
			command2 += ' && ' + 'make' + make_goals
		command2 += ' && ' + 'make' + make_args + ' vnd_images krn_images'
		if target_product.endswith('_uasan'):
			command2 += ' && ' + 'make' + make_args + ' SANITIZE_TARGET=address' + ' vnd_images krn_images'
		commands.append(command2)
		command3  = 'python ' + out_sys + '/target/product/' + system_project + '/images/split_build.py'
		command3 += ' --system-dir ' + out_sys + '/target/product/' + system_project + '/images'
		command3 += ' --vendor-dir ' + out_vnd + '/target/product/' + vendor_project + '/images'
		command3 += ' --kernel-dir ' + out_vnd + '/target/product/' + vendor_project + '/images'
		command3 += ' --output-dir ' + out_vnd + '/target/product/' + vendor_project + '/merged'
		command3 += py_args
		commands.append(command3)
	else:
		command1  = 'source build/envsetup.sh'
		command1 += ' && ' + 'export OUT_DIR=out'
		command1 += ' && ' + 'lunch ' + target_product + '-' + target_build_variant
		if make_goals != '':
			command1 += ' && ' + 'make' + make_goals
		command1 += ' && ' + 'make'
		if target_product.endswith('_uasan'):
			command1 += ' && ' + 'make' + ' SANITIZE_TARGET=address'
		commands.append(command1)
	return commands

def main():
	parser = argparse.ArgumentParser(description="Helper for split command build")
	parser.add_argument('--run', required=False, action='store_true', dest='run')
	parser.add_argument('--zip', required=False, action='store_true', dest='zip')
	parser.add_argument('--run-with-installclean', required=False, action='store_true', dest='installclean')
	parser.add_argument('--run-with-out', required=False, action='store_true', dest='run_out')
	parser.add_argument('lunch', type=str, help='target_product-mode, such as full_k71v1_64_bsp-userdebug')
	args = parser.parse_args()
	target = args.lunch.split('-')
	py_args = ''
	make_args = ''
	make_goals = ''
	if args.installclean:
		make_goals += ' installclean'
	if args.zip:
		py_args += ' --targetfiles'
	commands = perform_find(target[0], target[1], py_args, make_args, make_goals, args.run_out)
	if args.run or args.run_out or args.installclean:
		for command in commands:
			print(command)
			res = subprocess.Popen(command, shell=True, executable='/bin/bash')
			out, err = res.communicate()
			if err or (res.returncode != 0):
				print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
				return 1
	else:
		print('\n'.join(commands))
	return 0

if __name__ == '__main__':
	result = main()
	sys.exit(result)
