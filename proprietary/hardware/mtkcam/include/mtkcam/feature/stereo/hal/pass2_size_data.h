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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _PASS2_SIZE_DATA_H_
#define _PASS2_SIZE_DATA_H_

#include "stereo_common.h"
#include "StereoArea.h"

namespace StereoHAL {

enum ENUM_PASS2_ROUND
{
    PASS2A,
    PASS2A_2,
    PASS2A_3,
    PASS2A_P,
    PASS2A_P_2,
    PASS2A_P_3,
    PASS2A_B,
    PASS2A_CROP,
    PASS2A_B_CROP,
    PASS2A_P_CROP,

    //Put this line in the end
    PASS2_ROUND_COUNT
};

static const char *PASS2_ROUND_NAME[] =
{
    "PASS2A",
    "PASS2A_2",
    "PASS2A_3",
    "PASS2A_P",
    "PASS2A_P_2",
    "PASS2A_P_3",
    "PASS2A_B",
    "PASS2A_CROP",
    "PASS2A_B_CROP",
    "PASS2A_P_CROP",
};

//================================================
//  For Pass 2 Output
//================================================
struct Pass2SizeInfo
{
    StereoArea areaWDMA;
    StereoArea areaWROT;
    StereoArea areaIMG2O;
    StereoArea areaIMG3O;
    StereoArea areaFEO;

    inline  Pass2SizeInfo()
            : areaWDMA()
            , areaWROT()
            , areaIMG2O()
            , areaIMG3O()
            , areaFEO()
            {
            }

    inline  Pass2SizeInfo(StereoArea wdma, NSCam::MSize wrot, NSCam::MSize img2o, NSCam::MSize img3o, StereoArea feArea)
            : areaWDMA(wdma)
            , areaWROT(wrot)
            , areaIMG2O(img2o)
            , areaIMG3O(img3o)
            , areaFEO(feArea)
            {
            }

    inline  Pass2SizeInfo(StereoArea wdma, StereoArea wrot, StereoArea img2o, StereoArea img3o, StereoArea feArea)
            : areaWDMA(wdma)
            , areaWROT(wrot)
            , areaIMG2O(img2o)
            , areaIMG3O(img3o)
            , areaFEO(feArea)
            {
            }
};

const Pass2SizeInfo PASS2_SIZE_ZERO;

};  //namespace StereoHAL
#endif