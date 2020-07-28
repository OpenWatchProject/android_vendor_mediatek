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
#ifndef _I_AE_TEST_H_
#define _I_AE_TEST_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <custom/aaa/AEPlinetable.h>
using namespace NSIspTuning;
namespace NS3Av3
{
class IAeTest
{
public:

    typedef enum {
      AE_AT_TYPE_NON = 0,
      AE_AT_TYPE_ISO = 1, // for testing ISO change
      AE_AT_TYPE_SCENE = 2, // for testing scene change
      AE_AT_TYPE_EVCOMP = 3, // for testing EV comp change
      AE_AT_TYPE_Pre2Video = 4, // for testing preview->video
      AE_AT_TYPE_Pre2Cap = 5, // for testing preview->capture
    }E_AE_TEST_T;

    static IAeTest*                           getInstance(ESensorDev_T sensor);
    virtual MVOID                             AeTestInit() = 0;
    virtual MVOID                             monitorAvgY(MINT32 avgy) = 0;
    virtual MVOID                             recordAvgY(E_AE_TEST_T testtype ,MINT32 avgy) = 0;
    virtual MVOID                             resetAvgY(E_AE_TEST_T testtype ,MINT32 avgy) = 0;
    virtual MBOOL                             IsPerframeCtrl() = 0;
    virtual MVOID                             PerframeCtrl(MINT32 avgy, MBOOL istypeAsensor,ESensorMode_T m_eSensorMode,
                                                           AE_PLINETABLE_T* pAEPlineTable, strFinerEvPline* pPreviewTableF, MUINT32 u4IndexFMax) = 0;
    

protected:
    virtual                                     ~IAeTest(){}
};
};
#endif //_I_AE_TEST_H_