#
# build the interface library
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES +=  src/com/gd/mobicore/pa/ifc/RootPAServiceIfc.aidl \
                    src/com/gd/mobicore/pa/ifc/RootPADeveloperIfc.aidl \
                    src/com/gd/mobicore/pa/ifc/RootPAOemIfc.aidl


LOCAL_MODULE := rootpa_interface
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

APP_PIE := true
LOCAL_32_BIT_ONLY := true

include $(BUILD_STATIC_JAVA_LIBRARY)