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

#if 1 //TO_DO: delete after Cannon EP
#define E_CAMIO_Pattern E_CamPattern
#define ECamio_Normal   eCAM_NORMAL
#define ECamio_Dual_pix eCAM_DUAL_PIX
#endif

#include <Cam_Notify_datatype.h>
#include <mtkcam/drv/def/ICam_type.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

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
 * @enum E_CAMIO_SEN
 *
 * @total number of input sensor.
 *   ECamio_1_SEN : lower power consumption.
 *   ECamio_2_SEN : higher power consumption.
 *
 ******************************************************************************/
typedef enum{
    ECamio_1_SEN    = 1,    // total have 1 input sensor
    ECamio_2_SEN    = 2,    // total have 2 input sensor like pip/daul_cam
}E_CAMIO_SEN;



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
                MUINT32 bin_off          :  1;       // 1: force to off frontal binning
                MUINT32 DATA_PATTERN     :  3;       //plz reference to E_CamPattern
                MUINT32 SUBSAMPLE        :  5;       //subsample function,no subsample is subsample = 0.
                MUINT32 PBN_SEP_MODE     :  1;       //plz reference to E_CAMIO_SepMode
                MUINT32 Density          :  2;       // for Dual PD, 0 for Low,1 for High
                MUINT32 SensorNum        :  2;       // plz reference to E_CAMIO_SEN
                MUINT32 DynamicTwin      :  1;       // ON/OFF dynamic twin flow.
                MUINT32 IQlv             :  1;       //plz reference to E_CamIQLevel
                MUINT32 RMB_SEL          :  2;
                MUINT32 rsv              : 14;
        } Bits;
        MUINT32 Raw;
}CAMIO_Func;

/******************************************************************************
 * @enum E_BUF_STATUS
 *
 * @result status of buffer control
 *   eBuf_Fail : enque/deque fail
 *   eBuf_Pass : enque/deque pass
 *   eBuf_suspending : pipeline is under suspending when executing enque/deque
 *   eBuf_stopped : pipeline is already stopped when executing enque/deque
 *
 ******************************************************************************/
typedef enum{
    eBuf_Fail = 0,
    eBuf_Pass = 1,
    eBuf_suspending,    //pipeline is suspending.
    eBuf_stopped,       //pipeline is already stopped.
}E_BUF_STATUS;


/**
    enum for cam_a/cam_b/cam_c ...etc

*/
typedef enum {
    EPipe_UNKNOWN  = 0x00,
    EPipe_CAM_A    = 0x01,
    EPipe_CAM_B    = 0x02,
    //EPipe_CAM_C    = 0x04,
    //EPipe_CAM_D    = 0x08,
}E_CAMIO_CAM;

/**
    stuct for cropping informaiton at each cam.
*/
typedef struct{
    MUINT32 x;
    MUINT32 y;
    MUINT32 w;
    MUINT32 h;
}ST_RECT;

typedef struct{
    E_CAMIO_CAM             Pipe_path;
    std::vector<ST_RECT>    v_rect;     //if path == (ENPipe_CAM_A|ENPipe_CAM_B) =>vector at 0 map to CAM_A's crop, vector at 1 map to CAM_B's crop
}ST_CAMIO_CROP_INFO;
typedef std::vector<ST_CAMIO_CROP_INFO>    V_CAMIO_CROP_INFO;

/*******************************************************************************
* Pipe Interface
********************************************************************************/
class ICamIOPipe
{
public:     ////    Instantiation.
    typedef enum{
        CAMIO   = 0,    //camiopipe
        STTIO   = 1,    //sttiopipe
        CAMSVIO = 2,    //camsviopipe
    }E_PIPE_OPT;

    typedef enum{
        CAMIO_DROP      = 0x0000,
        CAMIO_D_Twin    = 0x0001,
        //CAMIO_D_BIN     = 0x0002,
        CAMIO_IQ        = 0x0002,
    }E_FRM_EVENT;

    typedef void (* CAMIO_CB) (MUINT32);

    static ICamIOPipe* createInstance(MINT8 const szUsrName[32], E_INPUT& InOutPut, E_PIPE_OPT opt, PortInfo const *portInfo = NULL);
    virtual MVOID   destroyInstance();

                    ICamIOPipe(void){m_pipe_opt = CAMIO;}
    virtual         ~ICamIOPipe(void){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL   init() = 0;
    virtual MBOOL   uninit() = 0;

    virtual MBOOL   start() = 0;
    virtual MBOOL   stop(MBOOL bForce = MFALSE) = 0;

    //for ESD flow
    typedef enum{
        HW_RST_SUSPEND = 0,     // for ESD recovery flow
        SW_SUSPEND,             // for pause/resume streaming
        NUM_SUSPEND_MODE
    }E_SUSPEND_MODE;
    virtual MBOOL   suspend(E_SUSPEND_MODE suspendMode = NUM_SUSPEND_MODE) = 0;  //for ESD stop
    virtual MBOOL   resume(E_SUSPEND_MODE suspendMode = NUM_SUSPEND_MODE) = 0;   //recover frorm ESD stop
    virtual MBOOL   preresume(MINT8 const szUsrName[32], MUINT32 cropx)
                    {return MTRUE;}
    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo) = 0;
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0;
    //
    virtual MINT32  enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0) = 0;
    virtual E_BUF_STATUS dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL) = 0;
    //
    virtual MBOOL   configFrame(E_FRM_EVENT event,CAMIO_Func func) = 0;
    //
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc = NULL) = 0;

    virtual MINT32  registerIrq(MINT8 const szUsrName[32]) = 0;
    virtual MBOOL   irq(Irq_t* pIrq) = 0;
    virtual MBOOL   signalIrq(Irq_t irq) = 0;
    virtual MBOOL   abortDma(PortID const port) = 0;
    virtual MUINT32* getIspReg(MUINT32 path) = 0;
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;
    virtual MUINT32 ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE) = 0;
    virtual MUINT32 UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE) = 0;

protected:
            E_PIPE_OPT  m_pipe_opt;
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_CAMIO_PIPE_H_

