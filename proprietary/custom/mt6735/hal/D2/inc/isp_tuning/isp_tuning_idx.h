/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _ISP_TUNING_IDX_H_
#define _ISP_TUNING_IDX_H_

#include <string.h>

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// INDEX_T
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    MUINT16  OBC;
    MUINT16  BPC;
    MUINT16  NR1;
    MUINT16  CFA;
    MUINT16  GGM;
    MUINT16  ANR;
    MUINT16  CCR;
    MUINT16  EE;
} CUSTOM_NVRAM_REG_INDEX_T;

typedef CUSTOM_NVRAM_REG_INDEX INDEX_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IndexMgr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct IndexMgr : protected INDEX_T
{
public:
    IndexMgr()
    {
        ::memset(static_cast<INDEX_T*>(this), 0, sizeof(INDEX_T));
    }

    IndexMgr(INDEX_T const& rIndex)
    {
        (*this) = rIndex;
    }

    IndexMgr& operator=(INDEX_T const& rIndex)
    {
        *static_cast<INDEX_T*>(this) = rIndex;
        return  (*this);
    }

public:
    void dump() const;

public: // Set Index
    MBOOL   setIdx_OBC  (MUINT16 const idx);
    MBOOL   setIdx_BPC  (MUINT16 const idx);
    MBOOL   setIdx_NR1  (MUINT16 const idx);
    MBOOL   setIdx_CFA  (MUINT16 const idx);
    MBOOL   setIdx_GGM  (MUINT16 const idx);
    MBOOL   setIdx_ANR  (MUINT16 const idx);
    MBOOL   setIdx_CCR  (MUINT16 const idx);
    MBOOL   setIdx_EE   (MUINT16 const idx);

public:     ////    Get Index
    inline  MUINT16 getIdx_OBC()  const { return OBC; }
    inline  MUINT16 getIdx_BPC()  const { return BPC; }
    inline  MUINT16 getIdx_NR1()  const { return NR1; }
    inline  MUINT16 getIdx_CFA()  const { return CFA; }
    inline  MUINT16 getIdx_GGM()  const { return GGM; }
    inline  MUINT16 getIdx_ANR()  const { return ANR; }
    inline  MUINT16 getIdx_CCR()  const { return CCR; }
    inline  MUINT16 getIdx_EE()   const { return EE; }
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Index Set Template
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <EIspProfile_T ispProfile, MUINT32 sensor = 0, MUINT32 scene = 0, MUINT32 iso = 0>
struct IdxSet
{
    static INDEX_T const idx;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IIdxSetMgrBase
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IdxSetMgrBase
{
public:

    static IdxSetMgrBase& getInstance();

    virtual ~IdxSetMgrBase() {}

public:
    virtual INDEX_T const*
    get(
        MUINT32 ispProfile, MUINT32 sensor=0, MUINT32 const scene=0, MUINT32 const iso=0
    ) const = 0;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IdxSetMgr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IdxSetMgr : public IdxSetMgrBase
{
    friend class IdxSetMgrBase;

private:
    INDEX_T const* m_pNormalPreview      [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pZsdPreview_CC      [eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pZsdPreview_NCC     [eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pNormalCapture      [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pVideoPreview       [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pVideoCapture       [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFCapturePass1     [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFCapturePass2     [eNUM_OF_ISO_IDX];

private:
    MVOID init();
    MVOID linkIndexSet_preview();
    MVOID linkIndexSet_capture();
    MVOID linkIndexSet_video();

private:    ////    Normal
    inline MBOOL isInvalid(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  ( sensor >= ESensorMode_NUM || scene >= eNUM_OF_SCENE_IDX || iso >= eNUM_OF_ISO_IDX );
    }
    inline INDEX_T const* get_NormalPreview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pNormalPreview[sensor][scene][iso];
    }
    inline INDEX_T const* get_ZsdPreview_CC(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pZsdPreview_CC[scene][iso];
    }
    inline INDEX_T const* get_ZsdPreview_NCC(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pZsdPreview_NCC[scene][iso];
    }
    inline INDEX_T const* get_NormalCapture(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pNormalCapture[sensor][scene][iso];
    }

private:
    inline INDEX_T const* get_VideoPreview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(sensor, 0, iso) ? NULL : m_pVideoPreview[sensor][scene][iso];
    }
    inline INDEX_T const* get_VideoCapture(MUINT32 const sensor, MUINT32 const /*scene*/, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(sensor, 0, iso) ? NULL : m_pVideoCapture[iso];
    }
    inline INDEX_T const* get_MFCapturePass1(MUINT32 const sensor, MUINT32 const /*scene*/, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(sensor, 0, iso) ? NULL : m_pMFCapturePass1[iso];
    }
    inline INDEX_T const* get_MFCapturePass2(MUINT32 const sensor, MUINT32 const /*scene*/, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(sensor, 0, iso) ? NULL : m_pMFCapturePass2[iso];
    }

public:
    virtual
    INDEX_T const*
    get(MUINT32 ispProfile, MUINT32 sensor/*=0*/, MUINT32 const scene/*=0*/, MUINT32 const iso/*=0*/) const;

};  //  class IdxSetMgr

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IDX_SET
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define IDX_SET(OBC, BPC, NR1, CFA, GGM, ANR, CCR, EE)\
    {\
        OBC, BPC, NR1, CFA, GGM, ANR, CCR, EE\
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Scene-Dep.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define IDX_MODE_NormalPreview(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_NormalPreview, sensor, scene, iso>::idx =

#define IDX_MODE_ZSDPreview_CC(scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_ZsdPreview_CC, scene, iso>::idx =

#define IDX_MODE_ZSDPreview_NCC(scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_ZsdPreview_NCC, scene, iso>::idx =

#define IDX_MODE_NormalCapture(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_NormalCapture, sensor, scene, iso>::idx =

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Scene-Indep.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define IDX_MODE_VideoPreview(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_VideoPreview, sensor, scene, iso>::idx =

#define IDX_MODE_VideoCapture(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_VideoCapture, iso>::idx =

#define IDX_MODE_MFCapturePass1(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFCapPass1, iso>::idx =

#define IDX_MODE_MFCapturePass2(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFCapPass2, iso>::idx =

};  //  NSIspTuning
#endif //  _ISP_TUNING_IDX_H_

