#ifndef _CAM_DUPCMDQMGR_H_
#define _CAM_DUPCMDQMGR_H_

#include "isp_drv_cam.h"


class DupCmdQMgr
{
public:
    DupCmdQMgr(){}
    virtual ~DupCmdQMgr(){}

     static DupCmdQMgr*     CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void) = 0;
     static DupCmdQMgr*     CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2) = 0;

    virtual MINT32          CmdQMgr_ImmStart(void) {return 0;};
    virtual MINT32          CmdQMgr_start(void) = 0;
    virtual MINT32          CmdQMgr_stop(void) = 0;


    virtual MINT32          CmdQMgr_suspend(void) = 0;  //for twin's ctrl
    virtual MINT32          CmdQMgr_resume(void) = 0;   //for twin's ctrl
    virtual MBOOL           CmdQMgr_updateCQMode(void) = 0; //for twin's ctrl.switch dummy page cq opeartion under twin

    virtual MINT32          CmdQMgr_update(void) = 0;

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx) = 0;

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void)  = 0;
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void) = 0;

    virtual MUINT32         CmdQMgr_GetBurstQ(void) = 0;
    virtual MUINT32         CmdQMgr_GetDuqQ(void) = 0;

    virtual MBOOL           CmdQMgr_GetCurStatus(void) = 0;
    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void) = 0;


    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE) = 0;


public:
    enum{
        REV_CQ = 15,
    };

};
#endif