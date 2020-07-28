/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_CORE_MODULE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_CORE_MODULE_H_
//
/******************************************************************************
 *
 ******************************************************************************/
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/******************************************************************************
 *
 ******************************************************************************/
#define MTKCAM_GET_MODULE_GROUP_ID(module_id)   (0xFFFF & (module_id >> 16))
#define MTKCAM_GET_MODULE_INDEX(module_id)      (0xFFFF & (module_id))


/**
 * mtkcam module group id
 */
enum
{
    MTKCAM_MODULE_GROUP_ID_DRV,
    MTKCAM_MODULE_GROUP_ID_AAA,
    MTKCAM_MODULE_GROUP_ID_FEATURE,
    MTKCAM_MODULE_GROUP_ID_CUSTOM,
    MTKCAM_MODULE_GROUP_ID_UTILS,
};


/**
 * mtkcam module id
 *
 * |     32     |      32      |
 * |  group id  | module index |
 */
enum
{
    /*****
     * drv
     */
    MTKCAM_MODULE_ID_DRV_START          = MTKCAM_MODULE_GROUP_ID_DRV << 16,
    MTKCAM_MODULE_ID_DRV_HAL_SENSORLIST,                /*!< include/mtkcam/drv/IHalSensor.h */
    MTKCAM_MODULE_ID_DRV_HW_SYNC_DRV,                   /*!< include/mtkcam/drv/IHwSyncDrv.h */
    MTKCAM_MODULE_ID_DRV_IOPIPE_CAMIO_NORMALPIPE,       /*!< include/mtkcam/drv/iopipe/CamIO/INormalPipe.h */
    //--------------------------------------------------------------------------
    MTKCAM_MODULE_ID_DRV_END,

    /*****
     * aaa
     */
    MTKCAM_MODULE_ID_AAA_START          = MTKCAM_MODULE_GROUP_ID_AAA << 16,
    MTKCAM_MODULE_ID_AAA_HAL_3A,                        /*!< include/mtkcam/aaa/IHal3A.h */
    MTKCAM_MODULE_ID_AAA_HAL_ISP,                       /*!< include/mtkcam/aaa/IHalISP.h */
    MTKCAM_MODULE_ID_AAA_HAL_FLASH,                     /*!< include/mtkcam/aaa/IHalFlash.h */
    MTKCAM_MODULE_ID_AAA_ISP_MGR,                       /*!< include/mtkcam/aaa/IIspMgr.h */
    MTKCAM_MODULE_ID_AAA_SYNC_3A_MGR,                   /*!< include/mtkcam/aaa/ISync3A.h */
    MTKCAM_MODULE_ID_AAA_SW_NR,                         /*!< include/mtkcam/aaa/ICaptureNR.h */
    MTKCAM_MODULE_ID_AAA_DNG_INFO,                      /*!< include/mtkcam/aaa/IDngInfo.h */
    MTKCAM_MODULE_ID_AAA_NVBUF_UTIL,                    /*!< include/mtkcam/aaa/INvBufUtil.h */
    MTKCAM_MODULE_ID_AAA_LSC_TABLE,                     /*!< include/mtkcam/aaa/ILscTable.h */
    MTKCAM_MODULE_ID_AAA_LCS_HAL,                       /*!< include/mtkcam/aaa/lcs/lcs_hal.h */
    //--------------------------------------------------------------------------
    MTKCAM_MODULE_ID_AAA_END,

    /*****
     * feature
     */
    MTKCAM_MODULE_ID_FEATURE_START      = MTKCAM_MODULE_GROUP_ID_FEATURE << 16,
    MTKCAM_MODULE_ID_HDR_PROC2,                         /*!< include/mtkcam/feature/hdr/IHDRProc2.h */
    MTKCAM_MODULE_ID_HDR_PERF,                          /*!< include/mtkcam/feature/hdr/Platform.h */
    MTKCAM_MODULE_ID_HDR_BOOST,                         /*!< include/mtkcam/feature/hdr/Platform.h */
    //--------------------------------------------------------------------------
    MTKCAM_MODULE_ID_FEATURE_END,

    /*****
     * custom
     */
    MTKCAM_MODULE_ID_CUSTOM_START       = MTKCAM_MODULE_GROUP_ID_CUSTOM << 16,
    MTKCAM_MODULE_ID_CUSTOM_DEBUG_EXIF,                 /*!< include/mtkcam/custom/ExifFactory.h */
    //--------------------------------------------------------------------------
    MTKCAM_MODULE_ID_CUSTOM_END,

    /*****
     * Utils
     */
    MTKCAM_MODULE_ID_UTILS_START        = MTKCAM_MODULE_GROUP_ID_UTILS << 16,
    MTKCAM_MODULE_ID_UTILS_LOGICALDEV,
    //--------------------------------------------------------------------------
    MTKCAM_MODULE_ID_UTILS_END,
};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * mtkcam module versioning control
 */

/**
 * The most significant bits (bit 24 ~ 31)
 * store the information of major version number.
 *
 * The least significant bits (bit 16 ~ 23)
 * store the information of minor version number.
 *
 * bit 0 ~ 15 are reserved for future use
 */
#define MTKCAM_MAKE_API_VERSION(major,minor) \
    ((((major) & 0xff) << 24) | (((minor) & 0xff) << 16))

#define MTKCAM_GET_MAJOR_API_VERSION(version) \
    (((version) >> 24) & 0xff)

#define MTKCAM_GET_MINOR_API_VERSION(version) \
    (((version) >> 16) & 0xff)



/**
 * mtkcam module interface
 */
struct mtkcam_module
{
    /**
     * The API version of the implemented module. The module owner is
     * responsible for updating the version when a module interface has changed.
     *
     * The derived modules own and manage this field.
     * The module user must interpret the version field to decide whether or not
     * to inter-operate with the supplied module implementation.
     *
     */
    uint32_t                        (*get_module_api_version)();

    /** Identifier of module. */
    uint32_t                        (*get_module_id)();

    /**
     * Module extension.
     * Return a pointer to the derived module interface.
     */
    void*                           (*get_module_extension)();

    /**
     * Return the supported sub-module api versions, sorted in the range
     * [*versions, *versions + count) in ascending order, for a given camera
     * device
     *
     * versions:    The pointer to the start address of an array of supported
     *              sub-module api versions, sorted in ascending order.
     *
     * count:       The number of supported sub-module api versions.
     *
     * index:       The camera/sensor index starting from 0.
     *              Not all modules need this field; ignored if not needed.
     *
     * Return values:
     *
     * 0:           On a successful operation
     *
     * -ENODEV:     The information cannot be provided due to an internal
     *              error.
     *
     * -EINVAL:     The input arguments are invalid, i.e. the index is invalid,
     *              and/or the module is invalid.
     *
     */
    int                             (*get_sub_module_api_version)(
                                        uint32_t const** versions,
                                        size_t* count,
                                        int index
                                    );

    /* reserved for future use */
    void*                           reserved[32-4];
};



/******************************************************************************
 *
 ******************************************************************************/
__END_DECLS
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_CORE_MODULE_H_

