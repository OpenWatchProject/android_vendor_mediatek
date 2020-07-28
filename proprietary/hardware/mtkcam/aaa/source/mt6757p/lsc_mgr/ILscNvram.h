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
#ifndef _I_LSC_NVRAM_H_
#define _I_LSC_NVRAM_H_

#include <aaa_types.h>
#include <isp_tuning.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <camera_custom_cam_cal.h>
#include <camera_custom_nvram.h>
#include <lsc/ILscTbl.h>
#include <pthread.h>
#include <mutex>

namespace NSIspTuning
{
class ILscNvram
{
public:
    typedef enum
    {
        E_LSC_OTP_ERROR         = 0,
        E_LSC_NO_OTP            = 1,
        E_LSC_WITH_MTK_OTP      = 2
    } E_LSC_OTP_T;

    static ILscNvram*                           getInstance(ESensorDev_T sensor);
    virtual NVRAM_CAMERA_ISP_PARAM_STRUCT*      getIspNvram() const = 0;
    virtual ISP_SHADING_STRUCT*                 getLscNvram() const = 0;
    virtual NVRAM_CAMERA_3A_STRUCT*             get3ANvram() const = 0;
    virtual const CAMERA_TSF_TBL_STRUCT*        getTsfNvram() const = 0;
    virtual const CAM_CAL_SINGLE_LSC_STRUCT*    getOtpData() const = 0;
    virtual E_LSC_OTP_T                         getOtpState() const = 0;

    virtual MUINT32*                            getLut(ESensorMode_T eLscScn) const = 0;
    virtual MUINT32*                            getLut(ESensorMode_T eLscScn, MUINT32 u4CtIdx) const = 0;

//    virtual MUINT32                             getTotalLutSize(ESensorMode_T eLscScn) const = 0;
//    virtual MUINT32                             getPerLutSize(ESensorMode_T eLscScn) const = 0;
    virtual MBOOL                               check123InNvram() const = 0;
    virtual const ILscTbl*                      getGolden() const = 0;
    virtual const ILscTbl*                      getUnit() const = 0;

    virtual MBOOL                               readNvramTbl(MBOOL fgForce) = 0;
    virtual MBOOL                               writeNvramTbl(void) = 0;
protected:
    virtual                                     ~ILscNvram(){}
};
};
#endif //_I_LSC_NVRAM_H_
