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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ILSCTABLE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ILSCTABLE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>
#include <vector>

namespace NSIspTuning
{
    class ILscTable
    {
    public:
        typedef enum
        {
            HWTBL       = 0,
            GAIN_FIXED  = 1,
            GAIN_FLOAT  = 2
        } TBL_TYPE_T;

        typedef enum
        {
            BAYER_B     = 0,
            BAYER_GB    = 1,
            BAYER_GR    = 2,
            BAYER_R     = 3
        } TBL_BAYER_T;

        struct ConfigBlk
        {
            ConfigBlk()
                : i4BlkX(0), i4BlkY(0), i4BlkW(0), i4BlkH(0), i4BlkLastW(0), i4BlkLastH(0)
            {}

            ConfigBlk(MINT32 i4ImgWd, MINT32 i4ImgHt, MINT32 i4GridX, MINT32 i4GridY)
            {
                i4BlkX   = i4GridX - 2;
                i4BlkY   = i4GridY - 2;
                i4BlkW  = (i4ImgWd)/(2*(i4BlkX+1));
                i4BlkH  = (i4ImgHt)/(2*(i4BlkY+1));
                i4BlkLastW = i4ImgWd/2 - (i4BlkX*i4BlkW);
                i4BlkLastH = i4ImgHt/2 - (i4BlkY*i4BlkH);
            }

            ConfigBlk(MINT32 _i4BlkX, MINT32 _i4BlkY, MINT32 _i4BlkW, MINT32 _i4BlkH, MINT32 _i4BlkLastW, MINT32 _i4BlkLastH)
                : i4BlkX(_i4BlkX), i4BlkY(_i4BlkY), i4BlkW(_i4BlkW), i4BlkH(_i4BlkH), i4BlkLastW(_i4BlkLastW), i4BlkLastH(_i4BlkLastH)
            {}

            MINT32 i4BlkX;
            MINT32 i4BlkY;
            MINT32 i4BlkW;
            MINT32 i4BlkH;
            MINT32 i4BlkLastW;
            MINT32 i4BlkLastH;
        };

        struct Config
        {
            Config()
                : i4ImgWd(0), i4ImgHt(0), i4GridX(0), i4GridY(0), rCfgBlk()
            {}

            MINT32 i4ImgWd;
            MINT32 i4ImgHt;
            MINT32 i4GridX;
            MINT32 i4GridY;
            ConfigBlk rCfgBlk;
        };

        struct TransformCfg_T
        {
            TransformCfg_T(
                MUINT32 _u4ResizeW, MUINT32 _u4ResizeH, MUINT32 _u4GridX, MUINT32 _u4GridY,
                MUINT32 _u4X, MUINT32 _u4Y, MUINT32 _u4W, MUINT32 _u4H)
                    : u4ResizeW(_u4ResizeW)
                    , u4ResizeH(_u4ResizeH)
                    , u4GridX(_u4GridX)
                    , u4GridY(_u4GridY)
                    , u4X(_u4X)
                    , u4Y(_u4Y)
                    , u4W(_u4W)
                    , u4H(_u4H) {}
            TransformCfg_T()
                : u4ResizeW(0)
                , u4ResizeH(0)
                , u4GridX(0)
                , u4GridY(0)
                , u4X(0)
                , u4Y(0)
                , u4W(0)
                , u4H(0) {}
            MUINT32  u4ResizeW;
            MUINT32  u4ResizeH;
            MUINT32  u4GridX;
            MUINT32  u4GridY;
            MUINT32  u4X;
            MUINT32  u4Y;
            MUINT32  u4W;
            MUINT32  u4H;
        };

        struct RsvdData
        {
            RsvdData();
            RsvdData(MUINT32 u4HwRto);
            MUINT32 u4HwRto;
        };

    public:
        virtual                 ~ILscTable() {}

        static  ILscTable*      createInstance(TBL_TYPE_T eType, MINT32 i4W = -1, MINT32 i4H = -1, MINT32 i4GridX = -1, MINT32 i4GridY = -1);

        virtual TBL_TYPE_T      getType() const                                                     = 0;
        virtual TBL_BAYER_T     getBayer() const                                                    = 0;
        virtual const MVOID*    getData() const                                                     = 0;
        virtual MUINT32         getSize() const                                                     = 0;
        virtual const Config&   getConfig() const                                                   = 0;
        virtual const RsvdData& getRsvdData() const                                                 = 0;
        virtual MVOID*          editData()                                                          = 0;
#if (CAM3_3A_IP_BASE)
        virtual MVOID*          editData(int channel)                                               = 0;
#endif
        virtual RsvdData&       editRsvdData()                                                      = 0;
        virtual MBOOL           setData(const MVOID* src, MUINT32 size)                             = 0;
        virtual MBOOL           setConfig(const Config& rCfg)                                       = 0;
        virtual MBOOL           setConfig(const ConfigBlk& rCfgBlk)                                 = 0;
        virtual MBOOL           setConfig(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY)   = 0;
        virtual MBOOL           setBayer(TBL_BAYER_T bayer)                                         = 0;
        virtual MBOOL           setRsvdData(const RsvdData& rData)                                  = 0;
        virtual MBOOL           dump(const char* filename) const                                    = 0;

        virtual MBOOL           cropOut(const TransformCfg_T& trfm, ILscTable& output) const        = 0;
#if (CAM3_3A_ISP_50_EN)
        virtual MBOOL           cropToGain(const TransformCfg_T& trfm, ILscTable& output, std::vector<MUINT16*> &vecGain, MUINT32 mode) const =0;
#endif
        virtual MBOOL           mergeAndCrop(ILscTable& baseTbl, ILscTable& output) { return MFALSE; }
        virtual MBOOL           convert(ILscTable& output) const                                    = 0;
        virtual MBOOL           getRaTbl(MUINT32 u4Ratio, ILscTable& output) const                  = 0;
    };
};


/**
 * @brief The definition of the maker of ILscTable instance.
 */
typedef NSIspTuning::ILscTable* (*LscTable_FACTORY_T)(NSIspTuning::ILscTable::TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
static inline
NSIspTuning::ILscTable*
MAKE_LscTable(NSIspTuning::ILscTable::TBL_TYPE_T eType, MINT32 i4W = -1, MINT32 i4H = -1, MINT32 i4GridX = -1, MINT32 i4GridY = -1)
{
    return MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_LSC_TABLE, LscTable_FACTORY_T, eType, i4W, i4H, i4GridX, i4GridY);
}


#endif //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ILSCTABLE_H_
