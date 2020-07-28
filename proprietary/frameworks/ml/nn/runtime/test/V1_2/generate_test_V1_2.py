import os
from shutil import copy
import subprocess
import sys
import shlex
import time
from pathlib import Path
import glob

def copy_files(src_path, dst_path):
    src_files = os.listdir(src_path)
    for file_name in src_files:
        copy(src_path + file_name, dst_path)

def copy_mtk_files(src_path, dst_path):
    src_files = os.listdir(src_path)
    for file_name in src_files:
        if "mtk" in file_name:
            copy(src_path + file_name, dst_path)

#set path
all_path = os.getcwd() + '/' + sys.argv[0]
root_path = all_path[:all_path.find("vendor")]
android_path = root_path + "frameworks/ml/nn/runtime/test/"
vendor_path = all_path[:all_path.find("generate_test_V1_2.py")]

#copy mtk specs to aosp
subprocess.call(["rm -rf "+ android_path + "specs/mtk_specs/"], shell=True)
subprocess.call(["mkdir "+ android_path + "specs/mtk_specs/"], shell=True)
copy_files(vendor_path + "specs/", android_path + "specs/mtk_specs/")

#modify script
if not Path(android_path + "specs/generate_test_original.sh").is_file():
    subprocess.call(['cp ' + android_path+"specs/generate_test.sh " + android_path + \
                     "specs/generate_test_original.sh"], shell=True)

f = open(android_path + "specs/generate_test.sh")
sh_content = f.readlines()
f.close()

for i in range(len(sh_content)):
    if "NNAPI_VERSION=\"" in sh_content[i] and "mtk_specs" not in sh_content[i+1]:
        sh_content.insert(i+1, "mtk_specs\n")
    elif "cts_generator.py" in sh_content[i] and "python" not in sh_content[i]:
        sh_content[i] = "python " + sh_content[i].lstrip()

with open(android_path + "specs/generate_test.sh", "w") as fw:
    contents = "".join(sh_content)
    fw.write(contents)

if not Path(android_path + "for-cts/TestGeneratedOneFile_original.cpp").is_file():
    subprocess.call(['cp ' + android_path + "for-cts/TestGeneratedOneFile.cpp " + \
                     android_path + "for-cts/TestGeneratedOneFile_original.cpp"], \
                    shell=True)

#run script
subprocess.call([android_path+"specs/generate_test.sh"])


#copy test generated files to mtk
copy_mtk_files(android_path + "generated/examples/",vendor_path+"generated/examples/")
copy_mtk_files(android_path + "generated/models/",vendor_path+"generated/models/")
copy_mtk_files(android_path + "generated/tests/",vendor_path+"generated/tests/")

#modify Android.bp with default
if not Path(android_path + "Android_original.bp").is_file():
    subprocess.call(['cp ' + android_path + "Android.bp " + android_path + "Android_original.bp"], \
                    shell=True)
else:
    subprocess.call(['mv ' + android_path + "Android_original.bp " + android_path + "Android.bp"], \
                    shell=True)
    subprocess.call(['cp ' + android_path + "Android.bp " + android_path + "Android_original.bp"], \
                    shell=True)

#copy hardcode test cases to AOSP folder
copy_mtk_files(vendor_path + "hardcode/examples/",android_path+"generated/examples/")
copy_mtk_files(vendor_path + "hardcode/models/",android_path+"generated/models/")
copy_mtk_files(vendor_path + "hardcode/tests/",android_path+"generated/tests/")