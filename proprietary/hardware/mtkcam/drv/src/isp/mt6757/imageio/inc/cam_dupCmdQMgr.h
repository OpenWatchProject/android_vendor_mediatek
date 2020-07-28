#ifndef _CAM_DUPCMDQMGR_H_
#define _CAM_DUPCMDQMGR_H_

#include "isp_drv_cam.h"


class DupCmdQMgr
{
public:
    DupCmdQMgr(){m_module = MAX_ISP_HW_MODULE;}
    virtual ~DupCmdQMgr(){}

     static DupCmdQMgr*     CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2);

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);
    virtual MINT32          CmdQMgr_update(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void);
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);
    static DupCmdQMgr*      CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE);
public:
    enum{
        REV_CQ = 15,
    };
private:
    ISP_HW_MODULE           m_module;
};
#endif