#!/usr/bin/env python
__author__ = 'mtk'

import argparse
import os
import shutil
import subprocess
import sys
import zipfile

# AOSP releasetools
import common

# always keep images for MTK Flashtool
keep_img = True

def prepare_vintf_manifest_from_target_files(temp_dir, system_zip, product_zip, vendor_zip):
	if system_zip is None:
		system_zip = ''
	if product_zip is None:
		product_zip = ''
	if vendor_zip is None:
		vendor_zip = ''
	system_matrix = ''
	vendor_matrix = ''
	system_manifest = ''
	product_manifest = ''
	vendor_manifest = ''
	system_xmls = []
	product_xmls = []
	vendor_xmls = []
	assemble_vintf = os.path.join(temp_dir, 'bin', 'assemble_vintf')
	vintf_temp_dir = os.path.join(temp_dir, 'VINTF_TEMP')
	if not os.path.exists(vintf_temp_dir):
		os.makedirs(vintf_temp_dir)
	if system_zip != '':
		with zipfile.ZipFile(system_zip) as szf:
			for item in szf.infolist():
				if item.filename == 'META/system_matrix.xml':
					system_matrix = item.filename
				elif item.filename == 'META_TEMP/built_system_manifest.xml':
					system_manifest = item.filename
				elif item.filename.startswith('SYSTEM/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					system_xmls.append(item.filename)
				elif product_zip != '':
					continue
				elif item.filename == 'META_TEMP/built_product_manifest.xml':
					product_manifest = item.filename
				elif item.filename.startswith('PRODUCT/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					product_xmls.append(item.filename)
				elif item.filename.startswith('SYSTEM/product/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					product_xmls.append(item.filename)
				else:
					continue
				print('extract_vintf_manifest_from_target_files ' + item.filename)
				szf.extract(item, vintf_temp_dir)
	if product_zip != '':
		with zipfile.ZipFile(product_zip) as pzf:
			for item in pzf.infolist():
				if item.filename == 'META_TEMP/built_product_manifest.xml':
					product_manifest = item.filename
				elif item.filename.startswith('PRODUCT/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					product_xmls.append(item.filename)
				elif item.filename.startswith('SYSTEM/product/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					product_xmls.append(item.filename)
				else:
					continue
				print('extract_vintf_manifest_from_target_files ' + item.filename)
				pzf.extract(item, vintf_temp_dir)
	if vendor_zip != '':
		with zipfile.ZipFile(vendor_zip) as vzf:
			for item in vzf.infolist():
				if item.filename == 'META/vendor_matrix.xml':
					vendor_matrix = item.filename
				elif item.filename == 'META_TEMP/built_vendor_manifest.xml':
					vendor_manifest = item.filename
				elif item.filename.startswith('VENDOR/etc/vintf/manifest/') and item.filename.endswith('.xml'):
					vendor_xmls.append(item.filename)
				else:
					continue
				print('extract_vintf_manifest_from_target_files ' + item.filename)
				vzf.extract(item, vintf_temp_dir)
	current_dir = os.getcwd()
	os.chdir(vintf_temp_dir)
	# verified_assembled_framework_manifest.xml
	verified_assembled_framework_manifest = ''
	if (system_manifest != '') and (vendor_matrix != ''):
		verified_assembled_framework_manifest = os.path.join(vintf_temp_dir, 'verified_assembled_framework_manifest.xml')
		command = assemble_vintf + ' -o ' + verified_assembled_framework_manifest + ' -c ' + vendor_matrix + ' -i ' + system_manifest
		for xml in system_xmls:
			command += ' -i ' + xml
		if product_manifest != '':
			command += ' -i ' + product_manifest
		for xml in product_xmls:
			command += ' -i ' + xml
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			verified_assembled_framework_manifest = ''
	# verified_assembled_vendor_manifest.xml
	verified_assembled_vendor_manifest = ''
	if (vendor_manifest != '') and (system_matrix != ''):
		verified_assembled_vendor_manifest = os.path.join(vintf_temp_dir, 'verified_assembled_vendor_manifest.xml')
		command = assemble_vintf + ' -c ' + system_matrix + ' -i ' + vendor_manifest
		for xml in vendor_xmls:
			command += ' -i ' + xml
		command += ' -o ' + verified_assembled_vendor_manifest
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			verified_assembled_vendor_manifest = ''
	os.chdir(current_dir)
	return verified_assembled_framework_manifest, verified_assembled_vendor_manifest

def prepare_system_target_files(output_dir, output_zip, system_zip, product_zip, rsc_xml, verified_assembled_framework_manifest):
	if system_zip is None:
		system_zip = ''
	if product_zip is None:
		product_zip = ''
	if rsc_xml is None:
		rsc_xml = ''
	system_rsc_file = ''
	system_manifest = ''
	with zipfile.ZipFile(output_zip, 'w', compression=zipfile.ZIP_STORED, allowZip64=True) as ozf:
		product_list = set()
		if product_zip:
			print('prepare system_target_files ' + output_zip + ' from ' + product_zip)
			with zipfile.ZipFile(product_zip, 'r') as izf:
				for item in izf.infolist():
					if item.filename == 'META_TEMP/installed-files-product.txt':
						print('extract ' + item.filename + ' from ' + product_zip)
						izf.extract(item, output_dir)
						os.rename(os.path.join(output_dir, item.filename), os.path.join(output_dir, os.path.basename(item.filename)))
						shutil.rmtree(os.path.join(output_dir, os.path.dirname(item.filename)))
						product_list.add(item.filename)
					elif item.filename.startswith('PRODUCT/') or (item.filename == 'META/product_filesystem_config.txt'):
						buffer = izf.read(item.filename)
						ozf.writestr(item, buffer)
						product_list.add(item.filename)
		if system_zip:
			print('prepare system_target_files ' + output_zip + ' from ' + system_zip)
			with zipfile.ZipFile(system_zip, 'r') as izf:
				for item in izf.infolist():
					if item.filename.endswith('/rsc.xml'):
						system_rsc_file = item.filename
						continue
					elif item.filename in product_list:
						continue
					elif item.filename.startswith('PRODUCT/') and product_zip:
						continue
					elif item.filename.startswith('META_TEMP/installed-files') and item.filename.endswith('.txt'):
						print('extract ' + item.filename + ' from ' + system_zip)
						izf.extract(item, output_dir)
						os.rename(os.path.join(output_dir, item.filename), os.path.join(output_dir, os.path.basename(item.filename)))
						shutil.rmtree(os.path.join(output_dir, os.path.dirname(item.filename)))
					else:
						buffer = izf.read(item.filename)
						ozf.writestr(item, buffer)
						if item.filename == 'META/system_manifest.xml':
							system_manifest = item.filename
		if (verified_assembled_framework_manifest != '') and (system_manifest == ''):
			print('add ' + 'META/system_manifest.xml' + ' from ' + verified_assembled_framework_manifest)
			with open(verified_assembled_framework_manifest, 'r') as ixf:
				buffer = ixf.read()
				ozf.writestr('META/system_manifest.xml', buffer)
		if (system_rsc_file != '') and (rsc_xml != ''):
			print('add ' + system_rsc_file + ' from ' + rsc_xml)
			with open(rsc_xml, 'r') as ixf:
				buffer = ixf.read()
				ozf.writestr(system_rsc_file, buffer)
	return output_zip

def prepare_other_target_files(output_dir, output_zip, vendor_zip, kernel_zip, verified_assembled_vendor_manifest):
	if vendor_zip is None:
		vendor_zip = ''
	if kernel_zip is None:
		kernel_zip = ''
	info_dict = {}
	vendor_manifest = ''
	with zipfile.ZipFile(output_zip, 'w', compression=zipfile.ZIP_STORED, allowZip64=True) as ozf:
		kernel_list = set()
		if kernel_zip != '':
			print('prepare other_target_files ' + output_zip + ' from ' + kernel_zip)
			with zipfile.ZipFile(kernel_zip, 'r') as izf:
				for item in izf.infolist():
					buffer = izf.read(item.filename)
					ozf.writestr(item, buffer)
					kernel_list.add(item.filename)
		if vendor_zip != '':
			print('prepare other_target_files ' + output_zip + ' from ' + vendor_zip)
			with zipfile.ZipFile(vendor_zip, 'r') as izf:
				for item in izf.infolist():
					if item.filename in kernel_list:
						continue
					elif item.filename.startswith('META_TEMP/installed-files') and item.filename.endswith('.txt'):
						print('extract ' + item.filename + ' from ' + vendor_zip)
						izf.extract(item, output_dir)
						os.rename(os.path.join(output_dir, item.filename), os.path.join(output_dir, os.path.basename(item.filename)))
						shutil.rmtree(os.path.join(output_dir, os.path.dirname(item.filename)))
					else:
						buffer = izf.read(item.filename)
						ozf.writestr(item, buffer)
						if item.filename == 'META/misc_info.txt':
							info_dict = common.LoadDictionaryFromLines(buffer.split("\n"))
						elif item.filename == 'META/vendor_manifest.xml':
							vendor_manifest = item.filename
		if (verified_assembled_vendor_manifest != '') and (vendor_manifest == ''):
			print('add ' + 'META/vendor_manifest.xml' + ' from ' + verified_assembled_vendor_manifest)
			with open(verified_assembled_vendor_manifest, 'r') as ixf:
				buffer = ixf.read()
				ozf.writestr('META/vendor_manifest.xml', buffer)
	return output_zip, info_dict

def write_super_image(output_dir, output_zip, build_super_image_py, info_dict):
	if info_dict.get('use_dynamic_partitions') == 'true':
		output_super_img = os.path.join(output_dir, 'super.img')
		command = 'python ' + build_super_image_py + \
			' -v ' + output_zip + ' ' + output_super_img
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			return 1
	return 0

def write_mtk_sign_image(image_name, image_path, work_dir, info_dict):
	result = 0
	board_avb_enable = info_dict.get('avb_enable', 'false')
	mtk_sign_image_version = info_dict.get('mtk_sign_image_version')
	current_dir = os.getcwd()
	os.chdir(work_dir)
	if mtk_sign_image_version == 'v1':
		mtk_base_project = info_dict.get('mtk_base_project', 'MTK_BASE_PROJECT')
		mtk_sec_secro_ac_support = 'yes'
		mtk_nand_page_size = '2K'
		command = 'perl vendor/mediatek/proprietary/scripts/sign-image/SignTool.pl' + \
			' ' + mtk_base_project + \
			' ' + 'MTK_TARGET_PROJECT' + \
			' ' + 'MTK_PATH_CUSTOM' + \
			' ' + mtk_sec_secro_ac_support + \
			' ' + mtk_nand_page_size + \
			' ' + board_avb_enable + \
			' ' + image_path
		if image_name is not None:
			command += ' ' + image_name
		else:
			command += ' ' + 'all'
		if image_name == 'dtbo.img':
			command += ' ' + 'yes'
			shutil.copyfile(os.path.join(image_path, 'dtbo.img'), os.path.join(image_path, 'dtbo-verified.img'))
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	elif mtk_sign_image_version == 'v2':
		envir = os.environ
		envir['PYTHONDONTWRITEBYTECODE'] = 'True'
		envir['BOARD_AVB_ENABLE'] = board_avb_enable
		envir['PRODUCT_OUT'] = image_path
		mtk_platform_dir = info_dict.get('mtk_platform_dir', 'MTK_PLATFORM_DIR')
		command = 'python vendor/mediatek/proprietary/scripts/sign-image_v2/sign_flow.py'
		if image_name is not None:
			command += ' -target ' + image_name
		command += \
			' -env_cfg ' + 'vendor/mediatek/proprietary/scripts/sign-image_v2/env.cfg' + \
			' ' + mtk_platform_dir + \
			' ' + 'mtk_project_name'
		print(command)
		res = subprocess.Popen(command, shell=True, env=envir)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	os.chdir(current_dir)
	return result

def prepare_signed_target_files(output_dir, output_zip, merged_zip, vendor_zip, temp_dir, info_dict):
	has_avb = info_dict.get('avb_enable') == 'true'
	has_odmdtbo = info_dict.get('dtbo_image_name', 'dtbo.img') != 'dtbo.img'
	has_vbmeta_in_boot = False
	if has_avb and (info_dict.get('avb_main_vbmeta_in_boot') == 'true'):
		has_vbmeta_in_boot = True
	images_temp_dir = os.path.join(temp_dir, 'IMAGES_TEMP')
	if not os.path.exists(images_temp_dir):
		os.makedirs(images_temp_dir)
	images_resign = {}
	if has_odmdtbo:
		images_resign['IMAGES/dtbo.img'] = None
	if not has_avb:
		images_resign['IMAGES/boot.img'] = None
		images_resign['IMAGES/recovery.img'] = None
	elif has_vbmeta_in_boot:
		images_resign['IMAGES/boot.img'] = None
		images_resign['IMAGES/vbmeta.img'] = None
	elif True:
		images_resign['IMAGES/recovery-ramdisk.img'] = None
	if images_resign:
		with zipfile.ZipFile(merged_zip, 'r') as izf:
			for item in izf.infolist():
				if item.filename in images_resign:
					izf.extract(item, images_temp_dir)
	if has_odmdtbo and os.path.exists(os.path.join(images_temp_dir, 'IMAGES/dtbo.img')):
		print('prepare signed_target_files: odmdtbo')
		images_resign['IMAGES/odmdtbo.img'] = os.path.join(images_temp_dir, 'IMAGES/dtbo.img')
	if not has_avb:
		print('prepare signed_target_files: AVB disabled')
		if os.path.exists(os.path.join(images_temp_dir, 'IMAGES/boot.img')):
			images_resign['IMAGES/boot.img'] = os.path.join(images_temp_dir, 'IMAGES', 'boot-verified.img')
			result = write_mtk_sign_image('boot.img', os.path.join(images_temp_dir, 'IMAGES'), temp_dir, info_dict)
			if result != 0:
				return result
		if os.path.exists(os.path.join(images_temp_dir, 'IMAGES/recovery.img')):
			images_resign['IMAGES/recovery.img'] = os.path.join(images_temp_dir, 'IMAGES', 'recovery-verified.img')
			result = write_mtk_sign_image('recovery.img', os.path.join(images_temp_dir, 'IMAGES'), temp_dir, info_dict)
			if result != 0:
				return result
	elif has_vbmeta_in_boot:
		print('prepare signed_target_files: append vbmeta in boot')
		images_resign['IMAGES/boot.img'] = os.path.join(images_temp_dir, 'IMAGES/boot.img')
		avbtool = os.path.join(temp_dir, 'bin', info_dict.get('avb_avbtool'))
		command = avbtool + \
			' append_vbmeta_image' \
			' --image ' + images_resign['IMAGES/boot.img'] + \
			' --partition_size ' + info_dict.get('boot_size', '0') + \
			' --vbmeta_image ' + os.path.join(images_temp_dir, 'IMAGES', 'vbmeta.img')
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			return 1
	if os.path.exists(os.path.join(images_temp_dir, 'IMAGES/recovery-ramdisk.img')):
		print('prepare signed_target_files: recovery-ramdisk')
		images_resign['IMAGES/recovery-ramdisk.img'] = os.path.join(images_temp_dir, 'IMAGES', 'recovery-ramdisk-verified.img')
		result = write_mtk_sign_image('recovery-ramdisk.img', os.path.join(images_temp_dir, 'IMAGES'), temp_dir, info_dict)
		if result != 0:
			return result
	if vendor_zip is None:
		vendor_zip = ''
	with zipfile.ZipFile(output_zip, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=True) as ozf:
		if vendor_zip != '':
			print('prepare signed_target_files ' + output_zip + ' from ' + vendor_zip)
			with zipfile.ZipFile(vendor_zip, 'r') as izf:
				for item in izf.infolist():
					merge_zip = False
					if item.filename in images_resign:
						continue
					elif item.filename == 'IMAGES/':
						continue
					elif item.filename == 'RADIO/':
						continue
					elif item.filename.startswith('IMAGES/'):
						merge_zip = True
					elif item.filename.startswith('RADIO/'):
						if '-sign.' in item.filename:
							merge_zip = True
					else:
						continue
					print('prepare_signed_target_files add ' + item.filename)
					buffer = izf.read(item.filename)
					if merge_zip:
						ozf.writestr(item, buffer)
					if keep_img:
						with open(os.path.join(output_dir, os.path.basename(item.filename)), 'w') as oif:
							oif.write(buffer)
		if True:
			print('prepare signed_target_files ' + output_zip + ' from ' + merged_zip)
			with zipfile.ZipFile(merged_zip, 'r') as izf:
				for item in izf.infolist():
					if item.filename in images_resign:
						continue
					buffer = izf.read(item.filename)
					ozf.writestr(item, buffer)
					if keep_img:
						if item.filename == 'IMAGES/recovery-two-step.img':
							pass
						elif item.filename.endswith('/'):
							pass
						elif item.filename.startswith('BOOT_DEBUG/'):
							pass
						elif item.filename.endswith('/rsc.xml') or \
							(item.filename.startswith('IMAGES/') and not item.filename.endswith('.map')) or \
							(item.filename.startswith('OTA/') and item.filename.endswith('.txt')) or \
							item.filename.endswith('/kernel'):
							print('extract ' + item.filename + ' from ' + merged_zip)
							with open(os.path.join(output_dir, os.path.basename(item.filename)), 'w') as oif:
								oif.write(buffer)
		for filename in images_resign:
			if images_resign[filename] is None:
				continue
			print('prepare signed_target_files ' + output_zip + ' from ' + filename)
			with open(images_resign[filename], 'r') as iif:
				buffer = iif.read()
				ozf.writestr(filename, buffer)
			if keep_img:
				print('move ' + filename + ' from ' + images_resign[filename])
				os.rename(images_resign[filename], os.path.join(output_dir, os.path.basename(filename)))
	return 0

def extract_img_from_target_files(output_dir, input_zip, temp_dir, info_dict):
	result = 0
	images_output_dir = os.path.abspath(output_dir)
	images_temp_dir = os.path.join(temp_dir, 'IMAGES_TEMP')
	if (result == 0) and (info_dict.get('mtk_sign_image_version') == 'v1'):
		# secure download, hard code from SignTool.pl
		secure_download_images = ['dtbo.img', 'system.img', 'system_other.img', 'userdata.img', 'vendor.img', 'vbmeta.img', 'boot.img', 'recovery.img', 'super.img', 'vbmeta_system.img', 'vbmeta_vendor.img', 'cache.img']
		for image_name in secure_download_images:
			if result == 0:
				result = write_mtk_sign_image(image_name, images_output_dir, temp_dir, info_dict)
	if (result == 0) and (info_dict.get('use_mntl') == 'true'):
		command = os.path.join(temp_dir, 'bin', 'mkmntl')
		command += ' -m ' + info_dict.get('mntl_page_size')
		command += ' -f ' + os.path.join(output_dir, 'ftl_gpt.bin') + ' -p ' + '0'
		command += ' -f ' + os.path.join(output_dir, 'system.img') + ' -p ' + info_dict.get('mntl_systemimage_start_sector')
		command += ' -f ' + os.path.join(output_dir, 'vendor.img') + ' -p ' + info_dict.get('mntl_vendorimage_start_sector')
		command += ' -f ' + os.path.join(output_dir, 'cache.img') + ' -p ' + info_dict.get('mntl_cacheimage_start_sector')
		command += ' -f ' + os.path.join(output_dir, 'userdata.img') + ' -p ' + info_dict.get('mntl_userdataimage_start_sector')
		command += ' -o ' + os.path.join(output_dir, 'mntl.img')
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			return 1
	return result

def extract_data_from_target_files(output_dir, input_zip, keep_dir, info_dict):
	result = 0
	with zipfile.ZipFile(input_zip) as izf:
		for item in izf.infolist():
			if item.filename.endswith('/'):
				continue
			if keep_dir:
				pass
			elif item.filename.startswith('META_TEMP/installed-files') and item.filename.endswith('.txt'):
				pass
			else:
				continue
			print('extract_data_from_target_files ' + item.filename)
			izf.extract(item, output_dir)
			if not keep_dir:
				os.rename(os.path.join(output_dir, item.filename), os.path.join(output_dir, os.path.basename(item.filename)))
				shutil.rmtree(os.path.join(output_dir, os.path.dirname(item.filename)))
	return result

def main():
	parser = argparse.ArgumentParser(description="Post-process for split target-files-package")
	parser.add_argument('--system-target-files', type=str, required=True, dest='system_zip')
	parser.add_argument('--product-target-files', type=str, required=False, dest='product_zip')
	parser.add_argument('--vendor-target-files', type=str, required=True, dest='vendor_zip')
	parser.add_argument('--kernel-target-files', type=str, required=True, dest='kernel_zip')
	parser.add_argument('--vendor-dist-files', type=str, required=False, dest='vendor_dist_zip')
	parser.add_argument('--output-dir', type=str, required=True, dest='output_dir')
	parser.add_argument('--sign-apk-certs', type=str, required=False, dest='sign_apk_certs')
	parser.add_argument('--rsc-xml', type=str, required=False, dest='rsc_xml')
	parser.add_argument('--keep-target-files', required=False, action='store_true', dest='keep_zip')
	parser.add_argument('--keep-images',       required=False, action='store_true', dest='keep_img')
	parser.add_argument('--keep-otapackage',   required=False, action='store_true', dest='keep_ota')
	parser.add_argument('--keep-symbols',      required=False, action='store_true', dest='keep_sym')
	args = parser.parse_args()
	#
	result = 0
	temp_dir = os.path.dirname(os.path.dirname(__file__))
	#host_out = 'out/host/linux-x86'
	host_out = temp_dir
	host_out_executables = os.path.join(host_out, 'bin')
	#releasetools_dir = 'build/make/tools/releasetools'
	releasetools_dir = os.path.join(temp_dir, 'releasetools')
	mtk_releasetools_dir = os.path.dirname(__file__)
	if not os.path.exists(mtk_releasetools_dir):
		mtk_releasetools_dir = os.path.join(temp_dir, 'releasetools')
	#jdk_home_bin = 'prebuilts/jdk/jdk9/linux-x86/bin'
	jdk_home_bin = os.path.join(temp_dir, 'jdk/bin')
	envir = os.environ
	envir['PATH'] = host_out_executables + ':' + jdk_home_bin + ':' + envir['PATH']
	merge_target_files_py = os.path.join(releasetools_dir, 'merge_target_files.py')
	#
	build_super_image_py = os.path.join(releasetools_dir, 'build_super_image.py')
	ota_from_target_files_py = os.path.join(releasetools_dir, 'ota_from_target_files.py')
	sign_target_files_apks_py = os.path.join(releasetools_dir, 'sign_target_files_apks.py')
	system_item_list = os.path.join(mtk_releasetools_dir, 'system_item_list.txt')
	other_item_list = os.path.join(mtk_releasetools_dir, 'other_item_list.txt')
	system_misc_info_keys = os.path.join(mtk_releasetools_dir, 'system_misc_info_keys.txt')
	#
	verified_assembled_framework_manifest = ''
	verified_assembled_vendor_manifest = ''
	skip_compatibility_check = True
	if not skip_compatibility_check:
		verified_assembled_framework_manifest, verified_assembled_vendor_manifest = prepare_vintf_manifest_from_target_files(temp_dir, args.system_zip, args.product_zip, args.vendor_zip)
	#
	temp_system_target_files = os.path.join(temp_dir, 'system_target_files.zip')
	temp_other_target_files = os.path.join(temp_dir, 'other_target_files.zip')
	system_target_files = prepare_system_target_files(args.output_dir, temp_system_target_files, args.system_zip, args.product_zip, args.rsc_xml, verified_assembled_framework_manifest)
	other_target_files, other_info_dict = prepare_other_target_files(args.output_dir, temp_other_target_files, args.vendor_zip, args.kernel_zip, verified_assembled_vendor_manifest)
	# target-files-package
	target_files = os.path.join(args.output_dir, 'target_files.zip')
	temp_merged_target_files = os.path.join(temp_dir, 'merged_target_files.zip')
	temp_signed_target_files = temp_merged_target_files
	if result == 0:
		command = 'python ' + merge_target_files_py + \
			' --system-target-files ' + system_target_files + \
			' --other-target-files ' + other_target_files + \
			' --system-item-list ' + system_item_list + \
			' --other-item-list ' + other_item_list + \
			' --system-misc-info-keys ' + system_misc_info_keys + \
			' --output-target-files ' + temp_merged_target_files + \
			' --rebuild_recovery' + \
			' -v'
		print(command)
		res = subprocess.Popen(command, shell=True, env=envir)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	# sign_target_files_apks
	if (result == 0) and args.sign_apk_certs:
		target_files = os.path.join(args.output_dir, 'signed_target_files.zip')
		temp_signed_target_files = os.path.join(temp_dir, 'signed_target_files.zip')
		command = 'python ' + sign_target_files_apks_py + \
			' -p ' + host_out + \
			' -d ' + args.sign_apk_certs + \
			' ' + temp_merged_target_files + \
			' ' + temp_signed_target_files
		print(command)
		res = subprocess.Popen(command, shell=True, env=envir)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	# super.img
	if (result == 0) and keep_img:
		result = write_super_image(args.output_dir, temp_signed_target_files, build_super_image_py, other_info_dict)
	# MTK lk.img, main_vbmeta_in_boot ...
	if result == 0:
		result = prepare_signed_target_files(args.output_dir, target_files, temp_signed_target_files, args.vendor_zip, temp_dir, other_info_dict)
	if (result == 0) and keep_img and args.vendor_dist_zip:
		result = extract_data_from_target_files(args.output_dir, args.vendor_dist_zip, True, other_info_dict)
	if (result == 0) and keep_img:
		result = extract_img_from_target_files(args.output_dir, target_files, temp_dir, other_info_dict)
	# OTA
	otapackage = os.path.join(args.output_dir, 'otapackage.zip')
	if (result == 0) and args.keep_ota:
		command = 'python ' + ota_from_target_files_py
		if skip_compatibility_check:
			command += ' --skip_compatibility_check'
		command += \
			' -v' + \
			' --block' + \
			' -p ' + host_out + \
			' ' + target_files + \
			' ' + otapackage
		print(command)
		res = subprocess.Popen(command, shell=True)
		out, err = res.communicate()
		if res.returncode != 0:
			print("command \"{}\" returned {}: {}".format(command, res.returncode, err))
			result = 1
	# Symbols
	if (result == 0) and args.keep_sym:
		pass
	# clean up
	if os.path.exists(temp_system_target_files):
		os.remove(temp_system_target_files)
	if os.path.exists(temp_other_target_files):
		os.remove(temp_other_target_files)
	if os.path.exists(temp_merged_target_files):
		os.remove(temp_merged_target_files)
	if os.path.exists(temp_signed_target_files):
		os.remove(temp_signed_target_files)
	if (not args.keep_zip) and os.path.exists(target_files):
		os.remove(target_files)
	return result

if __name__ == '__main__':
	result = main()
	sys.exit(result)

