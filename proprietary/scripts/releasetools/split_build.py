#!/usr/bin/env python
__author__ = 'mtk'

import argparse
import os
import re
import shutil
import subprocess
import sys
import zipfile

def get_projectConfiguration(prjConfig):
	pattern = [re.compile("^([^=\s]+)\s*=\s*(.+)$"), re.compile("^([^=\s]+)\s*=$")]
	config = {}
	with open(prjConfig, 'r') as ff:
		for line in ff.readlines():
			result = (filter(lambda x: x, [x.search(line) for x in pattern]) or [None])[0]
			if not result: continue
			name, value = None, None
			if len(result.groups()) == 0: continue
			name = result.group(1)
			try:
				value = result.group(2)
			except IndexError:
				value = ""
			config[name] = value.strip()
	return config

def check_option_dependency(system_config, vendor_config):
	system_options = get_projectConfiguration(system_config)
	if not system_options:
		return 0
	vendor_options = get_projectConfiguration(vendor_config)
	check_list_same_name = [
		'BUILD_AGO_GMS',
		'FPGA_EARLY_PORTING',
		'MTK_AB_OTA_UPDATER',
		'MTK_AUDIO',
		'MTK_BASIC_PACKAGE',
		'MTK_BOOTCTRL_VERSION',
		'MTK_BSP_PACKAGE',
		'MTK_ENABLE_NATIVE_ASAN',
		'MTK_GMO_RAM_OPTIMIZE',
		'MTK_GMO_ROM_OPTIMIZE',
		'MTK_HIDL_PROCESS_CONSOLIDATION_ENABLED',
		'MTK_OPEN_PACKAGE',
		'OPTR_SPEC_SEG_DEF',
		'MTK_DYNAMIC_PARTITION_SUPPORT',
		'MTK_PRODUCT_LINE',
	]
	check_list_mssi_name = [
		'MTK_BT_INTEROP_EXTENSION',
		'MTK_EXCHANGE_SUPPORT',
		'MTK_LOG_CUSTOMER_SUPPORT',
		'MTK_SIGNATURE_CUSTOMIZATION',
		'MTK_TEE_TRUSTED_UI_SUPPORT',
		'MTK_USER_SPACE_DEBUG_FW'
	]
	result = 0
	for option in check_list_same_name:
		system_value = ''
		vendor_value = ''
		if option in system_options:
			system_value = system_options[option]
		if option in vendor_options:
			vendor_value = vendor_options[option]
		if system_value != vendor_value:
			print('ERROR: feature option ' + option + ' not align in SystemConfig.mk and ProjectConfig.mk')
			print(system_config + ': ' + option + ' = ' + system_value)
			print(vendor_config + ': ' + option + ' = ' + vendor_value)
			print("\n")
			result += 1
	for option in check_list_mssi_name:
		system_value = ''
		vendor_value = ''
		if 'MSSI_'+option in system_options:
			system_value = system_options['MSSI_'+option]
		if option in vendor_options:
			vendor_value = vendor_options[option]
		if system_value != vendor_value:
			print('ERROR: feature option ' + option + ' not align in SystemConfig.mk and ProjectConfig.mk')
			print(system_config + ': ' + 'MSSI_' + option + ' = ' + system_value)
			print(vendor_config + ': ' + option + ' = ' + vendor_value)
			print("\n")
			result += 1
	return result

def extract_all_with_permission(input_zip, output_dir):
	with zipfile.ZipFile(input_zip) as zf:
		for info in zf.infolist():
			extracted_path = zf.extract(info, output_dir)
			if info.create_system == 3:
				unix_attributes = info.external_attr >> 16
				if unix_attributes:
					os.chmod(extracted_path, unix_attributes)

def main():
	parser = argparse.ArgumentParser(description="Preprocess for split target-files-package")
	parser.add_argument('--system-dir', type=str, required=True, dest='system_dir')
	parser.add_argument('--product-dir', type=str, required=False, dest='product_dir')
	parser.add_argument('--vendor-dir', type=str, required=True, dest='vendor_dir')
	parser.add_argument('--kernel-dir', type=str, required=False, dest='kernel_dir')
	parser.add_argument('--output-dir', type=str, required=True, dest='output_dir')
	parser.add_argument('--certs-dir', type=str, required=False, dest='certs_dir')
	parser.add_argument('--otapackage', default=False, action='store_true')
	parser.add_argument('--targetfiles', default=False, action='store_true')
	parser.add_argument('--symbols', default=False, action='store_true')
	parser.add_argument('--checkonly', default=False, action='store_true')
	args = parser.parse_args()
	current_script_dir = os.path.dirname(__file__)
	# check dependency
	system_config = os.path.join(args.system_dir, 'SystemConfig.mk')
	vendor_config = os.path.join(args.vendor_dir, 'ProjectConfig.mk')
	if not os.path.exists(system_config):
		print('ERROR: system config ' + system_config + ' not found')
		return 1
	if not os.path.exists(vendor_config):
		print('ERROR: vendor config ' + vendor_config + ' not found')
		return 1
	result = check_option_dependency(system_config, vendor_config)
	if result != 0:
		return result
	if args.checkonly:
		return 0
	# check exist
	system_zip = os.path.abspath(os.path.join(args.system_dir, 'sys.target_files.zip'))
	if args.product_dir is None:
		product_zip = ''
	elif os.path.exists(os.path.abspath(os.path.join(args.product_dir, 'prd.target_files.zip'))):
		product_zip = os.path.abspath(os.path.join(args.product_dir, 'prd.target_files.zip'))
	else:
		product_zip = os.path.abspath(os.path.join(args.product_dir, 'sys.target_files.zip'))
	vendor_zip = os.path.abspath(os.path.join(args.vendor_dir, 'vnd.target_files.zip'))
	if args.kernel_dir is None:
		kernel_zip = os.path.abspath(os.path.join(args.vendor_dir, 'krn.target_files.zip'))
		kernel_rsc_xml = os.path.join(args.vendor_dir, 'krn.rsc.xml')
	else:
		kernel_zip = os.path.abspath(os.path.join(args.kernel_dir, 'krn.target_files.zip'))
		kernel_rsc_xml = os.path.join(args.kernel_dir, 'krn.rsc.xml')
	system_otatools_zip = os.path.join(args.system_dir, 'sys.otatools.zip')
	vendor_otatools_zip = os.path.join(args.vendor_dir, 'vnd.otatools.zip')
	system_rsc_xml = os.path.join(args.system_dir, 'sys.rsc.xml')
	vendor_rsc_xml = os.path.join(args.vendor_dir, 'vnd.rsc.xml')
	user_rsc_xml = os.path.join(args.vendor_dir, 'usr.rsc.xml')
	vendor_dist_zip = os.path.abspath(os.path.join(args.vendor_dir, 'vnd.dist_files.zip'))
	otatools_zip = os.path.join(args.system_dir, 'otatools.zip')
	if (system_zip != '') and (not os.path.exists(system_zip)):
		print('ERROR: system target-files-package ' + system_zip + ' not found')
		return 1
	if (product_zip != '') and (not os.path.exists(product_zip)):
		print('ERROR: product target-files-package ' + product_zip + ' not found')
		return 1
	if (vendor_zip != '') and (not os.path.exists(vendor_zip)):
		print('ERROR: vendor target-files-package ' + vendor_zip + ' not found')
		return 1
	if (kernel_zip != '') and (not os.path.exists(kernel_zip)):
		print('ERROR: kernel target-files-package ' + kernel_zip + ' not found')
		return 1
	if not os.path.exists(system_otatools_zip):
		print('ERROR: system otatools ' + system_otatools_zip + ' not found')
		return 1
	if not os.path.exists(vendor_otatools_zip):
		print('ERROR: vendor otatools ' + vendor_otatools_zip + ' not found')
		return 1
	if not os.path.exists(vendor_dist_zip):
		print('ERROR: vendor dist package ' + vendor_dist_zip + ' not found')
		return 1
	if not os.path.exists(otatools_zip):
		print('ERROR: AOSP otatools ' + otatools_zip + ' not found')
		return 1
	# otatools
	result = 0
	temp_dir = os.path.join(args.output_dir, 'temp')
	print('temporary dir is ' + temp_dir)
	if os.path.exists(temp_dir):
		shutil.rmtree(temp_dir)
	extract_all_with_permission(otatools_zip, temp_dir)
	extract_all_with_permission(system_otatools_zip, temp_dir)
	extract_all_with_permission(vendor_otatools_zip, temp_dir)
	post_process_target_files_py = os.path.abspath(os.path.join(temp_dir, 'releasetools', 'post_process_target_files.py'))
	rsc_xml_gen_py = os.path.abspath(os.path.join(temp_dir, 'releasetools', 'rsc_xml_gen.py'))
	final_rsc_xml = ''
	if not os.path.exists(rsc_xml_gen_py):
		print('NOTE: ' + rsc_xml_gen_py + ' not found')
	elif not os.path.exists(system_rsc_xml):
		print('NOTE: ' + system_rsc_xml + ' not found')
	elif not os.path.exists(vendor_rsc_xml):
		print('NOTE: ' + vendor_rsc_xml + ' not found')
	elif not os.path.exists(kernel_rsc_xml):
		print('NOTE: ' + kernel_rsc_xml + ' not found')
	elif not os.path.exists(user_rsc_xml):
		print('NOTE: ' + user_rsc_xml + ' not found')
	elif os.path.getsize(vendor_rsc_xml) < 120:
		print('NOTE: ' + vendor_rsc_xml + ' is dummy')
	else:
		final_rsc_xml = os.path.join(temp_dir, 'rsc.xml')
		command = 'python ' + rsc_xml_gen_py + ' -final ' + system_rsc_xml + ' ' + vendor_rsc_xml + ' ' + kernel_rsc_xml + ' ' + user_rsc_xml + ' ' + final_rsc_xml
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if err or (res.returncode != 0):
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	if result == 0:
		command = 'python ' + post_process_target_files_py + \
			' --system-target-files ' + system_zip + \
			' --vendor-target-files ' + vendor_zip + \
			' --kernel-target-files ' + kernel_zip + \
			' --vendor-dist-files ' + vendor_dist_zip + \
			' --output-dir ' + args.output_dir
		if product_zip != '':
			command += ' --product-target-files ' + product_zip
		if final_rsc_xml != '':
			command += ' --rsc-xml ' + final_rsc_xml
		if True:
			command += ' --keep-images'
		if args.certs_dir:
			command += ' --sign-apk-certs ' + args.certs_dir
			command += ' --keep-target-files'
		elif args.targetfiles:
			command += ' --keep-target-files'
		if args.otapackage:
			command += ' --keep-otapackage'
		if args.symbols:
			command += ' --keep-symbols'
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if err or (res.returncode != 0):
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	# clean up
	if result == 0:
		shutil.rmtree(temp_dir)
	return result

if __name__ == '__main__':
	result = main()
	sys.exit(result)

