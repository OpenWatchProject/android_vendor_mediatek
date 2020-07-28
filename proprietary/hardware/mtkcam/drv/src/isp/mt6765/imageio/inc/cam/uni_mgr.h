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
#if 0 //marked, so far, mux of flk2 & hds2 must from the same source
    //
    typedef enum{
        _RAWI_  = 0,
        _FLK_   = 1,
        _HDS_   = 2,
    }E_UNI_PATH;
#endif
    typedef enum{
        _EIS_CB_    = 0,
        _SGG2_CB_   = 1,
        _RSS_CB_    = 2,
        _Max_CB_    = 3,
    }E_UNI_CB;

    typedef enum{
        _UNI_UNKNOW_    = 0,
        _UNI_EIS_       = 1,
        _UNI_FLK_       = 2,
        _UNI_RSS_       = 3,
    }E_UNI_WDMA;

            UNI_HW_MODULE   UniMgr_getUniModule(ISP_HW_MODULE module);
    virtual MINT32          UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn = NULL);
    virtual MINT32          UniMgr_detach(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_start(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_stop(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_suspend(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_resume(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_recover(E_UNI_STEP op,ISP_HW_MODULE module);

    virtual UniDrvImp*      UniMgr_GetCurObj(void);

    virtual ISP_HW_MODULE   UniMgr_GetCurLink(void);

    virtual MBOOL           UniMgr_SetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB,MVOID* pNotify);
    virtual MVOID*          UniMgr_GetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB);

    virtual MBOOL           UniMgr_GetModuleObj(E_UNI_WDMA module, MUINTPTR* pWdma,MUINTPTR* pFbc);
public:
    UNI_HW_MODULE           m_module;

};
#endif

