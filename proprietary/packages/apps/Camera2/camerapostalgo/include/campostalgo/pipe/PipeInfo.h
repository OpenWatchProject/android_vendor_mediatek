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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef INCLUDE_CAMPOSTALGO_PIPE_PIPEINFO_H_
#define INCLUDE_CAMPOSTALGO_PIPE_PIPEINFO_H_
#include <stdint.h>

namespace com {
namespace mediatek {
namespace campostalgo {

enum AlgoType_t {
    NONE            = -1,
    HDR             = 0x0,
    FILTER_PREVIEW  = 0x1,
    FILTER_CAPTURE  = 0x2,
    FILTER_MATRIX   = 0x3,
    FB              = 0x4,
    AUTOPANORAMA    = 0x5,
    PORTRAIT        = 0x6,
    VFB_CAPTURE     = 0x7,
    VFB_PREVIEW     = 0x8
};
typedef AlgoType_t AlgoType;

enum StreamType_t {
    UNKNOWN = -1, CAPTURE = 0, PREVIEW = 1, TINY = 2
};
typedef StreamType_t StreamType;

enum MetaType
{
    UNKNOWN_KEY              = -1,
    HDR_KEY              = 0x0000,
    BOKEH_KEY            = 0x0001,
    LOMO_KEY             = 0x0002,
    FACEBEAUTIFY_KEY     = 0x0003,
    PANORAMA_KEY         = 0x0004,
    PORTRAIT_KEY         = 0x0005
};

extern const char* getStreamTypeName(int32_t type);

}
}
}

#endif /* INCLUDE_CAMPOSTALGO_PIPE_PIPEINFO_H_ */
