ifneq ($(TARGET_SIMULATOR),true)

#libft
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES := src/PortHandle.cpp\
                   src/SerPort.cpp\
                   src/Device.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op

LOCAL_SHARED_LIBRARIES += libc  libcutils liblog
LOCAL_MODULE:= libft

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_STATIC_LIBRARY)

META_DRIVER_PATH := $(MTK_PATH_SOURCE)/hardware/meta/adaptor

#meta_tst
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
CORE_SRC_FILES := src/tst_main.cpp

LOCAL_SRC_FILES := \
    $(CORE_SRC_FILES)\
    src/CmdTarget.cpp\
    src/Context.cpp\
    src/Device.cpp\
    src/Frame.cpp\
    src/FtModule.cpp\
    src/MdRxWatcher.cpp\
    src/Modem.cpp\
    src/SerPort.cpp\
    src/UsbRxWatcher.cpp\
    src/PortHandle.cpp\
    src/MSocket.cpp\
    ../misc/snappy/snappy-sinksource.cpp \
    ../misc/snappy/snappy-stubs-internal.cpp \
    ../misc/snappy/snappy.cpp \
    ../misc/snappy/snappy-c.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../misc/snappy

MTK_META_AUDIO_SUPPORT := yes
MTK_META_CCAP_SUPPORT := yes
MTK_META_GSENSOR_SUPPORT := yes
MTK_META_MSENSOR_SUPPORT := yes
MTK_META_ALSPS_SUPPORT := yes
MTK_META_GYROSCOPE_SUPPORT := yes
MTK_META_TOUCH_SUPPORT := yes
MTK_META_LCDBK_SUPPORT := yes
MTK_META_KEYPADBK_SUPPORT := yes
MTK_META_LCD_SUPPORT := yes
MTK_META_VIBRATOR_SUPPORT := yes
MTK_META_CPU_SUPPORT := yes
MTK_META_SDCARD_SUPPORT := yes
MTK_META_ADC_SUPPORT := yes
MTK_META_NVRAM_SUPPORT := yes
MTK_META_GPIO_SUPPORT := no
MTK_META_NFC_SUPPORT := yes
MTK_META_C2K_SUPPORT := yes
MTK_META_ATTESTATIONKEY_SUPPORT := yes
MTK_META_SYSENV_SUPPORT := yes
MTK_META_COMPRESS_SUPPORT := yes


#inlcude libft
LOCAL_STATIC_LIBRARIES += libft


#CCCI interface
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include

ifneq ($(MTK_MD3_SUPPORT),)
ifneq ($(filter 0,$(MTK_MD3_SUPPORT)),$(MTK_MD3_SUPPORT))
ifeq ($(MTK_META_C2K_SUPPORT),yes)
ifneq ($(MTK_ECCCI_C2K),yes)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/c2k/include
LOCAL_SHARED_LIBRARIES += libc2kutils
endif
LOCAL_CFLAGS += \
    -DTST_C2K_SUPPORT
endif
endif
endif

LOCAL_SHARED_LIBRARIES += libc libdl libhardware_legacy libcutils libbase liblog libutils

LOCAL_SHARED_LIBRARIES += libhwm libccci_util


ifeq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_SHARED_LIBRARIES += \
                          libselinux
endif

# DriverInterface Begin

ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/wifi
LOCAL_STATIC_LIBRARIES += libmeta_wifi
LOCAL_SHARED_LIBRARIES += libnetutils
LOCAL_CFLAGS += \
    -DFT_WIFI_FEATURE
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/gps
LOCAL_STATIC_LIBRARIES += libmeta_gps
LOCAL_CFLAGS += \
    -DFT_GPS_FEATURE
endif

ifeq ($(MTK_META_NFC_SUPPORT),yes)
ifeq ($(strip $(NFC_CHIP_SUPPORT)), yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/nfc/inc
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/nfc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/mtknfc/inc
LOCAL_STATIC_LIBRARIES += libmeta_nfc
LOCAL_CFLAGS += \
    -DFT_NFC_FEATURE
endif
ifeq ($(strip $(NFC_CHIP_SUPPORT)), yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/nfc/inc
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/nfc

LOCAL_STATIC_LIBRARIES += libmeta_nfc

LOCAL_CFLAGS += \
    -DFT_NFC_FEATURE
endif

endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/bluetooth
LOCAL_STATIC_LIBRARIES += libmeta_bluetooth
LOCAL_CFLAGS += \
    -DFT_BT_FEATURE
endif

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/fm
LOCAL_STATIC_LIBRARIES += libmeta_fm
LOCAL_CFLAGS += \
    -DFT_FM_FEATURE
endif

ifeq ($(MTK_TELEPHONY_FEATURE_SWITCH_DYNAMICALLY), yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/rat
LOCAL_STATIC_LIBRARIES += libmeta_rat
LOCAL_CFLAGS += \
    -DFT_RAT_FEATURE
endif

ifeq ($(MTK_TELEPHONY_FEATURE_SWITCH_DYNAMICALLY), yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/msim
LOCAL_STATIC_LIBRARIES += libmeta_msim
LOCAL_CFLAGS += \
    -DFT_MSIM_FEATURE
endif

ifeq ($(MTK_META_AUDIO_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/Audio
#LOCAL_SHARED_LIBRARIES += libaudio.primary.default
LOCAL_STATIC_LIBRARIES += libmeta_audio
LOCAL_CFLAGS += \
    -DFT_AUDIO_FEATURE
endif

ifeq ($(MTK_FACTORY_GAMMA_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/gamma
LOCAL_STATIC_LIBRARIES += libmeta_gamma
LOCAL_CFLAGS += \
    -DFT_GAMMA_FEATURE
endif
ifeq ($(FPGA_EARLY_PORTING), yes)
MTK_META_CCAP_SUPPORT := no
endif

ifeq ($(MTK_META_CCAP_SUPPORT),yes)

#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/cameratool/CCAP \
#                    $(MTK_PATH_SOURCE)/hardware/mtkcam/include
                    
#LOCAL_SHARED_LIBRARIES += libccap

LOCAL_CFLAGS += \
    -DFT_CCAP_FEATURE 
#-DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM) \
    
endif


ifeq ($(MTK_META_GSENSOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/gsensor
LOCAL_STATIC_LIBRARIES += libmeta_gsensor
LOCAL_CFLAGS += \
    -DFT_GSENSOR_FEATURE
endif

ifeq ($(MTK_META_MSENSOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/msensor
LOCAL_STATIC_LIBRARIES += libmeta_msensor
LOCAL_CFLAGS += \
    -DFT_MSENSOR_FEATURE
endif

ifeq ($(MTK_META_ALSPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/alsps
LOCAL_STATIC_LIBRARIES += libmeta_alsps
LOCAL_CFLAGS += \
    -DFT_ALSPS_FEATURE
endif

ifeq ($(MTK_META_GYROSCOPE_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/gyroscope
LOCAL_STATIC_LIBRARIES += libmeta_gyroscope
LOCAL_CFLAGS += \
    -DFT_GYROSCOPE_FEATURE
endif

ifeq ($(MTK_META_TOUCH_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/touch
LOCAL_STATIC_LIBRARIES += libmeta_touch
LOCAL_CFLAGS += \
    -DFT_TOUCH_FEATURE
endif

ifeq ($(MTK_META_LCDBK_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/LCDBK
LOCAL_STATIC_LIBRARIES += libmeta_lcdbk
LOCAL_CFLAGS += \
    -DFT_LCDBK_FEATURE
endif

ifeq ($(MTK_META_KEYPADBK_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/keypadbk
LOCAL_STATIC_LIBRARIES += libmeta_keypadbk
LOCAL_CFLAGS += \
    -DFT_KEYPADBK_FEATURE
endif

ifeq ($(MTK_META_LCD_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/lcd
LOCAL_STATIC_LIBRARIES += libmeta_lcd
LOCAL_CFLAGS += \
    -DFT_LCD_FEATURE
endif

ifeq ($(MTK_META_VIBRATOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/vibrator
LOCAL_STATIC_LIBRARIES += libmeta_vibrator
LOCAL_CFLAGS += \
    -DFT_VIBRATOR_FEATURE
endif

ifeq ($(MTK_META_SDCARD_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/sdcard
LOCAL_STATIC_LIBRARIES += libmeta_sdcard
LOCAL_CFLAGS += \
    -DFT_SDCARD_FEATURE
endif

#ifeq ($(MTK_META_EMMC_SUPPORT),yes)
ifneq ($(filter yes,$(MTK_EMMC_SUPPORT) $(MTK_UFS_SUPPORT)),)
# eMMC or UFS projects
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/emmc\
            $(META_DRIVER_PATH)/cryptfs

LOCAL_STATIC_LIBRARIES += libmeta_clr_emmc \
                          libmeta_cryptfs \
                          libstorageutil \
                          libselinux \
                          libz

LOCAL_CFLAGS += \
    -DFT_EMMC_FEATURE
LOCAL_CFLAGS += \
    -DFT_CRYPTFS_FEATURE

else
# NAND projects

LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/emmc

LOCAL_STATIC_LIBRARIES += libmeta_clr_emmc \
                          libstorageutil \
                          libselinux \
                          libz

LOCAL_CFLAGS += \
    -DFT_NAND_FEATURE
endif
#endif

ifeq ($(MNTL_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/cryptfs
LOCAL_STATIC_LIBRARIES += libmeta_cryptfs
LOCAL_CFLAGS += \
    -DFT_CRYPTFS_FEATURE
endif

ifeq ($(MTK_META_ADC_SUPPORT),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/ADC
LOCAL_STATIC_LIBRARIES += libmeta_adc_old
LOCAL_CFLAGS += \
    -DFT_ADC_FEATURE
endif

ifneq ($(filter yes,$(TRUSTONIC_TEE_SUPPORT) $(MICROTRUST_TEE_SUPPORT)),)
ifneq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(MTK_DRM_KEY_MNG_SUPPORT), yes)
LOCAL_CFLAGS += -DFT_DRM_KEY_MNG_FEATURE
LOCAL_CFLAGS += -DFT_DRM_KEY_MNG_TRUSTONIC_FEATURE
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/trustonic/source/trustlets/keyinstall/common/TlcKeyInstall/public
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/keyinstall/1.0
LOCAL_SHARED_LIBRARIES += liburee_meta_drmkeyinstall
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.keyinstall@1.0
LOCAL_STATIC_LIBRARIES += vendor.mediatek.hardware.keyinstall@1.0-util
endif
endif
endif
ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
LOCAL_CFLAGS += -DFT_DRM_KEY_MNG_FEATURE
LOCAL_CFLAGS += -DFT_DRM_KEY_MNG_TEE_FEATURE

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/keymanage/1.0
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/trustzone/mtee/include/tz_cross
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/drmkey

LOCAL_SHARED_LIBRARIES += libtz_uree liburee_meta_drmkeyinstall_v2
LOCAL_STATIC_LIBRARIES += liburee_meta_drmkey_if

#key manager HIDL
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.keymanage@1.0
LOCAL_STATIC_LIBRARIES += vendor.mediatek.hardware.keymanage@1.0-util_vendor
endif
endif

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.nvram@1.1
ifeq ($(strip $(MTK_META_NVRAM_SUPPORT)),yes)
#LOCAL_STATIC_LIBRARIES += libfft
LOCAL_SHARED_LIBRARIES += libnvram
LOCAL_SHARED_LIBRARIES += libfile_op
LOCAL_SHARED_LIBRARIES += \
    liblog \
    libcutils \
    libdl \
    libbase \
    libhardware \
    libbinder \
    libhidlbase \
    libhidltransport \
    libutils \
    vendor.mediatek.hardware.lbs@1.0 \

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/Meta_APEditor
LOCAL_STATIC_LIBRARIES += libmeta_apeditor
LOCAL_CFLAGS += \
    -DFT_NVRAM_FEATURE
endif

ifeq ($(strip $(MTK_META_GPIO_SUPPORT)),yes)
LOCAL_C_INCLUDES += $(META_DRIVER_PATH)/gpio
LOCAL_STATIC_LIBRARIES += libmeta_gpio
LOCAL_CFLAGS += \
    -DFT_GPIO_FEATURE
endif

ifeq ($(MTK_SPEAKER_MONITOR_SUPPORT),yes)
LOCAL_CFLAGS += \
    -DMTK_SPEAKER_MONITOR_SUPPORT
endif

ifeq ($(MTK_META_RSSITRIGGER_SUPPORT),yes)
LOCAL_CFLAGS += \
    -DMTK_META_RSSITRIGGER_SUPPORT
endif

ifeq ($(MTK_ATM_METAWIFIONLY),yes)
LOCAL_CFLAGS += \
    -DMTK_ATM_METAWIFIONLY
endif
ifeq ($(MTK_TC1_FEATURE),yes)
LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif


ifeq ($(MTK_DT_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

ifeq ($(MTK_EXTMD_NATIVE_DOWNLOAD_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_EXTMD_NATIVE_DOWNLOAD_SUPPORT
endif

ifneq ($(MTK_EXTERNAL_MODEM_SLOT),0)
LOCAL_CFLAGS += -DMTK_EXTERNAL_MODEM
endif

ifeq ($(MTK_ECCCI_C2K),yes)
LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(strip $(MTK_TEE_SUPPORT)), yes)
ifeq ($(MTK_META_ATTESTATIONKEY_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libkmsetkey
LOCAL_CFLAGS += \
    -DFT_ATTESTATION_KEY_FEATURE
endif
endif

ifeq ($(MTK_SINGLE_BIN_MODEM_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_SINGLE_BIN_MODEM_SUPPORT
endif

ifeq ($(MTK_META_SYSENV_SUPPORT),yes)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/libsysenv
LOCAL_SHARED_LIBRARIES += libsysenv
LOCAL_CFLAGS += -DMTK_META_SYSENV_SUPPORT
endif

ifeq ($(MTK_META_COMPRESS_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_META_COMPRESS_SUPPORT
endif


# DriverInterface End

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_MODULE:=meta_tst

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_EXECUTABLE)

endif   # !TARGET_SIMULATOR


