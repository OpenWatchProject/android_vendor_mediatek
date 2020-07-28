.PHONY: all clean
.SILENT: all clean

export ANDROID_VERSION ?= N
export ANDROID_VENDOR_PATH ?= /vendor
export MICROTRUST_TEE_SUPPORT ?= yes

all:
	echo Building gatekeeper hal library for Android $(ANDROID_VERSION)
	ndk-build -B V=1 NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=./ NDK_APP_OUT=./out || exit 1;
	find . -name 'libgatekeeper.so' | xargs file
	echo Finished compiling gatekeeper hal library

clean:
	echo Cleaning...
	rm -rf $(CURDIR)/out $(CURDIR)/libs
	find . -name 'libgatekeeper.so'
	echo Done
