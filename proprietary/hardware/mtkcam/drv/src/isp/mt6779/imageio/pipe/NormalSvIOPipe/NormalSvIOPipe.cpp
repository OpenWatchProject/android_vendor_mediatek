/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "normalsviopipe"

//
#include <mtkcam/utils/std/Log.h>
//
#include "NormalSvIOPipe.h"
//
#include <cutils/properties.h>  // For property_get().
//
#include "cam_capibility.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#define NORMALSV_SUBSAMPLE
#define TGTOP1_MAXBANDWIDTH 4*1024*1024*30 //4M@30fps per group

EXTERN_DBG_LOG_VARIABLE(pipe);

#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_ERR

#if 1
#define PIPE_DBG(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_INF(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_ERR(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)
#else
#define PIPE_DBG(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#define PIPE_INF(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#define PIPE_ERR(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#endif

/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/

//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
NormalSvIOPipe::NormalSvIOPipe()
{
    //
    DBG_LOG_CONFIG(imageio, pipe);


    m_szUsrName[0] = '\0';


    m_FSM = op_unknown;
    m_pipe_opt = ICamIOPipe::NORMALSVIO;
    m_pResMgr = m_pSlaveResMgr = NULL;
    m_hwMasterModule = CAM_A;
    m_hwModule = CAMSV_0;
    m_pIspDrvCamsv = NULL;
    m_pIspDrvCamsvSlave = NULL;
    m_pCmdQMgr = NULL;
    m_TwinMode_En = m_Dcif_En = MFALSE;
    m_cam_dfs = NULL;
    m_hwModuleCamsvSlave = CAMSV_1;
    m_Subsample = 0;
    m_Data_Pattern = 0;
    m_Dcif_MasterRaw = 0;
    m_Dcif_Offset = 0;
    m_UFEO_En = 0;
    PIPE_INF(":X");
}

NormalSvIOPipe::~NormalSvIOPipe()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::FSM_CHECK(MUINT32 op)
{
    MBOOL ret = MTRUE;
    this->m_FSMLock.lock();
    switch(op){
        case op_unknown:
            if(this->m_FSM != op_uninit)
                ret = MFALSE;
            break;
        case op_init:
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
            break;
        case op_cfg:
            if(this->m_FSM != op_init)
                ret = MFALSE;
            break;
        case op_start:
            if(this->m_FSM != op_cfg)
                ret = MFALSE;
            break;
        case op_stop:
            if(this->m_FSM != op_start)
                ret = MFALSE;
            break;
        case op_uninit:
            switch(this->m_FSM){
                case op_init:
                case op_cfg:
                case op_stop:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_cmd:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                case op_stop:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        case op_endeq:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE)
        PIPE_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);
    this->m_FSMLock.unlock();
    return ret;
}

MBOOL NormalSvIOPipe::FSM_UPDATE(MUINT32 op)
{
    PIPE_DBG("NormalSvIOPipe::FSM_UPDATE [%d,%d]", op, this->m_FSM);
    if(op == op_cmd) {
        return MTRUE;
    }
    this->m_FSMLock.lock();
    this->m_FSM = (E_FSM)op;
    this->m_FSMLock.unlock();
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
NormalSvIOPipe* NormalSvIOPipe::Create(MINT8 const szUsrName[32], vector<E_INPUT>& InOutPut, PortInfo const *portInfo)
{
    int i = 0;
    ISP_HW_MODULE master;
    NormalSvIOPipe* pNormalSvIOPipe = NULL;
    //CamSVMgr need MIPI_pixRate
    //DCIF mode should save TG info for sendCommand, EPIPECmd_GET_TG_INDEX, EPIPECmd_GET_TG_OUT_SIZE
    MUINT32 mMipiPixrate = portInfo->MIPI_PixRate;

    BASE_LOG_INF(":E:user:%s \n", szUsrName);

    switch(InOutPut[0]){
        case TG_CAMSV_0:
        case TG_CAMSV_1:
        case TG_CAMSV_2:
        case TG_CAMSV_3:
        case TG_CAMSV_4:
        case TG_CAMSV_5:
        #if 0
        case TG_CAMSV_6:
        case TG_CAMSV_7:
        #endif
            break;
        default:
            BASE_LOG_ERR("unsupported input source(%d)\n", InOutPut[0]);
            return NULL;
    }

    //check TG vector size
    if(InOutPut.size() == 0)
    {
        InOutPut.push_back(TG_CAMSV_0);
    } else if (InOutPut.size() >= 2) {
        BASE_LOG_ERR("not support vector size before camsv_resMgr\n");
        return NULL;
    }

    //new NormalSvIOPipe object
    pNormalSvIOPipe = new(std::nothrow) NormalSvIOPipe();
    if(pNormalSvIOPipe == NULL){
        BASE_LOG_ERR("new NormalSvIOPipe fail\n");
        return NULL;
    }

    //get clock to decide TG numbers, crate temp isp_drv_camsv instance
    pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_5);
    pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_5");
    pNormalSvIOPipe->clk_gear.clear();
    pNormalSvIOPipe->getClockSet();
    pNormalSvIOPipe->m_pIspDrvCamsv->uninit("NormalSvIOPipe CamSV_5");
    pNormalSvIOPipe->m_pIspDrvCamsv->destroyInstance();
    pNormalSvIOPipe->m_pIspDrvCamsv = NULL;
    //end temporary

    BASE_LOG_INF("CurrClk %d, max_bandwidth %d, mMipiPixrate %d\n",
        pNormalSvIOPipe->clk_gear[0], TGTOP1_MAXBANDWIDTH, mMipiPixrate);

    //twin case check, mipi lixel rate > 4Mx30fps and > CurrClk*4
    if( (mMipiPixrate>TGTOP1_MAXBANDWIDTH) && (mMipiPixrate>(4*pNormalSvIOPipe->clk_gear[0]))) {
        pNormalSvIOPipe->m_TwinMode_En = MTRUE;
        InOutPut.push_back(TG_CAMSV_1);//add one TG to enlarge vector size
    }
    BASE_LOG_INF("InOutPut.size() %d \n", InOutPut.size());

    //DCIF/twin/UFEO, mipi_pix > 4m*30fps case use TOP_0_0 and TOP_0_1 only, temporary mipi_pixelrate = 0
    if( (mMipiPixrate>TGTOP1_MAXBANDWIDTH) || (pNormalSvIOPipe->m_TwinMode_En == MTRUE))
    {
        ISP_HW_MODULE mCamSVModule = CAMSV_0;
        for(i=0; i<InOutPut.size(); i++) {
            //ResMgr attach
            if(pNormalSvIOPipe->m_pResMgr == NULL)
                pNormalSvIOPipe->m_pResMgr = ResMgr::Res_Attach(user_normalsvio, 0, (char *)szUsrName, InOutPut);
            else if(pNormalSvIOPipe->m_pSlaveResMgr == NULL)
                pNormalSvIOPipe->m_pSlaveResMgr = ResMgr::Res_Attach(user_normalsvio, 0, (char *)szUsrName, InOutPut);
        }
        if(pNormalSvIOPipe->m_pResMgr == NULL ||
            (InOutPut.size() == 2 && pNormalSvIOPipe->m_pSlaveResMgr == NULL)) {
            BASE_LOG_ERR("Res_attach fail. \n");
            goto FAIL_EXIT;
        }
        BASE_LOG_INF("m_pResMgr Module %d, m_pSlaveResMgr Module %d \n", pNormalSvIOPipe->m_pResMgr->Res_GetModule(),
            (InOutPut.size() == 2)?pNormalSvIOPipe->m_pSlaveResMgr->Res_GetModule():(-1));
        mCamSVModule = pNormalSvIOPipe->m_pResMgr->Res_GetModule();
        switch(mCamSVModule){
            case CAMSV_0:
                pNormalSvIOPipe->m_hwModule = CAMSV_0;
                std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
                if(pNormalSvIOPipe->m_pIspDrvCamsv == NULL)
                {
                    pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
                    pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_0");
                }
                InOutPut[0] = TG_CAMSV_0;
                break;
            case CAMSV_1:
                pNormalSvIOPipe->m_hwModule = CAMSV_1;
                std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
                if(pNormalSvIOPipe->m_pIspDrvCamsv == NULL)
                {
                    pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
                    pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_1");
                }
                InOutPut[0] = TG_CAMSV_1;
                break;
            default:
                BASE_LOG_ERR("unsupported camsv : (%d)\n", mCamSVModule);
                goto FAIL_EXIT;
                break;
        }
        BASE_LOG_INF("InOutPut[0] %d \n", InOutPut[0]);
        if(InOutPut.size() == 2 && pNormalSvIOPipe->m_pSlaveResMgr != NULL) {
            mCamSVModule = pNormalSvIOPipe->m_pSlaveResMgr->Res_GetModule();
            switch(mCamSVModule){
                case CAMSV_0:
                    pNormalSvIOPipe->m_hwModuleCamsvSlave = CAMSV_0;
                    std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
                    if(pNormalSvIOPipe->m_pIspDrvCamsvSlave == NULL)
                    {
                        pNormalSvIOPipe->m_pIspDrvCamsvSlave = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
                        pNormalSvIOPipe->m_pIspDrvCamsvSlave->init("NormalSvIOPipe CamSV_0");
                    }
                    InOutPut[1] = TG_CAMSV_0;
                    break;
                case CAMSV_1:
                    pNormalSvIOPipe->m_hwModuleCamsvSlave = CAMSV_1;
                    std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
                    if(pNormalSvIOPipe->m_pIspDrvCamsvSlave == NULL)
                    {
                        pNormalSvIOPipe->m_pIspDrvCamsvSlave = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
                        pNormalSvIOPipe->m_pIspDrvCamsvSlave->init("NormalSvIOPipe CamSV_1");
                    }
                    InOutPut[1] = TG_CAMSV_1;
                    break;
                default:
                    BASE_LOG_ERR("unsupported camsv : (%d)\n", mCamSVModule);
                    goto FAIL_EXIT;
                    break;
            }
            BASE_LOG_INF("InOutPut[1] %d \n", InOutPut[1]);
        }
        BASE_LOG_INF("get crop1 (w %d, h %d)\n", portInfo->crop1.w,
            portInfo->crop1.h);

        pNormalSvIOPipe->crop.w = (MUINT32)portInfo->crop1.w;
        pNormalSvIOPipe->crop.h = (MUINT32)portInfo->crop1.h;
        return pNormalSvIOPipe;
    }

    //o.w., non-DCIF/normal case prefer attaching from TOP_1_0/TOP_1_1/TOP_2_0/TOP_2_1
    for(i=0; i<InOutPut.size(); i++){
        ISP_HW_MODULE mCamSVModule = CAMSV_2;
        //cause camsv_ResMgr select TG from TG_CAMSV_2~TG_CAMSV_5
        InOutPut[0] = TG_CAMSV_2;
        //ResMgr attach
        //support without TG info and output SV TG
        if(pNormalSvIOPipe->m_pResMgr == NULL) {
            pNormalSvIOPipe->m_pResMgr = ResMgr::Res_Attach(user_normalsvio, mMipiPixrate, (char *)szUsrName, InOutPut);

            if(pNormalSvIOPipe->m_pResMgr == NULL){
                BASE_LOG_ERR("Res_attach fail.n");
                goto FAIL_EXIT;
            }
            mCamSVModule = pNormalSvIOPipe->m_pResMgr->Res_GetModule();
            //filter avaliable camSV modules
            switch(mCamSVModule){
                case CAMSV_2:
                case CAMSV_3:
                case CAMSV_4:
                case CAMSV_5:
                    break;
                default:
                    BASE_LOG_ERR("unsupported camsv source: (%d)\n", mCamSVModule);
                    goto FAIL_EXIT;
                    break;
            }
            pNormalSvIOPipe->m_hwModule = mCamSVModule;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            if(pNormalSvIOPipe->m_pIspDrvCamsv == NULL)
            {
                pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(mCamSVModule);
                pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe");
            }
            InOutPut[0] = (E_INPUT)(TG_CAMSV_0 + (mCamSVModule-CAMSV_0));
            BASE_LOG_INF("InOutPut[0] %d \n", InOutPut[0]);
        }
        else if(pNormalSvIOPipe->m_pSlaveResMgr == NULL) {
            BASE_LOG_INF("Shoud not happen! \n");
            #if 0
            pNormalSvIOPipe->m_pSlaveResMgr = ResMgr::Res_Attach(user_normalsvio, mMipiPixrate, (char *)szUsrName, InOutPut);

            if(pNormalSvIOPipe->m_pSlaveResMgr == NULL){
                BASE_LOG_ERR("Res_attach fail.n");
                goto FAIL_EXIT;
            }
            mCamSVModule = pNormalSvIOPipe->m_pSlaveResMgr->Res_GetModule();
            //filter avaliable camSV modules
            switch(mCamSVModule){
                case CAMSV_2:
                case CAMSV_3:
                case CAMSV_4:
                case CAMSV_5:
                    break;
                default:
                    BASE_LOG_ERR("unsupported camsv source: (%d)\n", mCamSVModule);
                    goto FAIL_EXIT;
                    break;
            }
            if(pNormalSvIOPipe->m_pIspDrvCamsvSlave == NULL)
            {
                pNormalSvIOPipe->m_pIspDrvCamsvSlave = (IspDrvCamsv*)IspDrvCamsv::createInstance(mCamSVModule);
                pNormalSvIOPipe->m_pIspDrvCamsvSlave->init("m_pIspDrvCamsvSlave");
            }
            InOutPut[1] = (E_INPUT)(TG_CAMSV_0 + (mCamSVModule-CAMSV_0));
            BASE_LOG_INF("InOutPut[1] %d \n", InOutPut[1]);
            #endif
        }
    }

    pNormalSvIOPipe->crop.w = (MUINT32)portInfo->crop1.w;
    pNormalSvIOPipe->crop.h = (MUINT32)portInfo->crop1.h;

    return pNormalSvIOPipe;
FAIL_EXIT:
    if(NULL != pNormalSvIOPipe->m_pIspDrvCamsv){
        pNormalSvIOPipe->m_pIspDrvCamsv->uninit("NormalSvIOPipe");
        pNormalSvIOPipe->m_pIspDrvCamsv->destroyInstance();
        pNormalSvIOPipe->m_pIspDrvCamsv = NULL;
    }
    if(NULL != pNormalSvIOPipe->m_pIspDrvCamsvSlave){
        pNormalSvIOPipe->m_pIspDrvCamsvSlave->uninit("NormalSvIOPipe");
        pNormalSvIOPipe->m_pIspDrvCamsvSlave->destroyInstance();
        pNormalSvIOPipe->m_pIspDrvCamsvSlave = NULL;
    }

    if(NULL != pNormalSvIOPipe->m_pResMgr){
        if(MFALSE == pNormalSvIOPipe->m_pResMgr->Res_Detach()){
            BASE_LOG_ERR("Res_Detach fail.\n");
        }
        else{
            pNormalSvIOPipe->m_pResMgr = NULL;
        }
    }
    if(NULL != pNormalSvIOPipe->m_pSlaveResMgr){
        if(MFALSE == pNormalSvIOPipe->m_pSlaveResMgr->Res_Detach()){
            BASE_LOG_ERR("Res_Detach fail.\n");
        }
        else{
            pNormalSvIOPipe->m_pSlaveResMgr = NULL;
        }
    }
    if(NULL != pNormalSvIOPipe){
        delete pNormalSvIOPipe;
        pNormalSvIOPipe = NULL;
    }
    return NULL;

}

NormalSvIOPipe* NormalSvIOPipe::Create(MINT8 const szUsrName[32], E_INPUT& InOutPut)
{
    ISP_HW_MODULE master;
    NormalSvIOPipe* pNormalSvIOPipe = NULL;

    BASE_LOG_INF(":E:user:%s, with input source:0x%x\n", szUsrName, InOutPut);

    switch(InOutPut){
        case TG_CAMSV_0:
            master = CAMSV_0;
            break;
        case TG_CAMSV_1:
            master = CAMSV_1;
            break;
        case TG_CAMSV_2:
            master = CAMSV_2;
            break;
        case TG_CAMSV_3:
            master = CAMSV_3;
            break;
        case TG_CAMSV_4:
            master = CAMSV_4;
            break;
        case TG_CAMSV_5:
            master = CAMSV_5;
            break;
        default:
            BASE_LOG_ERR("unsupported input source(%d)\n", InOutPut);
            return NULL;
            break;
    }

    //new NormalSvIOPipe object
    pNormalSvIOPipe = new(std::nothrow) NormalSvIOPipe();
    if(pNormalSvIOPipe == NULL){
        BASE_LOG_ERR("new NormalSvIOPipe fail\n");
        return NULL;
    }

    //ResMgr attach
    pNormalSvIOPipe->m_pResMgr = ResMgr::Res_Attach(user_normalsvio, master, (char *)szUsrName);
    if(pNormalSvIOPipe->m_pResMgr == NULL){
        BASE_LOG_ERR("Res_attach fail.n");
        goto FAIL_EXIT;
    }

    switch(pNormalSvIOPipe->m_pResMgr->Res_GetModule()){
        case CAMSV_0:
            pNormalSvIOPipe->m_hwModule = CAMSV_0;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_0");
            InOutPut = TG_CAMSV_0;
            break;
        case CAMSV_1:
            pNormalSvIOPipe->m_hwModule = CAMSV_1;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_1");
            InOutPut = TG_CAMSV_1;
            break;
        case CAMSV_2:
            pNormalSvIOPipe->m_hwModule = CAMSV_2;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_2);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_2");
            InOutPut = TG_CAMSV_2;
            break;
        case CAMSV_3:
            pNormalSvIOPipe->m_hwModule = CAMSV_3;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_3);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_3");
            InOutPut = TG_CAMSV_3;
            break;
        case CAMSV_4:
            pNormalSvIOPipe->m_hwModule = CAMSV_4;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_4);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_4");
            InOutPut = TG_CAMSV_4;
            break;
        case CAMSV_5:
            pNormalSvIOPipe->m_hwModule = CAMSV_5;
            std::strncpy((char*)pNormalSvIOPipe->m_szUsrName, (char const*)szUsrName, sizeof(pNormalSvIOPipe->m_szUsrName) - 1);
            pNormalSvIOPipe->m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_5);
            pNormalSvIOPipe->m_pIspDrvCamsv->init("NormalSvIOPipe CamSV_5");
            InOutPut = TG_CAMSV_5;
            break;
        default:
            BASE_LOG_ERR("unsupported camsv source(%d)\n", InOutPut);
            goto FAIL_EXIT;
            break;
    }

    return pNormalSvIOPipe;
FAIL_EXIT:
    if(NULL != pNormalSvIOPipe->m_pResMgr){
        if(MFALSE == pNormalSvIOPipe->m_pResMgr->Res_Detach()){
            BASE_LOG_ERR("Res_Detach fail.\n");
        }
        else{
            pNormalSvIOPipe->m_pResMgr = NULL;
        }
    }
    if(NULL != pNormalSvIOPipe){
        delete pNormalSvIOPipe;
        pNormalSvIOPipe = NULL;
    }
    return NULL;
}

void NormalSvIOPipe::Destroy(void)
{
    PIPE_INF(":E:user:%s, with input source:0x%x and 0x%x\n",
        this->m_szUsrName,this->m_hwModule,this->m_hwModuleCamsvSlave);

    this->m_cam_dfs->destroyInstance(LOG_TAG);

    this->m_szUsrName[0] = '\0';
    this->m_FSMLock.lock();
    this->m_FSM = op_unknown;
    this->m_FSMLock.unlock();
    if(NULL != this->m_pIspDrvCamsv){
        this->m_pIspDrvCamsv->uninit("NormalSvIOPipe");
        this->m_pIspDrvCamsv->destroyInstance();
        this->m_pIspDrvCamsv = NULL;
    }
    if(NULL != this->m_pIspDrvCamsvSlave){
        this->m_pIspDrvCamsvSlave->uninit("NormalSvIOPipe");
        this->m_pIspDrvCamsvSlave->destroyInstance();
        this->m_pIspDrvCamsvSlave = NULL;
    }

    if(NULL != this->m_pResMgr){
        if(this->m_pResMgr->Res_GetModule() == this->m_hwModule){
            if(MFALSE == this->m_pResMgr->Res_Detach()){
                PIPE_ERR("Res_Detach fail.\n");
            }else{
                this->m_pResMgr = NULL;
            }
        }
        else{
            PIPE_ERR("hwModule not match (%d_%d)", this->m_pResMgr->Res_GetModule(), this->m_hwModule);
        }
    }
    if(NULL != this->m_pSlaveResMgr){
        if(this->m_pSlaveResMgr->Res_GetModule() == this->m_hwModuleCamsvSlave){
            if(MFALSE == this->m_pSlaveResMgr->Res_Detach()){
                PIPE_ERR("Res_Detach fail.\n");
            }else{
                this->m_pSlaveResMgr = NULL;
            }
        }
        else{
            PIPE_ERR("hwModule not match (%d_%d)", this->m_pSlaveResMgr->Res_GetModule(), this->m_hwModuleCamsvSlave);
        }
    }

    //this->m_cam_dfs->destroyInstance(LOG_TAG);
    this->m_pCmdQMgr = NULL;
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::init()
{
    PIPE_INF("(%s):E",this->m_szUsrName);
    if(this->m_szUsrName[0] == '0'){
        PIPE_ERR("can't null username\n");
        return MFALSE;
    }
    if(this->FSM_CHECK(op_init) == MFALSE)
        return MFALSE;
    this->FSM_UPDATE(op_init);

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::uninit()
{
    PIPE_INF("NormalSvIOPipe::uninit");

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;

    this->FSM_UPDATE(op_uninit);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::start()
{
    void* ptr = NULL;
    PIPE_INF("+");


    if(this->FSM_CHECK(op_start) == MFALSE)
        return MFALSE;

    //
    if(this->m_cam_dfs){
        if(this->m_cam_dfs->CAM_DFS_START() == MFALSE){
            return MFALSE;
        }
    }

    // Start top
    this->m_TopCtrl.enable(NULL);

    if(this->m_TwinMode_En)
    {
        this->m_TopCtrlSlave.enable(NULL);
    }

    this->FSM_UPDATE(op_start);
    PIPE_INF("-");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::stop(MBOOL bForce)
{
    PIPE_INF("NormalSvIOPipe::stop+");

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;

    //
    if(this->m_cam_dfs){
        if(this->m_cam_dfs->CAM_DFS_STOP() == MFALSE){
            return MFALSE;
        }
    }

    this->m_TopCtrl.disable(&bForce);

    this->m_FbcImgo.disable();

    this->m_DmaImgo.disable();

    this->m_TGCtrl.disable();


    if(this->m_TwinMode_En)
    {
        this->m_TopCtrlSlave.disable(&bForce);

        this->m_FbcImgoSlave.disable();

        this->m_DmaImgoSlave.disable();

        this->m_TGCtrlSlave.disable();
    }

    this->FSM_UPDATE(op_stop);

    PIPE_INF("NormalSvIOPipe::stop-");

    return  MTRUE;
}

MBOOL NormalSvIOPipe::suspend(E_SUSPEND_MODE suspendMode)
{
    suspendMode;

    //
    if(this->m_cam_dfs){
        if(this->m_cam_dfs->CAM_DFS_STOP() == MFALSE){
        return MFALSE;
    }
    }

    //
    if(this->m_TwinMode_En)
    {
        this->m_FbcImgoSlave.suspend();
    }
    return this->m_FbcImgo.suspend();
}

MBOOL NormalSvIOPipe::resume(E_SUSPEND_MODE suspendMode)
{
    suspendMode;

    //
    if(this->m_cam_dfs){
        if(this->m_cam_dfs->CAM_DFS_START() == MFALSE){
            return MFALSE;
        }
    }

    //
    if(this->m_TwinMode_En)
    {
        this->m_FbcImgoSlave.resume();
    }
    return this->m_FbcImgo.resume();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::abortDma(PortID const port)
{
    MUINT32         dmaChannel = 0;
    ISP_WAIT_IRQ_ST irq;

    //
    if(this->m_FSM != op_stop){
        PIPE_ERR("CamsvIOPipe FSM(%d)", this->m_FSM);
        return MFALSE;
    }

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.Timeout = 1000;

    dmaChannel = (MUINT32)this->PortID_MAP(port.index);
    switch(dmaChannel){
        case _imgo_:
            irq.St_type = SIGNAL_INT;
            irq.Status = SV_SW_PASS1_DON_ST;
            break;
        default:
            PIPE_ERR("Not support dma(0x%x)\n", port.index);
            return MFALSE;

    }

    if( this->m_pIspDrvCamsv->signalIrq(&irq) == MFALSE )
    {
        PIPE_ERR("flush DMA error!");
        return  MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
_isp_dma_enum_ NormalSvIOPipe::PortID_MAP(MUINT32 PortID)
{
    switch(PortID){
        case EPortIndex_IMGO:
        case EPortIndex_CAMSV_IMGO:
            return _imgo_;
            break;
        default:
            PIPE_ERR("un-supported portID:0x%x\n",PortID);
            break;
    }

    return _cam_max_;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MBOOL bImdMode)
{
    MBOOL ret = MTRUE;

    MUINT32         dmaChannel = 0;
    ST_BUF_INFO     ImageAndHeaderInfo;
    MUINT32 targetIdx = 0;
    //warning free
    (void)bImdMode;

    PIPE_DBG("NormalSvIOPipe::enqueOutBuf+");

    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_ERR("op_cmd(%d)", op_cmd);
        return MFALSE;
    }

    #ifdef NORMALSV_SUBSAMPLE
    if(rQBufInfo.vBufInfo.size() != (this->m_TopCtrl.SubSample+1))
    #else
    if(rQBufInfo.vBufInfo.size() != 1)
    #endif
    {
        PIPE_ERR("NormalSvIOPipe support only enque-1-frame-at-1-time %zu\n", rQBufInfo.vBufInfo.size());
        return MFALSE;
    }

    PIPE_INF("m_pIspDrv %p ", this->m_FbcImgo.m_pIspDrv);

    if(this->m_Dcif_En && this->m_pCmdQMgr)
    {
        targetIdx = this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx();
        PIPE_INF("targetIdx %d, GetBurst %d, m_pIspDrv %p \n", targetIdx, this->m_pCmdQMgr->CmdQMgr_GetBurstQ(), this->m_FbcImgo.m_pIspDrv);
    }

    for(MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++){
        stISP_BUF_INFO  bufInfo;
        if(this->m_Dcif_En && this->m_pCmdQMgr)
        {
            IspDrvVir *_pCurDrv = (IspDrvVir*)this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[0];
            //update CQ object target for normalSV FBC to call updateSVModule
            this->m_FbcImgo.m_pIspDrv = _pCurDrv;
            this->m_FbcImgo.m_pMasterCamDrv = this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[0];
            bufInfo.bufidx = i;//use bufidx to identify subsample index
        }

        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_ERR("replace function are not supported in NormalSvIOPipe\n");
            return MFALSE;
        }
        ImageAndHeaderInfo.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.magicIdx  = rQBufInfo.vBufInfo[i].m_num;                 //magic number

        //header
        ImageAndHeaderInfo.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st];
        ImageAndHeaderInfo.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st];
        ImageAndHeaderInfo.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st];
        ImageAndHeaderInfo.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID[ePlane_1st];
        ImageAndHeaderInfo.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu[ePlane_1st];
        ImageAndHeaderInfo.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe[ePlane_1st];
        bufInfo.u_op.enque.push_back(ImageAndHeaderInfo);

        if(!bufInfo.u_op.enque.empty()){
            PIPE_DBG("burstidx:%d: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)",\
                i,\
                portID.index,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.memID,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.va_addr,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr,\
                bufInfo.u_op.enque.at(ePlane_1st).header.va_addr);
        }else{
            PIPE_ERR("CamsvIOPipe bufInfo.u_op.enque empty\n");
        }

        if(this->m_Dcif_En == MFALSE)
        {
        if(MFALSE == this->configDmaPort(rQBufInfo.vBufInfo[i], this->m_DmaImgo.dma_cfg)){
            PIPE_ERR("configDmaPort fail at burst:0x%x\n",i);
            ret = MFALSE;
        }

        if(0 != this->m_DmaImgo.config()){
            PIPE_ERR("dmaImgo config fail at burst:0x%x\n",i);
            ret = MFALSE;
        }
        }

        if(MFALSE == this->m_FbcImgo.enqueueHwBuf(bufInfo)){
            PIPE_ERR("enque fail at burst:0x%x\n",i);
            ret = MFALSE;
        }

    }

    //
    if(this->m_cam_dfs){
        this->m_cam_dfs->CAM_DFS_Vsync();
        this->m_cam_dfs->CAM_DBS_SetBw(&this->m_dfs_info);
    }

    this->FSM_UPDATE(op_cmd);

    PIPE_DBG("NormalSvIOPipe::enqueOutBuf-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
E_BUF_STATUS NormalSvIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQTBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    E_BUF_STATUS ret = eBuf_Pass;
    MUINT32 dmaChannel = 0;
    BufInfo buf;
    NORMALSV_BUF_CTRL::E_BC_STATUS state;
    int i = 0;
    (void)pNotify;
    (void)u4TimeoutMs;
    //warning free

    PIPE_DBG("NormalSvIOPipe::dequeOutBuf+");

    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_ERR("op_cmd(%d)", op_cmd);
        return eBuf_Fail;
    }

    //clear remained data in container
    rQTBufInfo.vBufInfo.clear();

    #ifdef NORMALSV_SUBSAMPLE
    for (MUINT32 i = 0; i <= this->m_TopCtrl.SubSample; i++)
    #endif
    {
        //check if there is already filled buffer
        //wait P1_done
        state = this->m_FbcImgo.waitBufReady();
        switch(state){
            case NORMALSV_BUF_CTRL::eCmd_Fail:
                return eBuf_Fail;
                break;
            case NORMALSV_BUF_CTRL::eCmd_Suspending_Pass:
                return eBuf_suspending;
                break;
            case NORMALSV_BUF_CTRL::eCmd_Pass:
                break;
            default:
                PIPE_ERR("unsupported state:%d\n",state);
                return eBuf_Fail;
                break;
        }

        switch(this->m_FbcImgo.dequeueHwBuf(buf)){
            case NORMALSV_BUF_CTRL::eCmd_Fail:
                PIPE_ERR("data is rdy on dram,but deque fail\n");
                ret = eBuf_Fail;
                break;
            case NORMALSV_BUF_CTRL::eCmd_Suspending_Pass:
                ret = eBuf_suspending;
                break;
            case NORMALSV_BUF_CTRL::eCmd_Pass:
                ret = eBuf_Pass;
                break;
            default:
                PIPE_ERR("unsupported state:%d\n",state);
                return eBuf_Fail;
                break;
        }

        rQTBufInfo.vBufInfo.push_back(buf);

        PIPE_DBG("PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)",\
            portID.index, rQTBufInfo.vBufInfo[i].memID[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].u4BufVA[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].u4BufPA[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]);
    }

    #ifdef NORMALSV_SUBSAMPLE
    if (rQTBufInfo.vBufInfo.size() != (this->m_TopCtrl.SubSample+1))
    #else
    if(rQTBufInfo.vBufInfo.size() != 1)
    #endif
    {
        PIPE_ERR("dequeout data length(%zu) is mismatch", rQTBufInfo.vBufInfo.size());
        ret = eBuf_Fail;
    }

    this->FSM_UPDATE(op_cmd);

    PIPE_DBG("NormalSvIOPipe::dequeOutBuf-");

    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc)
{
    MBOOL   bPak = MTRUE;

    MUINT32 imgo_fmt = 0;

    MINT32 idx_in_src = -1;
    MINT32 idx_in_src_s = -1;
    MINT32 InSrcFmt = -1;
    MINT32 InSrcTGSw = -1;

    MINT32 idx_imgo = -1;
    MINT32 pix_id_tg = -1;
    MINT32 pixel_byte_imgo = -1;

    //ENUM_CAM_CTL_EN
    MUINT32 en_p1 = 0;

    PIPE_INF("NormalSvIOPipe::configPipe+");
    //
    if(this->FSM_CHECK(op_cfg) == MFALSE) {
        return MFALSE;
    }

    // OutPort is only IMGO, InPort support twin case
    if ( (vOutPorts.size() != 1) || (vInPorts.size() > 2)) {
        PIPE_ERR("inport:0x%zx || outport:x%zx size err\n", vInPorts.size(), vOutPorts.size());
        return MFALSE;
    }

    //input source
    //inport support 1~2 at current driver design
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) {
            PIPE_INF("dummy input vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vInPorts_%d:fmt(0x%x),PM(%d),w/h(%d_%d),crop(%d_%d_%d_%d),tg_idx(%d),dir(%d),fps(%d),MIPI_PixRate(%d),timeclk(0x%x)\n",\
                i, \
                vInPorts[i]->eImgFmt, \
                vInPorts[i]->ePxlMode,  \
                vInPorts[i]->u4ImgWidth, vInPorts[i]->u4ImgHeight,\
                vInPorts[i]->crop1.x,\
                vInPorts[i]->crop1.y,\
                vInPorts[i]->crop1.w,\
                vInPorts[i]->crop1.h,\
                vInPorts[i]->index, \
                vInPorts[i]->inout,\
                vInPorts[i]->tgFps,\
                vInPorts[i]->MIPI_PixRate,\
                vInPorts[i]->tTimeClk);

        //
        switch ( vInPorts[i]->index ) {
            case EPortIndex_CAMSV_0_TGI:
                PIPE_INF("EPortIndex_CAMSV_0_TGI");
                if(this->m_hwModule != CAMSV_0){
                    if(this->m_TwinMode_En && (this->m_hwModule == CAMSV_1))
                    {
                        PIPE_INF("twin case \n");
                    }
                    else
                    {
                    PIPE_ERR("TG0 must be with CAMSV_0\n");
                    return MFALSE;
                }
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG0 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG0 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_1_TGI:
                PIPE_INF("EPortIndex_CAMSV_1_TGI");
                if(this->m_hwModule != CAMSV_1){
                    if(this->m_TwinMode_En && (this->m_hwModule == CAMSV_0))
                    {
                        PIPE_INF("twin case \n");
                    }
                    else
                    {
                        PIPE_ERR("TG0 must be with CAMSV_1\n");
                    return MFALSE;
                }
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG1 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG1 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_2_TGI:
                PIPE_INF("EPortIndex_CAMSV_2_TGI");
                if(this->m_hwModule != CAMSV_2){
                    PIPE_ERR("TG2 must be with CAMSV_2\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG2 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG2 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_3_TGI:
                PIPE_INF("EPortIndex_CAMSV_3_TGI");
                if(this->m_hwModule != CAMSV_3){
                    PIPE_ERR("TG3 must be with CAMSV_3\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG3 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG3 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_4_TGI:
                PIPE_INF("EPortIndex_CAMSV_4_TGI");
                if(this->m_hwModule != CAMSV_4){
                    PIPE_ERR("TG4 must be with CAMSV_4\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG4 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG4 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_5_TGI:
                PIPE_INF("EPortIndex_CAMSV_5_TGI");
                if(this->m_hwModule != CAMSV_5){
                    PIPE_ERR("TG5 must be with CAMSV_5\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG5 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw = this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG5 input SW error\n");
                    return MFALSE;
                }

                break;
            default:
                PIPE_ERR("Not supported input source:0x%x !!!!!!!!!!!!!!!!!!!!!!",vInPorts[i]->index);
                return MFALSE;
                break;
        }

        this->m_dfs_info.input.lineT = vInPorts[i]->lineT;
        this->m_dfs_info.input.lineblkT = vInPorts[i]->lineblkT;
        this->m_dfs_info.input.frameT = 10000000 / vInPorts[i]->tgFps;
        this->m_dfs_info.input.frameblkT = vInPorts[i]->frameblkT;

        PIPE_INF("signal(%d_%d_%d_%d)\n",this->m_dfs_info.input.lineT, this->m_dfs_info.input.lineblkT,
            this->m_dfs_info.input.frameT, this->m_dfs_info.input.frameblkT);

        idx_in_src = 0;
        if(vInPorts.size() == 2)
            idx_in_src_s = 1;

    }

    // output port
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {

        if ( 0 == vOutPorts[i] ) {
            PIPE_INF("dummy output vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vOutPorts:[%d]:fmt(0x%x),PureRaw(%d),w(%d),h(%d),stirde(%d),dmao(%d),dir(%d)", \
                 i, vOutPorts[i]->eImgFmt, vOutPorts[i]->u4PureRaw, \
                 vOutPorts[i]->u4ImgWidth, vOutPorts[i]->u4ImgHeight,   \
                 vOutPorts[i]->u4Stride[ePlane_1st], \
                 vOutPorts[i]->index, vOutPorts[i]->inout);

        switch(vOutPorts[i]->index){
            case EPortIndex_IMGO:
            case EPortIndex_CAMSV_IMGO:
                PIPE_INF("EPortIndex_IMGO");
                idx_imgo = i;
                this->getOutPxlByteNFmt(vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&imgo_fmt);
                break;
            default:
                PIPE_INF("OutPorts index(%d) err\n", vOutPorts[i]->index);
                //break;
                return MFALSE;
        }
    }

    this->m_Subsample = pInFunc->Bits.SUBSAMPLE;
    this->m_Data_Pattern = pInFunc->Bits.DATA_PATTERN;
    this->m_Dcif_En = pInFunc->Bits.DCIF_En;
    this->m_Dcif_MasterRaw = pInFunc->Bits.DCIF_MasterRaw;
    this->m_TwinMode_En = pInFunc->Bits.DCIF_SvTwin_En;

    if(this->m_Dcif_En)
    {
        enum {
            NPIPE_CAM_NONE    = 0,
            NPIPE_CAM_A = 1,
            NPIPE_CAM_B = 2,
            NPIPE_CAM_C = 3,
            NPIPE_CAM_MAX = 4,
        } NPIPE_PATH_ENUM;

        //NPIPE_CAM_A = 1,
        this->m_hwMasterModule = (ISP_HW_MODULE)(CAM_A + (this->m_Dcif_MasterRaw - NPIPE_CAM_A));
        this->m_pCmdQMgr = DupCmdQMgr::CmdQMgr_GetCurMgrObj(this->m_hwMasterModule, ISP_DRV_CQ_THRE0);//this->m_Dcif_MasterRaw
        if(this->m_pCmdQMgr == NULL){
            PIPE_ERR("camiopipe configure fail or not configure yet!\n");
            //return MFALSE;
        }
    }

    //
    if((this->m_cam_dfs = CAM_DFS_Mgr::createInstance(LOG_TAG, this->m_hwModule)) == 0){
        PIPE_ERR("cam_dfs createInstance fail,no bw func at camsv\n");
    }
    else{
        if(this->m_cam_dfs->CAM_DFS_INIT(NULL) == MFALSE){
            PIPE_ERR("cam dfs init fail\n");
            return MFALSE;
        }
    }

    // Top ctrl setting
    this->m_TopCtrl.m_pDrv = this->m_pIspDrvCamsv;

    // subsample
    PIPE_INF("pInFunc->Bits.SUBSAMPLE %d", pInFunc->Bits.SUBSAMPLE);
    this->m_TopCtrl.SubSample = pInFunc->Bits.SUBSAMPLE;
    //DCIF
    this->m_TopCtrl.m_dcif_en = pInFunc->Bits.DCIF_En;
    this->m_TopCtrl.m_TwinMode_En = pInFunc->Bits.DCIF_SvTwin_En;
    // fmt sel
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Raw = 0x00;
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Bits.TG1_FMT = InSrcFmt;
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Bits.TG1_SW = InSrcTGSw;
    this->m_TopCtrl.m_PixMode = this->m_TGCtrl.m_PixMode = this->m_DmaImgo.m_PixMode = vInPorts[idx_in_src]->ePxlMode;
    //UFEO
    this->m_TopCtrl.m_ufeo_en = 0;

    // TG setting
    this->m_TGCtrl.m_pDrv = this->m_pIspDrvCamsv;
    this->m_TGCtrl.m_SubSample = pInFunc->Bits.SUBSAMPLE;
    this->m_TGCtrl.m_continuous = MTRUE; //TG input support only continuous mode
    //DCIF
    this->m_TGCtrl.m_dcif_en = pInFunc->Bits.DCIF_En;
    this->m_TGCtrl.m_TwinMode_En = pInFunc->Bits.DCIF_SvTwin_En;
     //UFEO
    this->m_TGCtrl.m_UFEEnable = 0;

    this->m_TGCtrl.m_Crop.x = vInPorts[idx_in_src]->crop1.x;
    this->m_TGCtrl.m_Crop.y = vInPorts[idx_in_src]->crop1.y;
    this->m_TGCtrl.m_Crop.w = vInPorts[idx_in_src]->crop1.w;
    this->m_TGCtrl.m_Crop.h = vInPorts[idx_in_src]->crop1.h;

    if((this->m_TwinMode_En == 1) && (this->m_Dcif_En))
    {
        int size_increament = (this->m_TGCtrl.m_Crop.w/2)%64
                                ? (((this->m_TGCtrl.m_Crop.w/2)/64) + 1) * 64
                                : (this->m_TGCtrl.m_Crop.w/2)/64 * 64;
        this->m_TGCtrl.m_Crop.w = size_increament;
    }

    // asign timestamp clk rate
    CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule, NULL, this->m_pIspDrvCamsv);
    pTime->TimeStamp_SrcClk(vInPorts[idx_in_src]->tTimeClk);

    // IMGO setting
    if(vOutPorts[idx_imgo]->crop1.floatX || vOutPorts[idx_imgo]->crop1.floatY){
        PIPE_ERR("imgo support no floating-crop_start , replaced by 0\n");
    }

    // use output dma crop
    this->configDmaPort(vOutPorts[idx_imgo], this->m_DmaImgo.dma_cfg, (MUINT32)pixel_byte_imgo);
    this->m_DmaImgo.m_pDrv = this->m_pIspDrvCamsv;
    this->m_DmaImgo.fmt_sel.Raw = this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Raw;
    this->m_DmaImgo.m_fps = vInPorts[idx_in_src]->tgFps;
    this->m_DmaImgo.m_dcif_en = this->m_Dcif_En;
    //this->m_DmaImgo.m_ufeo_en;
    this->m_DmaImgo.m_twin_en = this->m_TwinMode_En;


    /// IMGO FBC setting
    this->m_FbcImgo.m_pDrv = this->m_pIspDrvCamsv;
    if(this->m_Dcif_En && this->m_pCmdQMgr)
    {
        this->m_FbcImgo.m_pMasterCamDrv = this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];

    }
    this->m_FbcImgo.m_fps = vInPorts[idx_in_src]->tgFps;
    this->m_FbcImgo.m_pTimeStamp = pTime;
    //DCIF
    this->m_FbcImgo.m_dcif_en = pInFunc->Bits.DCIF_En;
    this->m_FbcImgo.m_twin_en = pInFunc->Bits.DCIF_SvTwin_En;
    this->m_FbcImgo.m_Subsample = pInFunc->Bits.SUBSAMPLE;
    //UFEO
    this->m_FbcImgo.m_ufeo_en = 0;

    if(this->m_TwinMode_En == 1)
    {
        // Top ctrl setting
        this->m_TopCtrlSlave.m_pDrv = this->m_pIspDrvCamsvSlave;

        // subsample
        PIPE_INF("pInFunc->Bits.SUBSAMPLE %d", pInFunc->Bits.SUBSAMPLE);
        this->m_TopCtrlSlave.SubSample = pInFunc->Bits.SUBSAMPLE;
        //DCIF
        this->m_TopCtrlSlave.m_dcif_en = pInFunc->Bits.DCIF_En;
        this->m_TopCtrlSlave.m_TwinMode_En = pInFunc->Bits.DCIF_SvTwin_En;
        // fmt sel
        this->m_TopCtrlSlave.camsv_top_ctl.FMT_SEL.Raw = 0x00;
        this->m_TopCtrlSlave.camsv_top_ctl.FMT_SEL.Bits.TG1_FMT = InSrcFmt;
        this->m_TopCtrlSlave.camsv_top_ctl.FMT_SEL.Bits.TG1_SW = InSrcTGSw;
        this->m_TopCtrlSlave.m_PixMode = this->m_TGCtrlSlave.m_PixMode = this->m_DmaImgoSlave.m_PixMode = vInPorts[idx_in_src_s]->ePxlMode;
        //UFEO
        this->m_TopCtrlSlave.m_ufeo_en = 0;

        //TGCtrl
        this->m_TGCtrlSlave.m_PixMode = vInPorts[idx_in_src_s]->ePxlMode;
        // TG setting
        this->m_TGCtrlSlave.m_pDrv = this->m_pIspDrvCamsvSlave;
        this->m_TGCtrlSlave.m_SubSample = pInFunc->Bits.SUBSAMPLE;
        this->m_TGCtrlSlave.m_continuous = MTRUE; //TG input support only continuous mode
        //DCIF
        this->m_TGCtrlSlave.m_dcif_en = pInFunc->Bits.DCIF_En;
        this->m_TGCtrlSlave.m_TwinMode_En = pInFunc->Bits.DCIF_SvTwin_En;
        //UFEO
        this->m_TGCtrlSlave.m_UFEEnable = 0;
        this->m_TGCtrlSlave.m_Crop.x = vInPorts[idx_in_src_s]->crop1.x;
        this->m_TGCtrlSlave.m_Crop.y = vInPorts[idx_in_src_s]->crop1.y;
        this->m_TGCtrlSlave.m_Crop.w = vInPorts[idx_in_src_s]->crop1.w;
        this->m_TGCtrlSlave.m_Crop.h = vInPorts[idx_in_src_s]->crop1.h;

        {
            int size_increament = (this->m_TGCtrlSlave.m_Crop.w/2)%64
                                    ? (((this->m_TGCtrlSlave.m_Crop.w/2)/64) + 1) * 64
                                    : (this->m_TGCtrlSlave.m_Crop.w/2)/64 * 64;
            this->m_TGCtrlSlave.m_Crop.w = this->m_TGCtrlSlave.m_Crop.w - size_increament;
        }

        //DMAIMGO
        // use output dma crop
        this->configDmaPort(vOutPorts[idx_imgo], this->m_DmaImgoSlave.dma_cfg, (MUINT32)pixel_byte_imgo, 1/*slave, IMGO_B*/);
        this->m_DmaImgoSlave.m_pDrv = this->m_pIspDrvCamsvSlave;
        this->m_DmaImgoSlave.fmt_sel.Raw = this->m_TopCtrlSlave.camsv_top_ctl.FMT_SEL.Raw;
        this->m_DmaImgoSlave.m_fps = vInPorts[idx_in_src_s]->tgFps;
        this->m_DmaImgoSlave.m_dcif_en = this->m_Dcif_En;
        //this->m_DmaImgoSlave.m_ufeo_en;
        this->m_DmaImgoSlave.m_twin_en = this->m_TwinMode_En;


        //FBCIMGO
        // asign timestamp clk rate
        CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModuleCamsvSlave, NULL, this->m_pIspDrvCamsvSlave);
        pTime->TimeStamp_SrcClk(vInPorts[idx_in_src_s]->tTimeClk);
        /// IMGO FBC setting
        this->m_FbcImgoSlave.m_pDrv = this->m_pIspDrvCamsvSlave;
        if(this->m_Dcif_En && this->m_pCmdQMgr)
        {
            this->m_FbcImgoSlave.m_pMasterCamDrv = this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];

        }
        this->m_FbcImgoSlave.m_fps = vInPorts[idx_in_src_s]->tgFps;
        this->m_FbcImgoSlave.m_pTimeStamp = pTime;
        //DCIF
        this->m_FbcImgoSlave.m_dcif_en = pInFunc->Bits.DCIF_En;
        this->m_FbcImgoSlave.m_twin_en = pInFunc->Bits.DCIF_SvTwin_En;
        this->m_FbcImgoSlave.m_Subsample = pInFunc->Bits.SUBSAMPLE;
        //UFEO
        this->m_FbcImgoSlave.m_ufeo_en = 0;
    }

    /// 1. TG config, enable
    /// 2. Top config
    /// 3. IMGO & FBC config, enable
    if(0 != this->m_TGCtrl.config()){
        PIPE_ERR("m_TGCtrl.config fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_TGCtrlSlave.config()){
            PIPE_ERR("m_TGCtrlSlave.config fail");
            return MFALSE;
        }
    }

    if(0 != this->m_TopCtrl.config()){
        PIPE_ERR("m_TopCtrl.config fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_TopCtrlSlave.config()){
            PIPE_ERR("m_TopCtrlSlave.config fail");
            return MFALSE;
        }
    }

    if(0 != this->m_FbcImgo.config()){
        PIPE_ERR("m_FbcImgo.config fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_FbcImgoSlave.config()){
            PIPE_ERR("m_FbcImgoSlave.config fail");
        return MFALSE;
    }
    }

    if(0 != this->m_DmaImgo.config()){
        PIPE_ERR("m_DmaImgo.config fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_DmaImgoSlave.config()){
            PIPE_ERR("m_DmaImgoSlave.config fail");
        return MFALSE;
    }
    }

    if(0 != this->m_TGCtrl.enable(NULL)){
        PIPE_ERR("m_TGCtrl.enable fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_TGCtrlSlave.enable(NULL)){
            PIPE_ERR("m_TGCtrlSlave.enable fail");
        return MFALSE;
    }
    }

    if(0 != this->m_FbcImgo.enable(NULL)){
        PIPE_ERR("m_FbcImgo.enable fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_FbcImgoSlave.enable(NULL)){
            PIPE_ERR("m_FbcImgoSlave.enable fail");
            return MFALSE;
        }
    }

    if(0 != this->m_DmaImgo.enable(NULL)){
        PIPE_ERR("m_DmaImgo.enable fail");
        return MFALSE;
    }

    if(this->m_TwinMode_En)
    {
        if(0 != this->m_DmaImgoSlave.enable(NULL)){
            PIPE_ERR("m_DmaImgoSlave.enable fail");
            return MFALSE;
        }
    }

    this->FSM_UPDATE(op_cfg);

    PIPE_INF("NormalSvIOPipe::configPipe-");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL NormalSvIOPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL    ret = MTRUE; // 0 for ok , -1 for fail

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08zx,0x%08zx,0x%08zx)", gettid(), cmd, arg1, arg2, arg3);

    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch ( cmd ) {
        case EPIPECmd_SET_FRM_TIME:
            if (MFALSE == this->FSM_CHECK(op_cmd)) {
                PIPE_ERR("EPIPECmd_SET_FRM_TIME FSM error");
                ret = MFALSE;
                break;
            }

            if (MFALSE == this->m_BufCtrl.updateFrameTime(
                    (MUINT32)arg1, (MUINT32)arg2, this->m_hwModule)) {
                PIPE_ERR("Update frame time fail: arg1:%d arg2:%d hwModule:%d", (MUINT32)arg1, (MUINT32)arg2, this->m_hwModule);
                ret = MFALSE;
                break;
            }

            break;
        case EPIPECmd_GET_HEADER_SIZE:
            {
                capibility CamInfo;
                tCAM_rst rst;

                if(CamInfo.GetCapibility(arg1,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                            rst,E_CAM_HEADER_size) == MFALSE){
                    ret = MFALSE;
                    goto EXIT;
                }

                *(MUINT32*)arg2 = rst.HeaderSize;
            }
            break;
        case EPIPECmd_GET_CUR_FRM_STATUS:
            {
                MUINT32 _flag = 0;
                switch(this->m_hwModule){
                    case CAMSV_0:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                        break;
                    case CAMSV_1:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                        break;
                    case CAMSV_2:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                        break;
                    case CAMSV_3:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                        break;
                    case CAMSV_4:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                        break;
                    case CAMSV_5:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                        break;
                    case CAMSV_6:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_6_ST;
                        break;
                    case CAMSV_7:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_7_ST;
                        break;
                    default:
                        PIPE_ERR("unsupported module(%d)\n", this->m_hwModule);
                        return MFALSE;
                        break;
                }
                m_pIspDrvCamsv->getDeviceInfo(_GET_DROP_FRAME_STATUS,(MUINT8 *)&_flag);
                *(MUINT32*)arg1 = _flag;
            }
            break;
        case EPIPECmd_GET_CUR_SOF_IDX:
            {
                MUINT32 _flag = 0;
                switch(this->m_hwModule){
                    case CAMSV_0:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                        break;
                    case CAMSV_1:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                        break;
                    case CAMSV_2:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                        break;
                    case CAMSV_3:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                        break;
                    case CAMSV_4:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                        break;
                    case CAMSV_5:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                        break;
                    case CAMSV_6:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_6_ST;
                        break;
                    case CAMSV_7:
                        _flag = ISP_IRQ_TYPE_INT_CAMSV_7_ST;
                        break;
                    default:
                        PIPE_ERR("unsupported module(%d)\n", this->m_hwModule);
                        return MFALSE;
                        break;
                }
                m_pIspDrvCamsv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_flag);

                *(MUINT32*)arg1 = _flag;
            }
            break;

        case EPIPECmd_GET_TG_INDEX:
            //arg1 : TG index
            //arg2 : 2nd-TG index
            //arg3 : mpSensorIdx from normalpipe
            if (arg1 && (InPutTG.size() >= 1)) {
                *(MINT32*)arg1 = NSVPIPE_MAP_SEN_TG(InPutTG.at(0));
                if(arg2 && (InPutTG.size() >= 2)) {
                    *(MINT32*)arg2 = NSVPIPE_MAP_SEN_TG(InPutTG.at(1));
                    PIPE_DBG("get 2nd-TG index (%d) \n", *(MINT32*)arg2);
                }
                PIPE_DBG("get TG index (%d)\n", *(MINT32*)arg1);
            }
            else {
                ret = MFALSE;
                goto EXIT;
            }
            break;

        case EPIPECmd_SET_TG_INT_LINE:
            //arg1: TG_INT1_LINENO
            //arg2: sensor pxlMode
            {
                REG_CAMSV_TG_SEN_GRAB_PXL TG_W;
                REG_CAMSV_TG_SEN_GRAB_LIN TG_H;
                MUINT32 lineno = (MUINT32)arg1;
                MUINT32 pixMode = (E_CamPixelMode)arg2;
                MUINT32 size_h,size_v,pixno;

                PIPE_DBG("set TgLine(%d)pxmod(%d)\n", lineno, pixMode);
                #if 1
                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),
                    TG_R1_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),
                    TG_R1_TG_SEN_GRAB_LIN);
                size_h   = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
                size_v   = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                /*sensor pix mode*/
                if(pixMode == ePixMode_1){
                    pixno = size_h;
                }
                else if(pixMode == ePixMode_2){
                    pixno = (size_h / 2) * 2;
                }else if(pixMode == ePixMode_4){
                    pixno = (size_h / 4) * 4;
                }else{
                    PIPE_ERR("ERROR: unknown pix mode:%d", pixMode);
                    return MFALSE;
                }

                if(pixno > size_h) {
                    PIPE_ERR("ERROR: TG pixno(%d) must < %d\n",pixno,size_h);
                    return MFALSE;
                }

                if(lineno > size_v) {
                    PIPE_ERR("ERROR: TG lineno(%d) must < %d\n",lineno,size_v);
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),
                    TG_R1_TG_INT1,TG_TG_INT1_LINENO,lineno);
                CAM_WRITE_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),
                    TG_R1_TG_INT1,TG_TG_INT1_PXLNO,pixno);
                #endif
                break;
            }

        case EPIPECmd_GET_TG_OUT_SIZE:
            //arg1 : width
            //arg2 : height
            //arg3 : mpSensorIdx from normalpipe
            {
                *((MINT32*)arg1) = crop.w;
                *((MINT32*)arg2) = crop.h;
                break;
            }

        default:
            PIPE_ERR("NOT support command!");
            ret = MFALSE;
            break;
    }
EXIT:
    if( ret != MTRUE )
    {
        PIPE_ERR("sendCommand(0x%x) error!",cmd);
    }

    return  ret;
}

MINT32 NormalSvIOPipe::registerIrq(MINT8 const szUsrName[32])
{
    ISP_REGISTER_USERKEY_STRUCT _irq;
    //
    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return -1;

    if(this->m_pIspDrvCamsv == NULL){
        PIPE_ERR("ispDrvCamsv is null pointer, registerIrq fail");
        return -1;
    }

    memcpy((void*)_irq.userName, (void*)szUsrName, sizeof(_irq.userName));

    if(m_pIspDrvCamsv->registerIrq(&_irq) == MFALSE)
    {
        PIPE_ERR("registerIrq error!!!");
        return -1;
    }

    this->FSM_UPDATE(op_cmd);
    return _irq.userKey;
}

MINT32 NormalSvIOPipe::SrcFmtToHwVal_TG( ImgInfo::EImgFmt_t imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8: return TG_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:return TG_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:return TG_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:return TG_FMT_RAW14;
            break;
        //case eImgFmt_RGB565: return TG_FMT_RGB565;
        //case eImgFmt_RGB888: return TG_FMT_RGB888;
        //case eImgFmt_JPEG:   return TG_FMT_JPG;

        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
             return TG_FMT_YUV422;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}

MINT32 NormalSvIOPipe::SrcFmtToHwVal_TG_SW( ImgInfo::EImgFmt_t imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_BAYER14:
            return TG_SW_UYVY; // 0
            break;
        //case eImgFmt_RGB565: return TG_FMT_RGB565;
        //case eImgFmt_RGB888: return TG_FMT_RGB888;
        //case eImgFmt_JPEG:   return TG_FMT_JPG;

        case eImgFmt_YUY2:
            return TG_SW_YUYV;
            break;
        case eImgFmt_UYVY:
            return TG_SW_UYVY;
            break;
        case eImgFmt_YVYU:
            return TG_SW_YVYU;
            break;
        case eImgFmt_VYUY:
             return TG_SW_VYUY;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}


//return HW register format
MINT32 NormalSvIOPipe::getOutPxlByteNFmt(ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt)
{
    //
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }

    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_RAW8;
            break;
        case eImgFmt_FG_BAYER8:
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = RRZO_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = IMGO_FMT_RAW10;
            break;
        case eImgFmt_FG_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = RRZO_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW12;
            break;
        case eImgFmt_FG_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW14;
            break;
        case eImgFmt_FG_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW14;
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_YUV422_1P;
            break;
        case eImgFmt_RGB565:
            //*pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB565;
            //break;
        case eImgFmt_RGB888:
            //*pPixel_byte = 3 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB888;
            //break;
        case eImgFmt_JPEG:
        case eImgFmt_BLOB:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW10):(CAM_FMT_SEL_BAYER10);
            //break;
        case eImgFmt_UFO_BAYER10:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = CAM_FMT_SEL_TG_FMT_RAW10;
            //break;
        default:
            PIPE_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    //
    PIPE_INF("input imgFmt(0x%x),output fmt:0x%x,*pPixel_byte(%d)",imgFmt,*pFmt,*pPixel_byte);
    //PIPE_INF(" ");

    return 0;
}


MBOOL NormalSvIOPipe::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 slave,MUINT32 swap, MUINT32 isBypassOffset,E_BufPlaneID planeNum)
{
    capibility CamInfo;
    tCAM_rst rst;
    MBOOL tmp = MFALSE;
    (void)isBypassOffset;

    a_dma.memBuf.size        = portInfo->u4BufSize[planeNum];
    a_dma.memBuf.base_vAddr  = portInfo->u4BufVA[planeNum];
    a_dma.memBuf.base_pAddr  = portInfo->u4BufPA[planeNum];
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->u4ImgWidth;
    a_dma.size.h            = portInfo->u4ImgHeight;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->u4Stride[planeNum];

    //
    a_dma.lIspColorfmt = portInfo->eImgFmt;

    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte

    a_dma.size.xsize        =  portInfo->u4ImgWidth;

    if(this->m_TwinMode_En == 1 && (slave == 0))
    {
        int size_increament = (portInfo->u4ImgWidth/2)%64
                                ? (((portInfo->u4ImgWidth/2)/64) + 1) * 64
                                : (portInfo->u4ImgWidth/2)/64 * 64;

        a_dma.size.w = a_dma.size.xsize = size_increament;
    }
    else if(this->m_TwinMode_En == 1 && (slave == 1))
    {
        int size_increament = (portInfo->u4ImgWidth/2)%64
                                ? (((portInfo->u4ImgWidth/2)/64) + 1) * 64
                                : (portInfo->u4ImgWidth/2)/64 * 64;

        a_dma.size.w = a_dma.size.xsize = (portInfo->u4ImgWidth - size_increament);
    }

    tmp = CamInfo.GetCapibility(
        portInfo->index,
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)portInfo->eImgFmt, a_dma.size.w, ePixMode_NONE),
        rst, E_CAM_UNKNOWNN);

    if(tmp == MFALSE) {
        PIPE_ERR("CamInfo.GetCapibility error");
    }
    else
    {
        a_dma.size.xsize        =  rst.xsize_byte[0];
    }

    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum == ePlane_1st) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    a_dma.crop.x            = portInfo->crop1.x;
    a_dma.crop.floatX       = 0;//portInfo->crop1.floatX;
    a_dma.crop.y            = portInfo->crop1.y;
    a_dma.crop.floatY       = 0;//portInfo->crop1.floatY;
    a_dma.crop.w            = portInfo->crop1.w;
    a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    if(slave == 1)//IMGO B, offset address = IMGO A X size + 1
    {
        a_dma.memBuf.ofst_addr = m_Dcif_Offset;
    }
    else
    {
        m_Dcif_Offset = a_dma.size.xsize;//keep 1st camsv module dma x-size
        a_dma.memBuf.ofst_addr = 0;//offset at isp function
    }
    //

    switch( portInfo->eImgFmt ) {
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=2;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            //break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=2;
            //break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=3;
            //break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in cam\n",portInfo->eImgFmt);
            break;
        case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=1;
            break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
        default:
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=0;
            break;
    }
    //

    PIPE_INF("w(%lu),h(%lu),stride(%lu),xsize(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%zx),pa(0x%zx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_INF("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

MBOOL NormalSvIOPipe::configDmaPort(BufInfo bufInfo, IspDMACfg &a_dma)
{
#if 0
    a_dma.size.w            =  bufInfo.img_w;
    a_dma.size.h            =  bufInfo.img_h;
    a_dma.size.stride       =  bufInfo.img_stride;
    a_dma.size.xsize        =  bufInfo.img_w;
    a_dma.crop.x            =  bufInfo.crop_win.p.x;
    a_dma.crop.y            =  bufInfo.crop_win.p.y;
    a_dma.crop.floatX       =  0;
    a_dma.crop.floatY       =  0;
    a_dma.crop.w            =  bufInfo.crop_win.s.w;
    a_dma.crop.h            =  bufInfo.crop_win.s.h;
#endif
    PIPE_INF("img_w(%d),img_h(%d),img_stride(%d),xsize(%d),crop(%d_%d_%d_%d)", a_dma.size.w, a_dma.size.h, a_dma.size.stride, a_dma.size.xsize, a_dma.crop.x, a_dma.crop.y, a_dma.crop.w, a_dma.crop.h);

    return MTRUE;
}

MBOOL NormalSvIOPipe::irq(Irq_t* pIrq)
{
    ISP_WAIT_IRQ_ST _irq;

    PIPE_INF("NormalSvIOPipe::irq+");
    //
    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_INF("");
        return MFALSE;
    }

    if(this->m_pIspDrvCamsv == NULL){
        PIPE_ERR("ispDrvCamsv is null pointer, irq fail");
        return MFALSE;
    }

    switch(pIrq->Type){
        case Irq_t::_CLEAR_NONE:
            PIPE_INF("Irq_t::_CLEAR_NONE");
            _irq.Clear = ISP_IRQ_CLEAR_NONE;
            break;
        case Irq_t::_CLEAR_WAIT:
            PIPE_INF("Irq_t::_CLEAR_WAIT");
            _irq.Clear = ISP_IRQ_CLEAR_WAIT;
            break;
        default:
            PIPE_ERR("unsupported type:0x%x\n",pIrq->Type);
            return MFALSE;
            break;
    }

    switch(pIrq->StatusType){
        case Irq_t::_SIGNAL_INT_:
            PIPE_INF("Irq_t::_SIGNAL_INT_");
            switch(pIrq->Status){
                case Irq_t::_VSYNC_:
                    PIPE_INF("Irq_t::_VSYNC_");
                    _irq.Status = SV_VS1_ST;
                    break;
                case Irq_t::_SOF_:
                    PIPE_INF("Irq_t::_SOF_");
                    _irq.Status = SV_SOF_INT_ST;
                    break;
                case Irq_t::_EOF_:
                    PIPE_INF("Irq_t::_EOF_");
                    _irq.Status = SV_EXPDON_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",pIrq->Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = SIGNAL_INT;
            break;
        case Irq_t::_DMA_INT_:
            PIPE_INF("Irq_t::_DMA_INT_");
            switch(pIrq->Status){
                default:
                    PIPE_ERR("unsupported DMA signal:0x%x\n",pIrq->Status);
                    break;
            }
            _irq.St_type = DMA_INT;
            return MFALSE;
            break;
        default:
            PIPE_ERR("unsupported status type:0x%x\n",pIrq->StatusType);
            return MFALSE;
            break;
    }

    _irq.UserKey = pIrq->UserKey;
    _irq.Timeout = pIrq->Timeout;

    PIPE_INF("pIrq->Timeout(%d)", pIrq->Timeout);

    if(this->m_pIspDrvCamsv->waitIrq(&_irq) == MFALSE)
    {
        PIPE_ERR("waitirq error!\n");
        return  MFALSE;
    }

    pIrq->TimeInfo.tLastSig_sec = (MUINT32)_irq.TimeInfo.tLastSig_sec;
    pIrq->TimeInfo.tLastSig_usec = (MUINT32)_irq.TimeInfo.tLastSig_usec;
    pIrq->TimeInfo.tMark2WaitSig_sec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_sec;
    pIrq->TimeInfo.tMark2WaitSig_usec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_usec;
    pIrq->TimeInfo.tLastSig2GetSig_sec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_sec;
    pIrq->TimeInfo.tLastSig2GetSig_usec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_usec;
    pIrq->TimeInfo.passedbySigcnt = (MUINT32)_irq.TimeInfo.passedbySigcnt;

    this->FSM_UPDATE(op_cmd);

    return MTRUE;
}

MBOOL NormalSvIOPipe::signalIrq(Irq_t irq)
{
    ISP_WAIT_IRQ_ST _irq;

    PIPE_INF("NormalSvIOPipe::signalIrq+");
    //
    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_INF("");
        return MFALSE;
    }

    if(this->m_pIspDrvCamsv == NULL){
        PIPE_ERR("ispDrvCamsv is null pointer, signalIrq fail");
        return MFALSE;
    }

    switch(irq.Type){
        case Irq_t::_CLEAR_NONE:
            PIPE_INF("Irq_t::_CLEAR_NONE");
            _irq.Clear = ISP_IRQ_CLEAR_NONE;
            break;
        case Irq_t::_CLEAR_WAIT:
            PIPE_INF("Irq_t::_CLEAR_WAIT");
            _irq.Clear = ISP_IRQ_CLEAR_WAIT;
            break;
        default:
            PIPE_ERR("unsupported type:0x%x\n",irq.Type);
            return MFALSE;
            break;
    }

    switch(irq.StatusType){
        case Irq_t::_SIGNAL_INT_:
            PIPE_INF("Irq_t::_SIGNAL_INT_");
            switch(irq.Status){
                case Irq_t::_VSYNC_:
                    PIPE_INF("Irq_t::_VSYNC_");
                    _irq.Status = SV_VS1_ST;
                    break;
                case Irq_t::_SOF_:
                    PIPE_INF("Irq_t::_SOF_");
                    _irq.Status = SV_SOF_INT_ST;
                    break;
                case Irq_t::_EOF_:
                    PIPE_INF("Irq_t::_EOF_");
                    _irq.Status = SV_EXPDON_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",irq.Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = SIGNAL_INT;
            break;
        case Irq_t::_DMA_INT_:
            PIPE_INF("Irq_t::_DMA_INT_");
            switch(irq.Status){
                default:
                    PIPE_ERR("unsupported DMA signal:0x%x\n",irq.Status);
                    break;
            }
            _irq.St_type = DMA_INT;
            return MFALSE;
            break;
        default:
            PIPE_ERR("unsupported status type:0x%x\n",irq.StatusType);
            return MFALSE;
            break;
    }

    _irq.UserKey = irq.UserKey;
    _irq.Timeout = irq.Timeout;

    PIPE_INF("irq.Timeout(%d)", irq.Timeout);

    if(this->m_pIspDrvCamsv->signalIrq(&_irq) == MFALSE)
    {
        PIPE_ERR("signalirq error!\n");
        return  MFALSE;
    }

    this->FSM_UPDATE(op_cmd);

    return MTRUE;
}

MBOOL NormalSvIOPipe::getClockSet(void)
{
    #define MHZ 1000000
    MBOOL ret = MTRUE;
    ISP_CLK_INFO supportedClk;
    // fake isp clk: 315 416 560
    char fakeclk[3][PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.isp.fakeclk1", fakeclk[0], "0");
    property_get("vendor.isp.fakeclk2", fakeclk[1], "0");
    property_get("vendor.isp.fakeclk3", fakeclk[2], "0");

    if(atoi(fakeclk[0]) || atoi(fakeclk[1]) || atoi(fakeclk[2])){
        // use fake clock
        if(atoi(fakeclk[0])){
            PIPE_INF("SupportedFakeClk[0]=%d",atoi(fakeclk[0]));
            //m_SupportedClk.push_back(atoi(fakeclk[0])*MHZ);
            clk_gear.push_back(atoi(fakeclk[0])*MHZ);
        }
        if(atoi(fakeclk[1])){
            PIPE_INF("SupportedFakeClk[1]=%d",atoi(fakeclk[1]));
            //m_SupportedClk.push_back(atoi(fakeclk[1])*MHZ);
            clk_gear.push_back(atoi(fakeclk[1])*MHZ);
        }
        if(atoi(fakeclk[2])){
            PIPE_INF("SupportedFakeClk[2]=%d",atoi(fakeclk[2]));
            //m_SupportedClk.push_back(atoi(fakeclk[2])*MHZ);
            clk_gear.push_back(atoi(fakeclk[2])*MHZ);
        }
    }
    else {
        if((ret = this->m_pIspDrvCamsv->getDeviceInfo(_GET_SUPPORTED_ISP_CLOCKS, (MUINT8*)&supportedClk)) == MFALSE)
            PIPE_ERR("Error in _GET_SUPPORTED_ISP_CLOCKS!!");

        for(MINT32 i=(supportedClk.clklevelcnt-1); i >= 0; i--){
            PIPE_INF("SupportedClk[%d]=%d",i, supportedClk.clklevel[i]);
            //m_SupportedClk.push_back(supportedClk.clklevel[i]*MHZ);
            clk_gear.push_back(supportedClk.clklevel[i]*MHZ);
        }
    }

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio


