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
#ifndef _OPEN_SHADING_H_
#define _OPEN_SHADING_H_

#include <ILscTsf.h>
#include <lsc/ILscTbl.h>

namespace NSIspTuning
{
class OpenShading : public ILscTsf
{
public:
    static ILscTsf*     createInstance(MUINT32 u4SensorDev);
    virtual void        destroyInstance();
    virtual MBOOL       init();
    virtual MBOOL       uninit();
    virtual MBOOL       setOnOff(MBOOL fgOnOff);
    virtual MBOOL       getOnOff() const;
    virtual MBOOL       setIsFactory(MBOOL fgOnOff);
    virtual MBOOL       setConfig(ESensorMode_T eSensorMode, MUINT32 u4W, MUINT32 u4H, MBOOL fgForce);
    virtual MBOOL       update(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo);
    virtual MBOOL       updateRatio(MUINT32 /*u4MagicNum*/, MUINT32 /*u4Rto*/) {return MTRUE;}
    virtual E_LSC_TSF_TYPE_T    getType() const {return E_LSC_TSF_TYPE_OpenShading;}

protected:
    #define CT_NUM 3
                        OpenShading(MUINT32 u4SensorDev);
    virtual             ~OpenShading();

    virtual MBOOL       start();
    virtual MBOOL       perframe(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo);

    MUINT32             m_u4SensorDev;

    MBOOL               m_fgOnOff;

    ILscMgr*            m_pLsc;
    ILscTbl*            m_pTbl[CT_NUM];  // 3 color temperature
};
};
#endif //_TSF_DFT_H_

