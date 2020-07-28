#!/bin/bash
#!/bin/bash
if [[ $1 = "" ]]; then
  echo ""
  echo " Please enter the source folder."
  echo " Usage: ./loadTAs <source folder>"
  echo ""
  exit 0
fi
# Wait for Device to be adb connected
echo "*******************************************************"
echo "* Please connect your device, wait for device...      *"
echo "*******************************************************"
adb wait-for-devices; rc=$?;
# Wait in devices
if [[ $rc != 0 ]]; then
  echo "*******************************************************"
  echo "* Connecting device aborted.                          *"
  echo "*******************************************************"
  exit $?
fi
adb root
sleep 2
adb remount
adb shell su 0 setenforce 0
for f in *.tabin
do
  adb push $f /sdcard/trustonic_tests/mcRegistry
done
for f in *.tlbin
do
  adb push $f /sdcard/trustonic_tests/mcRegistry
done
echo "*******************************************************"
echo "* Secure test binaries copied to DUT.                 *"
echo "*******************************************************"

