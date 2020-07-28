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

#ifndef ENCODECAP_H
#define ENCODECAP_H

enum resolution_type
{
    Resolution_VGA = 0,
    Resolution_QVGA = 1,
    Resolution_QQVGA = 2,
    Resolution_OTHERS = 3,

};

typedef enum CONFIG_VCODECCAPTYPE {

    CONFIG_VCODECCAPTYPE_GED_VILTE_MODE,
} CONFIG_INDEXTYPE;

int32_t getAdaptativeEncodeInfo(uint32_t modify_expected_bitrate, uint32_t minbitrate,
                                          uint32_t init_framerate, uint32_t *target_framerate);


int32_t getH264AdaptativeEncodeInfo(uint32_t expected_bitrate, uint32_t *target_bitrate, uint32_t minbitrate,
                                          uint32_t init_framerate, uint32_t *target_framerate,
                                          uint32_t AdaptiveResolution = 0, uint32_t init_resolution = 0, uint32_t width = 0, uint32_t height = 0,
                                          uint32_t *target_width = 0, uint32_t *target_height = 0);

int32_t getHEVCAdaptativeEncodeInfo(uint32_t expected_bitrate, uint32_t *target_bitrate, uint32_t minbitrate,
                                          uint32_t init_framerate, uint32_t *target_framerate);

int32_t SetcEncodeconfig(int32_t config_index, int32_t param);

uint32_t getH264ResolutionChangeCapabiltiy();

uint32_t getHEVCResolutionChangeCapabiltiy();

uint32_t getH264AdaptiveDropFrameCapabiltiy();

uint32_t getHEVCAdaptiveDropFrameCapabiltiy();

#endif

