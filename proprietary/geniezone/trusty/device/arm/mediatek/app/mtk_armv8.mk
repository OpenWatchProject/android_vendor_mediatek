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

GZ_USER_TASK_LIST += fod_sample.elf
include $(GZ_ROOT_DIR)/trusty/app/sample/fod_sample/Android.mk

#Active Stereo Depth Post-processing MTEE TA
#ifneq ($(wildcard $(GZ_ROOT_DIR)/trusty/app/camalgo-adp/Android.mk),)
#    include $(GZ_ROOT_DIR)/trusty/app/camalgo-adp/Android.mk
#    GZ_USER_TASK_LIST += camalgo-adp.elf
#endif

#ifeq ($(MTK_TARGET_PROJECT),k71v1_64_bsp_as)
#    GZ_USER_TASK_LIST += fralgo.elf
#    include $(GZ_ROOT_DIR)/trusty/app/fralgo/Android.mk
#endif

#ifeq ($(MTK_TARGET_PROJECT),$(filter $(MTK_TARGET_PROJECT),k85v1_64_tee_nebula_svp_vdo k85v1_64_tee_geniezone_svp_lm k85v1_64_teei_nebula_svp_vdo k85v1_64_teei_geniezone_svp))
#    GZ_USER_TASK_LIST += camalgo-fdft.elf
#    include $(GZ_ROOT_DIR)/trusty/app/camalgo-fdft/Android.mk
#endif
