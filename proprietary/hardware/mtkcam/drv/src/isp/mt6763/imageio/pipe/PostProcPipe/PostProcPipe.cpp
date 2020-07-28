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
#define LOG_TAG "PostProcPipe"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PostProcPipe.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <ispio_utility.h>
//
#include <cutils/properties.h>  // For property_get().
#include <utils/Trace.h> //for systrace
//#include <mtkcam/featureio/eis_type.h>

/*******************************************************************************
*
********************************************************************************/

#define FORCE_EN_DIFFERENR_VIEW_TEST 0

namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(PostProc);
// Clear previous define, use our own define.
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (PostProc_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (PostProc_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (PostProc_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (PostProc_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (PostProc_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (PostProc_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FM_ACC_RES_ADDR 0x15022f48

static unsigned int addrList = FM_ACC_RES_ADDR;
//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

MINT32 u4PortID[DMA_PORT_TYPE_NUM][DMA_OUT_PORT_NUM] = {/*0*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_JPEGO, (MINT32)EPortIndex_FEO, (MINT32)EPortIndex_VIPI},
                                                        /*1*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*2*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*3*/{(MINT32)EPortIndex_WDMAO, (MINT32)-1, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*4*/{(MINT32)EPortIndex_WROTO, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*5*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*6*/{(MINT32)EPortIndex_IMG2O, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*7*/{(MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*8*/{(MINT32)EPortIndex_FEO,   (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        };

 Scen_Map_CropPathInfo_STRUCT mCropPathInfo[]
	=	{
    //eDrvScenario_CC
    {eDrvScenario_P2A,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_P2A,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_P2A,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_P2A,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_VSS
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_CC_MFB_Blending
    {eDrvScenario_MFB_Blending, (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    //eDrvScenario_CC_MFB_Mixing
    {eDrvScenario_MFB_Mixing,   (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    {eDrvScenario_MFB_Mixing,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {eDrvScenario_MFB_Mixing,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_CC_vFB_FB
    {eDrvScenario_VFB_FB,       (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {eDrvScenario_VFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {eDrvScenario_VFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_Bokeh
    {eDrvScenario_P2B_Bokeh,       (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {eDrvScenario_P2B_Bokeh,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {eDrvScenario_P2B_Bokeh,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //temp
    {eDrvScenario_FE,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    {eDrvScenario_FE,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_FE,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //temp
    {eDrvScenario_FM,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    //
    {eDrvScenario_Color_Effect,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_Color_Effect,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_Color_Effect,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_Color_Effect,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //Denoise
    {eDrvScenario_DeNoise,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_DeNoise,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_DeNoise,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_DeNoise,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
};

#define SENSOR_DEV_NUM 7
/* NONE, MAIN, SUB, ATV, MAIN2, MAIN3D, SUB2 */

//TODO, need refine table
E_ISP_DIP_CQ const geDevScenCQMapping[eScenarioID_NUM][SENSOR_DEV_NUM]=
{ //none, main,sub, pip, main2, main3d, sub2 (ignore none/pip/main3d, assume main2 as auxsensor1)
    {ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE4,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//p2a
    {ISP_DRV_DIP_CQ_THRE1,ISP_DRV_DIP_CQ_THRE1,ISP_DRV_DIP_CQ_THRE5,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//p2b
    {ISP_DRV_DIP_CQ_THRE2,ISP_DRV_DIP_CQ_THRE2,ISP_DRV_DIP_CQ_THRE6,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE9,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE9},//blending
    {ISP_DRV_DIP_CQ_THRE3,ISP_DRV_DIP_CQ_THRE3,ISP_DRV_DIP_CQ_THRE7,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE10,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE10},//mixing
    {ISP_DRV_DIP_CQ_THRE11,ISP_DRV_DIP_CQ_THRE11,ISP_DRV_DIP_CQ_THRE11,\
        ISP_DRV_DIP_CQ_THRE11,ISP_DRV_DIP_CQ_THRE11,ISP_DRV_DIP_CQ_THRE11, ISP_DRV_DIP_CQ_THRE11},//vss
    {ISP_DRV_DIP_CQ_THRE1,ISP_DRV_DIP_CQ_THRE1,ISP_DRV_DIP_CQ_THRE5,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//p2b-bokeh
    {ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE4,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//fe
    {ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE4,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//fm
    {ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE4,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE8,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE8},//color effect
    {ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE9,ISP_DRV_DIP_CQ_THRE10,\
        ISP_DRV_DIP_CQ_THRE0,ISP_DRV_DIP_CQ_THRE10,ISP_DRV_DIP_CQ_THRE0, ISP_DRV_DIP_CQ_THRE10}//denoise
};

MUINT32 ConvertPortIndexToCapInfoIdx(
        MUINT32 portIDIdx
)
{
	MUINT32 CapInfoIdx;

    switch(portIDIdx){
        case EPortIndex_IMGO:
			CapInfoIdx = 0;
			break;
        case EPortIndex_IMG2O:
			CapInfoIdx = 1;
			break;
        case EPortIndex_IMG3O:
        case EPortIndex_IMG3BO:
        case EPortIndex_IMG3CO:
			CapInfoIdx = 2;
			break;
        case EPortIndex_WDMAO:
			CapInfoIdx = 3;
			break;
        case EPortIndex_WROTO:
			CapInfoIdx = 4;
			break;
        case EPortIndex_JPEGO:
			CapInfoIdx = 5;
            break;
        default:
            PIPE_ERR("[Error]Not support this PortIndex(%d) mapping",portIDIdx);
            CapInfoIdx = 0;
            break;
    };
	return CapInfoIdx;

}

////////////////////////////////////////////////////////////////////////////////

IPostProcPipe* IPostProcPipe::createInstance(void)
{
    PostProcPipe* pPipeImp = new PostProcPipe();
    if  ( ! pPipeImp )
    {
        PIPE_ERR("[IPostProcPipe] fail to new PostProcPipe");
        return  NULL;
    }
    return  pPipeImp;
}


/*******************************************************************************
*
********************************************************************************/
MVOID IPostProcPipe:: destroyInstance(void)
{
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
PostProcPipe::
PostProcPipe()
    : m_pIspDrvShell(NULL),
      jpg_WorkingMem_memId(-1),
      jpg_WorkingMem_Size(0),
      pJpgConfigVa(NULL),
      m_pipePass(EPipePass_Dip_Th0),
      m_pass2_CQ(DIP_ISP_CQ_NONE),
      m_isImgPlaneByImgi(MFALSE),
      m_Nr3dEn(MFALSE),
      m_Nr3dDmaSel(MFALSE),
      m_CrzEn(MFALSE),
      m_SeeeEn(MFALSE),
      m_Img3oCropInfo(NULL)      
{
    //
    DBG_LOG_CONFIG(imageio, PostProc);
    //DBG_LOG_CONFIG(imageio, pipe);
    //
    //std::strcpy((char*)m_szUsrName,(char const*)szUsrName);
    //PIPE_INF("usr(%s)",m_szUsrName);
    this->m_vBufImgi.resize(1);
    this->m_vBufVipi.resize(1);
    this->m_vBufVip2i.resize(1);
    this->m_vBufDispo.resize(1);
    this->m_vBufVido.resize(1);

    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();

    this->m_dipCmdQMgr = DipCmdQMgr::createInstance();
}

PostProcPipe::
~PostProcPipe()
{
    /*** release isp driver ***/
    m_pIspDrvShell->destroyInstance();

    if( this->m_dipCmdQMgr)
    {
        this->m_dipCmdQMgr->destroyInstance();
        this->m_dipCmdQMgr=NULL;
    }

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
init(char const* szCallerName)
{
    MBOOL ret=MTRUE;
    PIPE_INF("+, userName(%s), szCallerName(%s)",this->m_szUsrName, szCallerName);
    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init("PostProcPipe");
    }

    if(m_dipCmdQMgr)
    {
        ret=m_dipCmdQMgr->init();
    }
    //
    PIPE_DBG("-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
uninit(char const* szCallerName)
{
    PIPE_INF("+, userName(%s), szCallerName(%s)",this->m_szUsrName, szCallerName);
    MBOOL ret=MTRUE;
    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->uninit("PostProcPipe");
    }
    //
    if(m_dipCmdQMgr)
    {
        m_dipCmdQMgr->uninit();
    }
    PIPE_DBG("-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
start(void* pParam)
{
    int ret = MTRUE;

    ret= ispDipPipe.enable(pParam);

EXIT:
    if(ret<0)
    {
    	PIPE_ERR("start ERR");
    	return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
stop(void* pParam)
{
    pParam;
    PIPE_WRN("+ we do not need to call PostProc stop");

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup)
{
	MUINT32 idx,j;
//	MUINT32 numPortId;
//	ESoftwareScenario scenTmp = swScen;
	if (eScenarioID_NUM <= drvScen)
	{
		PIPE_ERR("getCropFunctionEnable drv scen error:(%d)",drvScen);
		return MTRUE;
	}

	PIPE_DBG("cropPath Array Size = (%lu), Scen_Map_CropPathInfo_STRUCT Size =(%lu), scenTmp:(%d)", (unsigned long)sizeof(mCropPathInfo), (unsigned long)sizeof(Scen_Map_CropPathInfo_STRUCT), drvScen);
	for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
	{
		if (drvScen == mCropPathInfo[idx].u4DrvScenId)
		{
//			numPortId = mCropPathInfo[idx].u4NumPortId;
			CropGroup = mCropPathInfo[idx].u4CropGroup;

			for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
			{
			    if (portID == mCropPathInfo[idx].u4PortID[j])
			    {
			    	return MTRUE;
			    }
			}
		}
	}
	return MFALSE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getCrzPosition(
    EDrvScenario drvScen,
    MBOOL &isSl2eAheadCrz,
    MBOOL &isLceAheadCrz
)
{
    MBOOL ret = MTRUE;

    switch(drvScen){
        case eDrvScenario_P2A:
        case eDrvScenario_MFB_Blending:
        case eDrvScenario_MFB_Mixing:
        case eDrvScenario_VSS:
        case eDrvScenario_FE:
        case eDrvScenario_FM:
        case eDrvScenario_DeNoise:
            isSl2eAheadCrz = MTRUE;
            isLceAheadCrz = MTRUE;
            break;
        case eDrvScenario_VFB_FB:
        case eDrvScenario_P2B_Bokeh:
            isSl2eAheadCrz = MFALSE;
            isLceAheadCrz = MFALSE;
            break;
        default:
            PIPE_ERR("[Error]Not support this drvScen(%d)",drvScen);
            ret = MFALSE;
            break;
    };
    PIPE_DBG("drvScen(%d),isSl2eAheadCrz(%d),isLceAheadCrz(%d)",drvScen,isSl2eAheadCrz,isLceAheadCrz);

    return ret;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum)
{
    MUINT32 size = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    MUINT32 i, num;

    num = 0;
    for(i=0;i<size;i++){
        if(mCropPathInfo[i].u4DrvScenId == drvScen)
            num++;
    }
    pathNum = num;

	PIPE_DBG("drvScen(%d),pathNum(%d)", drvScen, pathNum);
	return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths)
{
    //temp
    //ESoftwareScenario swScen=eSoftwareScenario_Main_Normal_Stream;

	MUINT32 idx,j,i;
	MUINT32 portidx = 0;
    MUINT32 u4NumGroup;
	//ESoftwareScenario scenTmp = swScen;
	if (eScenarioID_NUM <= drvScen)
	{
		PIPE_ERR("query scenario info drv scen error:(%d)",drvScen);
		return MTRUE;
	}

	PIPE_DBG("cropPath Array Size = (%lu), Scen_Map_CropPathInfo_STRUCT Size =(%lu), drvScen:(%d)", (unsigned long)sizeof(mCropPathInfo), (unsigned long)sizeof(Scen_Map_CropPathInfo_STRUCT), drvScen);
	for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
	{
		if (drvScen == mCropPathInfo[idx].u4DrvScenId)
		{
			if (portidx >= vCropPaths.size())
			{
				PIPE_ERR("query scenario info portidx error:(%d)",portidx);
				return MFALSE;
			}
            if(vCropPaths[portidx].PortIdxVec.size()>0)
			{
			    vCropPaths[portidx].PortIdxVec.clear();
            }
            u4NumGroup = 0;
            for(i=ECropGroupShiftIdx_1;i<ECropGroupShiftIdx_Num;i++){
               if((1<<i)&mCropPathInfo[idx].u4CropGroup)
                   u4NumGroup++;
            }
			vCropPaths[portidx].u4NumGroup = u4NumGroup;
            vCropPaths[portidx].u4CropGroup = mCropPathInfo[idx].u4CropGroup;
			//pCropPathInfo->PortIdxVec.resize(numPortId);
			for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
			{
                if(mCropPathInfo[idx].u4PortID[j])
                {
                    vCropPaths[portidx].PortIdxVec.push_back(mCropPathInfo[idx].u4PortID[j]);
                }
                else
                {
                    break;
                }
			}
			//vCropPaths.push_back(pathInfo);
			//vCropPaths.at(portidx) = pCropPathInfo;
			portidx++;
		}
	}
	return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeMdpFrameEnd( MINT32 const eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx)
{
    MBOOL ret = MTRUE;
    //
    if ( 0 != this->ispBufCtrl.dequeueMdpFrameEnd(eDrvSce,dequeCq, dequeBurstQIdx,dequeDupCqIdx, moduleIdx) ) {
        PIPE_ERR("ERROR:dequeMdpFrameEnd");
        ret = MFALSE;
    }

    //
    return  ret;
}



/*******************************************************************************
*
********************************************************************************/
MUINT32
PostProcPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, EDrvScenario eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    EIspRetStatus ret;
    rQBufInfo;

    PIPE_DBG("dequeInBuf:tid(%d),dequeCq(%d),PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(),dequeCq, portID.type, portID.index, portID.inout, u4TimeoutMs);

    //
    PIPE_DBG("dQDBGin path(%d)  cq=(%d, %d, %d, %d)", eDrvSce, dequeCq,dequeDupCqIdx,dequeBurstQIdx,moduleIdx);
    this->ispBufCtrl.m_BufdipTh = dequeCq;
    this->ispBufCtrl.m_BufdupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.m_BufburstQueIdx = dequeBurstQIdx;
    this->ispBufCtrl.m_BufdrvScenario = eDrvSce;
    this->ispBufCtrl.m_BufmoduleIdx = moduleIdx;

    ret = this->ispBufCtrl.dequeueHwBuf( portID.index);

    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }

EXIT:
    //
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
PostProcPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, EDrvScenario eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    int idex=0;
    EIspRetStatus ret;
    rQBufInfo;
    //
    ISP_TRACE_CALL();

    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    PIPE_DBG("dQDBGout path(%d)  cq=(%d, %d, %d, %d)", eDrvSce, dequeCq,dequeDupCqIdx,dequeBurstQIdx,moduleIdx);
    this->ispBufCtrl.m_BufdipTh = dequeCq;
    this->ispBufCtrl.m_BufdupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.m_BufburstQueIdx = dequeBurstQIdx;
    this->ispBufCtrl.m_BufdrvScenario = eDrvSce;
    this->ispBufCtrl.m_BufmoduleIdx = moduleIdx;
    
    ret = this->ispBufCtrl.dequeueHwBuf( portID.index);

    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }
    //
EXIT:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
PostProcPipe::
dequeBuf(MBOOL& isVencContained, QTimeStampBufInfo& rQBufInfo, EDrvScenario eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    int idex=0;
    EIspRetStatus ret;
    rQBufInfo;
    //
    ISP_TRACE_CALL();

    //
    PIPE_DBG("dQDBGout path(%d)  cq=(%d, %d, %d, %d), timeoutMs(%d)", eDrvSce, dequeCq,dequeDupCqIdx,dequeBurstQIdx,moduleIdx,u4TimeoutMs);
    this->ispBufCtrl.m_BufdipTh = dequeCq;
    this->ispBufCtrl.m_BufdupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.m_BufburstQueIdx = dequeBurstQIdx;
    this->ispBufCtrl.m_BufdrvScenario = eDrvSce;
    this->ispBufCtrl.m_BufmoduleIdx = moduleIdx;

    ret = this->ispBufCtrl.dequeHwBuf(isVencContained);

    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }
    //
EXIT:
    return  ret;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
HandleExtraCommand(PipePackageInfo *pPipePackageInfo)
{
    //Parsing Extra Parameter.
    MBOOL ret = MTRUE;
    NSCam::NSIoPipe::ExtraParam CmdInfo;
    android::Vector<NSCam::NSIoPipe::ExtraParam>::const_iterator iter;
    dip_x_reg_t* pTuningIspReg = NULL;
    NSCam::NSIoPipe::FEInfo* pFEInfo;
    NSCam::NSIoPipe::FMInfo* pFMInfo;
    NSCam::NSIoPipe::PQParam* pPQParam;
    ispMdpPipe.WDMAPQParam = NULL;
    ispMdpPipe.WROTPQParam = NULL;
    m_Img3oCropInfo = NULL;

    if(pPipePackageInfo->pExtraParam == NULL)
    {
        PIPE_WRN("empty extraParam");
        return ret;
    }

    for(iter = pPipePackageInfo->pExtraParam->begin();iter<pPipePackageInfo->pExtraParam->end();iter++)
    {
        CmdInfo = (*iter);
        switch (CmdInfo.CmdIdx)
        {
            case NSCam::NSIoPipe::EPIPE_FE_INFO_CMD:
                pFEInfo = (NSCam::NSIoPipe::FEInfo*)CmdInfo.moduleStruct;
                if(pPipePackageInfo->pTuningQue != NULL){
                    pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
                    //Update FE Register
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_DSCR_SBIT     = pFEInfo->mFEDSCR_SBIT;
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_TH_C          = pFEInfo->mFETH_C;
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_TH_G          = pFEInfo->mFETH_G;
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_FLT_EN        = pFEInfo->mFEFLT_EN;
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_PARAM         = pFEInfo->mFEPARAM;
                    pTuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE          = pFEInfo->mFEMODE;
                    pTuningIspReg->DIP_X_FE_IDX_CTRL.Bits.FE_YIDX       = pFEInfo->mFEYIDX;
                    pTuningIspReg->DIP_X_FE_IDX_CTRL.Bits.FE_XIDX       = pFEInfo->mFEXIDX;
                    pTuningIspReg->DIP_X_FE_CROP_CTRL1.Bits.FE_START_X  = pFEInfo->mFESTART_X;
                    pTuningIspReg->DIP_X_FE_CROP_CTRL1.Bits.FE_START_Y  = pFEInfo->mFESTART_Y;
                    pTuningIspReg->DIP_X_FE_CROP_CTRL2.Bits.FE_IN_HT    = pFEInfo->mFEIN_HT;
                    pTuningIspReg->DIP_X_FE_CROP_CTRL2.Bits.FE_IN_WD    = pFEInfo->mFEIN_WD;
                }
                break;
            case NSCam::NSIoPipe::EPIPE_FM_INFO_CMD:
                pFMInfo = (NSCam::NSIoPipe::FMInfo*)CmdInfo.moduleStruct;
                if(pPipePackageInfo->pTuningQue != NULL){
                    pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
                    //Update FM Register
                    pTuningIspReg->DIP_X_CTL_YUV2_EN.Bits.FM_EN       = 0x1;
                    pTuningIspReg->DIP_X_FM_SIZE.Bits.FM_HEIGHT       = pFMInfo->mFMHEIGHT;
                    pTuningIspReg->DIP_X_FM_SIZE.Bits.FM_WIDTH        = pFMInfo->mFMWIDTH;
                    pTuningIspReg->DIP_X_FM_SIZE.Bits.FM_SR_TYPE      = pFMInfo->mFMSR_TYPE;
                    pTuningIspReg->DIP_X_FM_SIZE.Bits.FM_OFFSET_X     = pFMInfo->mFMOFFSET_X;
                    pTuningIspReg->DIP_X_FM_SIZE.Bits.FM_OFFSET_Y     = pFMInfo->mFMOFFSET_Y;
                    pTuningIspReg->DIP_X_FM_TH_CON0.Bits.FM_RES_TH    = pFMInfo->mFMRES_TH;
                    pTuningIspReg->DIP_X_FM_TH_CON0.Bits.FM_SAD_TH    = pFMInfo->mFMSAD_TH;
                    pTuningIspReg->DIP_X_FM_TH_CON0.Bits.FM_MIN_RATIO = pFMInfo->mFMMIN_RATIO;
                }
                break;
            case NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD:
                break;
            case NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
                pPQParam = (NSCam::NSIoPipe::PQParam*)CmdInfo.moduleStruct;
                ispMdpPipe.WDMAPQParam = (MVOID *)pPQParam->WDMAPQParam;
                ispMdpPipe.WROTPQParam = (MVOID *)pPQParam->WROTPQParam;
                break;
            case NSCam::NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:
                m_Img3oCropInfo = (NSCam::NSIoPipe::CrspInfo*)CmdInfo.moduleStruct;
                break;


#if 0
            case NSCam::NSIoPipe::EPIPE_VSDOFPQ_CMD:
                ispMdpPipe.VSDOFPQParam = (MVOID *)CmdInfo.moduleStruct;
                break;
            case NSCam::NSIoPipe::EPIPE_DC_CMD:
                ispMdpPipe.DCParam = (MVOID *)CmdInfo.moduleStruct;
                break;
            case NSCam::NSIoPipe::EPIPE_CZ_CMD:
                ispMdpPipe.CZParam = (MVOID *)CmdInfo.moduleStruct;
                break;
#endif
            default:
                PIPE_WRN("receive extra cmd(%d)\n",CmdInfo.CmdIdx);
                break;
        }
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
PrintPipePortInfo(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{

    switch (vInPorts.size())
    {
        case 1:
            if ( 0 == vInPorts[0] ) { break; }
            PIPE_INF("vInPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vInPorts[0]->eImgFmt,
                                                            vInPorts[0]->u4ImgWidth,
                                                            vInPorts[0]->u4ImgHeight,
                                                            vInPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[0]->type,
                                                            vInPorts[0]->index,
                                                            vInPorts[0]->inout,
                                                            vInPorts[0]->u4BufSize[0],
                                                            vInPorts[0]->u4BufSize[1],
                                                            vInPorts[0]->u4BufSize[2],
                                                            (unsigned long)vInPorts[0]->u4BufVA[0],
                                                            (unsigned long)vInPorts[0]->u4BufVA[1],
                                                            (unsigned long)vInPorts[0]->u4BufVA[2],
                                                            (unsigned long)vInPorts[0]->u4BufPA[0],
                                                            (unsigned long)vInPorts[0]->u4BufPA[1],
                                                            (unsigned long)vInPorts[0]->u4BufPA[2]);
            break;
        case 2:
            if ( (0 == vInPorts[0]) ||  (0 == vInPorts[1])) { break; }
            PIPE_INF("vInPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vInPorts[0]->eImgFmt,
                                                            vInPorts[0]->u4ImgWidth,
                                                            vInPorts[0]->u4ImgHeight,
                                                            vInPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[0]->type,
                                                            vInPorts[0]->index,
                                                            vInPorts[0]->inout,
                                                            vInPorts[0]->u4BufSize[0],
                                                            vInPorts[0]->u4BufSize[1],
                                                            vInPorts[0]->u4BufSize[2],
                                                            (unsigned long)vInPorts[0]->u4BufVA[0],
                                                            (unsigned long)vInPorts[0]->u4BufVA[1],
                                                            (unsigned long)vInPorts[0]->u4BufVA[2],
                                                            (unsigned long)vInPorts[0]->u4BufPA[0],
                                                            (unsigned long)vInPorts[0]->u4BufPA[1],
                                                            (unsigned long)vInPorts[0]->u4BufPA[2],
                                                            vInPorts[1]->eImgFmt,
                                                            vInPorts[1]->u4ImgWidth,
                                                            vInPorts[1]->u4ImgHeight,
                                                            vInPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[1]->type,
                                                            vInPorts[1]->index,
                                                            vInPorts[1]->inout,
                                                            vInPorts[1]->u4BufSize[0],
                                                            vInPorts[1]->u4BufSize[1],
                                                            vInPorts[1]->u4BufSize[2],
                                                            (unsigned long)vInPorts[1]->u4BufVA[0],
                                                            (unsigned long)vInPorts[1]->u4BufVA[1],
                                                            (unsigned long)vInPorts[1]->u4BufVA[2],
                                                            (unsigned long)vInPorts[1]->u4BufPA[0],
                                                            (unsigned long)vInPorts[1]->u4BufPA[1],
                                                            (unsigned long)vInPorts[1]->u4BufPA[2]);
            break;
        case 3:
            if ( (0 == vInPorts[0]) || (0 == vInPorts[1]) || (0 == vInPorts[2])) { break; }
            PIPE_INF("vInPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vInPorts[0]->eImgFmt,
                                                            vInPorts[0]->u4ImgWidth,
                                                            vInPorts[0]->u4ImgHeight,
                                                            vInPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[0]->type,
                                                            vInPorts[0]->index,
                                                            vInPorts[0]->inout,
                                                            vInPorts[0]->u4BufSize[0],
                                                            vInPorts[0]->u4BufSize[1],
                                                            vInPorts[0]->u4BufSize[2],
                                                            (unsigned long)vInPorts[0]->u4BufVA[0],
                                                            (unsigned long)vInPorts[0]->u4BufVA[1],
                                                            (unsigned long)vInPorts[0]->u4BufVA[2],
                                                            (unsigned long)vInPorts[0]->u4BufPA[0],
                                                            (unsigned long)vInPorts[0]->u4BufPA[1],
                                                            (unsigned long)vInPorts[0]->u4BufPA[2],
                                                            vInPorts[1]->eImgFmt,
                                                            vInPorts[1]->u4ImgWidth,
                                                            vInPorts[1]->u4ImgHeight,
                                                            vInPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[1]->type,
                                                            vInPorts[1]->index,
                                                            vInPorts[1]->inout,
                                                            vInPorts[1]->u4BufSize[0],
                                                            vInPorts[1]->u4BufSize[1],
                                                            vInPorts[1]->u4BufSize[2],
                                                            (unsigned long)vInPorts[1]->u4BufVA[0],
                                                            (unsigned long)vInPorts[1]->u4BufVA[1],
                                                            (unsigned long)vInPorts[1]->u4BufVA[2],
                                                            (unsigned long)vInPorts[1]->u4BufPA[0],
                                                            (unsigned long)vInPorts[1]->u4BufPA[1],
                                                            (unsigned long)vInPorts[1]->u4BufPA[2],
                                                            vInPorts[2]->eImgFmt,
                                                            vInPorts[2]->u4ImgWidth,
                                                            vInPorts[2]->u4ImgHeight,
                                                            vInPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[2]->type,
                                                            vInPorts[2]->index,
                                                            vInPorts[2]->inout,
                                                            vInPorts[2]->u4BufSize[0],
                                                            vInPorts[2]->u4BufSize[1],
                                                            vInPorts[2]->u4BufSize[2],
                                                            (unsigned long)vInPorts[2]->u4BufVA[0],
                                                            (unsigned long)vInPorts[2]->u4BufVA[1],
                                                            (unsigned long)vInPorts[2]->u4BufVA[2],
                                                            (unsigned long)vInPorts[2]->u4BufPA[0],
                                                            (unsigned long)vInPorts[2]->u4BufPA[1],
                                                            (unsigned long)vInPorts[2]->u4BufPA[2]
                                                            );
            break;
        case 4:
            if ( (0 == vInPorts[0]) || (0 == vInPorts[1]) || (0 == vInPorts[2]) || (0 == vInPorts[3])) { break; }
            PIPE_INF("vInPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[3]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vInPorts[0]->eImgFmt,
                                                            vInPorts[0]->u4ImgWidth,
                                                            vInPorts[0]->u4ImgHeight,
                                                            vInPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[0]->type,
                                                            vInPorts[0]->index,
                                                            vInPorts[0]->inout,
                                                            vInPorts[0]->u4BufSize[0],
                                                            vInPorts[0]->u4BufSize[1],
                                                            vInPorts[0]->u4BufSize[2],
                                                            (unsigned long)vInPorts[0]->u4BufVA[0],
                                                            (unsigned long)vInPorts[0]->u4BufVA[1],
                                                            (unsigned long)vInPorts[0]->u4BufVA[2],
                                                            (unsigned long)vInPorts[0]->u4BufPA[0],
                                                            (unsigned long)vInPorts[0]->u4BufPA[1],
                                                            (unsigned long)vInPorts[0]->u4BufPA[2],
                                                            vInPorts[1]->eImgFmt,
                                                            vInPorts[1]->u4ImgWidth,
                                                            vInPorts[1]->u4ImgHeight,
                                                            vInPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[1]->type,
                                                            vInPorts[1]->index,
                                                            vInPorts[1]->inout,
                                                            vInPorts[1]->u4BufSize[0],
                                                            vInPorts[1]->u4BufSize[1],
                                                            vInPorts[1]->u4BufSize[2],
                                                            (unsigned long)vInPorts[1]->u4BufVA[0],
                                                            (unsigned long)vInPorts[1]->u4BufVA[1],
                                                            (unsigned long)vInPorts[1]->u4BufVA[2],
                                                            (unsigned long)vInPorts[1]->u4BufPA[0],
                                                            (unsigned long)vInPorts[1]->u4BufPA[1],
                                                            (unsigned long)vInPorts[1]->u4BufPA[2],
                                                            vInPorts[2]->eImgFmt,
                                                            vInPorts[2]->u4ImgWidth,
                                                            vInPorts[2]->u4ImgHeight,
                                                            vInPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[2]->type,
                                                            vInPorts[2]->index,
                                                            vInPorts[2]->inout,
                                                            vInPorts[2]->u4BufSize[0],
                                                            vInPorts[2]->u4BufSize[1],
                                                            vInPorts[2]->u4BufSize[2],
                                                            (unsigned long)vInPorts[2]->u4BufVA[0],
                                                            (unsigned long)vInPorts[2]->u4BufVA[1],
                                                            (unsigned long)vInPorts[2]->u4BufVA[2],
                                                            (unsigned long)vInPorts[2]->u4BufPA[0],
                                                            (unsigned long)vInPorts[2]->u4BufPA[1],
                                                            (unsigned long)vInPorts[2]->u4BufPA[2],
                                                            vInPorts[3]->eImgFmt,
                                                            vInPorts[3]->u4ImgWidth,
                                                            vInPorts[3]->u4ImgHeight,
                                                            vInPorts[3]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[3]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[3]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[3]->type,
                                                            vInPorts[3]->index,
                                                            vInPorts[3]->inout,
                                                            vInPorts[3]->u4BufSize[0],
                                                            vInPorts[3]->u4BufSize[1],
                                                            vInPorts[3]->u4BufSize[2],
                                                            (unsigned long)vInPorts[3]->u4BufVA[0],
                                                            (unsigned long)vInPorts[3]->u4BufVA[1],
                                                            (unsigned long)vInPorts[3]->u4BufVA[2],
                                                            (unsigned long)vInPorts[3]->u4BufPA[0],
                                                            (unsigned long)vInPorts[3]->u4BufPA[1],
                                                            (unsigned long)vInPorts[3]->u4BufPA[2]
                                                            );


            break;
        case 5:
            if ( (0 == vInPorts[0]) || (0 == vInPorts[1]) || (0 == vInPorts[2]) || (0 == vInPorts[3]) || (0 == vInPorts[4])) { break; }
            PIPE_INF("vInPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[3]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vInPorts:[4]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vInPorts[0]->eImgFmt,
                                                            vInPorts[0]->u4ImgWidth,
                                                            vInPorts[0]->u4ImgHeight,
                                                            vInPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[0]->type,
                                                            vInPorts[0]->index,
                                                            vInPorts[0]->inout,
                                                            vInPorts[0]->u4BufSize[0],
                                                            vInPorts[0]->u4BufSize[1],
                                                            vInPorts[0]->u4BufSize[2],
                                                            (unsigned long)vInPorts[0]->u4BufVA[0],
                                                            (unsigned long)vInPorts[0]->u4BufVA[1],
                                                            (unsigned long)vInPorts[0]->u4BufVA[2],
                                                            (unsigned long)vInPorts[0]->u4BufPA[0],
                                                            (unsigned long)vInPorts[0]->u4BufPA[1],
                                                            (unsigned long)vInPorts[0]->u4BufPA[2],
                                                            vInPorts[1]->eImgFmt,
                                                            vInPorts[1]->u4ImgWidth,
                                                            vInPorts[1]->u4ImgHeight,
                                                            vInPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[1]->type,
                                                            vInPorts[1]->index,
                                                            vInPorts[1]->inout,
                                                            vInPorts[1]->u4BufSize[0],
                                                            vInPorts[1]->u4BufSize[1],
                                                            vInPorts[1]->u4BufSize[2],
                                                            (unsigned long)vInPorts[1]->u4BufVA[0],
                                                            (unsigned long)vInPorts[1]->u4BufVA[1],
                                                            (unsigned long)vInPorts[1]->u4BufVA[2],
                                                            (unsigned long)vInPorts[1]->u4BufPA[0],
                                                            (unsigned long)vInPorts[1]->u4BufPA[1],
                                                            (unsigned long)vInPorts[1]->u4BufPA[2],
                                                            vInPorts[2]->eImgFmt,
                                                            vInPorts[2]->u4ImgWidth,
                                                            vInPorts[2]->u4ImgHeight,
                                                            vInPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[2]->type,
                                                            vInPorts[2]->index,
                                                            vInPorts[2]->inout,
                                                            vInPorts[2]->u4BufSize[0],
                                                            vInPorts[2]->u4BufSize[1],
                                                            vInPorts[2]->u4BufSize[2],
                                                            (unsigned long)vInPorts[2]->u4BufVA[0],
                                                            (unsigned long)vInPorts[2]->u4BufVA[1],
                                                            (unsigned long)vInPorts[2]->u4BufVA[2],
                                                            (unsigned long)vInPorts[2]->u4BufPA[0],
                                                            (unsigned long)vInPorts[2]->u4BufPA[1],
                                                            (unsigned long)vInPorts[2]->u4BufPA[2],
                                                            vInPorts[3]->eImgFmt,
                                                            vInPorts[3]->u4ImgWidth,
                                                            vInPorts[3]->u4ImgHeight,
                                                            vInPorts[3]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[3]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[3]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[3]->type,
                                                            vInPorts[3]->index,
                                                            vInPorts[3]->inout,
                                                            vInPorts[3]->u4BufSize[0],
                                                            vInPorts[3]->u4BufSize[1],
                                                            vInPorts[3]->u4BufSize[2],
                                                            (unsigned long)vInPorts[3]->u4BufVA[0],
                                                            (unsigned long)vInPorts[3]->u4BufVA[1],
                                                            (unsigned long)vInPorts[3]->u4BufVA[2],
                                                            (unsigned long)vInPorts[3]->u4BufPA[0],
                                                            (unsigned long)vInPorts[3]->u4BufPA[1],
                                                            (unsigned long)vInPorts[3]->u4BufPA[2],
                                                            vInPorts[4]->eImgFmt,
                                                            vInPorts[4]->u4ImgWidth,
                                                            vInPorts[4]->u4ImgHeight,
                                                            vInPorts[4]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vInPorts[4]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vInPorts[4]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vInPorts[4]->type,
                                                            vInPorts[4]->index,
                                                            vInPorts[4]->inout,
                                                            vInPorts[4]->u4BufSize[0],
                                                            vInPorts[4]->u4BufSize[1],
                                                            vInPorts[4]->u4BufSize[2],
                                                            (unsigned long)vInPorts[4]->u4BufVA[0],
                                                            (unsigned long)vInPorts[4]->u4BufVA[1],
                                                            (unsigned long)vInPorts[4]->u4BufVA[2],
                                                            (unsigned long)vInPorts[4]->u4BufPA[0],
                                                            (unsigned long)vInPorts[4]->u4BufPA[1],
                                                            (unsigned long)vInPorts[4]->u4BufPA[2]
                                                            );
            break;
        default:
            PIPE_ERR("please add PrintPipePortInfo vInPorts switch case:%lu\n", (unsigned long)vInPorts.size());
            break;
    }

    switch (vOutPorts.size())
    {
        case 1:
            if ( 0 == vOutPorts[0]) { break; }
            PIPE_INF("vOutPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vOutPorts[0]->eImgFmt,
                                                            vOutPorts[0]->u4ImgWidth,
                                                            vOutPorts[0]->u4ImgHeight,
                                                            vOutPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[0]->type,
                                                            vOutPorts[0]->index,
                                                            vOutPorts[0]->inout,
                                                            vOutPorts[0]->u4BufSize[0],
                                                            vOutPorts[0]->u4BufSize[1],
                                                            vOutPorts[0]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[2]);
            break;
        case 2:
            if ( (0 == vOutPorts[0]) || (0 == vOutPorts[1])) { break; }
            PIPE_INF("vOutPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vOutPorts[0]->eImgFmt,
                                                            vOutPorts[0]->u4ImgWidth,
                                                            vOutPorts[0]->u4ImgHeight,
                                                            vOutPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[0]->type,
                                                            vOutPorts[0]->index,
                                                            vOutPorts[0]->inout,
                                                            vOutPorts[0]->u4BufSize[0],
                                                            vOutPorts[0]->u4BufSize[1],
                                                            vOutPorts[0]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[2],
                                                            vOutPorts[1]->eImgFmt,
                                                            vOutPorts[1]->u4ImgWidth,
                                                            vOutPorts[1]->u4ImgHeight,
                                                            vOutPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[1]->type,
                                                            vOutPorts[1]->index,
                                                            vOutPorts[1]->inout,
                                                            vOutPorts[1]->u4BufSize[0],
                                                            vOutPorts[1]->u4BufSize[1],
                                                            vOutPorts[1]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[2]
                                                            );
            break;
        case 3:
            if ( (0 == vOutPorts[0]) || (0 == vOutPorts[1]) || (0 == vOutPorts[2])) { break; }
            PIPE_INF("vOutPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vOutPorts[0]->eImgFmt,
                                                            vOutPorts[0]->u4ImgWidth,
                                                            vOutPorts[0]->u4ImgHeight,
                                                            vOutPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[0]->type,
                                                            vOutPorts[0]->index,
                                                            vOutPorts[0]->inout,
                                                            vOutPorts[0]->u4BufSize[0],
                                                            vOutPorts[0]->u4BufSize[1],
                                                            vOutPorts[0]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[2],
                                                            vOutPorts[1]->eImgFmt,
                                                            vOutPorts[1]->u4ImgWidth,
                                                            vOutPorts[1]->u4ImgHeight,
                                                            vOutPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[1]->type,
                                                            vOutPorts[1]->index,
                                                            vOutPorts[1]->inout,
                                                            vOutPorts[1]->u4BufSize[0],
                                                            vOutPorts[1]->u4BufSize[1],
                                                            vOutPorts[1]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[2],
                                                            vOutPorts[2]->eImgFmt,
                                                            vOutPorts[2]->u4ImgWidth,
                                                            vOutPorts[2]->u4ImgHeight,
                                                            vOutPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[2]->type,
                                                            vOutPorts[2]->index,
                                                            vOutPorts[2]->inout,
                                                            vOutPorts[2]->u4BufSize[0],
                                                            vOutPorts[2]->u4BufSize[1],
                                                            vOutPorts[2]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[2]
                                                            );
            break;
        case 4:
            if ( (0 == vOutPorts[0]) || (0 == vOutPorts[1]) || (0 == vOutPorts[2]) || (0 == vOutPorts[3])) { break; }
            PIPE_INF("vOutPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[3]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vOutPorts[0]->eImgFmt,
                                                            vOutPorts[0]->u4ImgWidth,
                                                            vOutPorts[0]->u4ImgHeight,
                                                            vOutPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[0]->type,
                                                            vOutPorts[0]->index,
                                                            vOutPorts[0]->inout,
                                                            vOutPorts[0]->u4BufSize[0],
                                                            vOutPorts[0]->u4BufSize[1],
                                                            vOutPorts[0]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[2],
                                                            vOutPorts[1]->eImgFmt,
                                                            vOutPorts[1]->u4ImgWidth,
                                                            vOutPorts[1]->u4ImgHeight,
                                                            vOutPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[1]->type,
                                                            vOutPorts[1]->index,
                                                            vOutPorts[1]->inout,
                                                            vOutPorts[1]->u4BufSize[0],
                                                            vOutPorts[1]->u4BufSize[1],
                                                            vOutPorts[1]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[2],
                                                            vOutPorts[2]->eImgFmt,
                                                            vOutPorts[2]->u4ImgWidth,
                                                            vOutPorts[2]->u4ImgHeight,
                                                            vOutPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[2]->type,
                                                            vOutPorts[2]->index,
                                                            vOutPorts[2]->inout,
                                                            vOutPorts[2]->u4BufSize[0],
                                                            vOutPorts[2]->u4BufSize[1],
                                                            vOutPorts[2]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[2],
                                                            vOutPorts[3]->eImgFmt,
                                                            vOutPorts[3]->u4ImgWidth,
                                                            vOutPorts[3]->u4ImgHeight,
                                                            vOutPorts[3]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[3]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[3]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[3]->type,
                                                            vOutPorts[3]->index,
                                                            vOutPorts[3]->inout,
                                                            vOutPorts[3]->u4BufSize[0],
                                                            vOutPorts[3]->u4BufSize[1],
                                                            vOutPorts[3]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[2]
                                                            );
            break;
        case 5:
            if ( (0 == vOutPorts[0]) || (0 == vOutPorts[1]) || (0 == vOutPorts[2]) || (0 == vOutPorts[3]) || (0 == vOutPorts[4])) { break; }
            PIPE_INF("vOutPorts:[0]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[1]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[2]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[3]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),\n\
                      vOutPorts:[4]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                            vOutPorts[0]->eImgFmt,
                                                            vOutPorts[0]->u4ImgWidth,
                                                            vOutPorts[0]->u4ImgHeight,
                                                            vOutPorts[0]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[0]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[0]->type,
                                                            vOutPorts[0]->index,
                                                            vOutPorts[0]->inout,
                                                            vOutPorts[0]->u4BufSize[0],
                                                            vOutPorts[0]->u4BufSize[1],
                                                            vOutPorts[0]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[0]->u4BufPA[2],
                                                            vOutPorts[1]->eImgFmt,
                                                            vOutPorts[1]->u4ImgWidth,
                                                            vOutPorts[1]->u4ImgHeight,
                                                            vOutPorts[1]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[1]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[1]->type,
                                                            vOutPorts[1]->index,
                                                            vOutPorts[1]->inout,
                                                            vOutPorts[1]->u4BufSize[0],
                                                            vOutPorts[1]->u4BufSize[1],
                                                            vOutPorts[1]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[1]->u4BufPA[2],
                                                            vOutPorts[2]->eImgFmt,
                                                            vOutPorts[2]->u4ImgWidth,
                                                            vOutPorts[2]->u4ImgHeight,
                                                            vOutPorts[2]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[2]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[2]->type,
                                                            vOutPorts[2]->index,
                                                            vOutPorts[2]->inout,
                                                            vOutPorts[2]->u4BufSize[0],
                                                            vOutPorts[2]->u4BufSize[1],
                                                            vOutPorts[2]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[2]->u4BufPA[2],
                                                            vOutPorts[3]->eImgFmt,
                                                            vOutPorts[3]->u4ImgWidth,
                                                            vOutPorts[3]->u4ImgHeight,
                                                            vOutPorts[3]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[3]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[3]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[3]->type,
                                                            vOutPorts[3]->index,
                                                            vOutPorts[3]->inout,
                                                            vOutPorts[3]->u4BufSize[0],
                                                            vOutPorts[3]->u4BufSize[1],
                                                            vOutPorts[3]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[3]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[3]->u4BufPA[2],
                                                            vOutPorts[4]->eImgFmt,
                                                            vOutPorts[4]->u4ImgWidth,
                                                            vOutPorts[4]->u4ImgHeight,
                                                            vOutPorts[4]->u4Stride[ESTRIDE_1ST_PLANE],
                                                            vOutPorts[4]->u4Stride[ESTRIDE_2ND_PLANE],
                                                            vOutPorts[4]->u4Stride[ESTRIDE_3RD_PLANE],
                                                            vOutPorts[4]->type,
                                                            vOutPorts[4]->index,
                                                            vOutPorts[4]->inout,
                                                            vOutPorts[4]->u4BufSize[0],
                                                            vOutPorts[4]->u4BufSize[1],
                                                            vOutPorts[4]->u4BufSize[2],
                                                            (unsigned long)vOutPorts[4]->u4BufVA[0],
                                                            (unsigned long)vOutPorts[4]->u4BufVA[1],
                                                            (unsigned long)vOutPorts[4]->u4BufVA[2],
                                                            (unsigned long)vOutPorts[4]->u4BufPA[0],
                                                            (unsigned long)vOutPorts[4]->u4BufPA[1],
                                                            (unsigned long)vOutPorts[4]->u4BufPA[2]
                                                            );
            break;

        default:
            PIPE_ERR("please add PrintPipePortInfo vOutPorts switch case:%lu\n", (unsigned long)vOutPorts.size());
            break;
    }

    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, PipePackageInfo *pPipePackageInfo)
{
    MBOOL ret = MTRUE;

    //judge drvscenaio and p2 cq
    EDrvScenario drvScen=pPipePackageInfo->drvScen;
    MINT32 p2CQ=pPipePackageInfo->p2cqIdx;
    MINT32 dupCQIdx=pPipePackageInfo->dupCqIdx;
    MINT32 burstQIdx=pPipePackageInfo->burstQIdx;
    MBOOL lastFrame=pPipePackageInfo->lastframe;
    NSCam::NSIoPipe::CrspInfo Img3oCrspTmp;

    if (HandleExtraCommand(pPipePackageInfo) == MFALSE)
    {
        PIPE_ERR("parsing frame cmd error!!\n");
        return MFALSE; //error command  enque
    }

    if(dupCQIdx>=MAX_DUP_CQ_NUM || dupCQIdx<0 || burstQIdx>=MAX_BURST_QUE_NUM || burstQIdx<0 ||p2CQ>=ISP_DRV_DIP_CQ_NUM || p2CQ<0 )
    {
        PIPE_ERR("exception drvScen/cq/dup/burst(%d/%d/%d/%d), last(%d)",drvScen, p2CQ, dupCQIdx, burstQIdx, lastFrame);
        return MFALSE;
    }
    m_CrzEn = MFALSE;
    //////////////////////////////////////////////////
    //variables
    //dma
    int idx_imgi = -1;
    int idx_imgbi = -1;
    int idx_imgci = -1;
    int idx_vipi = -1;
    int idx_vip2i = -1;
    int idx_vip3i = -1;
    int idx_ufdi = -1;
    int idx_lcei = -1;
    int idx_dmgi = -1;
    int idx_depi = -1;
    int idx_regi = -1;
    int idx_img2o = -1;
    int idx_img2bo = -1;
    int idx_img3o = -1;
    int idx_img3bo = -1;
    int idx_img3co = -1;
    int idx_mfbo = -1;
    int idx_feo = -1;
    int idx_wroto = -1;
    int idx_wdmao = -1;
    int idx_jpego = -1;
    int idx_venco = -1;

    PortInfo portInfo_imgi;
    PortInfo portInfo_imgbi;
    PortInfo portInfo_imgci;
    PortInfo portInfo_vipi;
    PortInfo portInfo_vip2i;
    PortInfo portInfo_vip3i;
    PortInfo portInfo_ufdi;
    PortInfo portInfo_lcei;
    PortInfo portInfo_dmgi;
    PortInfo portInfo_depi;
    PortInfo portInfo_regi;
    PortInfo portInfo_img2o;
    PortInfo portInfo_img2bo;
    PortInfo portInfo_img3o;
    PortInfo portInfo_img3bo;
    PortInfo portInfo_img3co;
    PortInfo portInfo_feo;
    PortInfo portInfo_mfbo;
    PortInfo portInfo_wroto;
    PortInfo portInfo_wdmao;
    PortInfo portInfo_jpego;
    PortInfo portInfo_venco;

    MUINT32 dip_ctl_dma_en=0x0;
    MINT32 imgi_uv_h_ratio=2; // for imgbi & imgci
    MINT32 imgi_uv_v_ratio=2;
    MBOOL imgi_uv_swap=MFALSE;
    MINT32 vipi_uv_h_ratio=2;   // for vip2i & vip3i
    MINT32 vipi_uv_v_ratio=2;
    MBOOL vipi_uv_swap=MFALSE;
    MINT32 img2o_uv_h_ratio = 2;    // for img2bo
    MINT32 img2o_uv_v_ratio = 2;
    MBOOL   img2o_uv_swap = MFALSE;
    MINT32 img3o_uv_h_ratio = 2;    // for img3bo & img3co
    MINT32 img3o_uv_v_ratio = 2;
    MBOOL   img3o_uv_swap = MFALSE;
    MBOOL   b12bitMode = MTRUE;

    MINT32 pixelInByte=1;

    int mdp_imgxo_p2_en = 0;

    PortID portID;
    QBufInfo rQBufInfo;
    //path, module and select
    MINT32 dip_cq_th, dip_cq_thr_ctl;
    MUINT32 dip_ctl_yuv_en=DIP_X_REG_CTL_YUV_EN_C42;
    MUINT32 dip_ctl_yuv2_en=0x0, dip_ctl_rgb_en=0x0;
    MINT32 imgxi_en=0, vipxi_en=0, img2xo_en=0, img3xo_en=0, ufdi_en = 0, mfbo_en = 0;
    MINT32 imgi_fmt=DIP_IMGI_FMT_BAYER10, vipi_fmt=DIP_VIPI_FMT_YUV422_1P, img2o_fmt=DIP_IMG2O_FMT_YUV422_1P, img3o_fmt=DIP_IMG3O_FMT_YUV422_1P, mfbo_fmt = DIP_IMGI_FMT_BAYER10;
    MINT32 ufdi_fmt=DIP_UFDI_FMT_UFO_LENGTH, dmgi_fmt=DIP_DMGI_FMT_DRZ, depi_fmt=DIP_DEPI_FMT_DRZ;
    MINT32 fg_mode=DIP_FG_MODE_DISABLE;
    MUINT32 dip_ctl_path_sel=0x0;
    MUINT32 dip_misc_en=0x10; //DE: default sram _mode = 1
    MINT32 g2c_sel=0, pgn_sel=0, g2g_sel=0, nbc_sel=0, rcp2_sel = 0, imgi_sel = 0, feo_sel = 0;
    MUINT32 i, tableSize, mdpCheck1, mdpCheck2, mdpCheck3;
    MINT32 crop1TableIdx = -1;
    int dmai_swap = 0;
    MBOOL isWaitBuf=MTRUE;
    MBOOL isSetMdpOut = MFALSE;
    MUINT32 yuv_en=0;
    MUINT32 yuv2_en=0;
    MUINT32 rgb_en=0;
    MUINT32 dip_yuv_tuning_tag = DIP_X_REG_CTL_YUV_EN_NBC|DIP_X_REG_CTL_YUV_EN_NBC2|DIP_X_REG_CTL_YUV_EN_PCA|DIP_X_REG_CTL_YUV_EN_SEEE|DIP_X_REG_CTL_YUV_EN_NR3D\
		|DIP_X_REG_CTL_YUV_EN_SL2B|DIP_X_REG_CTL_YUV_EN_SL2C|DIP_X_REG_CTL_YUV_EN_SL2D|DIP_X_REG_CTL_YUV_EN_SL2E|DIP_X_REG_CTL_YUV_EN_MIX3|DIP_X_REG_CTL_YUV_EN_G2C;// |DIP_X_REG_CTL_YUV_EN_C42
    MUINT32 dip_yuv2_tuning_tag = DIP_X_REG_CTL_YUV2_EN_HFG|DIP_X_REG_CTL_YUV2_EN_SL2I|DIP_X_REG_CTL_YUV2_EN_NDG|DIP_X_REG_CTL_YUV2_EN_NDG2; //DIP_X_REG_CTL_YUV2_EN_SRZ3|DIP_X_REG_CTL_YUV2_EN_SRZ4;
    MUINT32 dip_rgb_tuning_tag = DIP_X_REG_CTL_RGB_EN_PGN|DIP_X_REG_CTL_RGB_EN_SL2|DIP_X_REG_CTL_RGB_EN_LCE\
		|DIP_X_REG_CTL_RGB_EN_DBS2|DIP_X_REG_CTL_RGB_EN_OBC2|DIP_X_REG_CTL_RGB_EN_RMG2|DIP_X_REG_CTL_RGB_EN_BNR2|DIP_X_REG_CTL_RGB_EN_RMM2|DIP_X_REG_CTL_RGB_EN_LSC2\
		|DIP_X_REG_CTL_RGB_EN_G2G|DIP_X_REG_CTL_RGB_EN_GGM|DIP_X_REG_CTL_RGB_EN_RNR|DIP_X_REG_CTL_RGB_EN_SL2G|DIP_X_REG_CTL_RGB_EN_SL2H|DIP_X_REG_CTL_RGB_EN_HLR2|DIP_X_REG_CTL_RGB_EN_SL2K|DIP_X_REG_CTL_RGB_EN_FLC|DIP_X_REG_CTL_RGB_EN_FLC2;//DIP_X_REG_CTL_RGB_EN_UDM;

    MBOOL isCropG1En = MFALSE;
    MUINT32  CropGroup;

    dip_x_reg_t *pTuningDipReg = NULL;

    ispDipPipe.crzPipe.conf_cdrz=MFALSE;
    #if 1 //temp  //kk test default:1
    int tpipe = CAM_MODE_TPIPE;
    #else   //frame mode first
    int tpipe = CAM_MODE_FRAME;
    #endif
    int tdr_tcm_en = 0x00;
    int tdr_tcm2_en = DIP_X_REG_CTL_TDR_C42_TCM_EN;
    int tdr_tpipe = 0x00000000;
    int tdr_ctl = 0x00;
    //////////////////////////////////////////////////////////////////////////////////////
    //judge pass type
    this->m_pipePass = static_cast<EPipePass>(EPipePass_Dip_Th0+p2CQ);  //send pass information through package instead of dam port
    PIPE_DBG("m_pipePass:[%d]",this->m_pipePass);
    switch (this->m_pipePass) {
        case EPipePass_Dip_Th0:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE0;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE; //immediately mode
            break;
        case EPipePass_Dip_Th1:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE1;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th2:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE2;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th3:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE3;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th4:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE4;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th5:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE5;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th6:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE6;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th7:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE7;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;

        case EPipePass_Dip_Th8:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE8;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;

        case EPipePass_Dip_Th9:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE9;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;

        case EPipePass_Dip_Th10:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE10;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;

        case EPipePass_Dip_Th11:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE11;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;

        case EPipePass_Dip_Th12:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE12;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th13:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE13;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        case EPipePass_Dip_Th14:
            dip_cq_th = ISP_DRV_DIP_CQ_THRE14;
            dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE;
            break;
        default:
            PIPE_ERR("[Error]NOT Support concurrency m_pipePass(%d)",this->m_pipePass);
            return MFALSE;
    }
    PIPE_DBG("dip_cq_th(%d)",dip_cq_th);

    ///////////////////////////////////////////////////////////////////////////////////////
    //parse the dma ports contained in the frame
    PIPE_DBG("drvScen[%d]/cq[%d]/in[%lu]/out[%lu]", pPipePackageInfo->drvScen, pPipePackageInfo->p2cqIdx, (unsigned long)vInPorts.size(), (unsigned long)vOutPorts.size());
    PrintPipePortInfo(vInPorts,vOutPorts);
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ )
    {
        if ( 0 == vInPorts[i] ) { continue; }
        //
        PIPE_DBG("vInPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",\
                                                        i,
                                                        vInPorts[i]->eImgFmt,
                                                        vInPorts[i]->u4ImgWidth,
                                                        vInPorts[i]->u4ImgHeight,
                                                        vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vInPorts[i]->type,
                                                        vInPorts[i]->index,
                                                        vInPorts[i]->inout,
                                                        vInPorts[i]->u4BufSize[0],
                                                        vInPorts[i]->u4BufSize[1],
                                                        vInPorts[i]->u4BufSize[2],
                                                        (unsigned long)vInPorts[i]->u4BufVA[0],
                                                        (unsigned long)vInPorts[i]->u4BufVA[1],
                                                        (unsigned long)vInPorts[i]->u4BufVA[2],
                                                        (unsigned long)vInPorts[i]->u4BufPA[0],
                                                        (unsigned long)vInPorts[i]->u4BufPA[1],
                                                        (unsigned long)vInPorts[i]->u4BufPA[2]);
        //
        switch(vInPorts[i]->index)
        {
	        case EPortIndex_IMGI:   //may multi-plane
                idx_imgi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGI_TCM_EN;
                portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];

                // input port limitation check for FullG format
                if ((vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER8) || (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER10) ||
                        (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER12) || (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER14)){
                    //TODO: size must be align 4 for FUllG format, USER TAKE CARE THIS
                }
                //
                if(vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                        && vInPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE])
                { // check imgbi
                    switch (vInPorts[i]->eImgFmt)
                    {
                        case eImgFmt_YV16:
                            imgi_uv_v_ratio = 1;
                            imgi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I422:
                            imgi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_NV16:
                            imgi_uv_h_ratio = 1;
                            imgi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV12:
                            imgi_uv_swap = MTRUE;
                            break;
                        default:
                            break;
                    }

                    if ((vInPorts[i]->eImgFmt == eImgFmt_UFO_FG_BAYER10) || (vInPorts[i]->eImgFmt == eImgFmt_UFO_FG_BAYER12))
                    {
                        idx_ufdi = i;
                        dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_UFDI;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_UFDI_TCM_EN;
                        //if (vInPorts[idx_ufdi]->eImgFmt != eImgFmt_UFO_FG)
                        //   dip_ctl_yuv_en |=  (DIP_X_REG_CTL_YUV_EN_SRZ1|DIP_X_REG_CTL_YUV_EN_MIX1);
                        portInfo_ufdi =  (PortInfo)*vInPorts[idx_ufdi];
                    }
                    else
                    {
    	                idx_imgbi = i;
    	                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGBI;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGBI_TCM_EN;
    	                portInfo_imgbi =  (PortInfo)*vInPorts[idx_imgbi];

                        if(vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_3RD_PLANE]
                            && vInPorts[i]->u4BufPA[ESTRIDE_3RD_PLANE])
                        { // check imgci
                            idx_imgci = i;
                            dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGCI;
                            tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGCI_TCM_EN;
                            portInfo_imgci =  (PortInfo)*vInPorts[idx_imgci];
                        }
                    }
                }
                PIPE_DBG("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),dupCqIdx(%d)", \
                    portInfo_imgi.crop1.x,portInfo_imgi.crop1.floatX,portInfo_imgi.crop1.y,portInfo_imgi.crop1.floatY, \
                    portInfo_imgi.crop1.w,portInfo_imgi.crop1.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h, \
                    portInfo_imgi.crop2.x,portInfo_imgi.crop2.floatX,portInfo_imgi.crop2.y,portInfo_imgi.crop2.floatY, \
                    portInfo_imgi.crop2.w,portInfo_imgi.crop2.h,portInfo_imgi.resize2.tar_w,portInfo_imgi.resize2.tar_h,
                    portInfo_imgi.crop3.x,portInfo_imgi.crop3.floatX,portInfo_imgi.crop3.y,portInfo_imgi.crop3.floatY, \
                    portInfo_imgi.crop3.w,portInfo_imgi.crop3.h,portInfo_imgi.resize3.tar_w,portInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx);

                break;
            case EPortIndex_IMGBI:  //Mixing path weighting table
                idx_imgbi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGBI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGBI_TCM_EN;
                portInfo_imgbi =  (PortInfo)*vInPorts[idx_imgbi];
                break;
            case EPortIndex_IMGCI:  //Mixing path weighting table
                idx_imgci = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGCI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGCI_TCM_EN;
                portInfo_imgci =  (PortInfo)*vInPorts[idx_imgci];
                break;
            case EPortIndex_VIPI:	//may multi-plane, vipi~vip3i
                idx_vipi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIPI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIPI_TCM_EN;
                portInfo_vipi =  (PortInfo)*vInPorts[idx_vipi];
                if(vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                        && vInPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE])
                { // check vip2i
                    switch (vInPorts[i]->eImgFmt)
                    {
                        case eImgFmt_YV16:
                            vipi_uv_v_ratio = 1;
                            vipi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I422:
                            vipi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_NV16:
                            vipi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_NV12:
                            vipi_uv_h_ratio = 1;
                            break;
                        case eImgFmt_YV12:
                            vipi_uv_swap = MTRUE;
                            break;
                        default:
                            break;
                    }
                    
                    idx_vip2i = i;
                    dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIP2I;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIP2I_TCM_EN;
                    portInfo_vip2i =  (PortInfo)*vInPorts[idx_vip2i];

                    if(vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_3RD_PLANE]
                            && vInPorts[i]->u4BufPA[ESTRIDE_3RD_PLANE])
                    { // check vip3i
                        idx_vip3i = i;
                        dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIP3I;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIP3I_TCM_EN;
                        portInfo_vip3i =  (PortInfo)*vInPorts[idx_vip3i];
                        }
                    }

                    if (pPipePackageInfo->pTuningQue != NULL)
                    {
                        pTuningDipReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;// check tuning enable bit on isp_function_dip
                        if (pTuningDipReg->DIP_X_CTL_YUV_EN.Bits.NR3D_EN & pTuningDipReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN)
                        {
                            switch(vInPorts[i]->eImgFmt)
                            {
                                case eImgFmt_YUY2:
                                    pixelInByte=2;
                                    portInfo_vipi.xoffset = (pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw % portInfo_vipi.u4Stride[0])/pixelInByte;
                                    portInfo_vipi.yoffset = pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw / portInfo_vipi.u4Stride[0];
                                    portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vipi.xoffset * pixelInByte);
                                    break;
                                case eImgFmt_YV12:
                                default:
                                    pixelInByte=1;
                                    portInfo_vipi.xoffset = (pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw % portInfo_vipi.u4Stride[0])/pixelInByte;
                                    portInfo_vipi.yoffset = pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw / portInfo_vipi.u4Stride[0];
                                    portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vipi.xoffset * pixelInByte);
                                    portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vipi.yoffset/2 * portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vipi.xoffset/2 * pixelInByte);
                                    portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vipi.yoffset/2 * portInfo_vipi.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vipi.xoffset/2 * pixelInByte);

                                    portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip2i.xoffset * pixelInByte);
                                    portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);
                                    portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);

                                    portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                                    portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                                    portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                                    break;
                            }
                            portInfo_vipi.u4ImgWidth = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                            portInfo_vipi.u4ImgHeight = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                            portInfo_vip2i.u4ImgWidth = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                            portInfo_vip2i.u4ImgHeight = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                            portInfo_vip3i.u4ImgWidth = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                            portInfo_vip3i.u4ImgHeight = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                            PIPE_DBG("[vipi]ofst(%d),rW/H(%d/%d)",pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT);
                        }
                    }

                    break;
            case EPortIndex_VIP3I: //only vip3i input(weighting table), there are only vipi+vip3i without vip2i for mfb mixing path
                idx_vip3i = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIP3I;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIP3I_TCM_EN;
                portInfo_vip3i =  (PortInfo)*vInPorts[idx_vip3i];
                break;
        	 case EPortIndex_LCEI:
                idx_lcei = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_LCEI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_LCEI_TCM_EN;

                if(drvScen==eDrvScenario_VFB_FB || drvScen==eDrvScenario_P2B_Bokeh){
                    dip_ctl_yuv_en |=  (DIP_X_REG_CTL_YUV_EN_SRZ2|DIP_X_REG_CTL_YUV_EN_MIX2);
                }

                portInfo_lcei =  (PortInfo)*vInPorts[idx_lcei];
                break;
            case EPortIndex_UFDI:
                idx_ufdi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_UFDI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_UFDI_TCM_EN;
                if (vInPorts[idx_ufdi]->eImgFmt != eImgFmt_UFO_FG)
                    dip_ctl_yuv_en |=  (DIP_X_REG_CTL_YUV_EN_SRZ1|DIP_X_REG_CTL_YUV_EN_MIX1);
                portInfo_ufdi =  (PortInfo)*vInPorts[idx_ufdi];
                #if 0 //TODO path
                if((meScenarioID!=eDrvScenario_CC_vFB_FB)&&(meScenarioID!=eDrvScenario_CC_MFB_Mixing))
                {
                    en_p2 |= CAM_CTL_EN_P2_UFD_EN;
                    ufd_sel = 1;
                }
                #endif
                break;
            case EPortIndex_DEPI:
                idx_depi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_DEPI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_DEPI_TCM_EN;
                //if(drvScen == eDrvScenario_P2B_Bokeh) {
                //    dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ4);
                //}
                portInfo_depi =  (PortInfo)*vInPorts[idx_depi];
                break;
            case EPortIndex_DMGI:
                idx_dmgi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_DMGI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_DMGI_TCM_EN;
                //if(drvScen == eDrvScenario_P2B_Bokeh) {
                //    dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ3);
                //}
                portInfo_dmgi =  (PortInfo)*vInPorts[idx_dmgi];
                break;
            case EPortIndex_REGI:
                idx_regi = i;
                portInfo_regi =  (PortInfo)*vInPorts[idx_regi];
                break;
            default:
                PIPE_ERR("[Error]Not support this input port(%d),i(%d)",vInPorts[i]->index,i);
                break;
        }
    }

    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ )
    {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_DBG("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx)",i,\
                                                        vOutPorts[i]->eImgFmt,
                                                        vOutPorts[i]->u4ImgWidth,
                                                        vOutPorts[i]->u4ImgHeight,
                                                        vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vOutPorts[i]->type,
                                                        vOutPorts[i]->index,
                                                        vOutPorts[i]->inout,
                                                        vOutPorts[i]->u4BufSize[0],
                                                        vOutPorts[i]->u4BufSize[1],
                                                        vOutPorts[i]->u4BufSize[2],
                                                        (unsigned long)vOutPorts[i]->u4BufVA[0],
                                                        (unsigned long)vOutPorts[i]->u4BufVA[1],
                                                        (unsigned long)vOutPorts[i]->u4BufVA[2],
                                                        (unsigned long)vOutPorts[i]->u4BufPA[0],
                                                        (unsigned long)vOutPorts[i]->u4BufPA[1],
                                                        (unsigned long)vOutPorts[i]->u4BufPA[2]);

        if ((MTRUE == getCropFunctionEnable(drvScen, (MINT32)vOutPorts[i]->index, CropGroup))
            && (CropGroup & ECropGroupIndex_1))
        {
            //Check the crop setting.
            isCropG1En = MTRUE;
        }
        switch(vOutPorts[i]->index)
        {
            case EPortIndex_IMG2O: //may multi-plane, img2o+IMG2bo
                    idx_img2o = i;
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG2O;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG2O_TCM_EN;
                    portInfo_img2o =  (PortInfo)*vOutPorts[idx_img2o];
                    if(vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                            && vOutPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE])
                    { // check img2bo
                        switch (vOutPorts[i]->eImgFmt)
                        {
                            case eImgFmt_YV16:
                                img2o_uv_v_ratio = 1;
                                break;
                            case eImgFmt_I422:
                                img2o_uv_v_ratio = 1;
                                break;
                            case eImgFmt_NV16:
                                img2o_uv_v_ratio = 1;
                                img2o_uv_h_ratio = 1;
                                break;
                            case eImgFmt_YV12:
                                PIPE_ERR("IMG2O do not support YV12");
                                break;
                            case eImgFmt_NV12:
                                img2o_uv_h_ratio = 1;
                                break;
                            default:
                                break;
                        }

                        idx_img2bo = i;
                        dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG2BO;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG2BO_TCM_EN;
                        portInfo_img2bo =  (PortInfo)*vOutPorts[idx_img2bo];
                    }
                break;
            case EPortIndex_IMG3O:  //may multi-plane, img3o+img3bo+img3co
                    idx_img3o = i;
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG3O;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG3O_TCM_EN;
                    portInfo_img3o =  (PortInfo)*vOutPorts[idx_img3o];
                    if(vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                            && vOutPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE])
                    { // check img3bo
                        switch (vOutPorts[i]->eImgFmt)
                        {
                            case eImgFmt_YV16:
                                img3o_uv_v_ratio = 1;
                                img3o_uv_swap = MTRUE;
                                break;
                            case eImgFmt_I422:
                                img3o_uv_v_ratio = 1;
                                break;
                            case eImgFmt_NV12:
                            case eImgFmt_NV21:
                                img3o_uv_h_ratio = 1;
                                break;
                           case eImgFmt_NV16:
                                img3o_uv_v_ratio = 1;
                                img3o_uv_h_ratio = 1;
                                break;
                            case eImgFmt_YV12:
                                img3o_uv_swap = MTRUE;
                                break;
                            default:
                                break;
                        }

                        idx_img3bo = i;
                        dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG3BO;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG3BO_TCM_EN;
                        portInfo_img3bo =  (PortInfo)*vOutPorts[idx_img3bo];

                        if(vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE] && vOutPorts[i]->u4BufSize[ESTRIDE_3RD_PLANE] && vOutPorts[i]->u4BufVA[ESTRIDE_3RD_PLANE]
                                && vOutPorts[i]->u4BufPA[ESTRIDE_3RD_PLANE])
                        { // check img3co
                            idx_img3co = i;
                            dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG3CO;
                            tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG3CO_TCM_EN;
                            portInfo_img3co =  (PortInfo)*vOutPorts[idx_img3co];
                        }
                    }
                break;
            case EPortIndex_WROTO:
                    idx_wroto = i;
                    dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_WROTO;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                    portInfo_wroto =  (PortInfo)*vOutPorts[idx_wroto];
                break;
            case EPortIndex_WDMAO:
                    idx_wdmao = i;
                    dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_WDMAO;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                    portInfo_wdmao =  (PortInfo)*vOutPorts[idx_wdmao];
                break;
            case EPortIndex_JPEGO:
                    idx_jpego = i;
                    dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_JPEGO;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                    portInfo_jpego =  (PortInfo)*vOutPorts[idx_jpego];
                break;
            case EPortIndex_FEO:
                    if(drvScen == eDrvScenario_VFB_FB)
                    {
                        PIPE_ERR("[Error]do not support FE when P2B path");
                        return MFALSE;
                    }
                    else if (drvScen == eDrvScenario_Y16_Dump)
                    {
                        idx_feo = i;
                        dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_FEO;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_FEO_TCM_EN;
                        portInfo_feo =  (PortInfo)*vOutPorts[idx_feo];
                    }
                    else
                    {
                        idx_feo = i;
                        dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_FEO;
                        dip_ctl_yuv_en |=DIP_X_REG_CTL_YUV_EN_FE;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_FEO_TCM_EN;
                        tdr_tcm2_en |= DIP_X_REG_CTL_TDR_FE_TCM_EN;
                        portInfo_feo =  (PortInfo)*vOutPorts[idx_feo];
                    }
                break;
            case EPortIndex_MFBO: // mfb blending
                    idx_mfbo = i;
                    portInfo_mfbo =  (PortInfo)*vOutPorts[idx_mfbo];
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_MFBO;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_MFBO_TCM_EN;
                break;
            case EPortIndex_VENC_STREAMO:
                    idx_venco = i;
                    portInfo_venco =  (PortInfo)*vOutPorts[idx_venco];
                    dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_VENCO;
                break;
            default:
                PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",vOutPorts[i]->index,i);
                break;
        };
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    //calculate crop offset of img3o for tpipe if mdp crop is set, based on tpipe limitation
    #if 1
    if(idx_img3o >= 0)
    {
        if (m_Img3oCropInfo != NULL)
        {
            //support different view angle
            portInfo_img3o.xoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.x; //img3o offset must be even
            portInfo_img3o.yoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            portInfo_img3bo.xoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            portInfo_img3bo.yoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            portInfo_img3co.xoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            portInfo_img3co.yoffset = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
        }
        else
        {
            //support different view angle
            portInfo_img3o.xoffset = portInfo_img3o.xoffset & (~1);	//img3o offset must be even
            portInfo_img3o.yoffset = portInfo_img3o.yoffset & (~1);
            portInfo_img3bo.xoffset = portInfo_img3o.xoffset;
            portInfo_img3bo.yoffset = portInfo_img3o.yoffset;
            portInfo_img3co.xoffset = portInfo_img3o.xoffset;
            portInfo_img3co.yoffset = portInfo_img3o.yoffset;
        }
        PIPE_DBG("IMG3O cpoft(%d/%d)",portInfo_img3o.xoffset,portInfo_img3o.yoffset);
    }
    #endif

    /////////////////////////////////////////////////////////////////////////////////////////////
    //specific module setting
    for (MUINT32 i = 0 ; i < pPipePackageInfo->vModuleParams.size() ; i++ )
    {
        switch(pPipePackageInfo->vModuleParams[i].eDipModule)
        {
            case EDipModule_SRZ1:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    dip_ctl_yuv_en |=DIP_X_REG_CTL_YUV_EN_SRZ1;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_SRZ1_TCM_EN;

                    this->configSrz(&ispDipPipe.srz1Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ1:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d)",ispDipPipe.srz1Cfg.inout_size.in_w,ispDipPipe.srz1Cfg.inout_size.in_h,\
                        ispDipPipe.srz1Cfg.crop.x, ispDipPipe.srz1Cfg.crop.floatX, ispDipPipe.srz1Cfg.crop.y, ispDipPipe.srz1Cfg.crop.floatY,\
                        ispDipPipe.srz1Cfg.crop.w, ispDipPipe.srz1Cfg.crop.h, ispDipPipe.srz1Cfg.inout_size.out_w,ispDipPipe.srz1Cfg.inout_size.out_h);
                }
                break;
            case EDipModule_SRZ2:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    dip_ctl_yuv_en |=DIP_X_REG_CTL_YUV_EN_SRZ2;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_SRZ2_TCM_EN;
                    this->configSrz(&ispDipPipe.srz2Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ2:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d)",ispDipPipe.srz2Cfg.inout_size.in_w,ispDipPipe.srz2Cfg.inout_size.in_h,\
                        ispDipPipe.srz2Cfg.crop.x, ispDipPipe.srz2Cfg.crop.floatX, ispDipPipe.srz2Cfg.crop.y, ispDipPipe.srz2Cfg.crop.floatY,\
                        ispDipPipe.srz2Cfg.crop.w, ispDipPipe.srz2Cfg.crop.h, ispDipPipe.srz2Cfg.inout_size.out_w,ispDipPipe.srz2Cfg.inout_size.out_h);
                }
                break;

            case EDipModule_SRZ3:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ3);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ3_TCM_EN;
                    this->configSrz(&ispDipPipe.srz3Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ3:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d)",ispDipPipe.srz3Cfg.inout_size.in_w,ispDipPipe.srz3Cfg.inout_size.in_h,\
                        ispDipPipe.srz3Cfg.crop.x, ispDipPipe.srz3Cfg.crop.floatX, ispDipPipe.srz3Cfg.crop.y, ispDipPipe.srz3Cfg.crop.floatY,\
                        ispDipPipe.srz3Cfg.crop.w, ispDipPipe.srz3Cfg.crop.h, ispDipPipe.srz3Cfg.inout_size.out_w,ispDipPipe.srz3Cfg.inout_size.out_h);
                }
                break;
            case EDipModule_SRZ4:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ4);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ4_TCM_EN;
                    this->configSrz(&ispDipPipe.srz4Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ4:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d)",ispDipPipe.srz4Cfg.inout_size.in_w,ispDipPipe.srz4Cfg.inout_size.in_h,\
                        ispDipPipe.srz4Cfg.crop.x, ispDipPipe.srz4Cfg.crop.floatX, ispDipPipe.srz4Cfg.crop.y, ispDipPipe.srz4Cfg.crop.floatY,\
                        ispDipPipe.srz4Cfg.crop.w, ispDipPipe.srz4Cfg.crop.h, ispDipPipe.srz4Cfg.inout_size.out_w,ispDipPipe.srz4Cfg.inout_size.out_h);
                }
                break;
            case EDipModule_SRZ5:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ5);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ5_TCM_EN;

                    this->configSrz(&ispDipPipe.srz5Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ4:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d)",ispDipPipe.srz5Cfg.inout_size.in_w,ispDipPipe.srz5Cfg.inout_size.in_h,\
                        ispDipPipe.srz5Cfg.crop.x, ispDipPipe.srz5Cfg.crop.floatX, ispDipPipe.srz5Cfg.crop.y, ispDipPipe.srz5Cfg.crop.floatY,\
                        ispDipPipe.srz5Cfg.crop.w, ispDipPipe.srz5Cfg.crop.h, ispDipPipe.srz5Cfg.inout_size.out_w,ispDipPipe.srz5Cfg.inout_size.out_h);
                }
                break;

            default:
                PIPE_ERR("[Error]do not support this HWmodule(%d),i(%d)",pPipePackageInfo->vModuleParams[i].eDipModule,i);
                break;
        };
    }

    /////////////////////////////////////////////////////////////
    //judge format and multi-plane

    ufdi_en = (int)((dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_UFDI)>>3);
	if (ufdi_en !=0)
	{
    switch( portInfo_ufdi.eImgFmt )
    {
        //case eImgFmt_FG_BAYER8:     //TODO, revise for ufdi format
        case eImgFmt_UFO_FG:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
            ufdi_fmt=DIP_UFDI_FMT_UFO_LENGTH;
            if(pPipePackageInfo->pTuningQue != NULL)
            {
                dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UFD|DIP_X_REG_CTL_RGB_EN_UDM);
            }
            else
            {
                dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UFD|DIP_X_REG_CTL_RGB_EN_G2G|DIP_X_REG_CTL_RGB_EN_GGM|DIP_X_REG_CTL_RGB_EN_UDM);
            }
            tdr_tcm_en |= DIP_X_REG_CTL_TDR_UFD_TCM_EN|DIP_X_REG_CTL_TDR_UDM_TCM_EN;
            pgn_sel=1;
            break;
            case eImgFmt_STA_BYTE:
                ufdi_fmt=DIP_UFDI_FMT_WEIGHTING;
                break;
            default:
                PIPE_ERR("[Error] ufdi NOT Support this format(0x%x)",portInfo_ufdi.eImgFmt);
                return MFALSE;
        }
	}
    //
    vipxi_en = (int)((dip_ctl_dma_en&0x00000070)>>4);
	if (vipxi_en!=0)
	{
        switch( portInfo_vipi.eImgFmt )
        {
            case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
                vipi_fmt = DIP_VIPI_FMT_YUV420_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_C02);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02_TCM_EN;
                if(vipxi_en != 3)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
                vipi_fmt = DIP_VIPI_FMT_YUV420_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_C02);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02_TCM_EN;
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i+vip3i when 3-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:      //= 0x00001000,   //422 format, 1 plane (YUYV)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:      //= 0x00008000,   //422 format, 1 plane (UYVY)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:      //422 format, 3 plane
            case eImgFmt_I422:      //422 format, 3 plane
                vipi_fmt = DIP_VIPI_FMT_YUV422_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR1);
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV16:      //422 format, 2 plane
                vipi_fmt = DIP_VIPI_FMT_YUV422_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR1);
                if(vipxi_en != 3)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_RGB565:    //= 0x00080000,   //RGB 565 (16-bit), 1 plane
                vipi_fmt = DIP_VIPI_FMT_RGB565;
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR2;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                g2g_sel=2;
                nbc_sel=1; //rgb in
                break;
            case eImgFmt_RGB888:    //= 0x00100000,   //RGB 888 (24-bit), 1 plane
                vipi_fmt = DIP_VIPI_FMT_RGB888;
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR2;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                g2g_sel=2;
                nbc_sel=1; //rgb in
                dmai_swap = 0; //0:RGB,2:BGR
                break;
            case eImgFmt_ARGB8888:   //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
                vipi_fmt = DIP_VIPI_FMT_XRGB8888;
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR2;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                g2g_sel=2;
                nbc_sel=1; //rgb in
                dmai_swap = 0;  //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;
            default:
                PIPE_ERR("[Error] vipxi NOT Support this format(0x%x)",portInfo_vipi.eImgFmt);
                return MFALSE;
        }
	}
    //
    imgxi_en = (int)((dip_ctl_dma_en&0x00000007));
    if (imgxi_en!=0)
	{
        switch( portInfo_imgi.eImgFmt )
        {
            case eImgFmt_FG_BAYER8:      //= 0x01000000,   //FG Bayer format, 8-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER8;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_FG_BAYER10:      //= 0x02000000,   //FG Bayer format, 10-bit
            case eImgFmt_UFO_FG_BAYER10:
                imgi_fmt = DIP_IMGI_FMT_BAYER10;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_FG_BAYER12:      //= 0x04000000,   //FG Bayer format, 12-bit
            case eImgFmt_UFO_FG_BAYER12:
                imgi_fmt = DIP_IMGI_FMT_BAYER12;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_FG_BAYER14:     //= 0x08000000,   //FG Bayer format, 14-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER14;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_BAYER8:    //= 0x00000001,   //Bayer format, 8-bit
            case eImgFmt_BAYER8_UNPAK:
                if (eImgFmt_BAYER8 == portInfo_imgi.eImgFmt)
                {
                imgi_fmt = DIP_IMGI_FMT_BAYER8;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER8_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        //TILE DRIVER don't check UNPAK, so unpak must be enabled when input is raw file.
                            dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                        }
                        else
                        {
                            dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                        }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_BAYER10:   //= 0x00000002,   //Bayer format, 10-bit
            case eImgFmt_BAYER10_UNPAK:
                if (eImgFmt_BAYER10 == portInfo_imgi.eImgFmt)
                {
                imgi_fmt = DIP_IMGI_FMT_BAYER10;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER10_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_BAYER12:   //= 0x00000004,   //Bayer format, 12-bit
            case eImgFmt_BAYER12_UNPAK:
                if (eImgFmt_BAYER12 == portInfo_imgi.eImgFmt)
                {
                imgi_fmt = DIP_IMGI_FMT_BAYER12;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER12_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_BAYER14:   //= 0x00000008,   //Bayer format, 14-bit
            case eImgFmt_BAYER14_UNPAK:
                if (eImgFmt_BAYER14 == portInfo_imgi.eImgFmt)
                {
                imgi_fmt = DIP_IMGI_FMT_BAYER14;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER14_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_BAYER10_MIPI:   //= 0x00000002,   //Bayer format, 10-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER10_MIPI;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                if(idx_ufdi <0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM);
                    }
                    else
                    {
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_UNP |DIP_X_REG_CTL_RGB_EN_UDM  | DIP_X_REG_CTL_RGB_EN_G2G |DIP_X_REG_CTL_RGB_EN_GGM);
                    }
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_UNP_TCM_EN | DIP_X_REG_CTL_TDR_UDM_TCM_EN;
                    pgn_sel=0;
                }
                g2c_sel=0;
                g2g_sel=0;
                break;
            case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
                imgi_fmt = DIP_IMGI_FMT_YUV420_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR2 | DIP_X_REG_CTL_YUV_EN_C02B);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02B_TCM_EN;
                g2c_sel=1;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                if(imgxi_en != 3)
                {
                    PIPE_ERR("should enable imgi+imgbi when 2-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
                imgi_fmt = DIP_IMGI_FMT_YUV420_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR2 | DIP_X_REG_CTL_YUV_EN_C02B);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02B_TCM_EN;
                g2c_sel=1;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                if(imgxi_en != 7)
                {
                    PIPE_ERR("should enable imgi+imgbi+imgci when 3-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:      //= 0x00001000,   //422 format, 1 plane (YUYV)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:      //= 0x00008000,   //422 format, 1 plane (UYVY)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
            	{
                	dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
            	}
                g2c_sel=1;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:      //422 format, 3 plane
            case eImgFmt_I422:
                imgi_fmt = DIP_IMGI_FMT_YUV422_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR2);
                g2c_sel=1;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                if(imgxi_en != 7)
                {
                    PIPE_ERR("should enable imgi+imgbi+imgci when 3-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV16:      //422 format, 2 plane
                imgi_fmt = DIP_IMGI_FMT_YUV422_2P;
                dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_PLNR2;
                //tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02B_TCM_EN;
                g2c_sel=1;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                if(imgxi_en != 3)
                {
                    PIPE_ERR("should enable imgi+imgbi when 2-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_RGB565:    //= 0x00080000,   //RGB 565 (16-bit), 1 plane
                imgi_fmt = DIP_IMGI_FMT_RGB565;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GGM;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR1;      
                g2g_sel=1;
                break;
            case eImgFmt_RGB888:    //= 0x00100000,   //RGB 888 (24-bit), 1 plane
                imgi_fmt = DIP_IMGI_FMT_RGB888;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GGM;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR1;          
                g2g_sel=1;
                dmai_swap = 0; //0:RGB,2:BGR
                break;
            case eImgFmt_ARGB8888:   //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
                imgi_fmt = DIP_IMGI_FMT_XRGB8888;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GGM;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR1;
                g2g_sel=1;
                dmai_swap = 0;  //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;
            case eImgFmt_RGB48:   //= 0x10000000,   //RGB121212 (48-bit), 1 plane 
                imgi_fmt = DIP_IMGI_FMT_RGB121212;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C;
                    dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_GGM |DIP_X_REG_CTL_RGB_EN_G2G);
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
                }
                dip_ctl_rgb_en |= DIP_X_REG_CTL_RGB_EN_GDR1;
                g2g_sel=1;
                dmai_swap = 0;  //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;

            case eImgFmt_STA_BYTE:
                break;
            case eImgFmt_NV21_BLK:  //= 0x00000200,   //420 format block mode, 2 plane (UV)
            case eImgFmt_NV12_BLK:  //= 0x00000080,   //420 format block mode, 2 plane (VU)
            case eImgFmt_JPEG:      //= 0x00000010,   //JPEG format
            default:
                PIPE_ERR("[Error] imgxi NOT Support this format(0x%x)",portInfo_imgi.eImgFmt);
				return MFALSE;
        }
    }
    //
    img2xo_en = (int)((dip_ctl_dma_en&0x00001800)>>11);
	if (img2xo_en !=0)
	{
    switch( portInfo_img2o.eImgFmt )
    {
        case eImgFmt_YUY2:      //= 0x00001000,   //422 format, 1 plane (YUYV)
            img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
            dmai_swap = 1;
            break;
        case eImgFmt_UYVY:      //= 0x00008000,   //422 format, 1 plane (UYVY)
            img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
            dmai_swap = 0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
            dmai_swap = 3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
            dmai_swap = 2;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            img2o_fmt = DIP_IMG2O_FMT_YUV422_2P;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW2);
            if(img2xo_en != 3)
            {
                PIPE_ERR("should enable img2o+img2bo when 2-plane input (0x%x)",img2xo_en);
                return MFALSE;
            }
            break;
        default:
            PIPE_ERR("[Error] img2o NOT Support this format(0x%x)",portInfo_img2o.eImgFmt);
            return MFALSE;
    }
	}
    //
    img3xo_en = (int)((dip_ctl_dma_en&0x0000e000)>>13);
	if (img3xo_en !=0)
	{
    switch( portInfo_img3o.eImgFmt )
    {
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
            img3o_fmt = DIP_IMG3O_FMT_YUV420_2P;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_CRSP);
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
            tdr_tcm2_en |= DIP_X_REG_CTL_TDR_CRSP_TCM_EN;
            if(img3xo_en != 3)
            {
                PIPE_ERR("should enable img3o+img3bo when 2-plane input (0x%x)",img3xo_en);
                return MFALSE;
            }
            break;
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
            img3o_fmt = DIP_IMG3O_FMT_YUV420_3P;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_CRSP);
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
            tdr_tcm2_en |= DIP_X_REG_CTL_TDR_CRSP_TCM_EN;
            if(img3xo_en != 7)
            {
                PIPE_ERR("should enable img3o+img3bo+img3co when 3-plane input (0x%x)",img3xo_en);
                return MFALSE;
            }
            break;
        case eImgFmt_YUY2:      //= 0x00001000,   //422 format, 1 plane (YUYV)
            img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
            dmai_swap = 1;
            break;
        case eImgFmt_UYVY:      //= 0x00008000,   //422 format, 1 plane (UYVY)
            img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
            dmai_swap = 0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
            dmai_swap = 3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
            dmai_swap = 2;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_I422:
            img3o_fmt = DIP_IMG3O_FMT_YUV422_3P;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
            if(img3xo_en != 7)
            {
                PIPE_ERR("should enable img3o+img3bo+img3co when 3-plane input (0x%x)",img3xo_en);
                return MFALSE;
            }
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            img3o_fmt= DIP_IMG3O_FMT_YUV422_2P;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
            if(img3xo_en != 3)
            {
                PIPE_ERR("should enable img3o+img3bo when 2-plane input (0x%x)",img3xo_en);
                return MFALSE;
            }
            break;
        default:
            PIPE_ERR("[Error] vipxi NOT Support this format(0x%x)",portInfo_vipi.eImgFmt);
            return MFALSE;
    }
    }


    mfbo_en = (int)((dip_ctl_dma_en&0x00000400)>>10);
    if (mfbo_en !=0)
    {
        if (drvScen != eDrvScenario_MFB_Blending)
        {
            dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_RCP2 | DIP_X_REG_CTL_RGB_EN_PAK2);
        }

        switch( portInfo_mfbo.eImgFmt )
        {
            case eImgFmt_BAYER8:    //= 0x00000001,   //Bayer format, 8-bit
                mfbo_fmt = DIP_IMGI_FMT_BAYER8;
                break;
            case eImgFmt_BAYER8_UNPAK:
                mfbo_fmt = DIP_IMGI_FMT_BAYER8_2BYTEs;
                break;
            case eImgFmt_BAYER10:   //= 0x00000002,   //Bayer format, 10-bit
                mfbo_fmt = DIP_IMGI_FMT_BAYER10;
                break;
            case eImgFmt_BAYER10_UNPAK:
                mfbo_fmt = DIP_IMGI_FMT_BAYER10_2BYTEs;
                break;
            case eImgFmt_BAYER12:   //= 0x00000004,   //Bayer format, 12-bit
                mfbo_fmt = DIP_IMGI_FMT_BAYER12;
                break;
            case eImgFmt_BAYER12_UNPAK:
                mfbo_fmt = DIP_IMGI_FMT_BAYER12_2BYTEs;
                break;                
            case eImgFmt_BAYER14:   //= 0x00000008,   //Bayer format, 14-bit
                mfbo_fmt = DIP_IMGI_FMT_BAYER14;
                break;
            case eImgFmt_BAYER14_UNPAK:
                mfbo_fmt = DIP_IMGI_FMT_BAYER14_2BYTEs;
                break;
            case eImgFmt_STA_BYTE:
                mfbo_fmt = DIP_IMGI_FMT_BAYER8;
                break;
            default:
                PIPE_ERR("[Error] mfbo NOT Support this format(0x%x)",portInfo_mfbo.eImgFmt);
                return MFALSE;
        }


    }
    /////////////////////////////////////////////////////////////
    //judge module enable and mux_sel based on different path
    switch (drvScen)
    {
        case eDrvScenario_P2A:
        case eDrvScenario_DeNoise:
            /*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=2;
	- nr3d_sel=1;
	- mdp_sel=0;
	- fe_sel=1;
	- crsp_sel=0;
	*/
            if (-1 != idx_mfbo)
            {
                dip_misc_en |= (0x110 | (mfbo_fmt << 12));  //MFBO_SEL, 1 : from after PAK2
                if ( eDrvScenario_DeNoise == drvScen )
                {
                    rcp2_sel = 0x0; // 0 : from after PGN,    1 : from after LSC2,      2 : from before LSC2
                }
                else
                {
                    rcp2_sel = 0x1; // 0 : from after PGN,    1 : from after LSC2,      2 : from before LSC2
                }
                dip_ctl_path_sel = 0x00036040;
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_RCP2_TCM_EN;
            }
            else
            {
                dip_ctl_path_sel = 0x00016040;
            }
            break;
        case eDrvScenario_VFB_FB:
            /*
	- sel notes
	- srz1_sel=0;
	- mix1_sel=1;
	- nbc_sel; (depend on vipi format)
	- nbcw_sel=0;
	- crz_sel=1;
	- nr3d_sel=1;
	- mdp_sel=0;
	- fe_sel=0;
	- crsp_sel=0;
	*/
            dip_ctl_path_sel = 0x00005100;
            dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MIX1|DIP_X_REG_CTL_YUV_EN_MIX2);
            break;
        case eDrvScenario_P2B_Bokeh:
            /*
    - sel notes
    - srz1_sel=0;
    - mix1_sel=1;
    - nbc_sel; (depend on vipi format)
    - nbcw_sel=0;
    - crz_sel=1;
    - nr3d_sel=1;
    - mdp_sel=0;
    - fe_sel=0;
    - crsp_sel=0;
    */
            dip_ctl_path_sel = 0x00005100;
            break;
        case eDrvScenario_MFB_Blending:
            /*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=2;
	- nr3d_sel=2;
	- mdp_sel=0;
	- fe_sel=0;
	- crsp_sel=0;
	*/
            imgi_fmt = DIP_IMGI_FMT_MFB_BLEND_MODE;
            dip_misc_en = ((dip_misc_en & (0xffffff0f)) | 0x20);
            dip_ctl_path_sel = 0x00000010;
            #if 1
            dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MFB;
            //dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_G2C| DIP_X_REG_CTL_YUV_EN_MFB;
            //tdr_tcm_en |= DIP_X_REG_CTL_TDR_G2C_TCM_EN;
            #else
            dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MFB;
            #endif
            tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MFB_TCM_EN;
            break;
        case eDrvScenario_MFB_Mixing:
            /*
	- sel notes
	- srz1_sel=0;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=1;
	- crz_sel=0;
	- nr3d_sel=0;
	- mdp_sel=0;
	- fe_sel=0;
	- crsp_sel=0;
	*/
	  imgi_fmt = DIP_IMGI_FMT_MFB_MIX_MODE;
            dip_ctl_path_sel = 0x00000040;
            //dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MFBW|DIP_X_REG_CTL_YUV_EN_MIX3);//remove mfbw sinclue the algo don't use MFBW anymore.
            dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MIX3;
            break;
        case eDrvScenario_VSS:
            /*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=2;
	- nr3d_sel=2;
	- mdp_sel=0;
	- fe_sel=0;
	- crsp_sel=0;
	*/
            dip_ctl_path_sel = 0x00006040;
            isWaitBuf=MFALSE;
            break;
        case eDrvScenario_FE:
	/*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=0;
	- nr3d_sel=0;
	- mdp_sel=0;
	- fe_sel=1;
	- crsp_sel=0;
	*/
	dip_ctl_path_sel = 0x00010040;
            break;
        case eDrvScenario_FM:
            //TODO
            //dip_ctl_path_sel = 0x00006040;
            dip_ctl_yuv2_en |= DIP_X_REG_CTL_YUV2_EN_FM;
            dip_misc_en |= 0x230;
            dmgi_fmt=DIP_DMGI_FMT_FM;
            depi_fmt=DIP_DEPI_FMT_FM;
            tpipe = CAM_MODE_FRAME;
            break;
        case eDrvScenario_Color_Effect: 
            /*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=2;
	- nr3d_sel=1;
	- mdp_sel=0;
	- fe_sel=1;
	- crsp_sel=0;
	*/
            dip_ctl_path_sel = 0x00016040;
            //
            g2g_sel=3;
            g2c_sel=0;
            break;
        case eDrvScenario_Y16_Dump:
            /*
	- sel notes
	- srz1_sel=1;
	- mix1_sel=0;
	- nbc_sel=0;
	- nbcw_sel=0;
	- crz_sel=2;
	- nr3d_sel=1;
	- mdp_sel=0;
	- fe_sel=1;
	- crsp_sel=0;
	- feo_sel=1;
	*/
            dip_ctl_path_sel = 0x00816040;
            feo_sel=1;
            break;
        default:
            PIPE_ERR("NOT Support scenario(%d)",drvScen);
            break;
    }
	//Check CRZ is enable or not.
    if (MTRUE == isCropG1En)
    {
        if ( -1 != idx_img2o)
    	{
    		m_CrzEn = MTRUE;
        }
    	else
    	{
    		if ((dip_ctl_path_sel & 0x4000) == 0x0)//NR3D_SEL = 0
    		{
    			if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
    			{
    				m_CrzEn = MTRUE;
    			}
    		}
    		else
    		{
    			if ((dip_ctl_path_sel & 0x100) == 0x100)//MIX1_SEL = 1
    			{
    				if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
    				{
    					m_CrzEn = MTRUE;
    				}
    			}
    		}
    	}
    }
    if (m_CrzEn == MTRUE)
    {
        dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_CRZ;
        tdr_tcm2_en |= DIP_X_REG_CTL_TDR_CRZ_TCM_EN;

    }
    PIPE_DBG("isCropG1En(%d),m_CrzEn(%d)",isCropG1En,m_CrzEn);
    //check mdp src crop information
    mdpCheck1 = 0;
    mdpCheck2 = 0;
    mdpCheck3 = 0;
    crop1TableIdx = -1;
    tableSize = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    for(i=0;i<tableSize;i++) {
        if(mCropPathInfo[i].u4DrvScenId==drvScen && mCropPathInfo[i].u4CropGroup==ECropGroupIndex_1){
            mdpCheck1 = 1;
            crop1TableIdx = i;
        }
        if(mCropPathInfo[i].u4DrvScenId==drvScen && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_2)){
            mdpCheck2 = 1;
        }

        if(mCropPathInfo[i].u4DrvScenId==drvScen && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_3)){
            mdpCheck3 = 1;
        }
    }
    PIPE_DBG("drvScen(%d),mdpCheck1(%d),mdpCheck2(%d),mdpCheck3(%d),crop1TableIdx(%d)",drvScen,mdpCheck1,mdpCheck2,mdpCheck3,crop1TableIdx);

    if((mdpCheck1 && mdpCheck2) || (mdpCheck1 && mdpCheck3)){
        MBOOL isMdpSrcCropBeSet;
        //
        isMdpSrcCropBeSet = MFALSE;
        for(i=0;i<DMA_OUT_PORT_NUM;i++){
            switch(mCropPathInfo[crop1TableIdx].u4PortID[i]){
                case EPortIndex_IMG2O:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_IMG2O){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcW = portInfo_img2o.u4ImgWidth;
                        ispMdpPipe.p2MdpSrcH = portInfo_img2o.u4ImgHeight;
                        ispMdpPipe.p2MdpSrcFmt = portInfo_img2o.eImgFmt;
                        switch(portInfo_img2o.eImgFmt){
                            case eImgFmt_NV16:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_img2o.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_img2bo.u4ImgHeight * portInfo_img2bo.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_img2bo.u4ImgHeight * portInfo_img2bo.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_IMG3O:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_IMG3O){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcW = portInfo_img3o.u4ImgWidth;
                        ispMdpPipe.p2MdpSrcH = portInfo_img3o.u4ImgHeight;
                        ispMdpPipe.p2MdpSrcFmt = portInfo_img3o.eImgFmt;
                        switch(portInfo_img3o.eImgFmt){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_img3o.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_img3o.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_FEO:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_FEO){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcW = portInfo_feo.u4ImgWidth;
                        ispMdpPipe.p2MdpSrcH = portInfo_feo.u4ImgHeight;
                        ispMdpPipe.p2MdpSrcYStride = portInfo_feo.u4Stride[ESTRIDE_1ST_PLANE];
                        ispMdpPipe.p2MdpSrcUVStride = 0; // only one plane
                        ispMdpPipe.p2MdpSrcFmt = portInfo_imgi.eImgFmt; //kk test
                    }
                    break;
                case EPortIndex_VIPI:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_VIPI){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcFmt = portInfo_vipi.eImgFmt;
                        ispMdpPipe.p2MdpSrcW = portInfo_vipi.u4ImgWidth;
                        ispMdpPipe.p2MdpSrcH = portInfo_vipi.u4ImgHeight;
                        switch(portInfo_vipi.eImgFmt){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            case eImgFmt_YV16:
                            case eImgFmt_I422:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcSize = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                ispMdpPipe.p2MdpSrcCSize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                ispMdpPipe.p2MdpSrcVSize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            if(isMdpSrcCropBeSet == MTRUE)
                break;
        }


        if(isMdpSrcCropBeSet == MFALSE){
            //set mdp src info. equal to imgi
            ispMdpPipe.p2MdpSrcW = portInfo_imgi.u4ImgWidth;
            ispMdpPipe.p2MdpSrcH = portInfo_imgi.u4ImgHeight;
            ispMdpPipe.p2MdpSrcFmt = portInfo_imgi.eImgFmt;
            switch(portInfo_imgi.eImgFmt){
                case eImgFmt_NV21:
                case eImgFmt_NV12:
                case eImgFmt_NV16:
                    ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                    ispMdpPipe.p2MdpSrcPlaneNum = 2;
                    break;
                case eImgFmt_YV12:
                case eImgFmt_I420:
                    ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                    ispMdpPipe.p2MdpSrcPlaneNum = 3;
                    break;
                case eImgFmt_YV16:
                case eImgFmt_I422:
                    ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                    ispMdpPipe.p2MdpSrcPlaneNum = 3;
                    break;
                default:
                    ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcUVStride = 0;
                    ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                    ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                    ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                    ispMdpPipe.p2MdpSrcPlaneNum = 1;
                    break;
            }
        }
    } else {
        //set mdp src info. equal to imgi       
        ispMdpPipe.p2MdpSrcW = portInfo_imgi.u4ImgWidth;
        ispMdpPipe.p2MdpSrcH = portInfo_imgi.u4ImgHeight;
        ispMdpPipe.p2MdpSrcFmt = portInfo_imgi.eImgFmt;
        switch(portInfo_imgi.eImgFmt){
            case eImgFmt_NV21:
            case eImgFmt_NV12:
            case eImgFmt_NV16:
                ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                break;
            case eImgFmt_YV12:
            case eImgFmt_I420:
                ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                break;
            case eImgFmt_YV16:
            case eImgFmt_I422:
                ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcUVStride = portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                break;
            default:
                ispMdpPipe.p2MdpSrcYStride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcUVStride = 0;
                ispMdpPipe.p2MdpSrcSize = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
                ispMdpPipe.p2MdpSrcCSize = portInfo_imgbi.u4ImgHeight * portInfo_imgbi.u4Stride[ESTRIDE_2ND_PLANE];
                ispMdpPipe.p2MdpSrcVSize = portInfo_imgci.u4ImgHeight * portInfo_imgci.u4Stride[ESTRIDE_3RD_PLANE];
                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                break;
            }
    }




    ///////////////////////////////////////////;//////////////////
    //information collection
    ispDipPipe.hwModule=DIP_A; //TODO, need pass from iopipe
    PIPE_DBG("hwModule(%d),dip_ctl_dma_en(0x%x)",ispDipPipe.hwModule,dip_ctl_dma_en);
    ispDipPipe.moduleIdx=DIP_A-DIP_START; //TODO, need pass from iopipe
    if(tpipe == CAM_MODE_TPIPE) {
        ispDipPipe.isDipOnly=MFALSE;
        tdr_ctl |= DIP_X_REG_CTL_TDR_RN | DIP_X_REG_CTL_CTL_EXTENSION_EN; //enable tpipe extension in default
    } else {
        ispDipPipe.isDipOnly=MTRUE;
        tdr_tpipe = 0x0f;//TILE EDGE must be set to 1 in frame mode
    }
    //involve tuning setting
    if(pPipePackageInfo->pTuningQue != NULL){
    	ispDipPipe.isApplyTuning = MTRUE;
    	ispDipPipe.pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;// check tuning enable bit on isp_function_dip
	    yuv_en = ispDipPipe.pTuningIspReg->DIP_X_CTL_YUV_EN.Raw;
        yuv2_en = ispDipPipe.pTuningIspReg->DIP_X_CTL_YUV2_EN.Raw;
        rgb_en = ispDipPipe.pTuningIspReg->DIP_X_CTL_RGB_EN.Raw;
        b12bitMode = ispDipPipe.pTuningIspReg->DIP_X_CTL_FMT_SEL.Bits.LP_MODE;
            //error handle 
            if(drvScen == eDrvScenario_Color_Effect)
            {
                if((yuv_en & 0x00000008) >> 3)
                {}
                else
                {
                    PIPE_ERR("G2C should be enabled in color effect , tuning buffer yuven(0x%x)",yuv_en);
                    return MFALSE;
                }
            } 
            //update tcm
            tdr_tcm_en |= (yuv_en & DIP_X_REG_CTL_YUV_EN_G2C) << 24; //G2C
            tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_LSC2) << 6; //LSC2
            tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_SL2) << 19; //SL2
            tdr_tcm_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_SL2B|DIP_X_REG_CTL_YUV_EN_SL2C|DIP_X_REG_CTL_YUV_EN_SL2D|DIP_X_REG_CTL_YUV_EN_SL2E)) << 12; //SL2B~SL2E
            tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_LCE) << 26; //LCE
            tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_FLC) << 5; //FLC
            tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_FLC2) >> 24; //FLC2
            //
            tdr_tcm2_en |= (yuv_en & DIP_X_REG_CTL_YUV_EN_NR3D) >> 8; //NR3D
            tdr_tcm2_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_PCA|DIP_X_REG_CTL_YUV_EN_SEEE)) << 4; //PCA, SEEE
            tdr_tcm2_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_NBC|DIP_X_REG_CTL_YUV_EN_NBC2)) << 8; //NBC, NBC2
            tdr_tcm2_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_DBS2) << 5; //DBS2
            tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_RMG2|DIP_X_REG_CTL_RGB_EN_BNR2|DIP_X_REG_CTL_RGB_EN_RMM2)) << 4; //RMG2, BNR2, RMM2
            tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_RNR|DIP_X_REG_CTL_RGB_EN_SL2G|DIP_X_REG_CTL_RGB_EN_SL2H)) << 5; //RNR, SL2G, SL2H
            tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_HLR2|DIP_X_REG_CTL_RGB_EN_SL2K)) << 7; //HLR2, SL2K
            tdr_tcm2_en |= (dip_ctl_dma_en & (DIP_X_REG_CTL_DMA_EN_ADL2)) << 13; //ADL2
            tdr_tcm2_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_HFG) << 23;
            tdr_tcm2_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_SL2I) << 21;
            tdr_tcm_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_NDG) >> 6;
            tdr_tcm_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_NDG2) << 21;



    	PIPE_DBG("pIspPhyReg(0x%lx)",(long)ispDipPipe.pTuningIspReg);
    } else {
    	ispDipPipe.isApplyTuning = MFALSE;
    	PIPE_WRN("[Warning]p2 tuning not be passed via imageio");
    }

    ispMdpPipe.drvScenario = drvScen;
    ispDipPipe.dupCqIdx = dupCQIdx;
    ispDipPipe.burstQueIdx = burstQIdx;
    ispMdpPipe.lastframe = lastFrame;
    ispMdpPipe.isWaitBuf = isWaitBuf;
    ispDipPipe.dipTh=dip_cq_th;
    ispDipPipe.dip_cq_thr_ctl=dip_cq_thr_ctl;

    if(pPipePackageInfo->pTuningQue != NULL)
    {
        ispDipPipe.isp_top_ctl.YUV_EN.Raw = (dip_ctl_yuv_en &(~dip_yuv_tuning_tag))|(yuv_en & dip_yuv_tuning_tag);
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = (dip_ctl_yuv2_en &(~dip_yuv2_tuning_tag))|(yuv2_en & dip_yuv2_tuning_tag);
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = (dip_ctl_rgb_en &(~dip_rgb_tuning_tag))|(rgb_en & dip_rgb_tuning_tag);
    }
    else
    {
        ispDipPipe.isp_top_ctl.YUV_EN.Raw = dip_ctl_yuv_en;
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = dip_ctl_yuv2_en;
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = dip_ctl_rgb_en;
    }

    ispDipPipe.isp_top_ctl.DMA_EN.Raw=dip_ctl_dma_en;
    ispDipPipe.isp_top_ctl.MISC_EN.Raw=dip_misc_en;
    ispDipPipe.isp_top_ctl.FMT_SEL.Raw=0x0;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMGI_FMT=imgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.VIPI_FMT=vipi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.UFDI_FMT=ufdi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DMGI_FMT=dmgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DEPI_FMT=depi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.FG_MODE=fg_mode;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMG3O_FMT=img3o_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMG2O_FMT=img2o_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.PIX_ID=pPipePackageInfo->pixIdP2;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.LP_MODE = b12bitMode;
    ispDipPipe.isp_top_ctl.PATH_SEL.Raw=dip_ctl_path_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.G2G_SEL=g2g_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.G2C_SEL=g2c_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.PGN_SEL=pgn_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.NBC_SEL=nbc_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.RCP2_SEL=rcp2_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.FEO_SEL=feo_sel;

    //TODO items
    ispDipPipe.tdr_tcm_en=tdr_tcm_en;
    ispDipPipe.tdr_tcm2_en=tdr_tcm2_en;
    ispDipPipe.tdr_tpipe=tdr_tpipe;
    ispDipPipe.tdr_ctl=tdr_ctl;

    //in order to print the log, temp solution.
    if (m_Img3oCropInfo != NULL)
    {
        Img3oCrspTmp.m_CrspInfo.p_integral.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
        Img3oCrspTmp.m_CrspInfo.p_integral.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;        
    }
    if (pPipePackageInfo->pTuningQue != NULL)
    {
        pTuningDipReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),dupCqIdx(%d),[vipi]ofst(%d),rW/H(%d/%d)\n, \
            isDipOnly(%d), hwmodule(%d/%d), dipCQ/dup/burst(%d/%d/%d), drvSc(%d), isWtf(%d),tdr_tpipe(%d),en_yuv/yuv2/rgb/dma/fmt_sel/ctl_sel/misc (0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x), last(%d), CRZ_EN(%d),img3o ofset(%d,%d)",\
            portInfo_imgi.crop1.x,portInfo_imgi.crop1.floatX,portInfo_imgi.crop1.y,portInfo_imgi.crop1.floatY, \
            portInfo_imgi.crop1.w,portInfo_imgi.crop1.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h, \
            portInfo_imgi.crop2.x,portInfo_imgi.crop2.floatX,portInfo_imgi.crop2.y,portInfo_imgi.crop2.floatY, \
            portInfo_imgi.crop2.w,portInfo_imgi.crop2.h,portInfo_imgi.resize2.tar_w,portInfo_imgi.resize2.tar_h,
            portInfo_imgi.crop3.x,portInfo_imgi.crop3.floatX,portInfo_imgi.crop3.y,portInfo_imgi.crop3.floatY, \
            portInfo_imgi.crop3.w,portInfo_imgi.crop3.h,portInfo_imgi.resize3.tar_w,portInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx, \
            pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.hwModule, ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.dupCqIdx,ispDipPipe.burstQueIdx,\
            ispMdpPipe.drvScenario,ispMdpPipe.isWaitBuf,\
            ispDipPipe.tdr_tpipe,\
            ispDipPipe.isp_top_ctl.YUV_EN.Raw,ispDipPipe.isp_top_ctl.YUV2_EN.Raw,ispDipPipe.isp_top_ctl.RGB_EN.Raw,ispDipPipe.isp_top_ctl.DMA_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw,ispDipPipe.isp_top_ctl.PATH_SEL.Raw,ispDipPipe.isp_top_ctl.MISC_EN.Raw,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en,  lastFrame, m_CrzEn, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y);

    }
    else
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),dupCqIdx(%d)\n, \
            isDipOnly(%d), hwmodule(%d/%d), dipCQ/dup/burst(%d/%d/%d), drvSc(%d), isWtf(%d),tdr_tpipe(%d),en_yuv/yuv2/rgb/dma/fmt_sel/ctl_sel/misc (0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x), last(%d), CRZ_EN(%d),img3o ofset(%d,%d)",\
            portInfo_imgi.crop1.x,portInfo_imgi.crop1.floatX,portInfo_imgi.crop1.y,portInfo_imgi.crop1.floatY, \
            portInfo_imgi.crop1.w,portInfo_imgi.crop1.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h, \
            portInfo_imgi.crop2.x,portInfo_imgi.crop2.floatX,portInfo_imgi.crop2.y,portInfo_imgi.crop2.floatY, \
            portInfo_imgi.crop2.w,portInfo_imgi.crop2.h,portInfo_imgi.resize2.tar_w,portInfo_imgi.resize2.tar_h,
            portInfo_imgi.crop3.x,portInfo_imgi.crop3.floatX,portInfo_imgi.crop3.y,portInfo_imgi.crop3.floatY, \
            portInfo_imgi.crop3.w,portInfo_imgi.crop3.h,portInfo_imgi.resize3.tar_w,portInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.hwModule, ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.dupCqIdx,ispDipPipe.burstQueIdx,\
            ispMdpPipe.drvScenario,ispMdpPipe.isWaitBuf,\
            ispDipPipe.tdr_tpipe,\
            ispDipPipe.isp_top_ctl.YUV_EN.Raw,ispDipPipe.isp_top_ctl.YUV2_EN.Raw,ispDipPipe.isp_top_ctl.RGB_EN.Raw,ispDipPipe.isp_top_ctl.DMA_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw,ispDipPipe.isp_top_ctl.PATH_SEL.Raw,ispDipPipe.isp_top_ctl.MISC_EN.Raw,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en,  lastFrame, m_CrzEn, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y);
    }

    ///////////////////////////////////////////;//////////////////
    //config ports
    if (-1 != idx_imgi )
    {
        this->configDmaPort(&portInfo_imgi, ispDipPipe.DMAImgi.dma_cfg, (MUINT32)1, (MUINT32)dmai_swap, (MUINT32)1, ESTRIDE_1ST_PLANE);

        PIPE_DBG("[imgi]size[%lu,%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImgi.dma_cfg.size.w,ispDipPipe.DMAImgi.dma_cfg.size.h,ispDipPipe.DMAImgi.dma_cfg.size.xsize,
                ispDipPipe.DMAImgi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAImgi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgi.dma_cfg.size.stride);

        if (eDrvScenario_MFB_Blending == drvScen)
        {
            //
            //Blending
            if (-1 != idx_imgbi )
            {   //multi-plane input image, imgi/imgbi/imgci
                this->configDmaPort(&portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)2, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE);
                PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                    ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                    (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                    ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
            }


            if (-1 != idx_imgci )
            {
                this->configDmaPort(&portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE);
                PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",
                        ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
            }

            ispDipPipe.mfb_cfg.out_xofst=0;

            //TODO under check?
            if(idx_imgci != -1)
            {   //other blending sequences
                ispDipPipe.mfb_cfg.bld_mode=1;
            }
            else
            {   //first blending
                ispDipPipe.mfb_cfg.bld_mode=0;
            }
            //m_camPass2Param.mfb_cfg.bld_mode=0;
            ispDipPipe.mfb_cfg.bld_ll_db_en=0;
            ispDipPipe.mfb_cfg.bld_ll_brz_en=1;
      }
      else if (eDrvScenario_MFB_Mixing == drvScen)
      {
            if (-1 != idx_imgbi )
            {
                this->configDmaPort(&portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE);
                PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                    ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                    (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                    ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
            }

            if (-1 != idx_imgci )
            {
                this->configDmaPort(&portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE);
                PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
            }
      }
      else
      {

            if (-1 != idx_imgbi )
            {   //multi-plane input image, imgi/imgbi/imgci
                this->configDmaPort(&portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_2ND_PLANE);
                ispDipPipe.DMAImgbi.dma_cfg.size.w /= imgi_uv_h_ratio;
                ispDipPipe.DMAImgbi.dma_cfg.size.h /= imgi_uv_v_ratio;
                ispDipPipe.DMAImgbi.dma_cfg.size.xsize /= imgi_uv_h_ratio;
                PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                    ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                    (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                    ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);

                //TODO, imgbi would be weighting table in mixing path


                if (-1 != idx_imgci )
                {
                    this->configDmaPort(&portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE);
                    ispDipPipe.DMAImgci.dma_cfg.size.w /= imgi_uv_h_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.h /= imgi_uv_v_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.xsize /= imgi_uv_h_ratio;
                    if(imgi_uv_swap)
                    {
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr = portInfo_imgi.u4BufVA[ESTRIDE_3RD_PLANE];
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr = portInfo_imgi.u4BufPA[ESTRIDE_3RD_PLANE];
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr = portInfo_imgi.u4BufVA[ESTRIDE_2ND_PLANE];
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr = portInfo_imgi.u4BufPA[ESTRIDE_2ND_PLANE];
                    }
                    PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }
            }
            else
            {
                if (-1 != idx_imgci )
                {
                    //special case, imgci is weighting table
                    this->configDmaPort(&portInfo_imgci,ispDipPipe.DMAImgci.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
                    ispDipPipe.DMAImgci.dma_cfg.size.stride = portInfo_imgci.u4Stride[ESTRIDE_1ST_PLANE];
                }
            }
        }

		if (MTRUE == m_CrzEn)
		{
            MBOOL crzResult = MTRUE;   // MTRUE: success. MFALSE: fail.

			ispDipPipe.crzPipe.conf_cdrz = MTRUE;;
			ispDipPipe.crzPipe.crz_out.w = portInfo_imgi.resize1.tar_w;
			ispDipPipe.crzPipe.crz_out.h = portInfo_imgi.resize1.tar_h;
			ispDipPipe.crzPipe.crz_in.w = portInfo_imgi.u4ImgWidth;
			ispDipPipe.crzPipe.crz_in.h = portInfo_imgi.u4ImgHeight;
			ispDipPipe.crzPipe.crz_crop.x = portInfo_imgi.crop1.x;
			//ispDipPipe.crzPipe.crz_crop.floatX = portInfo_imgi.crop1.floatX;
			ispDipPipe.crzPipe.crz_crop.floatX = ((portInfo_imgi.crop1.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
			ispDipPipe.crzPipe.crz_crop.y = portInfo_imgi.crop1.y;
			//ispDipPipe.crzPipe.crz_crop.floatY = portInfo_imgi.crop1.floatY;
			ispDipPipe.crzPipe.crz_crop.floatY = ((portInfo_imgi.crop1.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
			ispDipPipe.crzPipe.crz_crop.w = portInfo_imgi.crop1.w;
			ispDipPipe.crzPipe.crz_crop.h = portInfo_imgi.crop1.h;

            crzResult = ispDipPipe.CalAlgoAndCStep((CRZ_DRV_MODE_ENUM)ispDipPipe.crzPipe.tpipeMode,
                                                              CRZ_DRV_RZ_CRZ,
                                                              ispDipPipe.crzPipe.crz_in.w,
                                                              ispDipPipe.crzPipe.crz_in.h,
                                                              ispDipPipe.crzPipe.crz_crop.w,
                                                              ispDipPipe.crzPipe.crz_crop.h,
                                                              ispDipPipe.crzPipe.crz_out.w,
                                                              ispDipPipe.crzPipe.crz_out.h,
                                                              (CRZ_DRV_ALGO_ENUM*)&ispDipPipe.crzPipe.hAlgo,
                                                              (CRZ_DRV_ALGO_ENUM*)&ispDipPipe.crzPipe.vAlgo,
                                                              &ispDipPipe.crzPipe.hTable,
                                                              &ispDipPipe.crzPipe.vTable,
                                                              &ispDipPipe.crzPipe.hCoeffStep,
                                                              &ispDipPipe.crzPipe.vCoeffStep);
            if(!crzResult)
            {
                PIPE_ERR(" crzPipe.CalAlgoAndCStep error");
                return MFALSE;
            }
		}
        else
        {
			ispDipPipe.crzPipe.conf_cdrz = MFALSE;
			ispDipPipe.crzPipe.crz_out.w = 0;
			ispDipPipe.crzPipe.crz_out.h = 0;
			ispDipPipe.crzPipe.crz_in.w = 0;
			ispDipPipe.crzPipe.crz_in.h = 0;
			ispDipPipe.crzPipe.crz_crop.x = 0;
			ispDipPipe.crzPipe.crz_crop.floatX = 0;
			ispDipPipe.crzPipe.crz_crop.y = 0;
			ispDipPipe.crzPipe.crz_crop.floatY = 0;
			ispDipPipe.crzPipe.crz_crop.w = 0;
			ispDipPipe.crzPipe.crz_crop.h = 0;        
        }
    }
    
    if ((-1 != idx_regi) && (drvScen == eDrvScenario_FM))
    {
        ispDipPipe.regCount = 0x1;
        ispDipPipe.pReadAddrList = &addrList;
        ispDipPipe.pRegiAddr  = portInfo_regi.u4BufVA[ESTRIDE_1ST_PLANE];
    }
    else
    {
        ispDipPipe.regCount = 0x0;
        ispDipPipe.pReadAddrList = &addrList;
        ispDipPipe.pRegiAddr  = 0x0;
    }

    //
    if (-1 != idx_vipi )
    {
        this->configDmaPort(&portInfo_vipi, ispDipPipe.DMAVipi.dma_cfg, (MUINT32)1, (MUINT32)1, (MUINT32)1, ESTRIDE_1ST_PLANE);

        PIPE_DBG("[vipi]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAVipi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAVipi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVipi.dma_cfg.memBuf.alignment,ispDipPipe.DMAVipi.dma_cfg.size.stride);

        if (-1 != idx_vip2i )
        {	//multi-plane input image, vipi/vip2i/vip3i
            this->configDmaPort(&portInfo_vip2i, ispDipPipe.DMAVip2i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE);
            ispDipPipe.DMAVip2i.dma_cfg.size.w /= vipi_uv_h_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.h /= vipi_uv_v_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
            PIPE_DBG("[vip2i]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAVip2i.dma_cfg.size.w,ispDipPipe.DMAVip2i.dma_cfg.size.h,
                    ispDipPipe.DMAVip2i.dma_cfg.memBuf.size,ispDipPipe.DMAVip2i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip2i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip2i.dma_cfg.size.stride);

            if (-1 != idx_vip3i )
            {
                this->configDmaPort(&portInfo_vip3i, ispDipPipe.DMAVip3i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE);
                ispDipPipe.DMAVip3i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.h /= vipi_uv_v_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
                if(vipi_uv_swap) {
                    ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_3RD_PLANE];
                    ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE];
                    ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_2ND_PLANE];
                    ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE];
                }
                PIPE_DBG("[vip3i]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAVip3i.dma_cfg.size.w,ispDipPipe.DMAVip3i.dma_cfg.size.h,
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip3i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip3i.dma_cfg.size.stride,vipi_uv_swap);
            }
        }

    }
    //
    if (-1 != idx_ufdi ) {
        PIPE_DBG("config ufdi");
        if ((portInfo_ufdi.eImgFmt == eImgFmt_UFO_FG_BAYER10) || (portInfo_ufdi.eImgFmt == eImgFmt_UFO_FG_BAYER12))
        {
            this->configDmaPort(&portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_2ND_PLANE);
            ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_2ND_PLANE];
            ispDipPipe.DMAUfdi.dma_cfg.size.xsize = (((portInfo_ufdi.u4ImgWidth+63)/64)+7)/8*8;
        }
        else
        {
            this->configDmaPort(&portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
            ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_1ST_PLANE];
        }
    }
    //
    if (-1 != idx_dmgi ) {
        //PIPE_DBG("config dmgi");
        this->configDmaPort(&portInfo_dmgi,ispDipPipe.DMADmgi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        ispDipPipe.DMADmgi.dma_cfg.size.stride = portInfo_dmgi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_depi ) {
        //PIPE_DBG("config ufdi");
        this->configDmaPort(&portInfo_depi,ispDipPipe.DMADepi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        ispDipPipe.DMADepi.dma_cfg.size.stride = portInfo_depi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lcei )
    {
        //PIPE_DBG("config lcei");
        this->configDmaPort(&portInfo_lcei,ispDipPipe.DMALcei.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        ispDipPipe.DMALcei.dma_cfg.size.stride = portInfo_lcei.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if ( -1 != idx_wroto) {
        //PIPE_DBG("config wroto");
        //support different view angle
        ispMdpPipe.wroto_out.enSrcCrop=true;
        ispMdpPipe.wroto_out.srcCropX = portInfo_imgi.crop3.x;
        ispMdpPipe.wroto_out.srcCropFloatX = \
            ((portInfo_imgi.crop3.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wroto_out.srcCropY= portInfo_imgi.crop3.y;
        ispMdpPipe.wroto_out.srcCropFloatY =  \
            ((portInfo_imgi.crop3.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wroto_out.srcCropW = portInfo_imgi.crop3.w;
        ispMdpPipe.wroto_out.srcCropH = portInfo_imgi.crop3.h;
        //
        this->configMdpOutPort(vOutPorts[idx_wroto],ispMdpPipe.wroto_out);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_wdmao) {
        //PIPE_DBG("config wdmao");
        //
        // for digital zoom crop
        ispMdpPipe.wdmao_out.enSrcCrop=true;
        ispMdpPipe.wdmao_out.srcCropX = portInfo_imgi.crop2.x;
        ispMdpPipe.wdmao_out.srcCropFloatX = \
                ((portInfo_imgi.crop2.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wdmao_out.srcCropY= portInfo_imgi.crop2.y;
        ispMdpPipe.wdmao_out.srcCropFloatY =  \
            ((portInfo_imgi.crop2.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wdmao_out.srcCropW = portInfo_imgi.crop2.w;
        ispMdpPipe.wdmao_out.srcCropH = portInfo_imgi.crop2.h;
        //
        this->configMdpOutPort(vOutPorts[idx_wdmao],ispMdpPipe.wdmao_out);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_jpego) {
        //PIPE_DBG("config jpego");
        //
        //this->configMdpOutPort(vOutPorts[idx_jpego],m_camPass2Param.jpego);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_venco) {
        //PIPE_DBG("config venco");
        // for digital zoom crop
        ispMdpPipe.venco_out.enSrcCrop=true;
        ispMdpPipe.venco_out.srcCropX = portInfo_imgi.crop2.x;
        ispMdpPipe.venco_out.srcCropFloatX = portInfo_imgi.crop2.floatX;
        ispMdpPipe.venco_out.srcCropY= portInfo_imgi.crop2.y;
        ispMdpPipe.venco_out.srcCropFloatY =  portInfo_imgi.crop2.floatY;
        ispMdpPipe.venco_out.srcCropW = portInfo_imgi.crop2.w;
        ispMdpPipe.venco_out.srcCropH = portInfo_imgi.crop2.h;
        //
        this->configMdpOutPort(vOutPorts[idx_venco],ispMdpPipe.venco_out);
        isSetMdpOut = MTRUE;
        //we do not support wdmao if user enque venco
        if( -1 != idx_wdmao)
        {
            PIPE_ERR("we do not support wdmao if user enque venco (%d/%d)",idx_wdmao,idx_venco);
            return MFALSE;
        }

    }
    //
    if (-1 != idx_img2o )
    {
        PIPE_DBG("img2o u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_img2o]->u4BufSize[0],(unsigned long)vOutPorts[idx_img2o]->u4BufVA[0],(unsigned long)vOutPorts[idx_img2o]->u4BufPA[0]);
        this->configDmaPort(&portInfo_img2o,ispDipPipe.DMAImg2o.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        ispDipPipe.DMAImg2o.dma_cfg.size.stride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        PIPE_DBG("[img2o]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2o.dma_cfg.size.w,ispDipPipe.DMAImg2o.dma_cfg.size.h,
                    ispDipPipe.DMAImg2o.dma_cfg.memBuf.size,ispDipPipe.DMAImg2o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2o.dma_cfg.size.stride);

        if (-1 != idx_img2bo )
        {	//multi-plane input image, img2o/img2bo
            this->configDmaPort(&portInfo_img2bo, ispDipPipe.DMAImg2bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE);
            ispDipPipe.DMAImg2bo.dma_cfg.size.w /=img2o_uv_h_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.h /= img2o_uv_v_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.xsize /= img2o_uv_h_ratio;
            PIPE_DBG("[img2bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2bo.dma_cfg.size.w,ispDipPipe.DMAImg2bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg2bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2bo.dma_cfg.size.stride);
        }

        if(isSetMdpOut == MFALSE)
        {
            this->configMdpOutPort(vOutPorts[idx_img2o],ispMdpPipe.imgxo_out);
            mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_IMG2O;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_img3o )
    {
        this->configDmaPort(&portInfo_img3o, ispDipPipe.DMAImg3o.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE);

        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(vOutPorts[idx_img3o],ispMdpPipe.imgxo_out);
            mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_IMG3O;
            isSetMdpOut = MTRUE;
        }
        PIPE_DBG("[img3o]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImg3o.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3o.dma_cfg.size.stride);

        if (-1 != idx_img3bo )
        {
            this->configDmaPort(&portInfo_img3bo, ispDipPipe.DMAImg3bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE);
            ispDipPipe.DMAImg3bo.dma_cfg.size.w /= img3o_uv_h_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.h /= img3o_uv_v_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.xsize /= img3o_uv_h_ratio;
            PIPE_DBG("[img3bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg3bo.dma_cfg.size.w,ispDipPipe.DMAImg3bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg3bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3bo.dma_cfg.size.stride);

            if (-1 != idx_img3co )
            {
                this->configDmaPort(&portInfo_img3co, ispDipPipe.DMAImg3co.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE);
                ispDipPipe.DMAImg3co.dma_cfg.size.w /= img3o_uv_h_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.h /= img3o_uv_v_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.xsize /= img3o_uv_h_ratio;
                if(img3o_uv_swap) {
                    ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_3RD_PLANE];
                    ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_3RD_PLANE];
                    ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_2ND_PLANE];
                    ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_2ND_PLANE];
                }
                PIPE_DBG("[img3co]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),img3o_uv_swap(%d)",ispDipPipe.DMAImg3co.dma_cfg.size.w,ispDipPipe.DMAImg3co.dma_cfg.size.h,
                        ispDipPipe.DMAImg3co.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3co.dma_cfg.size.stride,img3o_uv_swap);
            }
        }
    }
    //
    if (-1 != idx_mfbo)
    {
        this->configDmaPort(&portInfo_mfbo, ispDipPipe.DMAMfbo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE);
        //if(isSetMdpOut == MFALSE){
        //    this->configMdpOutPort(vOutPorts[idx_mfbo],ispMdpPipe.imgxo_out);
        //    mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_MFBO;
        //    isSetMdpOut = MTRUE;
        //}
    }
    if (-1 != idx_feo )
    {
        	PIPE_DBG("feo u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_feo]->u4BufSize[0],(unsigned long)vOutPorts[idx_feo]->u4BufVA[0],(unsigned long)vOutPorts[idx_feo]->u4BufPA[0]);
            this->configDmaPort(vOutPorts[idx_feo], ispDipPipe.DMAFeo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE);

            PIPE_DBG("[feo]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x)",
                ispDipPipe.DMAFeo.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAFeo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAFeo.dma_cfg.memBuf.alignment);

        	 if(isSetMdpOut == MFALSE)
        	 {
        	     this->configMdpOutPort(vOutPorts[idx_feo],ispMdpPipe.imgxo_out);
        	     mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_FEO;
        	     isSetMdpOut = MTRUE;
        	 }

    }
    ispMdpPipe.mdp_imgxo_p2_en = mdp_imgxo_p2_en;

//new add

//DMA
    ispDipPipe.DMAImgi.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMGI_EN)? 0 : 1;
    //dma_imgbi
    ispDipPipe.DMAImgbi.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMGBI_EN)? 0 : 1;
    //dma_imgci
    ispDipPipe.DMAImgci.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMGCI_EN)? 0 : 1;
    //dma_ufdi
    ispDipPipe.DMAUfdi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.UFDI_EN)? 0 : 1;
    //dma_lcei
    ispDipPipe.DMALcei.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.LCEI_EN)? 0 : 1;
    //dma_vipi
    ispDipPipe.DMAVipi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.VIPI_EN)? 0 : 1;
    //dma_vip2i
    ispDipPipe.DMAVip2i.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.VIP2I_EN)? 0 : 1;
    //dma_vip3i
    ispDipPipe.DMAVip3i.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.VIP3I_EN)? 0 : 1;
    //dma_depi
    ispDipPipe.DMADepi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DEPI_EN)? 0 : 1;
    //dma_dmgi
    ispDipPipe.DMADmgi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DMGI_EN)? 0 : 1;
    //dma_mfbo
    ispDipPipe.DMAMfbo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.MFBO_EN)? 0 : 1;
    //dma_img2o
    ispDipPipe.DMAImg2o.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMG2O_EN)? 0 : 1;
    //dma_img2bo
    ispDipPipe.DMAImg2bo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMG2BO_EN)? 0 : 1;
    //dma_img3o
    ispDipPipe.DMAImg3o.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMG3O_EN)? 0 : 1;
    //dma_img3bo
    ispDipPipe.DMAImg3bo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMG3BO_EN)? 0 : 1;
    //dma_img3co
    ispDipPipe.DMAImg3co.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.IMG3CO_EN)? 0 : 1;
    //dma_feo
    ispDipPipe.DMAFeo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.FEO_EN)? 0 : 1;
    //mdp_pipe
    ispMdpPipe.bypass = ispDipPipe.isDipOnly;
    ispMdpPipe.dipCtlDmaEn = ispDipPipe.isp_top_ctl.DMA_EN.Raw; //duplicate parameter
    ispMdpPipe.dip_dma_en = ispDipPipe.isp_top_ctl.DMA_EN.Raw;
    ispMdpPipe.imgi_dma_cfg = ispDipPipe.DMAImgi.dma_cfg;
    ispMdpPipe.yuv2_en = ispDipPipe.isp_top_ctl.YUV2_EN.Raw;

    PIPE_DBG("[ispMdpPipe]p2MdpSrcFmt(0x%x),w(%d),h(%d),stride(%d,%d),size(%d,%d,%d),planeN(%d)", \
        ispMdpPipe.p2MdpSrcFmt,ispMdpPipe.p2MdpSrcW,ispMdpPipe.p2MdpSrcH,ispMdpPipe.p2MdpSrcYStride,ispMdpPipe.p2MdpSrcUVStride,ispMdpPipe.p2MdpSrcSize,ispMdpPipe.p2MdpSrcCSize,ispMdpPipe.p2MdpSrcVSize,ispMdpPipe.p2MdpSrcPlaneNum);
    PIPE_DBG("ispDipPipe.isp_top_ctl.YUV_EN.Bits.CRZ_EN %d/%d",ispDipPipe.isp_top_ctl.YUV_EN.Bits.CRZ_EN, m_CrzEn);

    ret= ret & ispDipPipe.ConfigDipPipe();
    ret = ret & ispMdpPipe.configMDP();


    PIPE_DBG("configPipe ret(%d)",ret);
    return ret;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0;
    MUINT32 dmaChannel = 0;

    PIPE_DBG("tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%lx,0x%lx,0x%lx)", gettid(), cmd, (long)arg1, (long)arg2, (long)arg3);

    switch ( cmd ) {
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_NR3D_EN:
            m_Nr3dEn = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_NR3D_DMA_SEL:
            m_Nr3dDmaSel = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_CRZ_EN:
            m_CrzEn = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_VENC_DRLINK:
            ret=ispMdpPipe.startVencLink((MUINT32)arg1, (MUINT32)arg2, (MUINT32)arg3);
            if(ret==MFALSE)
            {
                PIPE_ERR("[Error]startVencLink fail");
                ret=-1;
            }
            else
            {
                ret=0;
            }
            break;
        case EPIPECmd_RELEASE_VENC_DRLINK:
            ret=ispMdpPipe.stopVencLink();
            if(ret==MFALSE)
            {
                PIPE_ERR("[Error]stopVencLink fail");
                ret=-1;
            }
            else
            {
                ret=0;
            }
            break;
        case EPIPECmd_SET_JPEG_CFG:
        case EPIPECmd_SET_JPEG_WORKBUF_SIZE:
        case EPIPECmd_GET_NR3D_GAIN:
        default:
            PIPE_ERR("NOT support command!(%d)",cmd);
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("sendCommand error!");
        return MFALSE;
    }
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL PostProcPipe::bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout)
{
    MBOOL ret=MTRUE;

    MUINT8* data = (MUINT8*)malloc(12 *sizeof(MUINT8));
    data[0]=static_cast<MUINT8>(cmd);
    data[1]=static_cast<MUINT8>(p2Eng);
    data[2]=static_cast<MUINT8>(0);
    data[3]=static_cast<MUINT8>(callerID);
    data[4]=static_cast<MUINT8>(callerID>>8);
    data[5]=static_cast<MUINT8>(callerID>>16);
    data[6]=static_cast<MUINT8>(callerID>>24);
    data[7]=static_cast<MUINT8>(frameNum);
    data[8]=static_cast<MUINT8>(p2CQ);
    data[9]=static_cast<MUINT8>(p2dupCQIdx);
    data[10]=static_cast<MUINT8>(p2burstQIdx);
    data[11]=static_cast<MUINT8>(timeout);

    PIPE_DBG("input(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",cmd,p2Eng,0,callerID,frameNum,p2CQ,p2dupCQIdx,p2burstQIdx,timeout);
    PIPE_DBG("data(%d_%d_%d_0x%x_%x_%x_%x_%d_%d_%d_%d_%d)",data[0],data[1],data[2],data[6],data[5],data[4],data[3],data[7],data[8],data[9],data[10],data[11]);

    ret=this->m_pIspDrvShell->m_pDrvDipPhy->setDeviceInfo(_SET_DIP_BUF_INFO, data);
    PIPE_DBG("cmd(%d),ret(%d)",cmd,ret);

    free(data);

    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configSrz(SrzCfg* pSrz_cf, _SRZ_SIZE_INFO_* pSrz_size_info, EDipModule srzEnum)
{
    MUINT32 ctrl=0x3;   //srz1_vert_en=1 & srz1_hori_en=1

    pSrz_cf->inout_size.out_w=pSrz_size_info->out_w;
    pSrz_cf->inout_size.out_h=pSrz_size_info->out_h;
    pSrz_cf->crop.x=pSrz_size_info->crop_x;
    pSrz_cf->crop.y=pSrz_size_info->crop_y;
    pSrz_cf->crop.w=pSrz_size_info->crop_w;
    pSrz_cf->crop.h=pSrz_size_info->crop_h;

    if(pSrz_size_info->crop_w>pSrz_size_info->in_w)
    {
        ctrl |= 0x10;
    }
    pSrz_cf->ctrl=ctrl;
    //update floating offset for tpipe /*20 bit base*/
    pSrz_cf->crop.floatX= \
        ((pSrz_size_info->crop_floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cf->crop.floatY= \
        ((pSrz_size_info->crop_floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    //coeffienct value for tpipe structure
    if (EDipModule_SRZ5 == srzEnum) //in order to padding in mfb
    {
        pSrz_cf->h_step=((pSrz_size_info->crop_w-1)*32768+(((((pSrz_size_info->out_w+31)>>5)<<5)-1)>>1) )/((((pSrz_size_info->out_w+31)>>5)<<5)-1);
        pSrz_cf->v_step=((pSrz_size_info->crop_h-1)*32768+(((((pSrz_size_info->out_h+31)>>5)<<5)-1)>>1) )/((((pSrz_size_info->out_h+31)>>5)<<5)-1);
        pSrz_cf->inout_size.in_w=((((long long)pSrz_cf->h_step*(pSrz_size_info->out_w-1))+16384)>>15)+1;
        pSrz_cf->inout_size.in_h=((((long long)pSrz_cf->v_step*(pSrz_size_info->out_h-1))+16384)>>15)+1;
    }
    else
    {
        pSrz_cf->h_step=(( pSrz_size_info->crop_w-1)*32768+((pSrz_size_info->out_w-1)>>1) )/(pSrz_size_info->out_w-1);
        pSrz_cf->v_step=(( pSrz_size_info->crop_h-1)*32768+((pSrz_size_info->out_h-1)>>1) )/(pSrz_size_info->out_h-1);
        pSrz_cf->inout_size.in_w=pSrz_size_info->in_w;
        pSrz_cf->inout_size.in_h=pSrz_size_info->in_h;
    }
    //
     PIPE_DBG("SRZ1:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%lu, %lu) out[%d,%d, cof(0x%x/0x%x)], srz:%d ",pSrz_cf->ctrl, pSrz_cf->inout_size.in_w, pSrz_cf->inout_size.in_h, \
        pSrz_cf->crop.x, pSrz_cf->crop.y, pSrz_cf->crop.floatX, pSrz_cf->crop.floatY, \
        pSrz_cf->crop.w, pSrz_cf->crop.h, pSrz_cf->inout_size.out_w, pSrz_cf->inout_size.out_h,\
        pSrz_cf->h_step, pSrz_cf->v_step, srzEnum);
    return MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum)
{
    ISP_QUERY_RST queryRst;
    E_ISP_PIXMODE e_PixMode = (portInfo->ePxlMode == ePxlMode_Two_)? (ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE);
    //
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
    //dma starting offset
    a_dma.offset.x          =  portInfo->xoffset;
    a_dma.offset.y          =  portInfo->yoffset;
    //
    #if 0
    a_dma.format = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte

    ISP_QuerySize(portInfo->index,ISP_QUERY_XSIZE_BYTE,(EImageFormat)portInfo->eImgFmt,a_dma.size.w,queryRst,e_PixMode);

    a_dma.size.xsize		=  queryRst.xsize_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
	a_dma.crop.x			= portInfo->crop1.x;
	a_dma.crop.floatX		= portInfo->crop1.floatX;
	a_dma.crop.y			= portInfo->crop1.y;
	a_dma.crop.floatY		= portInfo->crop1.floatY;
	a_dma.crop.w			= portInfo->crop1.w;
	a_dma.crop.h			= portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( portInfo->crop1.y ? (portInfo->crop1.y- 1) : 0 )*(portInfo->u4ImgWidth)+portInfo->crop1.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;
    //
    ISP_QueryBusSize((EImageFormat)portInfo->eImgFmt, a_dma.bus_size);

    switch( portInfo->eImgFmt ) {
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            a_dma.swap=1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            a_dma.swap=0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
        	a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            a_dma.swap=3;
        	break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
        	a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            a_dma.swap=2;
        	break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=2;
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            //a_dma.bus_size=3;
            break;
        case eImgFmt_RGB48:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            break;
        case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
        case eImgFmt_BAYER10_MIPI:	  /*!< Bayer format, 10-bit (MIPI) */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            //a_dma.bus_size=1;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV61:      //422 format, 2 plane    
        	PIPE_ERR("NOT support this format(0x%x) in ISP port",portInfo->eImgFmt);
        	break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
            a_dma.swap=2;  // 2 or 3 , For VU - BGR 
        case eImgFmt_NV16:      //422 format, 2 plane
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
        case eImgFmt_STA_BYTE:
        case eImgFmt_UFO_FG:
        default:
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            //a_dma.bus_size=0;
            break;
    }
    //

    PIPE_DBG("w(%lu),h(%lu),stride(%lu),xsize(0x%lx)/h(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%lx),pa(0x%lx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.size.h,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        (unsigned long)a_dma.memBuf.base_vAddr, (unsigned long)a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}


MBOOL
PostProcPipe::
configMdpOutPort(PortInfo const* oImgInfo, MdpRotDMACfg &a_rotDma)
{
    MUINT32 plane_num = 1;
    MUINT32 uv_resample = 1;
    MUINT32 uv_h_ratio = 1;
    MUINT32 uv_v_ratio = 1;
    MUINT32 y_plane_size = 0;
    MUINT32 u_plane_size = 0;
    //
    PIPE_DBG("[oImgInfo]w(%d),h(%d),stride(%d/%d/%d),crop(%d,%d,%d,%d)_f(0x%x, 0x%x),rot(%d) eImgFmt(%d)",
        oImgInfo->u4ImgWidth,oImgInfo->u4ImgHeight,oImgInfo->u4Stride[ESTRIDE_1ST_PLANE],oImgInfo->u4Stride[ESTRIDE_2ND_PLANE],oImgInfo->u4Stride[ESTRIDE_3RD_PLANE], \
        oImgInfo->crop1.x,oImgInfo->crop1.y,oImgInfo->crop1.w,oImgInfo->crop1.h,oImgInfo->crop1.floatX,oImgInfo->crop1.floatY, \
        oImgInfo->eImgRot,oImgInfo->eImgFmt );
    //
    a_rotDma.uv_plane_swap = 0;
    //
    switch( oImgInfo->eImgFmt ) {
       case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
       case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
           a_rotDma.Format = CRZ_DRV_FORMAT_YUV420;
           a_rotDma.Plane = CRZ_DRV_PLANE_3;
           a_rotDma.Sequence = CRZ_DRV_SEQUENCE_UVUV; //don't care, but cdp_drv need to set this value , why?
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 2;
           a_rotDma.uv_plane_swap = (eImgFmt_YV12==oImgInfo->eImgFmt)?1:0;
           break;
        case eImgFmt_I422:      //422 format, 3 plane(YUV)
           a_rotDma.Format = CRZ_DRV_FORMAT_YUV422;
           a_rotDma.Plane = CRZ_DRV_PLANE_3;
           a_rotDma.Sequence = CRZ_DRV_SEQUENCE_UVUV; //don't care, but cdp_drv need to set this value , why?
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 1;
           break;
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV420;
            a_rotDma.Plane = CRZ_DRV_PLANE_2;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_UVUV; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV420;
            a_rotDma.Plane = CRZ_DRV_PLANE_2;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_VUVU; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_VYUY; //MSN->LSB
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_YVYU; //MSN->LSB
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CRZ_DRV_PLANE_3;
            a_rotDma.Sequence = (CRZ_DRV_SEQUENCE_ENUM)0; //MSN->LSB
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            a_rotDma.Format = CRZ_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CRZ_DRV_PLANE_2;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_VUVU; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_rotDma.Format = CRZ_DRV_FORMAT_RGB565;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_RGB; //MSN->LSB
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_rotDma.Format = CRZ_DRV_FORMAT_RGB888;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_BGR; //MSN->LSB
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_rotDma.Format = CRZ_DRV_FORMAT_XRGB8888;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_XRGB; //MSN->LSB
            break;
        case eImgFmt_RGBA8888:
            a_rotDma.Format = CRZ_DRV_FORMAT_XRGB8888;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_RGBX; //MSN->LSB
            break;
        case eImgFmt_Y800:		//= 0x040000, //Y plane only
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y16:
            a_rotDma.Format = CRZ_DRV_FORMAT_Y;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_Y; //MSN->LSB
            break;
        case eImgFmt_JPG_I420:
            a_rotDma.Format = CRZ_DRV_FORMAT_JPEG_YUV420;
            a_rotDma.Plane = CRZ_DRV_PLANE_3;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_UVUV; //don't care,
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_JPG_I422:
            a_rotDma.Format = CRZ_DRV_FORMAT_JPEG_YUV422;
            a_rotDma.Plane = CRZ_DRV_PLANE_3;
            a_rotDma.Sequence = CRZ_DRV_SEQUENCE_UVUV; //don't care,
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit     // for imgo
            a_rotDma.Format = CRZ_DRV_FORMAT_RAW8;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            break;
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit    // for imgo
            a_rotDma.Format = CRZ_DRV_FORMAT_RAW10;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            break;
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit    // for imgo
            a_rotDma.Format = CRZ_DRV_FORMAT_RAW12;
            a_rotDma.Plane = CRZ_DRV_PLANE_1;
            break;
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        case eImgFmt_UFO_FG:
        default:
            PIPE_ERR("vOutPorts[]->eImgFmt:Format(%d) NOT Support",oImgInfo->eImgFmt);
            break;
    }
    //
    //ROTATION, stride is after, others are before
    a_rotDma.Rotation = (CRZ_DRV_ROTATION_ENUM)oImgInfo->eImgRot;
    a_rotDma.Flip = (eImgFlip_ON == oImgInfo->eImgFlip)?MTRUE:MFALSE;
    //dma port capbility
    a_rotDma.capbility=oImgInfo->capbility;
    //Y
    a_rotDma.memBuf.base_pAddr = oImgInfo->u4BufPA[0];
    a_rotDma.memBuf.base_vAddr = oImgInfo->u4BufVA[0];
    a_rotDma.memBuf.size = oImgInfo->u4BufSize[0];
    a_rotDma.memBuf.ofst_addr = 0;
    a_rotDma.memBuf.alignment = 0;
    //after ROT
    a_rotDma.size.w = oImgInfo->u4ImgWidth;
    a_rotDma.size.h = oImgInfo->u4ImgHeight;
    //stride info after ROT
    a_rotDma.size.stride = oImgInfo->u4Stride[ESTRIDE_1ST_PLANE];
    //
    y_plane_size = oImgInfo->u4BufSize[ESTRIDE_1ST_PLANE];
    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}
    //
    if ( 2<=plane_num) {
        //U
        a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.ofst_addr = 0;
        a_rotDma.memBuf_c.alignment = 0;
        //after ROT
        a_rotDma.size_c.w =         a_rotDma.size.w/uv_h_ratio;
        a_rotDma.size_c.h =         a_rotDma.size.h/uv_v_ratio;
        //stride info after ROT
        a_rotDma.size_c.stride =    oImgInfo->u4Stride[ESTRIDE_2ND_PLANE];
        //
        u_plane_size = oImgInfo->u4BufSize[ESTRIDE_2ND_PLANE];
        //tpipemain lib need image info before ROT. stride info after ROT
        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //    a_rotDma.size_c.w = a_rotDma.size.h/uv_v_ratio;
        //    a_rotDma.size_c.h = a_rotDma.size.w/uv_h_ratio;
        //}
        //
		if ( 3 == plane_num ) {
	        //V
	        a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
	        a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
	        a_rotDma.memBuf_v.ofst_addr = 0;
	        a_rotDma.memBuf_v.alignment = 0;
            //after ROT
	        a_rotDma.size_v.w =        a_rotDma.size_c.w;
	        a_rotDma.size_v.h =        a_rotDma.size_c.h;
            //stride info after ROT
	        a_rotDma.size_v.stride =   oImgInfo->u4Stride[ESTRIDE_3RD_PLANE];
	        //tpipemain lib need image info BEFORE ROT. stride info AFTER ROT
	        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
            //    a_rotDma.size_v.w = a_rotDma.size_c.h;
            //    a_rotDma.size_v.h = a_rotDma.size_c.w;
	        //}
		}
        //
        /*
        if ( a_rotDma.uv_plane_swap ) {
            a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        }
        */
    }

    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
    //    //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}

    /*
        * DONOT USE CRZ CROP due to throughtput issue
        */
    //default set ENABLE, if need to be programmable, check cdp driver as well.
    a_rotDma.crop_en = 1; //always enable crop for rotation issue.
    //
    a_rotDma.crop.x = 0;
    a_rotDma.crop.floatX = 0;
    a_rotDma.crop.y = 0;
    a_rotDma.crop.floatY = 0;
    a_rotDma.crop.w = 0;
    a_rotDma.crop.h = 0;
    //
    PIPE_DBG("[a_rotDma]w(%lu),h(%lu),stride(pxl)(%lu),pa(0x%lx),va(0x%lx),size(%d)",a_rotDma.size.w,a_rotDma.size.h,a_rotDma.size.stride,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf.size);
    PIPE_DBG("[a_rotDma]crop(%d,%d,%lu,%lu)_f(0x%x,0x%x)",a_rotDma.crop.x,a_rotDma.crop.y,a_rotDma.crop.w,a_rotDma.crop.h,a_rotDma.crop.floatX,a_rotDma.crop.floatY);
    PIPE_DBG("[a_rotDma]rot(%d),fmt(%d),plane(%d),seq(%d),pxlByte((0x%x))",a_rotDma.Rotation,a_rotDma.Format,a_rotDma.Plane,a_rotDma.Sequence,a_rotDma.pixel_byte);
    PIPE_DBG("[a_rotDma]c_ofst_addr(0x%x),v_ofst_addr(0x%x),uv_plane_swap(%d)",a_rotDma.memBuf_c.ofst_addr,a_rotDma.memBuf_v.ofst_addr,a_rotDma.uv_plane_swap);
    PIPE_DBG("[a_rotDma]va[0x%lx,0x%lx,0x%lx]--pa[0x%lx,0x%lx,0x%lx]",
        (unsigned long)a_rotDma.memBuf.base_vAddr,(unsigned long)a_rotDma.memBuf_c.base_vAddr,(unsigned long)a_rotDma.memBuf_v.base_vAddr,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf_c.base_pAddr,(unsigned long)a_rotDma.memBuf_v.base_pAddr);


    return MTRUE;
}



MUINT32
PostProcPipe::
queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum)
{
    MUINT32 CQset=0x0;
    MINT32 p2CQ=0;
    MUINT32 dupCQIdx=0;
    p2CQ=static_cast<MUINT32>(geDevScenCQMapping[sDrvScen][sensorDev]);
    PIPE_DBG("sDrvScen (%d), sensorDev(0x%x), p2CQ = 0x%x", sDrvScen, sensorDev, p2CQ);
    _debug_info_ debug;
    debug.eDrvScen=static_cast<MUINT32>(sDrvScen);
    debug.senDev=sensorDev;
    dupCQIdx = m_dipCmdQMgr->getDupCQIdx(p2CQ, p2burstQIdx, frameNum, debug);

    //
    CQset = (p2CQ<<16) | dupCQIdx;
    PIPE_DBG("CQset = 0x%x", CQset);

    return CQset;
}

MBOOL
PostProcPipe::
updateCQUser(MINT32 p2CQ)
{
    MBOOL ret=MTRUE;
    ret=m_dipCmdQMgr->releaseCQUser(p2CQ);
    return ret;
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

