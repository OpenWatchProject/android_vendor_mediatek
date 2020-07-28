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
#define LOG_TAG "ccuif"

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <sys/mman.h>
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <m4u_lib_port.h>

//#include "ccu_sensor_if.h"
#include "ccuif.h"
#include "ccuif_compat.h"
#include "ccu_udrv.h"

#include <cutils/properties.h>  // For property_get().

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {
namespace NSCcuDrv   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "ccu_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        LOG_TAG
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(ccu_drv);

/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/


//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
ICcu* ICcu::createInstance( MINT8 const szUsrName[32],E_CCUIF_INPUT InPut,E_CCUIF_OPT opt)
{
    switch(InPut){
        case AEAWB:
			break;

        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InPut);
            return NULL;
            break;
    }

    switch(opt){
        case AEAWBCTRL:
            return  (ICcu*)CcuIF::Create(szUsrName,InPut);
            break;
        default:
            BASE_LOG_ERR("unsupported ccuif:0x%x\n",opt);
            return NULL;
            break;
    }

}

MVOID ICcu::destroyInstance(void)
{
    switch(this->m_ccuif_opt)
    {
        case AEAWBCTRL:
            ((CcuIF*)this)->Destroy();
            break;
        default:
            BASE_LOG_ERR("unsupported ccuif:0x%x\n",this->m_ccuif_opt);
            break;
    }
}

/*******************************************************************************
*
********************************************************************************/
CcuIF::CcuIF():
gIonDevFD(-1),
buf_share_fd_in_data(0),
buf_va_in_data(NULL),
buf_mva_in_data(0),
sz_in_data(SIZE_CCU_INOUT_DATA_BUFFER),
buf_share_fd_out_data(0),
buf_va_out_data(NULL),
buf_mva_out_data(0),
sz_out_data(SIZE_CCU_INOUT_DATA_BUFFER),
sz_ae_cust_data(SIZE_CCU_AE_CUST_DATA_BUFFER),
sz_ae_onchange_data(SIZE_CCU_AE_ONCHANGE_DATA_BUFFER),
pDrvCcu(NULL)
{
    //
    DBG_LOG_CONFIG(ccuif, ccu_drv);

	this->m_ccuif_opt = ICcu::AEAWBCTRL;
	//this->gIonDevFD = -1;
	//this->pDrvCcu = NULL;

    LOG_INF(":X");
}

CcuIF::~CcuIF()
{
}

/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*
********************************************************************************/

static CcuIF gCcu[CCU_MAX];
CcuIF* CcuIF::Create(MINT8 const szUsrName[32], E_CCUIF_INPUT InPut)
{
    CcuIF* ptr = NULL;
    BASE_LOG_INF(":E:user:%s, with input source:0x%x\n",szUsrName,InPut);
	//
    switch(InPut){
        case AEAWB:
            if(gCcu[CCU_A].occupied == MTRUE){
                BASE_LOG_ERR("AEAWB is occupied by user:%s\n",gCcu[CCU_A].m_szUsrName);
            }
            else{
                gCcu[CCU_A].m_hwModule = CCU_A;
                gCcu[CCU_A].occupied = MTRUE;
                std::strncpy((char*)gCcu[CCU_A].m_szUsrName,(char const*)szUsrName, MAX_USER_NAME_SIZE-1);
                ptr = &gCcu[CCU_A];
            }
            break;
        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InPut);
            break;
    }
	//
    if(ptr == NULL)
        return NULL;

    return ptr;
}

void CcuIF::Destroy(void)
{
    LOG_INF(":E:user:%sx\n",this->m_szUsrName);

    this->occupied = MFALSE;
    this->m_szUsrName[0] = '\0';

}

/*******************************************************************************
*
********************************************************************************/
MBOOL CcuIF::init()
{
    DBG_LOG_CONFIG(ccuif, ccu_drv);

    LOG_INF("+:(%s)\n",this->m_szUsrName);

    gIonDevFD = mt_ion_open("CCU");
    if  ( 0 > gIonDevFD )
    {
        LOG_ERR("mt_ion_open() return %d", gIonDevFD);
		return false;
    }

	pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance(CCU_A);
	if ( NULL == pDrvCcu ) {
		LOG_ERR("createInstance(CCU_A)return %d", pDrvCcu);
		return false;
	}

	this->ccu_get_buffer( gIonDevFD, this->sz_in_data, &buf_share_fd_in_data, &buf_va_in_data );
	if ( NULL == buf_va_in_data ) {
		LOG_ERR("ccu_get_buffer buf_va_in_data: %p", buf_va_in_data);
		return false;
	}
	this->mmapMVA( buf_share_fd_in_data, &ion_handle_in_data, &buf_mva_in_data);
	if ( 0 == buf_mva_in_data ) {
		LOG_ERR("mmapMVA(buf_mva_in_data) %x", buf_mva_in_data);
		return false;
	}

	this->ccu_get_buffer( gIonDevFD, this->sz_out_data, &buf_share_fd_out_data, &buf_va_out_data );
	if ( NULL == buf_va_out_data ) {
		LOG_ERR("ccu_get_buffer buf_va_out_data: %p", buf_va_out_data);
		return false;
	}
	this->mmapMVA( buf_share_fd_out_data, &ion_handle_out_data, &buf_mva_out_data);
	if ( 0 == buf_mva_out_data ) {
		LOG_ERR("mmapMVA(buf_mva_out_data) %x", buf_mva_out_data);
		return false;
	}

    this->ccu_get_buffer( gIonDevFD, this->sz_ae_cust_data, &buf_share_fd_ae_cust_data, &buf_va_ae_cust_data );
    if ( NULL == buf_va_ae_cust_data ) {
        LOG_ERR("ccu_get_buffer buf_va_ae_cust_data: %p", buf_va_ae_cust_data);
        return false;
    }
    this->mmapMVA( buf_share_fd_ae_cust_data, &ion_handle_ae_cust_data, &buf_mva_ae_cust_data);
    if ( 0 == buf_mva_ae_cust_data ) {
        LOG_ERR("mmapMVA(buf_mva_ae_cust_data) %x", buf_mva_ae_cust_data);
        return false;
    }

    this->ccu_get_buffer( gIonDevFD, this->sz_ae_onchange_data, &buf_share_fd_ae_onchange_data, &buf_va_ae_onchange_data );
    if ( NULL == buf_va_ae_onchange_data ) {
        LOG_ERR("ccu_get_buffer buf_va_ae_onchange_data: %p", buf_va_ae_onchange_data);
        return false;
    }
    this->mmapMVA( buf_share_fd_ae_onchange_data, &ion_handle_ae_onchange_data, &buf_mva_ae_onchange_data);
    if ( 0 == buf_mva_ae_onchange_data ) {
        LOG_ERR("mmapMVA(buf_mva_ae_onchange_data) %x", buf_mva_ae_onchange_data);
        return false;
    }

    LOG_INF("buf_va_in_data: %p\n", buf_va_in_data);
    LOG_INF("buf_mva_in_data: %x\n", buf_mva_in_data);
    LOG_INF("buf_va_out_data: %p\n", buf_va_out_data);
    LOG_INF("buf_mva_out_data: %x\n", buf_mva_out_data);
    LOG_INF("buf_va_ae_cust_data: %p\n", buf_va_ae_cust_data);
    LOG_INF("buf_mva_ae_cust_data: %x\n", buf_mva_ae_cust_data);
    LOG_INF("buf_mva_ae_onchange_data: %x\n", buf_mva_ae_onchange_data);
    LOG_INF("-:(%s)\n",this->m_szUsrName);

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CcuIF::uninit()
{
    LOG_DBG("+:(%s)\n",this->m_szUsrName);

	if ( false == this->munmapMVA( ion_handle_in_data ) ) {
		LOG_ERR("munmapMVA fail:ion_handle_in_data\n");
	}
	if ( false == this->munmapMVA( ion_handle_out_data ) ) {
		LOG_ERR("munmapMVA fail:ion_handle_out_data\n");
	}
    if ( false == this->munmapMVA( ion_handle_ae_cust_data ) ) {
		LOG_ERR("munmapMVA fail:ion_handle_ae_cust_data\n");
	}
    if ( false == this->munmapMVA( ion_handle_ae_onchange_data ) ) {
        LOG_ERR("munmapMVA fail:ion_handle_ae_onchange_data\n");
    }

	//
	if ( false == ccu_free_buffer( gIonDevFD, this->sz_in_data, buf_share_fd_in_data, buf_va_in_data) ) {
		LOG_ERR("ccu_free_buffer fail:buf_va_in_data\n");
	}
	if ( false == ccu_free_buffer( gIonDevFD, this->sz_out_data, buf_share_fd_out_data, buf_va_out_data) ) {
		LOG_ERR("ccu_free_buffer fail:buf_va_out_data\n");
	}
    if ( false == ccu_free_buffer( gIonDevFD, this->sz_ae_cust_data, buf_share_fd_ae_cust_data, buf_va_ae_cust_data) ) {
		LOG_ERR("ccu_free_buffer fail:buf_va_ae_cust_data\n");
	}
    if ( false == ccu_free_buffer( gIonDevFD, this->sz_ae_onchange_data, buf_share_fd_ae_onchange_data, buf_va_ae_onchange_data) ) {
        LOG_ERR("ccu_free_buffer fail:buf_va_ae_onchange_data\n");
    }
    //
	if ( pDrvCcu ) {
        //pDrvCcu->shutdown();
	    //pDrvCcu->uninit("CCU");
	    pDrvCcu->destroyInstance();
	}
	//
	if( 0 <= gIonDevFD ) {
		ion_close(gIonDevFD);
	}

    LOG_DBG("CcuIF uninitial done gracefully\n");
    LOG_DBG("-:(%s)\n",this->m_szUsrName);

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CcuIF::start()
{
    void* ptr = NULL;
    LOG_INF(":E\n");

    LOG_INF("-\n");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CcuIF::stop(MBOOL bForce, MBOOL detachUni)
{
    LOG_INF(":E\n");

	MBOOL test = bForce;
	test = detachUni;

    LOG_INF("-\n");
    return  MTRUE;
}


MBOOL CcuIF::suspend()
{
    return true;
}
MBOOL CcuIF::resume()
{
    return true;
}

/*******************************************************************************
*
********************************************************************************/
//initialization
MBOOL CcuIF::ccu_check_sensor_support(WHICH_SENSOR_T sensorDev)
{
    MBOOL rlt;
    
    rlt = pDrvCcu->checkSensorSupport(sensorDev);
    if(rlt == MFALSE)
    {
        LOG_ERR("pDrvCcu->checkSensorSupport, return %x", rlt);
        return MFALSE;
    }

    return MTRUE;
}

int CcuIF::ccu_boot(WHICH_SENSOR_T sensorDev)
{
	MBOOL rlt;

    LOG_DBG("+:\n");

	rlt = pDrvCcu->init((const char*)this->m_szUsrName, sensorDev);
	if(rlt == MFALSE)
    {
		LOG_ERR("pDrvCcu->init(CCU), return %x", rlt);
		return CCUIF_ERR_UNKNOWN;
	}

    LOG_DBG("-:\n");

	return CCUIF_SUCCEED;
}
//

int CcuIF::ccu_sensor_initialize(SENSOR_INFO_IN_T *infoIn, SENSOR_INFO_OUT_T *infoOut)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MUINT32 i2cBufferMva;
    MUINT32 i2cBufferLength = 4096; //i2c driver buffer default 4k
	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SENSOR_INIT, buf_mva_in_data, buf_mva_out_data}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;
    enum CCU_I2C_CHANNEL channel;
    int32_t current_fps;
    int32_t sensorI2cSlaveAddr[3] = {-1 , -1 , -1};

    //======== Set i2c channel of cam type to make kernel driver select correct i2c driver =========
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
    {
        channel = CCU_I2C_CHANNEL_MAINCAM;
    }
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
    {
        channel = CCU_I2C_CHANNEL_SUBCAM;
    }
    else
    {
        LOG_ERR("Sensor Type is not main nor sub \n");
        return CCUIF_ERR_UNKNOWN;
    }

    if (!pDrvCcu->setI2CChannel(channel))
    {
        LOG_ERR("setI2CChannel fail \n");
        return CCUIF_ERR_UNKNOWN;
    }

    //======== Get i2c dma buffer mva =========
    if (!pDrvCcu->getI2CDmaBufferAddr(&i2cBufferMva))
    {
        LOG_ERR("getI2CDmaBufferAddr fail \n");
        return CCUIF_ERR_UNKNOWN;
    }
    LOG_DBG("i2c_get_dma_buffer_addr: %x\n", i2cBufferMva);

    //======== Get current fps =========
    if (!pDrvCcu->getCurrentFps(&current_fps))
    {
        LOG_ERR("getCurrentFps fail \n");
        return CCUIF_ERR_UNKNOWN;
    }
    LOG_DBG("current_fps: %d\n", current_fps);
    if(current_fps == -1)
    {
        LOG_ERR("current_fps is not catched from kd_sensorlist!\n");
        current_fps = 0;
        //return CCUIF_ERR_UNKNOWN;
    }
    //======== Get Sensor I2c Slave Address =========
    if (!pDrvCcu->getSensorI2cSlaveAddr(&sensorI2cSlaveAddr[0]))
    {
        LOG_ERR("getSensorI2cSlaveAddr fail \n");
        return CCUIF_ERR_UNKNOWN;
    }
    LOG_DBG("[CCU If]Main  Sensor slave addr : %d\n", sensorI2cSlaveAddr[0]);
    LOG_DBG("[CCU If]Sub   Sensor slave addr : %d\n", sensorI2cSlaveAddr[1]);
    LOG_DBG("[CCU If]Main2 Sensor slave addr : %d\n", sensorI2cSlaveAddr[2]);
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
    {
        if(sensorI2cSlaveAddr[0] == -1)
            return CCUIF_ERR_UNKNOWN;
    }
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
    {
        if(sensorI2cSlaveAddr[1] == -1)
            return CCUIF_ERR_UNKNOWN;
    }


    //======== Call CCU to initial sensor drv =========
	//Convert to compatible structure
	COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)buf_va_in_data;

	//compatInfoIn->u32SensorId = infoIn->u32SensorId;
    compatInfoIn->eWhichSensor = infoIn->eWhichSensor;
    compatInfoIn->u16FPS = current_fps; //--todo: check where to get the fps
    compatInfoIn->eScenario = infoIn->eScenario;
    compatInfoIn->pu8BufferVAddr = i2cBufferMva;//buffer len is byte size
    compatInfoIn->u16BufferLen = i2cBufferLength;
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
        compatInfoIn->sensorI2cSlaveAddr = sensorI2cSlaveAddr[0];
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
        compatInfoIn->sensorI2cSlaveAddr = sensorI2cSlaveAddr[1];

    //LOG_DBG("ccu sensor in, compatInfoIn->u32SensorId: %x\n", compatInfoIn->u32SensorId);
    LOG_DBG("ccu sensor in, compatInfoIn->eWhichSensor: %x\n", compatInfoIn->eWhichSensor);
    LOG_DBG("ccu sensor in, compatInfoIn->u16FPS: %x\n", compatInfoIn->u16FPS);
    LOG_DBG("ccu sensor in, compatInfoIn->eScenario: %x\n", compatInfoIn->eScenario);
    LOG_DBG("ccu sensor in, compatInfoIn->pu8BufferVAddr: %x\n", compatInfoIn->pu8BufferVAddr);
    LOG_DBG("ccu sensor in, compatInfoIn->u16BufferLen: %x\n", compatInfoIn->u16BufferLen);
    //LOG_DBG("ccu sensor in, compatInfoIn->sensorId : %x\n", compatInfoIn->sensorI2cSlaveAddr);

    LOG_DBG("send&wait ccu sensor init cmd\n");
	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) )
    {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	memcpy(infoOut, buf_va_out_data, sizeof(SENSOR_INFO_OUT_T) );
    if(infoOut->u8SupportedByCCU == 0)
    {
        LOG_ERR("ccu not support sensor\n");
		return CCUIF_ERR_UNKNOWN;
    }

    LOG_DBG("ccu sensor init cmd done\n");
    LOG_DBG("ccu sensor out, u16TransferLen: %d\n", infoOut->u16TransferLen);
    //LOG_DBG("ccu sensor out, u8SlaveAddr: %x\n", infoOut->u8SlaveAddr);
    LOG_DBG("ccu sensor out, u8SupportedByCCU: %x\n", infoOut->u8SupportedByCCU);

    //======== Set i2c controller initial settings: slave addr, transfer length =========
    //if (!pDrvCcu->setI2CMode((unsigned int)(infoOut->u8SlaveAddr), (unsigned int)infoOut->u16TransferLen))
    if (!pDrvCcu->setI2CMode((unsigned int)(compatInfoIn->sensorI2cSlaveAddr), (unsigned int)infoOut->u16TransferLen))
    {
        LOG_ERR("setI2CMode fail \n");
        return CCUIF_ERR_UNKNOWN;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);

	return CCUIF_SUCCEED;
}
//
int CcuIF::ccu_ae_initialize(CCU_AE_INITI_PARAM_T *aeInitParam)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

    COMPAT_CCU_AE_INITI_PARAM_T *compatAeInitParamPtr = (COMPAT_CCU_AE_INITI_PARAM_T *)buf_va_in_data;
    ccu_cmd_st cmd_t = { {MSG_TO_CCU_AE_INIT, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

    compatAeInitParamPtr->control_cfg = aeInitParam->control_cfg;
    copy_ae_init_param_compat(aeInitParam, compatAeInitParamPtr);
    compatAeInitParamPtr->control_cfg.ae_dl_source = AE_STREAM_DL_SRC_AAO;

    LOG_DBG("CCU_AE_INITI_PARAM_T size:%x\n", sizeof(CCU_AE_INITI_PARAM_T));
    LOG_DBG("COMPAT_CCU_AE_INITI_PARAM_T size:%x\n", sizeof(COMPAT_CCU_AE_INITI_PARAM_T));
    LOG_DBG("CCU_AE_CONTROL_CFG_T size:%x\n", sizeof(CCU_AE_CONTROL_CFG_T));
    LOG_DBG("AE_CORE_INIT size:%x\n", sizeof(AE_CORE_INIT));
    LOG_DBG("COMPAT_AE_CORE_INIT size:%x\n", sizeof(COMPAT_AE_CORE_INIT));


	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);

	return CCUIF_SUCCEED;
}


int CcuIF::copy_ae_init_param_compat(CCU_AE_INITI_PARAM_T *aeInitParam, COMPAT_CCU_AE_INITI_PARAM_T *compatAeInitParam)
{
    MUINT32 aeCustParamSz = 0;
    MUINT8 *aeCustBufVa;
    MUINT32 aeCustBufMva;

    compatAeInitParam->algo_init_param.EndBankIdx = aeInitParam->algo_init_param.EndBankIdx;
    compatAeInitParam->algo_init_param.m_i4AEMaxBlockWidth = aeInitParam->algo_init_param.m_i4AEMaxBlockWidth;
    compatAeInitParam->algo_init_param.m_i4AEMaxBlockHeight = aeInitParam->algo_init_param.m_i4AEMaxBlockHeight;
    compatAeInitParam->algo_init_param.uTgBlockNumX = aeInitParam->algo_init_param.uTgBlockNumX;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    compatAeInitParam->algo_init_param.uTgBlockNumY = aeInitParam->algo_init_param.uTgBlockNumY;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    compatAeInitParam->algo_init_param.m_u4IndexMax = aeInitParam->algo_init_param.m_u4IndexMax;
    compatAeInitParam->algo_init_param.m_u4IndexMin = aeInitParam->algo_init_param.m_u4IndexMin;
    compatAeInitParam->algo_init_param.bSpeedupEscOverExp = aeInitParam->algo_init_param.bSpeedupEscOverExp;   //m_AeTuningParam->bSpeedupEscOverExp
    compatAeInitParam->algo_init_param.bSpeedupEscUnderExp = aeInitParam->algo_init_param.bSpeedupEscUnderExp;  //m_AeTuningParam->bSpeedupEscUnderExp
    compatAeInitParam->algo_init_param.pAETgTuningParam = aeInitParam->algo_init_param.pAETgTuningParam; //m_AeTuningParam->m_AeTuningParam

    memcpy(compatAeInitParam->algo_init_param.pTgWeightTbl, aeInitParam->algo_init_param.pTgWeightTbl, 9*12*sizeof(MUINT32));

    //for pointer types, copy data onto mva buffer
    aeCustBufVa = (MUINT8 *)buf_va_ae_cust_data;
    aeCustBufMva = buf_mva_ae_cust_data;

    LOG_DBG("pTgEVCompJumpRatioOverExp mva:%x\n", aeCustBufMva);
    aeCustParamSz = LIB3A_AE_EV_COMP_MAX*sizeof(MINT32);
    compatAeInitParam->algo_init_param.pTgEVCompJumpRatioOverExp = aeCustBufMva; //m_AeTuningParam->pTgEVCompJumpRatioOverExp
    memcpy(aeCustBufVa, aeInitParam->algo_init_param.pTgEVCompJumpRatioOverExp, aeCustParamSz);
    aeCustBufVa += aeCustParamSz;
    aeCustBufMva += aeCustParamSz;

    LOG_DBG("pTgEVCompJumpRatioUnderExp mva:%x\n", aeCustBufMva);
    aeCustParamSz = LIB3A_AE_EV_COMP_MAX*sizeof(MINT32);
    compatAeInitParam->algo_init_param.pTgEVCompJumpRatioUnderExp = aeCustBufMva; //m_AeTuningParam->pTgEVCompJumpRatioUnderExp
    memcpy(aeCustBufVa, aeInitParam->algo_init_param.pTgEVCompJumpRatioUnderExp, aeCustParamSz);
    aeCustBufVa += aeCustParamSz;
    aeCustBufMva += aeCustParamSz;

    LOG_DBG("LumLog2x1000 mva:%x\n", aeCustBufMva);
    aeCustParamSz = aeInitParam->algo_init_param.SzLumLog2x1000;
    compatAeInitParam->algo_init_param.LumLog2x1000 = aeCustBufMva;
    memcpy(aeCustBufVa, aeInitParam->algo_init_param.LumLog2x1000, aeCustParamSz);
    aeCustBufVa += aeCustParamSz;
    aeCustBufMva += aeCustParamSz;

    compatAeInitParam->algo_init_param.SzLumLog2x1000 = aeInitParam->algo_init_param.SzLumLog2x1000;

    LOG_DBG("pCurrentTable mva:%x\n", aeCustBufMva);
    aeCustParamSz = sizeof(strEvPline);
    compatAeInitParam->algo_init_param.pCurrentTable = aeCustBufMva; //m_pPreviewTableCurrent->pCurrentTable
    memcpy(aeCustBufVa, aeInitParam->algo_init_param.pCurrentTable, aeCustParamSz);
    aeCustBufVa += aeCustParamSz;
    aeCustBufMva += aeCustParamSz;

    compatAeInitParam->algo_init_param.i4MaxBV = aeInitParam->algo_init_param.i4MaxBV;       //m_pPreviewTableCurrent->i4MaxBV or instace in g_AE_PreviewAutoTable,custom\mt6757\hal\imgsensor\s5k2p8_mipi_raw\camera_AE_PLineTable_s5k2p8raw.h
    compatAeInitParam->algo_init_param.i4BVOffset = aeInitParam->algo_init_param.i4BVOffset;    //m_AeCCTConfig.i4BVOffset
    compatAeInitParam->algo_init_param.u4MiniISOGain = aeInitParam->algo_init_param.u4MiniISOGain; //m_pAENVRAM->rDevicesInfo.u4MiniISOGain
    compatAeInitParam->algo_init_param.iMiniBVValue = aeInitParam->algo_init_param.iMiniBVValue;  //m_AeTuningParam->strAEParasetting.iMiniBVValue
    compatAeInitParam->algo_init_param.EVBase = aeInitParam->algo_init_param.EVBase;
    compatAeInitParam->algo_init_param.StatisticStable = aeInitParam->algo_init_param.StatisticStable;

    compatAeInitParam->algo_init_param.m_rIspIsoEnv.u2Length = aeInitParam->algo_init_param.m_rIspIsoEnv.u2Length;
    //--todo: hardcode in ccu ae algo for now, fillup it after
    compatAeInitParam->algo_init_param.m_rIspIsoEnv.IDX_Partition = 0;
    //--todo: hardcode in ccu ae algo for now, fillup it after
    compatAeInitParam->algo_init_param.m_rIspIsoEnv.IDX_Part_Middle = 0;

    memcpy(compatAeInitParam->algo_init_param.OBC_Table, aeInitParam->algo_init_param.OBC_Table, 4*sizeof(CCU_ISP_NVRAM_OBC_T));
    memcpy(compatAeInitParam->algo_init_param.OBC_ISO_IDX_Range, aeInitParam->algo_init_param.OBC_ISO_IDX_Range, 5*sizeof(MINT32));

    return 0;
}

//--todo, replace current command sending codes with this
int CcuIF::send_ccu_command(ccu_msg_id msgId)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    LOG_DBG("sending ccu command :%d\n", msgId);
    ccu_cmd_st cmd_t = { {msgId, buf_mva_in_data, buf_mva_out_data}, CCU_ENG_STATUS_INVALID };
    ccu_cmd_st cmd_r;

    if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) )
    {
        LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
        return CCUIF_ERR_UNKNOWN;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCUIF_SUCCEED;
}


int CcuIF::ccu_ae_start()
{
	return send_ccu_command(MSG_TO_CCU_AE_START);
}


//per-frame
int CcuIF::ccu_ae_set_ap_ready()
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_AP_AE_READY, buf_mva_in_data, buf_mva_out_data}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) )
    {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;
}
//
int CcuIF::ccu_ae_set_frame_sync_data(ccu_ae_frame_sync_data *frameSyncdata)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	memcpy( buf_va_in_data, frameSyncdata, sizeof(ccu_ae_frame_sync_data) );

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;
}
//
int CcuIF::ccu_ae_set_onchange_data(ccu_ae_onchange_data *onchangeData)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    COMPAT_ccu_ae_onchange_data *compatOnchangeDataPtr = (COMPAT_ccu_ae_onchange_data *)buf_va_in_data;
    copy_onchange_data_compat(onchangeData, compatOnchangeDataPtr);

    return send_ccu_command(MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA);
}
int CcuIF::copy_onchange_data_compat(ccu_ae_onchange_data *onchangeData, COMPAT_ccu_ae_onchange_data *compatOnchangeData)
{
    int nonPtrMemberSize = offsetof(ccu_ae_onchange_data, pCurrentTable);

    memcpy(compatOnchangeData, onchangeData, nonPtrMemberSize);

    LOG_DBG("onchangeData mva:%x\n", buf_mva_ae_onchange_data);
    compatOnchangeData->pCurrentTable = buf_mva_ae_onchange_data;
    memcpy(buf_va_ae_onchange_data, onchangeData->pCurrentTable, sizeof(strEvPline));

    return 0;
}
//
int CcuIF::ccu_ae_get_output(AE_CORE_MAIN_OUT *output)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_GET_CCU_OUTPUT, 0, buf_mva_out_data}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	memcpy( output, buf_va_out_data, sizeof(AE_CORE_MAIN_OUT) );

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;

}
//
int CcuIF::ccu_ae_set_ap_output(AE_CORE_CTRL_CPU_EXP_INFO *apAeOuput)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_AP_AE_OUTPUT, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	memcpy( buf_va_in_data, apAeOuput, sizeof(AE_CORE_CTRL_CPU_EXP_INFO) );

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;

}

int CcuIF::ccu_ae_set_max_framerate( MUINT16 framerate, kal_bool min_framelength_en)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	/*ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_MAX_FRAMERATE, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	memcpy( buf_va_in_data, &framerate, sizeof(MUINT16) );

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}*/
	memcpy( buf_va_in_data, &framerate, sizeof(MUINT16) );

    int ret = send_ccu_command(MSG_TO_CCU_SET_MAX_FRAMERATE);

	LOG_DBG("-:%s\n",__FUNCTION__);

    return ret;
}


//on-change
int CcuIF::ccu_ae_set_ev_comp(LIB3A_AE_EVCOMP_T *apAeEv)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_AE_EV, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	memcpy( buf_va_in_data, apAeEv, sizeof(LIB3A_AE_EVCOMP_T) );

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;

}
//
int CcuIF::ccu_ae_set_skip_algo(MBOOL doSkip)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_SKIP_ALGO, (MUINT32)doSkip, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;

}
//
int CcuIF::ccu_ae_set_roi(ccu_ae_roi *apAeRoi)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	ccu_cmd_st cmd_t = { {MSG_TO_CCU_SET_AE_ROI, buf_mva_in_data, 0}, CCU_ENG_STATUS_INVALID };
	ccu_cmd_st cmd_r;

	memcpy( buf_va_in_data, apAeRoi, sizeof(ccu_ae_roi) );

	if ( !pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) ) {
		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
		return CCUIF_ERR_UNKNOWN;
	}

	LOG_DBG("-:%s\n",__FUNCTION__);
	return CCUIF_SUCCEED;

}

//un-initialization
int CcuIF::ccu_shutdown()
{
	MBOOL rlt;

    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    pDrvCcu->shutdown();
    if ( MFALSE == rlt )
    {
        LOG_ERR("pDrvCcu->init(CCU)return %b", rlt);
		return CCUIF_ERR_UNKNOWN;
    }

	rlt = pDrvCcu->uninit("CCU");
	if ( MFALSE == rlt ) {
		LOG_ERR("pDrvCcu->init(CCU)return %b", rlt);
		return CCUIF_ERR_UNKNOWN;
	}
	//
	//pDrvCcu->destroyInstance();


    LOG_DBG_MUST("-:%s\n",__FUNCTION__);

	return CCUIF_SUCCEED;
}

//memory/buffer
MBOOL CcuIF::ccu_get_buffer(int drv_h, int len, int *buf_share_fd, char **buf_va)
{
    ion_user_handle_t buf_handle;

    LOG_DBG("+:%s\n",__FUNCTION__);

    // allocate ion buffer handle
    if(ion_alloc_mm(drv_h, (size_t)len, 0, 0, &buf_handle))        // no alignment, non-cache
    {
        LOG_WRN("fail to get ion buffer handle (drv_h=0x%x, len=%d)", drv_h, len);
        return false;
    }
    // get ion buffer share handle
    if(ion_share(drv_h, buf_handle, buf_share_fd))
    {
        LOG_WRN("fail to get ion buffer share handle");
        if(ion_free(drv_h,buf_handle))
            LOG_WRN("ion free fail");
        return false;
    }
    // get buffer virtual address
    *buf_va = ( char *)ion_mmap(drv_h, NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, *buf_share_fd, 0);
    if(*buf_va == NULL) {
        LOG_WRN("fail to get buffer virtual address");
    }
    LOG_DBG("alloc ion: ion_buf_handle %d, share_fd %d, va: %p", buf_handle, *buf_share_fd, *buf_va);
    //
    LOG_DBG("-:%s\n",__FUNCTION__);

    return (*buf_va != NULL)?MTRUE:MFALSE;

}
MBOOL CcuIF::ccu_free_buffer(int drv_h, int len,int buf_share_fd, char *buf_va)
{
    ion_user_handle_t buf_handle;

    LOG_DBG("+:%s\n",__FUNCTION__);

    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        LOG_WRN("fail to get import share buffer fd");
        return false;
    }
    LOG_VRB("import ion: ion_buf_handle %d, share_fd %d", buf_handle, buf_share_fd);
    // 2. free for IMPORT ref cnt
    if(ion_free(drv_h, buf_handle))
    {
        LOG_WRN("fail to free ion buffer (free ion_import ref cnt)");
        return false;
    }
    // 3. unmap virtual memory address
    if(ion_munmap(drv_h, (void *)buf_va, (size_t)len))
    {
        LOG_WRN("fail to get unmap virtual memory");
        return false;
    }
    // 4. close share buffer fd
    if(ion_share_close(drv_h, buf_share_fd))
    {
        LOG_WRN("fail to close share buffer fd");
        return false;
    }
    // 5. pair of ion_alloc_mm
    if(ion_free(drv_h, buf_handle))
    {
        LOG_WRN("fail to free ion buffer (free ion_alloc_mm ref cnt)");
        return false;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;

}

MBOOL CcuIF::mmapMVA( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva )
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_DBG("+:%s\n",__FUNCTION__);

    //a. get handle from IonBufFd and increase handle ref count
    if(ion_import(gIonDevFD, buf_share_fd, p_ion_handle))
    {
        LOG_ERR("ion_import fail, ion_handle(0x%x)", *p_ion_handle);
        return false;
    }
    LOG_VRB("ion_import: share_fd %d, ion_handle %d", buf_share_fd, *p_ion_handle);
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = *p_ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_CAM_CCUG;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start  = 0x10000000;
    mm_data.config_buffer_param.reserve_iova_end    = 0xFFFFFFFF;
    err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        LOG_ERR("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        LOG_ERR("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
    }
    //c. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = *p_ion_handle;
    sys_data.get_phys_param.phy_addr = (M4U_PORT_CAM_CCUG<<24) | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        LOG_ERR("ion_custom_ioctl get_phys_param failed!");
        return false;
    }
	//
	*mva = (unsigned int)sys_data.get_phys_param.phy_addr;

    //req_buf->plane[plane_idx].ion_handle = ion_handle;
    //req_buf->plane[plane_idx].mva        = sys_data.get_phys_param.phy_addr;
    //LOG_VRB("get_phys: buffer[%d], plane[%d], mva 0x%x", buf_idx, plane_idx, buf_n->planes[plane_idx].ptr);

    LOG_DBG("\n");
    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;
}

MBOOL CcuIF::munmapMVA( ion_user_handle_t ion_handle )
{
       LOG_DBG("+:%s\n",__FUNCTION__);

        // decrease handle ref count
        if(ion_free(gIonDevFD, ion_handle))
        {
            LOG_ERR("ion_free fail");
            return false;
        }
        LOG_VRB("ion_free: ion_handle %d", ion_handle);

        LOG_DBG("-:%s\n",__FUNCTION__);
        return true;
}

MUINT32 CcuIF::readInfoReg(MUINT32 regNo)
{
    return pDrvCcu->readInfoReg(regNo);
}



/*******************************************************************************
*
********************************************************************************/

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCcuDrv
};  //namespace NSCcuIf

