
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(MTK_PATH_SOURCE)/external/libstorage_otp
###liblog for andorid log, libcutils for property_get, opt for opt, hardware for wake lock.
LOCAL_SHARED_LIBRARIES+=liblog libnvram libcutils libstorage_otp libhardware_legacy
