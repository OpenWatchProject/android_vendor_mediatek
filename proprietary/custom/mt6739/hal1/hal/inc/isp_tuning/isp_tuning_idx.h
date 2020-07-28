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

#define SCENE_MODE_MAPPING_EN 0

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// INDEX_T
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    MUINT16  DBS;
    MUINT16  OBC;
    MUINT16  BNR_BPC;
    MUINT16  BNR_NR1;
    MUINT16  BNR_PDC;
    MUINT16  CFA;
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
    MBOOL   setIdx_DBS  (MUINT16 const idx);
    MBOOL   setIdx_OBC  (MUINT16 const idx);
    MBOOL   setIdx_BPC  (MUINT16 const idx);
    MBOOL   setIdx_NR1  (MUINT16 const idx);
    MBOOL   setIdx_PDC  (MUINT16 const idx);
    MBOOL   setIdx_CFA  (MUINT16 const idx);
    MBOOL   setIdx_GGM  (MUINT16 const idx);
    MBOOL   setIdx_ANR  (MUINT16 const idx);
    MBOOL   setIdx_CCR  (MUINT16 const idx);
    MBOOL   setIdx_EE   (MUINT16 const idx);

public:     ////    Get Index
    inline  MUINT16 getIdx_DBS()  const { return DBS; }
    inline  MUINT16 getIdx_OBC()  const { return OBC; }
    inline  MUINT16 getIdx_BPC()  const { return BNR_BPC; }
    inline  MUINT16 getIdx_NR1()  const { return BNR_NR1; }
    inline  MUINT16 getIdx_PDC()  const { return BNR_PDC; }

    inline  MUINT16 getIdx_CFA()  const { return CFA; }
//    inline  MUINT16 getIdx_GGM()  const { return GGM; }
    inline  MUINT16 getIdx_ANR()  const { return ANR; }
    inline  MUINT16 getIdx_CCR()  const { return CCR; }
    inline  MUINT16 getIdx_EE()  const { return EE; }

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
        MUINT32 ispProfile, MUINT32 sensor/*=0*/, MUINT32 const /*scene_=0*/, MUINT32 const iso/*=0*/
    ) const = 0;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IdxSetMgr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IdxSetMgr : public IdxSetMgrBase
{
    friend class IdxSetMgrBase;

private:
    INDEX_T const* m_pPreview                   [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pVideo                     [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pCapture                   [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMulti_Pass_ANR1           [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMulti_Pass_ANR2           [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pFlash_Cap                 [eNUM_OF_ISO_IDX];

private:
    MVOID linkIndexSet();

private:    ////    Normal
    inline MBOOL isInvalid(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  ( sensor >= ESensorMode_NUM || scene >= eNUM_OF_SCENE_IDX || iso >= eNUM_OF_ISO_IDX );
    }
    inline INDEX_T const* get_Preview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pPreview[sensor][scene][iso];
    }
    inline INDEX_T const* get_Video(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pVideo[sensor][scene][iso];
    }
    inline INDEX_T const* get_Capture(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pCapture[sensor][scene][iso];
    }
    inline INDEX_T const* get_Multi_Pass_ANR1(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMulti_Pass_ANR1[iso];
    }
    inline INDEX_T const* get_Multi_Pass_ANR2(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMulti_Pass_ANR2[iso];
    }


public:
    virtual
    INDEX_T const*
    get(MUINT32 ispProfile, MUINT32 const sensor/*=0*/, MUINT32 const scene/*=0*/, MUINT32 const iso/*=0*/) const;

};  //  class IdxSetMgr

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IDX_SET
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define IDX_SET(DBS, OBC, BNR_BPC, BNR_NR1, BNR_PDC, CFA, ANR, CCR, EE)\
    {\
        DBS, OBC, BNR_BPC, BNR_NR1, BNR_PDC, CFA, ANR, CCR, EE\
    }

#define IDX_MODE_Preview(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Preview, sensor, scene, iso>::idx =

#define IDX_MODE_Video(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Video, sensor, scene, iso>::idx =

#define IDX_MODE_Capture(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture, sensor, scene, iso>::idx =

#define IDX_MODE_Multi_Pass_ANR1(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture_MultiPass_ANR_1, 0, 0, iso>::idx =

#define IDX_MODE_Multi_Pass_ANR2(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture_MultiPass_ANR_2, 0, 0, iso>::idx =

#define IDX_MODE_Flash_Capture(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_FLASH_Capture, 0, 0, iso>::idx =

};  //  NSIspTuning
#endif //  _ISP_TUNING_IDX_H_

