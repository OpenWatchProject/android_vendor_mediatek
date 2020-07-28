#ifndef _CAM_DUPCMDQMGR_TYPE2_H_
#define _CAM_DUPCMDQMGR_TYPE2_H_

#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.log.h"



//class for cq4/5/7/8 : cq running with lagecy cq0c mode
class CmdQMgrImp2 : public DupCmdQMgr
{
public:
    CmdQMgrImp2();
    virtual ~CmdQMgrImp2(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MINT32          CmdQMgr_suspend(void);
    virtual MINT32          CmdQMgr_resume(void);

    virtual MINT32          CmdQMgr_update(void){CmdQMgr_ERR("\n");return 1;}


    virtual MUINT32         CmdQMgr_GetBurstQ(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQ(void){return 1;}

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 0;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 0;}


    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}
    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2){arg1;arg2;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);
private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);

public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_suspend,
        op_stop,
    };

    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMAI_B*         m_pCQ;

    ISP_DRV_CAM*    pCamDrv;

    char*           m_pUserName;
};


#endif
