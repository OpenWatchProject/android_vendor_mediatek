#include "cam_capibility.h"     //for query dynamic twin is ON/OFF for AF


#include <stdlib.h>
#include <cutils/properties.h>  // For property_get().
#include "cam_dupCmdQMgr.log.h"

using namespace std;


DECLARE_DBG_LOG_VARIABLE(CmdQMgr);

#include "cam_dupCmdQMgr.type1.h"
#include "cam_dupCmdQMgr.type1.imme.h"
#include "cam_dupCmdQMgr.type2.h"
#include "cam_dupCmdQMgr.type3.h"
#include "cam_dupCmdQMgr.type3.imme.h"


/**
    Class of DupCmdQMgr
*/
DupCmdQMgr::DupCmdQMgr()
{
    DBG_LOG_CONFIG(imageio, CmdQMgr);
}

enum{
    TYPE3_CQ1   = 0,
    TYPE3_CQ28  = 1,
    TYPE3_MAX   = 2,
};
#define TYPE3_CQ_MAP(cq) ({\
    MUINT32 index = 0;\
    if(cq == ISP_DRV_CQ_THRE1){\
        index = TYPE3_CQ1;\
    }\
    else if(cq == ISP_DRV_CQ_THRE28){\
        index = TYPE3_CQ28;\
    }\
    else{\
        BASE_LOG_ERR("support only cq1&cq11\n");\
    }\
    index;})


DupCmdQMgr* DupCmdQMgr::CmdQMgr_attach_ext(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    BASE_LOG_ERR("unsupported CQ:%d\n",cq);
    return NULL;
}

DupCmdQMgr* DupCmdQMgr::CmdQMgr_GetCurMgrObj_ext(ISP_HW_MODULE module, E_ISP_CAM_CQ cq)
{
    BASE_LOG_ERR("unsupported module:0x%x\n",module);
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/**
    this is for case of d-twin ON.
    if d-twin OFF,  flow will run to DupCmdQMgr::CmdQMgr_attach_ext()
*/
#define TYPE2_STT_MIN   (ISP_DRV_CQ_THRE4)
#define TYPE2_STT_MAX   (ISP_DRV_CQ_THRE12)

//d-twin
#define VIRT_CAM_MIN    (CAM_A_TWIN_B)
#define VIRT_CAM_MAX    (CAM_A_TWIN_B)

typedef struct {
    CmdQMgrImp          CQ_TYPE1;                                           //support only cq0 currentlly
    CmdQMgrImp1_imme    CQ_TYPE1_IMME[VIRT_CAM_MAX - VIRT_CAM_MIN + 1];     //support only cq0 at cam_a_twin only, +1 is for index start from 0
    CmdQMgrImp2         CQ_TYPE2[TYPE2_STT_MAX - TYPE2_STT_MIN + 1];        //support cq4/5/6/7/8/9/10/12, +1 is for index start from 0
    CmdQMgrImp3         CQ_TYPE3[TYPE3_MAX];                                //cq1,cq11
    CmdQMgrImp3_imme    CQ_TYPE3_IMME[TYPE3_MAX][VIRT_CAM_MAX-VIRT_CAM_MIN + 1];  //cq1,cq11
} ST_CmdQ;

#define CQMGR_SUPPORTED_CAM     (CAM_C + 1)      //support only CAM_A && CAM_B

static ST_CmdQ gCmdQMgr[CQMGR_SUPPORTED_CAM];  //support CAM_A & CAM_B only

DupCmdQMgr* DupCmdQMgr::CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    ISP_HW_MODULE master_cam = CAM_MAX;
    ISP_HW_MODULE slave_cam = CAM_MAX;
    capibility CamInfo;

    switch(module){
    case CAM_A:
    case CAM_B:
    case CAM_C:
        master_cam = module;
        break;
    case CAM_A_TWIN_B:
        master_cam = CAM_A;
        slave_cam = module;
        break;
    default:
        BASE_LOG_ERR("unsupported module:0x%x\n",module);
        return NULL;
    }

    if (master_cam >= CQMGR_SUPPORTED_CAM) {
        BASE_LOG_ERR("unsupported module:0x%x\n",master_cam);
        return NULL;
    }

    switch (cq) {
    case ISP_DRV_CQ_THRE0:
        switch (slave_cam) {
        case CAM_MAX:
            return gCmdQMgr[master_cam].CQ_TYPE1.CmdQMgr_attach_imp(fps,subsample,module,cq);
        default:
            if((slave_cam < VIRT_CAM_MIN) || (slave_cam > VIRT_CAM_MAX)){
                BASE_LOG_ERR("unsupported module:0x%x\n",slave_cam);
                return NULL;
            }

            if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF\n");
                return NULL;
            }
            return gCmdQMgr[master_cam].CQ_TYPE1_IMME[slave_cam - VIRT_CAM_MIN].CmdQMgr_attach_imp(fps,subsample,module,cq);
        }
        break;
    case ISP_DRV_CQ_THRE1:
    case ISP_DRV_CQ_THRE28:
        switch(slave_cam){
        case CAM_MAX:
            return gCmdQMgr[master_cam].CQ_TYPE3[TYPE3_CQ_MAP(cq)].CmdQMgr_attach_imp(fps,subsample,module,cq);
            break;
        default:
            if((slave_cam < VIRT_CAM_MIN) || (slave_cam > VIRT_CAM_MAX)){
                BASE_LOG_ERR("unsupported module:0x%x\n",slave_cam);
                return NULL;
            }

            if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF\n");
                return NULL;
            }
            return gCmdQMgr[master_cam].CQ_TYPE3_IMME[TYPE3_CQ_MAP(cq)][slave_cam - VIRT_CAM_MIN].CmdQMgr_attach_imp(\
                fps,subsample,module,cq);

            break;
        }
        break;
    default:
        switch (slave_cam) {
        case CAM_MAX:
            return gCmdQMgr[master_cam].CQ_TYPE2[cq - TYPE2_STT_MIN].CmdQMgr_attach_imp(fps,subsample,module,cq);
        default:
            BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF at current CQ:%d\n",cq);
            return NULL;
        }

        break;
    }

    return NULL;
}



DupCmdQMgr* DupCmdQMgr::CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    capibility CamInfo;

    switch (module) {
    case CAM_A:
    case CAM_B:
    case CAM_C:
        break;
    default:
        BASE_LOG_ERR("unsupported module:0x%x\n",module);
        return NULL;

    }

    switch (cq) {
    case ISP_DRV_CQ_THRE0:
        return gCmdQMgr[module].CQ_TYPE1.CmdQMgr_GetCurMgrObj(cq);
    default:
        BASE_LOG_ERR("unsupported cq:0x%x\n",cq);
        return NULL;
    }

    return NULL;
}

MBOOL DupCmdQMgr::CmdQMgr_GetCurStatus(void)
{
    CmdQMgr_ERR("unsupported cmd\n");
    return MFALSE;
}


MBOOL DupCmdQMgr::CmdQMgr_updateCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain)
{
    capibility CamInfo;
    pMaster;targetIdxMain;
    if(CamInfo.m_DTwin.GetDTwin() == MTRUE){
        CmdQMgr_ERR("unsupported cmd\n");
        return MFALSE;
    }
    else
        return MTRUE;
}

MBOOL DupCmdQMgr::CmdQMgr_clearCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain)
{
    capibility CamInfo;
    pMaster;targetIdxMain;
    if(CamInfo.m_DTwin.GetDTwin() == MTRUE){
        CmdQMgr_ERR("unsupported cmd\n");
        return MFALSE;
    }
    else
        return MTRUE;
}


MINT32 DupCmdQMgr::CmdQMgr_update_IMME(MUINT32 idx)
{
    CmdQMgr_ERR("unsupported cmd\n");
    return 0;
}


