#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,drm,platform,drm_driver)
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,drm,platform,drm_ta)


ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)


### Secure Memory
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,secmem,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,secmem,common,ta)

### SEC
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,sec,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,sec,common,ta)

# internal use only
#ifeq ($(TARGET_BUILD_VARIANT),eng)
# tee_sanity is for secure os basic test
#  MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,tee_sanity,common,drv)
#  MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,tee_sanity,common,ta)
#endif

###############################################################################
# Keyinstall
###############################################################################
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,keyinstall,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,keyinstall,common,ta)
endif

###############################################################################
# SVP
###############################################################################
ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)

### Widevine DRM
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,modular_drm,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,modular_drm,common,ta)

### DRM HDCP
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,drm_hdcp_common,common,drv)

### M4U
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,ta)

### H264 VDEC
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Vdec/drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Vdec/ta)

### H264 VENC
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Venc/drv)
#MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,video,common,MtkH264Venc/ta)

### CMDQ
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,ta)

endif

###############################################################################
# IRIS Secure Camera
###############################################################################
ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)

### ISP
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,camera/isp,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,camera/isp,common,ta)

### ImgSensor
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,imgsensor,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,imgsensor,common,ta)

### M4U
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,ta)

### CMDQ
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,cmdq,common,ta)

endif

###############################################################################
# GENIEZONE
###############################################################################
ifeq ($(strip $(MTK_ENABLE_GENIEZONE)),yes)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,pmem,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,pmem,common,ta)
endif

endif # ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

