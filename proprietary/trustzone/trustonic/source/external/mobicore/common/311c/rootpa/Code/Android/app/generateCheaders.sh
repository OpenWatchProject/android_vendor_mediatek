#/bin/sh

#
# this is helper script for recreating the jni header file when the interface 
# changes. it is not executed every time when building
#

cd src
export CLASSPATH=.:/usr/local/android-sdk/:/usr/local/android-sdk/platforms/android-16/android.jar:../out/classes:../../lib/bin/classes
javac  com/gd/mobicore/pa/jni/CommonPAWrapper.java
javah  -d ../jni/CommonPAWrapper/ com.gd.mobicore.pa.jni.CommonPAWrapper

