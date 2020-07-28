#ifndef _CAM_DUPCMDQMGR_H_
#define _CAM_DUPCMDQMGR_H_

#include "isp_drv_cam.h"

class DupCmdQMgr
{
public:
    DupCmdQMgr();
    virtual ~DupCmdQMgr(){}

     static DupCmdQMgr*     CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void) = 0;
     static DupCmdQMgr*     CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2) = 0;

    virtual MINT32          CmdQMgr_start(void) = 0;
    virtual MINT32          CmdQMgr_stop(void) = 0;


    virtual MINT32          CmdQMgr_suspend(void) = 0;  //for twin's ctrl
    virtual MINT32          CmdQMgr_resume(void) = 0;   //for twin's ctrl
    virtual MBOOL           CmdQMgr_updateCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain);     //for twin's ctrl.switch dummy page cq opeartion under twin
    virtual MBOOL           CmdQMgr_clearCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain);

    virtual MINT32          CmdQMgr_update(void) = 0;
    virtual MINT32          CmdQMgr_update_IMME(MUINT32 idx);  //for twin's ctrl, specail case for twinmgr

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx) = 0;

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void)  = 0;
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void) = 0;

    virtual MUINT32         CmdQMgr_GetBurstQ(void) = 0;
    virtual MUINT32         CmdQMgr_GetDuqQ(void) = 0;

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void) = 0;


    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE) = 0;


protected:
    enum{
        DEFAULT_DUP_NUM = 3,        //duplicated q nember : 3
    };

            ISP_HW_MODULE   mModule;

private:
     static DupCmdQMgr*     CmdQMgr_attach_ext(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
     static DupCmdQMgr*     CmdQMgr_GetCurMgrObj_ext(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
};
#endif