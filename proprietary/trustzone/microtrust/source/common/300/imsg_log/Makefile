.PHONY: all clean
.SILENT: all clean

ANDROID_VERSION ?= N
ANDROID_VENDOR_PATH ?= /vendor
export MICROTRUST_TEE_SUPPORT ?= yes

all:
	echo Building libimsg_log.so for Android $(ANDROID_VERSION)
	ndk-build -B V=1 NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=./ NDK_APP_OUT=./out || exit 1;
	find . -name 'libimsg_log.so' | xargs file
	echo Finished compiling imsg_log

clean:
	echo Cleaning...
	rm -rf $(CURDIR)/out $(CURDIR)/libs
	find . -name 'libimsg_log.so'
	echo Done
