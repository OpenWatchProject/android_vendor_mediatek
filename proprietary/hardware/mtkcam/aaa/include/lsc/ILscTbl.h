/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef __I_LSC_TBL_H__
#define __I_LSC_TBL_H__

#include <vector>
#include <mtkcam/aaa/ILscTable.h>
#include <aaa_types.h>
#include "ILscBuf.h"

namespace NSIspTuning
{
    class ILscTbl : public ILscTable
    {
    public:
                                ILscTbl();
                                ILscTbl(TBL_TYPE_T eType);
                                ILscTbl(TBL_TYPE_T eType, MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
                                ILscTbl(ILscTbl const& other);
        virtual ILscTbl&        operator=(ILscTbl const& other);
        virtual                 ~ILscTbl();

        virtual TBL_TYPE_T      getType() const;
        virtual TBL_BAYER_T     getBayer() const;
        virtual const MVOID*    getData() const;
        virtual MUINT32         getSize() const;
        virtual const Config&   getConfig() const;
        virtual const RsvdData& getRsvdData() const;
        virtual MVOID*          editData();
#if (CAM3_3A_IP_BASE)
        virtual MVOID*          editData(int channel);
#endif
        virtual RsvdData&       editRsvdData();
        virtual MBOOL           setData(const MVOID* src, MUINT32 size);
        virtual MBOOL           setConfig(const Config& rCfg);
        virtual MBOOL           setConfig(const ConfigBlk& rCfgBlk);
        virtual MBOOL           setConfig(MINT32 i4W, MINT32 i4H, MINT32 i4GridX, MINT32 i4GridY);
        virtual MBOOL           setBayer(TBL_BAYER_T bayer);
        virtual MBOOL           setRsvdData(const RsvdData& rData);
        virtual MBOOL           reorder(TBL_BAYER_T bayer);
        virtual MBOOL           dump(const char* filename) const;

        virtual MBOOL           cropOut(const TransformCfg_T& trfm, ILscTable& output) const;
        virtual MBOOL           cropToGain(const TransformCfg_T& trfm, ILscTable& output, std::vector<MUINT16*> &vecGain, MUINT32 mode) const;
        virtual MBOOL           mergeAndCrop(ILscTable& baseTbl, ILscTable& output);

        virtual MBOOL           convert(ILscTable& output) const;
        virtual MBOOL           getRaTbl(MUINT32 u4Ratio, ILscTable& output) const;
        virtual MBOOL           toBuf(ILscBuf& buf) const;

        static MINT32           shadingAlign(const ILscTbl& golden, const ILscTbl& unit, const ILscTbl& input, ILscTbl& output, MUINT8 u1Rot=0);
        static ILscTable::Config makeConfig(MINT32 _i4ImgWd, MINT32 _i4ImgHt, MINT32 _i4GridX, MINT32 _i4GridY);

        class LscTblImp;

    protected:
        LscTblImp*              m_pImp;
    };
};

#endif //__I_LSC_TBL_H__
