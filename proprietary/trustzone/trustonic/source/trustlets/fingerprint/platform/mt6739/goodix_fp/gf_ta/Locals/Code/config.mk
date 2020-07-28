#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

#**arm64-v8a/armeabi-v7a
CONFIG_ABI:=armeabi-v7a

#y:support
#n:not support
CONFIG_NEON:=n

############################################################################
# Goodix Fingerprint chip type:
# GF316M/GF318M/GF3118M/GF518M/GF5118M/GF516M/GF816M/GF3208/GF5206/GF5216/GF5208. etc
############################################################################

#GF_CHIP_TYPE := ${CHIP_TYPE}
#GF_CHIP_SERIES := ${CHIP_SERIES}
GF_CHIP_TYPE := GF5216
GF_CHIP_SERIES := milan_a_series



ifeq ($(ANDROID_VERSION),android5.1)
TA_CHIP_FLAGS +=-D__ANDROID_51
endif

# GF_LOG_LEVEL is 4, will output verbose, debug, info, error log
# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq ($(TARGET_MODE), debug)
TA_CHIP_FLAGS += -DGF_LOG_LEVEL=4
else ifeq ($(TARGET_MODE), userdebug)
TA_CHIP_FLAGS += -DGF_LOG_LEVEL=3
else
TA_CHIP_FLAGS += -DGF_LOG_LEVEL=2
endif

TA_CHIP_FLAGS += \
    -D${GF_CHIP_TYPE} \
    -DGF_TA_VERSION=\"faf0fbf942edde384ee74a4599170c6f7ba7f91f_2017.08.07_11:34:18\" \
    -DGF_AUTHENTICATOR_VERSION_CODE=2017080711 \
    -DGF_TARGET_MODE=\"${TARGET_MODE}\" \
    -DGF_CHIP_TYPE=\"${CHIP_TYPE}\" \
    -DGF_PACKAGE_VERSION_CODE=\"10000\" \
    -DGF_PACKAGE_VERSION_NAME=\"V00.01.00.00\" \
    -DGF_GIT_BRANCH=\"master_merge_milan_a\" \
    -DGF_COMMIT_ID=\"faf0fbf942edde384ee74a4599170c6f7ba7f91f\" \
    -DGF_BUILD_TIME=\"2017.08.07_11:34:18\" \
    -D__TEE__=$(TEE) \
    -D__TEE_VERSION__=$(TEE_VERSION)

ifeq ($(GF_CHIP_SERIES), oswego_m)
    TA_CHIP_FLAGS += -DGF_CHIP_SERIES_OSWEGO
    ifeq ($(GF_CHIP_TYPE), $(filter $(GF_CHIP_TYPE), GF318M GF3118M GF518M GF5118M))
        ARMCC_COMPILATION_FLAGS += -DGF_RAWDATA_LEN=13824
    else
        ARMCC_COMPILATION_FLAGS += -DGF_RAWDATA_LEN=13216
    endif
else ifeq ($(GF_CHIP_SERIES), milan_f_series)
    TA_CHIP_FLAGS += -DGF_CHIP_SERIES_MILAN_F -DGF_RAWDATA_LEN=22176
else ifeq ($(GF_CHIP_SERIES), milan_a_series)
    TA_CHIP_FLAGS += -DGF_CHIP_SERIES_MILAN_A -DGF_RAWDATA_LEN=19360
else ifeq ($(GF_CHIP_SERIES), milan_hv)
    TA_CHIP_FLAGS += -DGF_CHIP_SERIES_MILAN_HV -DGF_RAWDATA_LEN=22176
endif

ifeq ($(SUPPORT_AUTH_TOKEN), y)
TA_CHIP_FLAGS += -D__SUPPORT_AUTH_TOKEN
endif

ifneq ($(TARGET_MODE), release)
TA_CHIP_FLAGS += -D__SUPPORT_GF_MEMMGR
endif

ifeq (x$(TEE), xqsee)
ifeq ($(TEE_VERSION), $(filter $(TEE_VERSION), qsee4 qsee4_0_5))
TA_CHIP_FLAGS += -DGX_FTS_V4 -DGX_FTS_BUILD
else ifeq ($(TEE_VERSION), qsee3)
TA_CHIP_FLAGS += -DGX_FTS_V3 -DGX_FTS_BUILD
TA_CHIP_FLAGS += \
        --enum_is_int \
        --interface_enums_are_32_bit \
        --apcs=/ropi/rwpi \
        --lower_ropi \
        --lower_rwpi \
        --vectorize \
        --cpu=7-A \
        --fpu=softvfp \
        -D__TARGET_FEATURE_NEON \
        -Otime \
        -O3

    ifeq ($(TARGET_PLATFORM), msm8952)
        TA_CHIP_FLAGS += -DCUST_H=\"custmakaanaaa.h\"
    else ifeq ($(TARGET_PLATFORM), msm8939)
        TA_CHIP_FLAGS += -DCUST_H=\"custmafaanaaa.h\"
    endif
endif
endif

CONFIG_C_FLAGS += $(TA_CHIP_FLAGS)
