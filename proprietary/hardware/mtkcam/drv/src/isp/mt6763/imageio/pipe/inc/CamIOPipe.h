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
#ifndef _ISPIO_CAMIO_PIPE_H_
#define _ISPIO_CAMIO_PIPE_H_
//
#include <vector>
#include <map>
#include <list>

//
using namespace std;
//
#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>
//
#include "cam_path_cam.h"
#include "kd_imgsensor_define.h"
#include <ICamIOPipe.h>
#include "uni_mgr.h"
#include "twin_mgr.h"
#include "rlb_mgr.h"
#include <cam_dfs.h>

//
/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
//class for dynamic path ctrl
class CamIO_PathCtrl
{
public:
                        CamIO_PathCtrl(){m_hwModule = CAM_MAX;m_isTwin = MFALSE;}
public:
            MBOOL       TwinPath(void);
            MBOOL       SinglePath(void);

public:
    TWIN_MGR_PATH_CFG_IN_PARAM  m_cfgInParam;   //for dynamic twin
    ISP_HW_MODULE               m_hwModule;
    NSImageio::NSIspio::PortID  m_Src;
    MBOOL                       m_isTwin;       //previous frame's result
};

class CamIOPipe : public ICamIOPipe
{
//
public:
    friend class CamIO_PathCtrl;
    static CamIOPipe*   Create(MINT8 const szUsrName[32], E_INPUT InPut);
                void    Destroy(void);


                    CamIOPipe();
                    virtual ~CamIOPipe();
private:
    //


    typedef enum _E_FSM{
        op_unknown  = 0,
        op_init     = 1,
        op_cfg      = 2,
        op_start    = 3,
        op_stop     = 4,
        op_uninit   = 5,
        op_cmd      = 6,
        op_endeq    = 7,
        op_suspend  = 8,
        op_resume   = 9
    }E_FSM;

    class ResMgr{
        public:
            ResMgr(){
                m_occupied = MFALSE;
                m_User[0] = '\0';
                m_hwModule = CAM_MAX;

            }

            MBOOL   Register_Res(char Name[32]);
            MBOOL   Release_Res(void);


            char*   Get_Res(void);                          // if [0] = '\0' -> available, else -> occupied
            MUINT32 Get_AvailNum(void);                    // avail cam for twin, count only occupied by other sensor, twin is excluded
        public:
            ISP_HW_MODULE   m_hwModule;
        private:
            MBOOL           m_occupied;
            char            m_User[32];
            mutable Mutex   mLock;
            static MUINT32  m_available;                        //count only occupied by other sensor, twin is excluded
    };

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();


    virtual MBOOL   start();
    virtual MBOOL   stop(MBOOL bForce = MFALSE, MBOOL detachUni = MFALSE);
    virtual MBOOL   suspend(E_SUSPEND_MODE sMode);
    virtual MBOOL   resume(E_SUSPEND_MODE sMode);


    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);
    //
    virtual MINT32  enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0);
    virtual E_BUF_STATUS dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL);


    virtual MBOOL   configFrame(E_FRM_EVENT event,CAMIO_Func func);
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts,CAMIO_Func func);

    ////    Interrupt handling
    virtual MINT32  registerIrq(MINT8 const szUsrName[32]);
    virtual MBOOL   irq(Irq_t* pIrq);
    virtual MBOOL   signalIrq(Irq_t irq);
    virtual MBOOL   abortDma(PortID const port);

    virtual MUINT32* getIspReg(MUINT32 path){path;return NULL;}
    ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    virtual MUINT32 ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE);
    virtual MUINT32 UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy = MTRUE);
//
private:
            _isp_dma_enum_  PortID_MAP(MUINT32 PortID);
            char *          Name_MAP(ISP_HW_MODULE _hwModule);

            MBOOL           FSM_CHECK(MUINT32 op);
            MBOOL           FSM_UPDATE(MUINT32 op);

            MINT32          SrcFmtToHwVal_TG( ImgInfo::EImgFmt_t imgFmt);
            MINT32          SrcFmtToHwVal_DMA( ImgInfo::EImgFmt_t imgFmt,MUINT32* pPixel_byte);
            MINT32          SrcPixIDToHwVal(ImgInfo::ERawPxlID_t pixId);
            MINT32          HwValToSrcFmt_TG( MUINT32 portid,MUINT32 imgFmt);
            MINT32          HwValToSrcFmt_DMA( MUINT32 portid,MUINT32 imgFmt);
            MINT32          HwValToSrcPixID(MUINT32 pixId);
            MINT32          PxlMode(EPxlMode pxlMode,ImgInfo::EImgFmt_t fmt, MINT32* bTwin, MINT32* two_pxl, MINT32* en_p1, MINT32* two_pxl_dmx);
            MINT32          getOutPxlByteNFmt( MUINT32 bRRZO, ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt);

            MBOOL           configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap=MFALSE, MUINT32 isBypassOffset=MTRUE,EIMAGE_STRIDE planeNum=ESTRIDE_1ST_PLANE);

            //class for rss/eis/lcs buf ctrl
            class CAMIO_STT{
                public:
                    CAMIO_STT(){pCamPath = NULL;m_hwModule = CAM_MAX;}
                    MUINT32         QueryPort(MUINT32 PortID);
                    MINT32          enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MUINT32 nBurst);
                    E_BUF_STATUS    dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,MUINT32 nBurst, CAM_STATE_NOTIFY *pNotify = NULL);

                private:
                    _isp_dma_enum_  PortID_MAP(MUINT32 PortID);

                public:
                    CamPathPass1*   pCamPath;
                    ISP_HW_MODULE   m_hwModule;
                    UniMgr          m_UniMgr;
            };

private:
    ResMgr                  m_ResMgr;

    CamPathPass1            m_CamPathPass1;
    CamPathPass1Parameter   m_camPass1Param;

    mutable Mutex           mRunTimeCfgLock;  // for multi-thread


    DupCmdQMgr*             m_pCmdQMgr;
    DupCmdQMgr*             m_pCmdQMgr_AE;  //for ae smoothing function
    DupCmdQMgr*             m_pHighSpeedCmdQMgr;
    UniMgr                  m_UniMgr;
    TwinMgr*                m_TwinMgr;
    MBOOL                   m_bStartUniStreaming;


    mutable Mutex           m_FSMLock;
    E_FSM                   m_FSM;

    CAMIO_STT               m_CamIO_stt;
    ISP_HW_MODULE           m_hwModule;

    MBOOL                   m_DynamicRawType;

    MBOOL                   m_bUpdate;  //update before start


    CamIO_PathCtrl          m_PathCtrl;

    CAM_DFS_Mgr*            m_cam_dfs;
    MBOOL                   m_bBin_On;  //this is for fix-binning under dynamic twin mode. (bin can be changed dynamically during streaming)
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_CAMIO_PIPE_H_

