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
#ifndef _ISPIO_I_CAMIO_PIPE_H_
#define _ISPIO_I_CAMIO_PIPE_H_

#include <Cam_Notify_datatype.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 * @enum E_CAMIO_Pattern
 *
 * @input data pattern
 *
 ******************************************************************************/
typedef enum{
    ECamio_Normal   = 0,
    ECamio_Dual_pix = 1,    //dual pd's pattern
}E_CAMIO_Pattern;


/******************************************************************************
 * @enum E_CAMIO_SepMode
 *
 * @input pbin seperate mode
 *
 ******************************************************************************/
typedef enum{
    ECamio_Non_Sep    = 0,
    ECamio_Seperate   = 1,
}E_CAMIO_SepMode;

/******************************************************************************
 * @struct CAMIO_Func
 *
 * @config cam's extra function
 *
 ******************************************************************************/
typedef union
{
        struct
        {
                MUINT32 bin_off          :  1;
                MUINT32 DATA_PATTERN     :  3;       //plz reference to E_CAMIO_Pattern
                MUINT32 SUBSAMPLE        :  5;       //subsample function,no subsample is subsample = 0.
                MUINT32 FAST_AF          :  1;
                MUINT32 rsv              : 22;
        } Bits;
        MUINT32 Raw;
}CAMIO_Func;


/*******************************************************************************
* Pipe Interface
********************************************************************************/
class ICamIOPipe
{
public:     ////    Instantiation.
    typedef enum{
        TG_A = 0,        //mapping to hw module CAM_A      0
        TG_B = 1,        //mapping to hw module CAM_B      1
        TG_CAMSV_0 = 10, //mapping to hw module CAMSV_0    2
        TG_CAMSV_1 = 11, //mapping to hw module CAMSV_1    3
        TG_CAMSV_2 = 12, //mapping to hw module CAMSV_2    4
        TG_CAMSV_3 = 13, //mapping to hw module CAMSV_3    5
        TG_CAMSV_4 = 14, //mapping to hw module CAMSV_4    6
        TG_CAMSV_5 = 15, //mapping to hw module CAMSV_5    7
    }E_INPUT;

    typedef enum{
        CAMIO     = 0,    //camiopipe
        STTIO     = 1,    //sttiopipe
        CAMSVIO   = 2,    //camsviopipe
        UNKNOWNIO = 3,    //unknowniopipe
    }E_PIPE_OPT;

    typedef enum _E_ENQ_DEQ_STATE{
        ST_FAIL = -1,
        ST_SUCCESS = 0,
        ST_STOP = 1,
        ST_SUSPEND = 2,
    }E_ENQ_DEQ_STATE;

    static ICamIOPipe* createInstance(MINT8 const szUsrName[32],E_INPUT InPut,E_PIPE_OPT opt);
    static ICamIOPipe* getInstance(E_PIPE_OPT opt,E_INPUT InPut);
    virtual MVOID   destroyInstance();

                    ICamIOPipe(void){ m_pipe_opt = UNKNOWNIO; }
    virtual         ~ICamIOPipe(void){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL   init() = 0;
    virtual MBOOL   uninit() = 0;

    virtual MBOOL   start() = 0;
    virtual MBOOL   stop(MBOOL bForce = MFALSE, MBOOL detachUni = MFALSE) = 0;

    //for ESD flow
    typedef enum{
        HW_RST_SUSPEND = 0,     // for ESD recovery flow
        SW_SUSPEND,             // for pause/resume streaming
        NUM_SUSPEND_MODE
    }E_SUSPEND_MODE;
    virtual MBOOL   suspend(E_SUSPEND_MODE suspendMode) = 0;  //for ESD stop
    virtual MBOOL   resume(E_SUSPEND_MODE suspendMode) = 0;   //recover frorm ESD stop

    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo) = 0;
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0;
    //
    virtual MINT32  enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0) = 0;
    virtual MINT32  dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL) = 0;

    //
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts,CAMIO_Func func) = 0;

    virtual MINT32  registerIrq(MINT8 const szUsrName[32]) = 0;
    virtual MBOOL   irq(Irq_t* pIrq) = 0;
    virtual MBOOL   signalIrq(Irq_t irq) = 0;
    virtual MBOOL   abortDma(PortID const port) = 0;
    virtual MUINT32* getIspReg(MUINT32 path) = 0;
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;
    virtual MUINT32 ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE) = 0;
    virtual MUINT32 UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE) = 0;

    virtual MVOID   rstBufCtrl(MBOOL rst) = 0;

protected:
            E_PIPE_OPT  m_pipe_opt;
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_CAMIO_PIPE_H_

