import subprocess
import os
import sys

def delete_mtk_files(src_path):
    src_files = os.listdir(src_path)
    for file_name in src_files:
        if "mtk" in file_name:
            subprocess.call(["rm " + src_path + file_name], shell=True)

#set path
all_path = os.getcwd() + '/' + sys.argv[0]
root_path = all_path[:all_path.find("vendor")]
android_path = root_path + "frameworks/ml/nn/runtime/test/"
vendor_path = all_path[:all_path.find("recover.py")]

subprocess.call(['mv ' + android_path + "specs/generate_test_original.sh " \
                 + android_path + "specs/generate_test.sh"], shell=True)
subprocess.call(['mv ' + android_path + "Android_original.bp " \
                 + android_path + "Android.bp"], shell=True)
subprocess.call(['mv ' + android_path + "for-cts/TestGeneratedOneFile_original.cpp " \
                 + android_path + "for-cts/TestGeneratedOneFile.cpp"], shell=True)
subprocess.call(["rm -rf " + android_path + "specs/mtk_specs/"], shell=True)

delete_mtk_files(android_path + "generated/examples/")
delete_mtk_files(android_path + "generated/models/")
delete_mtk_files(android_path + "generated/tests/")
