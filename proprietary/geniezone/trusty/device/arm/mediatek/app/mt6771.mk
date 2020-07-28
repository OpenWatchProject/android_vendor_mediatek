GZ_SEC_STORAGE_UT := 1

GZ_USER_TASK_LIST := storage.elf

ifeq ($(GZ_SEC_STORAGE_UT),1)
GZ_USER_TASK_LIST += storage-unittest.elf
endif

include $(GZ_ROOT_DIR)/trusty/app/gz-test/Android.mk
include $(GZ_ROOT_DIR)/trusty/app/storage/Android.mk

ifeq ($(GZ_SEC_STORAGE_UT),1)
include $(GZ_ROOT_DIR)/trusty/app/storage-unittest/Android.mk
endif

#Active Stereo Depth Post-processing MTEE TA
ifneq ($(wildcard $(GZ_ROOT_DIR)/trusty/app/camalgo-adp/Android.mk),)
    include $(GZ_ROOT_DIR)/trusty/app/camalgo-adp/Android.mk
    GZ_USER_TASK_LIST += camalgo-adp.elf 
endif


