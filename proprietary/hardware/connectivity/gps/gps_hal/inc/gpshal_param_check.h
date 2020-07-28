/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __GPS_HAL_PARAM_CHECK_H__
#define __GPS_HAL_PARAM_CHECK_H__

#include <string.h>

#if 0 != PARAM_CHECK  // PARAM_CHECK is enabled
    #define GPS_HW_MODULE_OPEN__CHECK_PARAM \
        do { \
            if (&HAL_MODULE_INFO_SYM != module) { \
                ALOGT("-: Bad module object"); \
                return -1;  /* Fail: any non-zero value */ \
            } else if (NULL == id || strcmp(id, HAL_MODULE_INFO_SYM.id)) { \
                ALOGT("-: Bad id"); \
                return -2; \
            } else if (NULL == device) { \
                ALOGT("-: Bad device"); \
                return -3; \
            } \
        } while (0)
    #define GPS_DEVICE__GET_GPS_INTERFACE__CHECK_PARAM \
        do { \
            if (NULL == device || \
                    &HAL_MODULE_INFO_SYM != device->common.module || \
                gps_device__get_gps_interface != device->get_gps_interface) { \
                ALOGT("-: Bad device object"); \
                return NULL; \
            } \
        } while (0)
#elif !defined(NDEBUG)  // assert() is effective
    #define GPS_HW_MODULE_OPEN__CHECK_PARAM \
        do { \
            assert(&HAL_MODULE_INFO_SYM == module); \
            assert(NULL != id); \
            assert(NULL != device); \
            assert(0 == strcmp(id, HAL_MODULE_INFO_SYM.id)); \
        } while (0)
    #define GPS_DEVICE__GET_GPS_INTERFACE__CHECK_PARAM \
        do { \
            assert(NULL != device); \
            assert(&HAL_MODULE_INFO_SYM == device->common.module); \
            assert(gps_device__get_gps_interface == device->get_gps_interface); \
        } while (0)
#else
    // PARAM_CHECK is disabled
    // And assert() is not effective
    #define GPS_HW_MODULE_OPEN__CHECK_PARAM \
        do { \
            UNUSED(module); \
            UNUSED(id); \
        } while (0)
    #define GPS_DEVICE__GET_GPS_INTERFACE__CHECK_PARAM \
        UNUSED(device)
#endif


#endif  // __GPS_HAL_PARAM_CHECK_H__