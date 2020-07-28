#!/usr/bin/env python

import os
import sys
import argparse
import string
import shutil
import re
import fnmatch
import time
import threading
import subprocess

def start(cmd,out=None,timeout_=0):
   handle = None
   
   timeout_=0
   
   if not out:
      out=subprocess.PIPE
   
   try:
      handle = subprocess.Popen(cmd, universal_newlines=True,stdin=out,stdout=out,stderr=subprocess.STDOUT)
      if timeout_>0:
         t = threading.Timer( timeout_, timeout, [handle] )
         t.start()
   except:
      pass

   return handle

def get_output(handle):
   return handle.communicate()[0]

def blocking_cmd(cmd):
   hndl = start(cmd,timeout_=60)
   return get_output(hndl)

# process mobiconvert params to get numeric parameters
def process_params(servicetype,memtype,flags):
   servicetype_value=servicetype
   memtype_value=memtype
   #servicetype processing
   if not servicetype.isdigit():
      if "driver" in  servicetype.lower():
         servicetype_value=1
      elif "middleware" in servicetype.lower():
         servicetype_value=4
      elif "service provider trustlet" in servicetype.lower() or "system trustlet" in servicetype.lower():
         servicetype_value=3
   # memtype processing
   if not memtype.isdigit():
      if "internal preferred" in memtype.lower():
         memtype_value=0
      elif "internal" in memtype.lower():
         memtype_value=1
      elif "external" in memtype.lower():
         memtype_value=2
   # flags processing
   flags_value=0
   for flag in flags:
      if flag.isdigit():
         flags_value|=flag
      else:
         if "permanent" in flag.lower():
            flags_value|=1
         elif "service has no wsm control" in flag.lower():
            flags_value|=2
         elif "debuggable" in flag.lower():
            flags_value|=4
         elif "new layout" in flag.lower():
            flags_value|=8

   return [servicetype_value,memtype_value,flags_value]

def sign_tas(args):

   all_axf_log_files=[]
   keyfile_name=args.keyfile

   # get all logs files
   for root, dirs, files in os.walk(args.indir):
      for i in files:
         if i.endswith(".log"):
            all_axf_log_files.append(os.path.join(root, i))

   for axf_log_file in sorted(all_axf_log_files):

      real_axf_file_name = os.path.basename(axf_log_file).split(".log")[0]
      axf_file_name,bin_name=real_axf_file_name.rsplit("_",1)[0]+".axf",real_axf_file_name.rsplit("_",1)[1]
      uuid_name,bin_extension=bin_name.split(".")[0],bin_name.split(".")[1]

      with open(axf_log_file,'r') as f:
         lines = f.readlines()

      flags_value = []
      for line in lines:
         newline=line.strip()
         if re.search("^\s*Service type.*=.*", newline, flags=0):
            servicetype_value=newline.split("=")[1].strip()
         elif re.search("^\s*NumberOfThreads.*=.*", newline, flags=0):
            numberofthreads_value=newline.split("=")[1].strip()
         elif re.search("^\s*NumberOfInstances.*=.*", newline, flags=0):
            numberofinstances_value=newline.split("=")[1].strip()
         elif re.search("^\s*MemType.*=.*", newline, flags=0):
            memtype_value=newline.split("=")[1].strip()
         elif re.search("^\s*Flags.*=.*", newline, flags=0):
            flags_value.append(newline.split("=")[1].strip())
         elif re.search("^\s*Interface Version.*=.*", newline, flags=0):
            interfaceversion_value=newline.split("=")[1]
         elif re.search("^\s*GPLevel.*=.*", newline, flags=0):
            gp_level=newline.split("=")[1].strip()
         elif re.search("^\s*UUID.*=.*", newline, flags=0):
            uuid_value=newline.split("=")[1].strip()
         elif re.search("^\s*InitHeapSize.*=.*", newline, flags=0):
            start_heap_size=newline.split("=")[1].strip()
         elif re.search("^\s*MaxHeapSize.*=.*", newline, flags=0):
            max_heap_size=newline.split("=")[1].strip()

      servicetype_value,memtype_value,flags_value=process_params(servicetype_value,memtype_value,flags_value)
      convert_params = " -servicetype "+str(servicetype_value)+" -numberofthreads "+numberofthreads_value+" -numberofinstances "+numberofinstances_value +" -memtype "+str(memtype_value)+" -flags "+str(flags_value)+" -interfaceversion "+ interfaceversion_value
      if max_heap_size>0:
         convert_params+=" -initheapsize "+start_heap_size+" -maxheapsize "+max_heap_size

      if gp_level.strip()=="1":
         convert_params += " -gp_level GP"

      output_path=args.outdir
      output_file_name=os.path.join(output_path,uuid_name)+"."+bin_extension

      new_output_file_name = os.path.join(output_path,bin_name)

      input_path=args.indir
      input_file_name=os.path.join(input_path,axf_file_name)

      convert_params+=" -bin "+ input_file_name +" -output "+output_file_name

      convert_params+=" -keyfile "+ args.keyfile

      print "Parameters: " + convert_params + "\n"

      output=blocking_cmd(["java","-jar", args.converttool]+convert_params.split())
      if not os.path.exists(output_file_name):
         print "Error generating binary"
         print output
         exit(1)
      else:
         print "File generated successfully"

#---------------------------------------------------------------------------
#                 Process input parameters
#---------------------------------------------------------------------------
def check_parameters(argv):

   parser = argparse.ArgumentParser(description='Re-generate a SYS TA taking the axf file, convert log file, and signing key file as input')

   working_folder = os.path.relpath(os.path.join(os.path.dirname(argv[0])), os.getcwd())

   parser.add_argument('-c', '--converttool', default=os.path.join(working_folder,"MobiConvert.jar"), help='Path to MobiConvert')
   parser.add_argument('-i', '--indir', default=working_folder, help='Path to folder containing axf and log files to convert')
   parser.add_argument('-o', '--outdir', default=working_folder, help='output directory to store the files')
   parser.add_argument('-k', '--keyfile', default=os.path.join(working_folder,"pairVendorTltSig.pem"), help='path to signing key file')

   args = parser.parse_args()

   #Check parameters
   if not os.path.isfile(args.converttool):
      parser.error("convert tool %s does not exist" % args.converttool)
      exit(1)
   if not os.path.isdir(args.indir):
      parser.error("axf file %s does not exist" % args.indir)
      exit(1)
   if not os.path.isfile(args.keyfile):
      parser.error("key file %s does not exist" % args.keyfile)
      exit(1)
   if not os.path.isdir(args.outdir):
      parser.error("output directory %s does not exist" % args.outdir)
      exit(1)
   return args

#---------------------------------------------------------------------------
#                 MAIN class
#---------------------------------------------------------------------------
def main(argv):
   print "+--------------------------------------------------------------------------"
   print "|                            CONVERT Trusted Applications ..."
   print "+--------------------------------------------------------------------------"
   sys.stdout.flush()
   args = check_parameters(argv)
   sign_tas(args)
   print "+--------------------------------------------------------------------------"


#--------------------------------------------------------------#
#             Calling the MAIN Function                        #
#--------------------------------------------------------------#
if __name__ == "__main__":
   main(sys.argv)

