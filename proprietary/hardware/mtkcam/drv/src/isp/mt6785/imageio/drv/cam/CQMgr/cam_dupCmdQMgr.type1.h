#ifndef _CAM_DUPCMDQMGR_TYPE1_H_
#define _CAM_DUPCMDQMGR_TYPE1_H_

#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.log.h"


class CmdQMgrImp : public DupCmdQMgr
{
public:
    CmdQMgrImp();
    virtual ~CmdQMgrImp(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);

    virtual MINT32          CmdQMgr_suspend(void);
    virtual MINT32          CmdQMgr_resume(void);

    virtual MINT32          CmdQMgr_update(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void);
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

            DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2){arg1;arg2;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);

private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);
            MBOOL           CmdQMgr_specialCQCtrl(void);
public:
    //public for slave cam
    ISP_DRV_CAM***  pCamDrv;
    ISP_DRV_CAM***  pDummyDrv;
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop,
        op_suspend,
    };
    MUINT32         mBurstQ;
    MUINT32         mDupCmdQ;

    MUINT32         mBurstQIdx;
    MUINT32         mDupCmdQIdx;

    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMA_CQ0         m_CQ0;

    char***         m_pUserName;
    char***         m_pDummyName;


};


#endif
