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
#ifndef _I_LSC_TSF_H_
#define _I_LSC_TSF_H_

#include <lsc/ILscMgr.h>

namespace NSIspTuning
{
class ILscTsf
{
public:
    typedef enum
    {
        E_LSC_TSF_TYPE_0 = 0,
        E_LSC_TSF_TYPE_1,
        E_LSC_TSF_TYPE_2,
        E_LSC_TSF_TYPE_OpenShading
    } E_LSC_TSF_TYPE_T;

    static ILscTsf*             createInstance(MUINT32 u4SensorDev, E_LSC_TSF_TYPE_T eType);
    virtual void                destroyInstance() = 0;
    virtual MBOOL               init(ESensorMode_T eFullSensorMode) = 0;
    virtual MBOOL               uninit() = 0;
    virtual MBOOL               setOnOff(MBOOL fgOnOff) = 0;
    virtual MBOOL               getOnOff() const = 0;
    virtual MBOOL               setConfig(ESensorMode_T eSensorMode, MUINT32 u4W, MUINT32 u4H, MBOOL fgForce) = 0;
    virtual MBOOL               update(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo) = 0;
    virtual MBOOL               updateRatio(MUINT32 u4MagicNum, MUINT32 u4Rto) = 0;
    virtual MBOOL               getTbl(MUINT32 /*u4GridX*/, MUINT32 /*u4GridY*/, MUINT32 /*u4MagicNum*/, ILscTbl& /*output*/) {return MFALSE;}
    virtual E_LSC_TSF_TYPE_T    getType() const = 0;
    virtual MBOOL               setDumpFileCfg(MBOOL /*fgSave*/, const char* /*pathname*/) {return MFALSE;}
    virtual MBOOL               setDebugInfo(MVOID* /*pDbgInfo*/) {return MFALSE;}
    virtual const MVOID*        getRsvdData() const {return NULL;}
    virtual MBOOL               waitDone() {return MTRUE;}

protected:
                                ILscTsf(){}
    virtual                     ~ILscTsf(){}
};
};
#endif //_I_LSC_TSF_H_
