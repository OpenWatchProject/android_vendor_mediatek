#ifndef _CAM_DUPCMDQMGR_TYPE3_IMME_H_
#define _CAM_DUPCMDQMGR_TYPE3_IMME_H_

#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.type3.h"



class CmdQMgrImp3_imme : public DupCmdQMgr
{
public:
    CmdQMgrImp3_imme();
    virtual ~CmdQMgrImp3_imme(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2);
    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);


    virtual MINT32          CmdQMgr_suspend(void){return 1;}
    virtual MINT32          CmdQMgr_resume(void){return 1;}

    virtual MINT32          CmdQMgr_update(void){CmdQMgr_ERR("\n");return 1;}

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx){DupIdx;CmdQMgr_ERR("\n");return NULL;}

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 0;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 0;}

    virtual MUINT32         CmdQMgr_GetBurstQ(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQ(void){return 1;}

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);
    virtual MBOOL           CmdQMgr_updateCQMode(DupCmdQMgr*  pMaster, MUINT32 targetIdxMain);     //for twin's ctrl.switch dummy page cq opeartion under twin
    virtual MBOOL           CmdQMgr_clearCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain);
private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);
public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_suspend,
    };

    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMAI_B*         m_pCQ;

    char*           m_pUserName;

    ISP_DRV_CAM*    pCamDrv;

    CmdQMgrImp3*    m_pMaster;
};


#endif
