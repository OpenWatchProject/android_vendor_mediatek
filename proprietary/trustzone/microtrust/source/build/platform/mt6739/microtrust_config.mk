#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,drm,platform,drm_driver)
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,drm,platform,drm_ta)


ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)


### Secure Memory
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,utils,platform,Drutils)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,utils,platform,Tlutils)

### SEC
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,sec,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,sec,common,ta)

### keyinstall
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,keyinstall,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,keyinstall,common,ta)

ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)

### CMDQ
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,ta)

### M4U
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,platform,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,ta)

### H264 VDEC
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Vdec/drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Vdec/ta)

### Widevine DRM
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,modular_drm,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,modular_drm,common,ta)

### H264 VENC
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,platform,MtkH264Venc/drv)
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,platform,MtkH264Venc/ta)

endif # ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)

endif # ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
