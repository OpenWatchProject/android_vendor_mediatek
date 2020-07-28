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
    TYPE3_CQ11  = 1,
    TYPE3_MAX   = 2,
};
#define TYPE3_CQ_MAP(cq) ({\
    MUINT32 index = 0;\
    if(cq == ISP_DRV_CQ_THRE1){\
        index = TYPE3_CQ1;\
    }\
    else if(cq == ISP_DRV_CQ_THRE11){\
        index = TYPE3_CQ11;\
    }\
    else{\
        BASE_LOG_ERR("support only cq1&cq11\n");\
    }\
    index;})


/////////////////////////////////////////////////////////////////////////////////////////////////
/**
    this is for case of dynamic twin OFF.
*/
static CmdQMgrImp  CQ_TYPE1_EXT;        //support only cq0 currentlly
static CmdQMgrImp2 CQ_TYPE2_AF;         //special case for CAM_C's af control when dynamic twin is OFF
static CmdQMgrImp3 CQ_TYPE3_EXT[TYPE3_MAX];     //cq1,cq11

DupCmdQMgr* DupCmdQMgr::CmdQMgr_attach_ext(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    BASE_LOG_INF("CQMgr:case of D-Twin OFF:\n");
    switch(module){
        case CAM_C:
            break;
        default:
            BASE_LOG_ERR("support only CAM_C\n");
            return NULL;
            break;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:

            return CQ_TYPE1_EXT.CmdQMgr_attach_imp(fps,subsample,module,cq);

            break;
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE11:

            return CQ_TYPE3_EXT[TYPE3_CQ_MAP(cq)].CmdQMgr_attach_imp(fps,subsample,module,cq);

            break;
        case ISP_DRV_CQ_THRE5:

            return CQ_TYPE2_AF.CmdQMgr_attach_imp(fps,subsample,module,cq);

            break;
        default:
            BASE_LOG_ERR("unsupported CQ:%d\n",cq);
            return NULL;
            break;
    }


    return NULL;
}

DupCmdQMgr* DupCmdQMgr::CmdQMgr_GetCurMgrObj_ext(ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    switch(module){
        case CAM_C:
            break;
        default:
            BASE_LOG_ERR("unsupported module:0x%x\n",module);
            return NULL;
            break;

    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:
            return CQ_TYPE1_EXT.CmdQMgr_GetCurMgrObj(cq);
            break;
        default:
            BASE_LOG_ERR("unsupported cq:0x%x\n",cq);
            return NULL;
            break;
    }


    return NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/**
    this is for case of d-twin ON.
    if d-twin OFF,  flow will run to DupCmdQMgr::CmdQMgr_attach_ext()
*/
#define Type2_min   (ISP_DRV_CQ_THRE4)
#define Type2_max   (ISP_DRV_CQ_THRE12)

#define Type4_min   (CAM_A_TWIN_B)
#define Type4_max   (CAM_A_TRIPLE_C)
    typedef struct{
        CmdQMgrImp          CQ_TYPE1;                                           //support only cq0 currentlly
        CmdQMgrImp1_imme    CQ_TYPE1_IMME[Type4_max-Type4_min + 1];             //support only cq0 at cam_a_twin only, +1 is for index start from 0
        CmdQMgrImp2         CQ_TYPE2[Type2_max - Type2_min + 1];                //support cq4/5/6/7/8/9/10/12, +1 is for index start from 0
        CmdQMgrImp3         CQ_TYPE3[TYPE3_MAX];                                //cq1,cq11
        CmdQMgrImp3_imme    CQ_TYPE3_IMME[TYPE3_MAX][Type4_max-Type4_min + 1];  //cq1,cq11
    }ST_CmdQ;

#define SupportedCam   (CAM_B + 1)      //support only CAM_A && CAM_B

static ST_CmdQ gCmdQMgr[SupportedCam];  //support CAM_A & CAM_B only

DupCmdQMgr* DupCmdQMgr::CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    ISP_HW_MODULE master_cam = CAM_MAX;
    ISP_HW_MODULE slave_cam = CAM_MAX;
    capibility CamInfo;

    switch(module){
        case CAM_A:
        case CAM_B:
            master_cam = module;
            break;
        case CAM_C:
            if(CamInfo.m_DTwin.GetDTwin() == MTRUE){
                BASE_LOG_ERR("unsupported module:CAM_C when dynamic twin ON\n");
                return NULL;
            }
            else{
                return DupCmdQMgr::CmdQMgr_attach_ext(fps,subsample,module,cq);
            }
            break;
        case CAM_A_TWIN_B:
        case CAM_A_TWIN_C:
        case CAM_A_TRIPLE_B:
        case CAM_A_TRIPLE_C:
            master_cam = CAM_A;
            slave_cam = module;
            break;
        case CAM_B_TWIN_C:
            master_cam = CAM_B;
            slave_cam = module;
            break;
        default:
            BASE_LOG_ERR("unsupported module:0x%x\n",module);
            return NULL;
            break;
    }

    if(master_cam >= SupportedCam){
        BASE_LOG_ERR("unsupported module:0x%x\n",master_cam);
        return NULL;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:
            switch(slave_cam){
                case CAM_MAX:   //master cam

                    return gCmdQMgr[master_cam].CQ_TYPE1.CmdQMgr_attach_imp(fps,subsample,module,cq);

                    break;
                default:        //slave cam
                    if((slave_cam < Type4_min) || (slave_cam > Type4_max)){
                        BASE_LOG_ERR("unsupported module:0x%x\n",slave_cam);
                        return NULL;
                    }

                    if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                        BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF\n");
                        return NULL;
                    }

                    return gCmdQMgr[master_cam].CQ_TYPE1_IMME[slave_cam - Type4_min].CmdQMgr_attach_imp(fps,subsample,module,cq);

                    break;
            }
            break;
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE11:

            switch(slave_cam){
                case CAM_MAX:   //master cam

                    return gCmdQMgr[master_cam].CQ_TYPE3[TYPE3_CQ_MAP(cq)].CmdQMgr_attach_imp(fps,subsample,module,cq);

                    break;
                default:        //slave cam
                    if((slave_cam < Type4_min) || (slave_cam > Type4_max)){
                        BASE_LOG_ERR("unsupported module:0x%x\n",slave_cam);
                        return NULL;
                    }

                    if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                        BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF\n");
                        return NULL;
                    }

                    return gCmdQMgr[master_cam].CQ_TYPE3_IMME[TYPE3_CQ_MAP(cq)][slave_cam - Type4_min].CmdQMgr_attach_imp(\
                        fps,subsample,module,cq);

                    break;
            }



            break;
        default:
            switch(slave_cam){
                case CAM_MAX:   //master cam

                    return gCmdQMgr[master_cam].CQ_TYPE2[cq - Type2_min].CmdQMgr_attach_imp(fps,subsample,module,cq);

                    break;
                default:        //slave cam
                    BASE_LOG_ERR("support no slave cam with immediate mode when D-twin OFF at current CQ:%d\n",cq);
                    return NULL;
                    break;
            }

            break;
    }


    return NULL;
}



DupCmdQMgr* DupCmdQMgr::CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    capibility CamInfo;
    switch(module){
        case CAM_A:
        case CAM_B:
            break;
        case CAM_C:
            if(CamInfo.m_DTwin.GetDTwin() == MTRUE){
                //this is for STT, when D-Twin ON, af should drive via CCU.
                BASE_LOG_ERR("unsupported module:CAM_C when dynamic twin ON\n");
                return NULL;
            }
            else
                return DupCmdQMgr::CmdQMgr_GetCurMgrObj_ext(module,cq);
            break;
        default:
            BASE_LOG_ERR("unsupported module:0x%x\n",module);
            return NULL;
            break;

    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:
            return gCmdQMgr[module].CQ_TYPE1.CmdQMgr_GetCurMgrObj(cq);
            break;
        default:
            BASE_LOG_ERR("unsupported cq:0x%x\n",cq);
            return NULL;
            break;
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


