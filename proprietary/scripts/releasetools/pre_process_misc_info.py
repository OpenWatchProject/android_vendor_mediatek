#!/usr/bin/env python

import argparse
import os

import common
import merge_target_files

def process_misc_info(text, keys):
	info_dict = common.LoadDictionaryFromLines(text.split("\n"))
	if keys is None:
		keys = info_dict.keys()
	lines = []
	merged_info_dict = {}
	for key in keys:
		value = ''
		if key in info_dict:
			value = info_dict[key]
		merged_info_dict[key] = value
	sorted_keys = sorted(merged_info_dict.keys())
	for key in sorted_keys:
		lines.append(key + '=' + merged_info_dict[key])
	return "\n".join(lines)

def main():
	parser = argparse.ArgumentParser(description="Preprocess for split misc_info.txt")
	parser.add_argument('--system', default=False, action='store_true')
	parser.add_argument('--vendor', default=False, action='store_true')
	parser.add_argument('--in', type=str, required=True, dest='in_txt')
	parser.add_argument('--out', type=str, required=True, dest='out_txt')
	args = parser.parse_args()

	keys = None
	if args.system:
		option_file = os.path.join(os.path.dirname(__file__), 'system_misc_info_keys.txt')
		if os.path.exists(option_file):
			keys = merge_target_files.read_config_list(option_file)
		else:
			keys = merge_target_files.default_system_misc_info_keys
		buffer = ''
		with open(args.in_txt, 'r') as input:
			buffer = input.read()
			buffer = process_misc_info(buffer, keys)
		with open(args.out_txt, 'w') as output:
			output.write(buffer)

if __name__ == '__main__':
	main()

