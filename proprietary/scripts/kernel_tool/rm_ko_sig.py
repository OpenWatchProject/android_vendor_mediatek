import base64
import os
import sys
import struct
import re
import shutil

ko_sig_info = ">8c I"
sig_info_length = 12
ko_sig_marker = "~Module signature appended~\n"
sig_marker_length = 28

def remove_signature(in_file):
	print "start to remove signature"
	pureName = os.path.basename(in_file)
	tmp_file = os.path.join(os.path.abspath(os.path.dirname(in_file)), pureName.split(".")[0] + "_tmp." + pureName.split(".")[1])
	if os.path.isfile(tmp_file):
		os.remove(tmp_file)
	f1 = open(in_file, 'rb')
	imgSize = os.path.getsize(in_file)
	pattern = re.compile(ko_sig_marker)
	f1.seek((imgSize - sig_marker_length), 0)
	file_sign_marker = f1.read(sig_marker_length)
	if (pattern.match(file_sign_marker)):
		print "with signature!"
		print file_sign_marker
		f2 = open(tmp_file, 'wb')
		f1.seek((imgSize - sig_marker_length - sig_info_length), 0)
		mod_sig_info = struct.unpack(ko_sig_info, f1.read(12))
		print "ko_sig_length = " + str(mod_sig_info[8])
		f1.seek(0, 0)
		f2.write(f1.read(imgSize - mod_sig_info[8] - sig_info_length - sig_marker_length))
		f1.close()
		f2.close()
		os.remove(os.path.abspath(in_file))
		shutil.move(tmp_file, os.path.abspath(in_file))
		print "remove ko signature: " + in_file
		print "success!"
	else:
		print "without signature!"
		print "dump: " + str(file_sign_marker)
		print "end~"
	return

def main():
	in_file = sys.argv[1]
	remove_signature(in_file)

if __name__ == '__main__':
	main()
