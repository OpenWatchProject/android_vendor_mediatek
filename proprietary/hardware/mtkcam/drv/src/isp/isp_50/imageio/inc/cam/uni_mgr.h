#ifndef _UNI_MGR_H_
#define _UNI_MGR_H_

#include "uni_drv.h"
#include "isp_datatypes.h"
#include "cam_dupCmdQMgr.h"

typedef enum{
    eCmd_Uni_stop,
    eCmd_Uni_restart
}E_UNI_STEP;

//support streaming only. rawi is via cam_path_cam
class UniMgr
{
public:
    UniMgr(){m_module = UNI_A;}
    virtual ~UniMgr(){}

    static UniMgr*          UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn = NULL);
    virtual MINT32          UniMgr_detach(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_start(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_stop(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_suspend(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_resume(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_recover(E_UNI_STEP op,ISP_HW_MODULE module);
    static UniDrvImp*       UniMgr_GetCurObj(void);

    virtual MBOOL           UniMgr_SetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB,MVOID* pNotify);
    virtual MVOID*          UniMgr_GetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB);

private:
    static UNI_HW_MODULE           m_module;

};
#endif

