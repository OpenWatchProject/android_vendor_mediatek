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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IMAGEBUFFERHEAP_H_
//
#include "IImageBuffer.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

struct PortBufInfo_v1
{
    MINT32      memID[3];
    MUINTPTR     virtAddr[3];
    MINT32      nocache;
    MINT32      security;
    MINT32      coherence;
    MBOOL       continuos;
    SecType     secType;
    // for continuous memory
    PortBufInfo_v1(
        MINT32 const    _memID,
        MUINTPTR const   _virtAddr,
        MINT32  _nocache = 0,
        MINT32  _security = 0,
        MINT32  _coherence = 0,
        SecType _secType = SecType::mem_normal
    )
        : nocache(_nocache)
        , security(_security)
        , coherence(_coherence)
        , continuos(MTRUE)
        , secType(_secType)
    {
        memID[0]    = _memID;
        virtAddr[0] = _virtAddr;
    }
    // for non-continuous memory
    PortBufInfo_v1(
        MINT32 const    _memID[],
        MUINTPTR const   _virtAddr[],
        MUINT32 const   _planeCount,
        MINT32  _nocache = 0,
        MINT32  _security = 0,
        MINT32  _coherence = 0,
        SecType _secType = SecType::mem_normal
    )
        : nocache(_nocache)
        , security(_security)
        , coherence(_coherence)
        , continuos(MFALSE)
        , secType(_secType)
    {
        for (MUINT32 i = 0; i < _planeCount; ++i)
        {
            memID[i]    = _memID[i];
            virtAddr[i] = _virtAddr[i];
        }
    }
};

/******************************************************************************
 *  Image Buffer Heap (Camera1).
 ******************************************************************************/
class ImageBufferHeap : public virtual IImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for v1 Buffer
    typedef IImageBufferAllocator::ImgParam ImgParam_t;


public:     ////                    Creation.
    static  ImageBufferHeap*        create(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam,
                                        PortBufInfo_v1 const& rPortBufInfo,
                                        MBOOL const enableLog = MTRUE
                                    );

public:     ////                    Attributes.
    static  char const*             magicName() { return "Cam1Heap"; }

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IMAGEBUFFERHEAP_H_

