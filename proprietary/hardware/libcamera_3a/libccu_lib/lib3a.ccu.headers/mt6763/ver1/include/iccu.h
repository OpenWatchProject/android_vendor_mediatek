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
#ifndef _I_CCU_H_
#define _I_CCU_H_

//#include <mtkcam/def/common.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {
namespace NSCcuDrv   {
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    CCUIF_SUCCEED = 0,
    CCUIF_ERR_UNKNOWN,
}CCU_ERROR_ENUM;
/*******************************************************************************
* Pipe Interface
********************************************************************************/
class ICcu
{
public:     ////    Instantiation.
    typedef enum{
        AE    = 0,        //mapping to hw module AEAWB      0
        AWB   = 1,        //mapping to hw module AWB          1
        AEAWB = 2,        //mapping to hw module AEAWB      2
    }E_CCUIF_INPUT;

    typedef enum{
        AECTRL    = 0,    //AECTRL
        AWBCTRL   = 1,    //AWBCTRL
        AEAWBCTRL = 2,    //AEAWB
    }E_CCUIF_OPT;
    static ICcu* createInstance(MINT8 const szUsrName[32],E_CCUIF_INPUT InPut,E_CCUIF_OPT opt);
    virtual MVOID  destroyInstance();

                    ICcu(void){}
    virtual         ~ICcu(void){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL   init() = 0;
    virtual MBOOL   uninit() = 0;

    //initialization
    virtual MBOOL ccu_check_sensor_support(WHICH_SENSOR_T sensorDev) = 0;
    virtual int ccu_boot(WHICH_SENSOR_T sensorDev) = 0;

    //ccu operation interface
    virtual int ccu_control(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr) = 0;

    //un-initialization
    virtual int ccu_shutdown() = 0;
    virtual int ccu_pause() = 0;

    //debug
    virtual MUINT32 readInfoReg(MUINT32 regNo) = 0;

protected:
    E_CCUIF_OPT  m_ccuif_opt;

};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCcuDrv
};  //namespace NSCcuIf
#endif  //  _I_CCU_H_

