
LOCAL_SHARED_LIBRARIES += libcutils liblog libc libnvram libccci_util libhardware_legacy libdl

#ifeq ($(MTK_TC1_FEATURE), yes)

#LOCAL_CFLAGS += -DMTK_TC1_FEATURE

#else

#LOCAL_SHARED_LIBRARIES += libmtkrillog
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log

#endif
##libmtklog not ready
### libcutils for property_get/set,
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_SRC_FILES += platform/mdinit_platform.c platform/mdinit_relate.c platform/md_init.c
##### md_init.c: for old platform, other platform no need.
