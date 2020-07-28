#ifndef _CAM_DUPCMDQMGR_TYPE3_H_
#define _CAM_DUPCMDQMGR_TYPE3_H_


#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.log.h"



class CmdQMgrImp3 : public DupCmdQMgr
{
public:
    CmdQMgrImp3();
    virtual ~CmdQMgrImp3(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2);
    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);


    virtual MINT32          CmdQMgr_suspend(void){CmdQMgr_ERR("\n");return 1;}
    virtual MINT32          CmdQMgr_resume(void){CmdQMgr_ERR("\n");return 1;}

    virtual MINT32          CmdQMgr_update(void){CmdQMgr_ERR("\n");return 1;}

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx){DupIdx;CmdQMgr_ERR("\n");return NULL;}

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 0;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 0;}

    virtual MUINT32         CmdQMgr_GetBurstQ(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQ(void){return 1;}

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);
private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);
     inline MBOOL           Load_CQ11_Addr(void);

 public:
    //public for slave cam
    ISP_DRV_CAM*    pCamDrv;
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
    };

    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;


    DMA_CQ10        m_CQ10;//use CQ10 to cfg CQ11 inner BA

    ISP_DRV_CAM*    pCQ10Drv;    //for CQ10

    DMAI_B*         m_pCQ;

    char*           m_pUserName;
    char*           m_pCQ10Name;//for CQ10
};


#endif
