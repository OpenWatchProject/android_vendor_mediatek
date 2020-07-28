LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= network_main.cpp\
	commandlistening.cpp\
	src/clear_begin.cpp\
	src/tcpdump_data.cpp\
	src/run_command.cpp\
	src/full_test_data.cpp

LOCAL_MODULE := netdiag
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := netdiag.rc
LOCAL_C_INCLUDES += \
    external/libcxx/include \
    bionic \
    bionic/libstdc++/include \
	external/openssl/include

LOCAL_MULTILIB := first

LOCAL_SHARED_LIBRARIES := libdl libsysutils libcutils libnetutils libcrypto liblog libc++ libutils libpcap_bak

#LOCAL_STATIC_LIBRARIES += libdrvb
include $(MTK_EXECUTABLE)
