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
#include <mtkcam/drv/def/ICam_type.h>
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

DECLARE_DBG_LOG_VARIABLE(PostProc );
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

#define MAX_LOG_SIZE    1024

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
    //WUV
    {eDrvScenario_WUV,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_WUV,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_WUV,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_WUV,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
};


#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h
TransformMapping mTransformMapping[TRANSFORM_MAX]=
{
    {0,                 NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageio::NSIspio::eImgRot_180,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_180,NSImageio::NSIspio::eImgRot_180,NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90, NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_270,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_270,NSImageio::NSIspio::eImgRot_270,NSImageio::NSIspio::eImgFlip_OFF}
};

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
      m_isImgPlaneByImgi(MFALSE),
      m_Nr3dEn(MFALSE),
      m_Nr3dDmaSel(MFALSE),
      m_CrzEn(MFALSE),
      m_SeeeEn(MFALSE),
      mp2CQDupIdx(0),
      pPortLogBuf(NULL),
      pMaxLogBuf(NULL),
      m_Img3oCropInfo(NULL),
      m_pDipWorkingBuf(NULL),
      m_pDequeDipWorkingBuf(NULL)
{

    //
    DBG_LOG_CONFIG(imageio, PostProc);

    m_pIspDrvShell = IspDrvShell::createInstance();

    this->m_dipCmdQMgr = DipCmdQMgr::createInstance();
    m_pDipRingBuf = DipRingBuffer::createInstance((DIP_HW_MODULE)DIP_HW_A);
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
init(char const* szCallerName, MUINT32 secTag)
{
    MBOOL ret=MTRUE;
    PIPE_INF("+, userName(%s), szCallerName(%s)",this->m_szUsrName, szCallerName);
    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init("PostProcPipe", secTag);
        m_pDipRingBuf = m_pIspDrvShell->getDipRingBufMgr();
        ispDipPipe.m_pIspDrvShell = this->m_pIspDrvShell;
    }

    if(m_dipCmdQMgr)
    {
        ret=m_dipCmdQMgr->init();
    }
    if (pPortLogBuf == NULL)
    {
        pPortLogBuf = (char *)malloc(MAX_LOG_SIZE);
    }
    if (pMaxLogBuf == NULL)
    {
        pMaxLogBuf = (char *)malloc(MAX_LOG_SIZE);
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
    if (pPortLogBuf != NULL)
    {
        free((void *)pPortLogBuf);
        pPortLogBuf = NULL;
    }
    if (pMaxLogBuf != NULL)
    {
        free((void *)pMaxLogBuf);
        pMaxLogBuf = NULL;
    }

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
MBOOL
PostProcPipe::
getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr, MBOOL& bDequeBufIsTheSameAaGCEDump)
{
    MUINTPTR dipwkbuf_cmdqaddr;
    bDequeBufIsTheSameAaGCEDump = MFALSE;
    if ( m_pIspDrvShell ) {
        this->m_pIspDrvShell->m_pDrvDipPhy->getDipDumpInfo(tdriaddr, cmdqaddr, imgiaddr);
        if(m_pDequeDipWorkingBuf != NULL)
        {
            PIPE_INF("tdriaddr(0x%x),cmdqaddr(0x%x),imgiaddr(0x%x),m_pIspDescript_phy(0x%x),tpipeTablePa(0x%x)!!\n", tdriaddr, cmdqaddr, imgiaddr,m_pDequeDipWorkingBuf->m_pIspDescript_phy,m_pDequeDipWorkingBuf->tpipeTablePa);

            dipwkbuf_cmdqaddr = (MUINTPTR)m_pDequeDipWorkingBuf->m_pIspDescript_phy;
            if ((dipwkbuf_cmdqaddr == cmdqaddr) && (m_pDequeDipWorkingBuf->tpipeTablePa == (unsigned long)tdriaddr))
            {
                bDequeBufIsTheSameAaGCEDump = MTRUE;
            }
        }
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getVssDipWBInfo(MUINTPTR& vsstdriaddr, MINT32 p2cqIdx, MINT32 p2RingBufIdx)
{
    DipWorkingBuffer* pDipWBuf;

    pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)p2cqIdx, p2RingBufIdx, MFALSE);

    if(pDipWBuf==NULL)
    {
        PIPE_ERR("getVssDipWBInfo:: getDipWorkingBufferbyIdx error!!\n");
        return MFALSE;
    }
    vsstdriaddr = (MUINTPTR)pDipWBuf->tpipeTableVa;

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
PostProcPipe::
dequeBuf(MBOOL& isVencContained, MUINTPTR& tpipeVa, MUINTPTR& ispVirRegVa, EDrvScenario eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const RingBufIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    int idex=0;
    EIspRetStatus ret;
    //
    ISP_TRACE_CALL();
    DipWorkingBuffer* pDipWBuf;
    //
    PIPE_DBG("dQDBGout path(%d)  cq=(%d, %d, %d), timeoutMs(%d)", eDrvSce, dequeCq,dequeDupCqIdx,dequeBurstQIdx,u4TimeoutMs);
    this->ispBufCtrl.m_BufdipTh = dequeCq;
    this->ispBufCtrl.m_BufdupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.m_BufburstQueIdx = dequeBurstQIdx;
    this->ispBufCtrl.m_BufdrvScenario = eDrvSce;
    this->ispBufCtrl.m_BufmoduleIdx = 0;//moduleIdx; hungwen

    PIPE_DBG("dequeCq(%d), dequeDupCqIdx(%d), RingBufIdx(%d)!!\n", dequeCq, dequeDupCqIdx, RingBufIdx);
    //Acquire the buffer from ring buffer
    //pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)dequeCq, dequeDupCqIdx);
    pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)dequeCq, RingBufIdx);

    if(pDipWBuf==NULL)
    {
        PIPE_ERR("getDipWorkingBufferbyIdx error!!\n");
        return MFALSE; //error command  enque
    }
    m_pDequeDipWorkingBuf = pDipWBuf;
    tpipeVa = (MUINTPTR)pDipWBuf->tpipeTableVa;
    ispVirRegVa = (MUINTPTR)pDipWBuf->m_pIspVirRegAddr_va;

    ret = this->ispBufCtrl.dequeHwBuf(isVencContained, pDipWBuf);

    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }

    if (ret == eIspRetStatus_Success){
        if(m_pDipRingBuf->ReleaseDipWorkingBuffer(pDipWBuf, (E_ISP_DIP_CQ)dequeCq)==false)
        {
            PIPE_ERR("ReleaseDipWorkingBuffer error!!\n");
            return MFALSE;
        }
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
PrintPipePortInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams)
{
    MINT32 planeNum=0;
    MINT32 portcnt =0;
    //char *pPortLogBuf = NULL;
    //char *pMaxLogBuf = NULL;
    unsigned int PortLogBufSize = MAX_LOG_SIZE;
    unsigned int MaxLogBufSize = MAX_LOG_SIZE;
    //pPortLogBuf = (char *)malloc(PortLogBufSize);
    //pMaxLogBuf = (char *)malloc(MaxLogBufSize);

    if (pPortLogBuf == NULL)
    {
        PIPE_ERR("pPortLogBuf is NULL!!!\n");
        return MFALSE;
    }
    if (pMaxLogBuf == NULL)
    {
        PIPE_ERR("pMaxLogBuf is NULL!!!\n");
        return MFALSE;
    }

    memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
    memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);

    snprintf(pPortLogBuf, PortLogBufSize,"\n");
    strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));

    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvIn[i] ) { continue; }
        //
        planeNum = pFrameParams.mvIn[i].mBuffer->getPlaneCount();
        switch (planeNum)
        {
            case 1:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,0,0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x)",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvIn[i].mPortID.index,
                                                            pFrameParams.mvIn[i].mPortID.inout,
                                                            pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0,0),idx(%d),dir(%d),Size(0x%lx,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x),secPort(%d)\n",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            pFrameParams.UniqueKey,
                                                            pFrameParams.mSecureFra,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvIn[i].mPortID.index,
                                                            pFrameParams.mvIn[i].mPortID.inout,
                                                            pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvIn[i].mOffsetInBytes,
                                                            pFrameParams.mvIn[i].mSecureTag);
                break;
            case 2:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,0),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0x%lx,0),idx(%d),dir(%d),Size(0x%lx,0x%lx,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes,
                                                                    pFrameParams.mvIn[i].mSecureTag);
                break;
            case 3:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0x%lx,0x%lx),idx(%d),dir(%d),Size(0x%lx,0x%lx,0x%lx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes,
                                                                    pFrameParams.mvIn[i].mSecureTag);
                break;
        }
        strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
        portcnt++;
        if (portcnt == 3)
        {
            PIPE_INF("%s",pMaxLogBuf);
            memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
            memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
            snprintf(pPortLogBuf, PortLogBufSize,"\n");
            strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
            portcnt = 0;
        }
        //
    }
    if (portcnt > 0)
    {
        PIPE_INF("%s",pMaxLogBuf);
        portcnt = 0;
    }
    memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
    memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
    snprintf(pPortLogBuf, PortLogBufSize,"\n");
    strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
    portcnt = 0;

    for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvOut[i] ) { continue; }
        //
        planeNum = pFrameParams.mvOut[i].mBuffer->getPlaneCount();
        switch (planeNum)
        {
            case 1:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,0,0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x)",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvOut[i].mPortID.index,
                                                            pFrameParams.mvOut[i].mPortID.inout,
                                                            pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0,0),idx(%d),dir(%d),Size(0x%lx,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x),secPort(%d)\n",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            pFrameParams.UniqueKey,
                                                            pFrameParams.mSecureFra,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvOut[i].mPortID.index,
                                                            pFrameParams.mvOut[i].mPortID.inout,
                                                            pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvOut[i].mOffsetInBytes,
                                                            pFrameParams.mvOut[i].mSecureTag);
                break;
            case 2:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,0),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0x%lx,0),idx(%d),dir(%d),Size(0x%lx,0x%lx,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes,
                                                                    pFrameParams.mvOut[i].mSecureTag);
                break;
            case 3:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(0x%lx,0x%lx,0x%lx),idx(%d),dir(%d),Size(0x%lx,0x%lx,0x%lx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes,
                                                                    pFrameParams.mvOut[i].mSecureTag);
                break;
        }
        strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
        portcnt++;
        if (portcnt == 3)
        {
            PIPE_INF("%s",pMaxLogBuf);
            memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
            memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
            snprintf(pPortLogBuf, PortLogBufSize,"\n");
            strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
            portcnt = 0;
        }

    }
    if (portcnt > 0)
    {
        PIPE_INF("%s",pMaxLogBuf);
        portcnt = 0;
    }


    //free((void *)pPortLogBuf);
    //free((void *)pMaxLogBuf);

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
DipRawPxlID
PostProcPipe::PixelIDMapping(MUINT32 pixIdP2)
{
    DipRawPxlID p2hwpixelId = DipRawPxlID_B;

    switch (pixIdP2)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
        {
         p2hwpixelId = DipRawPxlID_B;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gb:
        {
         p2hwpixelId = DipRawPxlID_Gb;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gr:
        {
         p2hwpixelId = DipRawPxlID_Gr;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_R:
        {
         p2hwpixelId = DipRawPxlID_R;
         break;
        }
        default:
        {
         break;
        }
    }
    return p2hwpixelId;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipe(const NSCam::NSIoPipe::FrameParams &pFrameParams, PipePackageInfo *pPipePackageInfo, char const* szCallerName)
{
    MBOOL ret = MTRUE;

    //judge drvscenaio and p2 cq
    EDrvScenario drvScen=pPipePackageInfo->drvScen;
    MINT32 p2CQ=pPipePackageInfo->p2cqIdx;
    MINT32 dupCQIdx=pPipePackageInfo->dupCqIdx;
    MINT32 burstQIdx=pPipePackageInfo->burstQIdx;
    MBOOL lastFrame=pPipePackageInfo->lastframe;
    MINT32 planeNum=0, imgi_planeNum=0;
    int bSmartTileEn;
    int bMix4En;
    NSCam::NSIoPipe::CrspInfo Img3oCrspTmp;
    MINT32 pixIdImgBuf=0;
    dip_x_reg_t* pTuningBuf = NULL;

    bSmartTileEn = ::property_get_int32("vendor.camera.smartile.enable", 1);
    bMix4En = ::property_get_int32("vendor.camera.mixingmodeusemix4.enable", 0);

    int dipConfigPipeLog = ::property_get_int32("vendor.camera.dipConfigPipeLog.enable", 1);
    int mtkcamLogLevel = ::property_get_int32("persist.vendor.mtk.camera.log_level", 0);

    // TODO: hungwen modify
    if(m_pDipWorkingBuf==NULL)
    {
        PIPE_ERR("State machine is wrong, no DIP working buffer!!\n");
        return MFALSE; //error command  enque
    }
    if (HandleExtraCommand(pPipePackageInfo) == MFALSE)
    {
        PIPE_ERR("parsing frame cmd error!!\n");
        return MFALSE; //error command  enque
    }

#if 0
    if(dupCQIdx>=MAX_DUP_CQ_NUM || dupCQIdx<0 || burstQIdx>=MAX_BURST_QUE_NUM || burstQIdx<0 ||p2CQ>=ISP_DRV_DIP_CQ_NUM || p2CQ<0 )
    {
        PIPE_ERR("exception drvScen/cq/dup/burst(%d/%d/%d/%d), last(%d)",drvScen, p2CQ, dupCQIdx, burstQIdx, lastFrame);
        return MFALSE;
    }
#endif
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
    //int idx_mfbo = -1;
    int idx_pak2o = -1;
    int idx_feo = -1;
    int idx_wroto = -1;
    int idx_wdmao = -1;
    int idx_jpego = -1;
    int idx_venco = -1;

    NSCam::NSIoPipe::Input *portInfo_imgi;
    NSCam::NSIoPipe::Input *portInfo_imgbi;
    NSCam::NSIoPipe::Input *portInfo_imgci;
    NSCam::NSIoPipe::Input *portInfo_vipi;
    NSCam::NSIoPipe::Input *portInfo_vip2i;
    NSCam::NSIoPipe::Input *portInfo_vip3i;
    NSCam::NSIoPipe::Input *portInfo_ufdi;
    NSCam::NSIoPipe::Input *portInfo_lcei;
    NSCam::NSIoPipe::Input *portInfo_dmgi;
    NSCam::NSIoPipe::Input *portInfo_depi;
    NSCam::NSIoPipe::Input *portInfo_regi;
    NSCam::NSIoPipe::Output *portInfo_img2o;
    NSCam::NSIoPipe::Output *portInfo_img2bo;
    NSCam::NSIoPipe::Output *portInfo_img3o;
    NSCam::NSIoPipe::Output *portInfo_img3bo;
    NSCam::NSIoPipe::Output *portInfo_img3co;
    NSCam::NSIoPipe::Output *portInfo_feo;
    //NSCam::NSIoPipe::Output *portInfo_mfbo;
    NSCam::NSIoPipe::Output *portInfo_pak2o;
    NSCam::NSIoPipe::Output *portInfo_wroto;
    NSCam::NSIoPipe::Output *portInfo_wdmao;
    NSCam::NSIoPipe::Output *portInfo_jpego;
    NSCam::NSIoPipe::Output *portInfo_venco;

    NSCam::EImageFormat eImgFmt_imgi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_imgbi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_imgci = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_vipi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_vip2i = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_vip3i = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_ufdi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_lcei = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_dmgi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_depi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_regi = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_img2o = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_img2bo = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_img3o = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_img3bo = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_img3co = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_feo = eImgFmt_UNKNOWN;
    //NSCam::EImageFormat eImgFmt_mfbo = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_pak2o = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_wroto = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_wdmao = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_jpego = eImgFmt_UNKNOWN;
    NSCam::EImageFormat eImgFmt_venco = eImgFmt_UNKNOWN;

    CropInfo CropInfo_imgi;

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
    //QBufInfo rQBufInfo;
    //path, module and select
    //MINT32 dip_cq_th, dip_cq_thr_ctl;
    MINT32 dip_cq_thr_ctl;
    MUINT32 dip_ctl_yuv_en=DIP_X_REG_CTL_YUV_EN_C42;
    MUINT32 dip_ctl_yuv2_en=0x0, dip_ctl_rgb_en=0x0, dip_ctl_rgb2_en=0x0;
    MINT32 imgxi_en=0, vipxi_en=0, img2xo_en=0, img3xo_en=0, ufdi_en = 0, pak2o = 0;//mfbo_en = 0;
    MINT32 imgi_fmt=DIP_IMGI_FMT_BAYER10, vipi_fmt=DIP_VIPI_FMT_YUV422_1P, img2o_fmt=DIP_IMG2O_FMT_YUV422_1P, img3o_fmt=DIP_IMG3O_FMT_YUV422_1P, pak2o_fmt = DIP_IMGI_FMT_BAYER10, pakg2_fmt = DIP_IMGI_FMT_BAYER10; //mfbo_fmt = DIP_IMGI_FMT_BAYER10
    MINT32 ufdi_fmt=DIP_UFDI_FMT_UFO_LENGTH, dmgi_fmt=DIP_DMGI_FMT_DRZ, depi_fmt=DIP_DEPI_FMT_DRZ;
    MINT32 fg_mode=DIP_FG_MODE_DISABLE;
    MUINT32 dip_ctl_path_sel=0x0;
    MUINT32 dip_misc_en=0, sram_mode =1; //DE: default sram _mode = 1
    MUINT32 wuv_mode = 0, nbc_gmap_ltm_mode = 0;
    MINT32 g2c_sel=0, pgn_sel=0, g2g_sel=0, nbc_sel=0, rcp2_sel = 0, imgi_sel = 0, feo_sel = 0, g2g2_sel=0, pak2o_sel=0;
    MUINT32 i, tableSize, mdpCheck1, mdpCheck2, mdpCheck3;
    MINT32 crop1TableIdx = -1;
    int dmai_swap = 0;
    MBOOL isWaitBuf=MTRUE;
    MBOOL isSetMdpOut = MFALSE;
    MUINT32 yuv_en=0;
    MUINT32 yuv2_en=0;
    MUINT32 rgb_en=0,rgb2_en=0;
    MUINT32 dip_yuv_tuning_tag = DIP_X_REG_CTL_YUV_EN_NBC|DIP_X_REG_CTL_YUV_EN_NBC2|DIP_X_REG_CTL_YUV_EN_PCA|DIP_X_REG_CTL_YUV_EN_SEEE|DIP_X_REG_CTL_YUV_EN_COLOR|DIP_X_REG_CTL_YUV_EN_NR3D\
		|DIP_X_REG_CTL_YUV_EN_SL2B|DIP_X_REG_CTL_YUV_EN_SL2C|DIP_X_REG_CTL_YUV_EN_SL2D|DIP_X_REG_CTL_YUV_EN_SL2E|DIP_X_REG_CTL_YUV_EN_G2C|DIP_X_REG_CTL_YUV_EN_MIX3;// |DIP_X_REG_CTL_YUV_EN_C42
    MUINT32 dip_yuv2_tuning_tag = DIP_X_REG_CTL_YUV2_EN_HFG|DIP_X_REG_CTL_YUV2_EN_SL2I|DIP_X_REG_CTL_YUV2_EN_NDG|DIP_X_REG_CTL_YUV2_EN_NDG2|DIP_X_REG_CTL_YUV2_EN_MIX4;//Mfnr will use DIP_X_REG_CTL_YUV_EN_MIX3. the other feature will possible use DIP_X_REG_CTL_YUV2_EN_MIX4 ?!
    MUINT32 dip_rgb_tuning_tag = DIP_X_REG_CTL_RGB_EN_PGN|DIP_X_REG_CTL_RGB_EN_SL2|DIP_X_REG_CTL_RGB_EN_LCE\
		|DIP_X_REG_CTL_RGB_EN_DBS2|DIP_X_REG_CTL_RGB_EN_OBC2|DIP_X_REG_CTL_RGB_EN_RMG2|DIP_X_REG_CTL_RGB_EN_BNR2|DIP_X_REG_CTL_RGB_EN_RMM2|DIP_X_REG_CTL_RGB_EN_LSC2\
		|DIP_X_REG_CTL_RGB_EN_G2G|DIP_X_REG_CTL_RGB_EN_GGM|DIP_X_REG_CTL_RGB_EN_RNR|DIP_X_REG_CTL_RGB_EN_SL2G|DIP_X_REG_CTL_RGB_EN_SL2H|DIP_X_REG_CTL_RGB_EN_SL2K|DIP_X_REG_CTL_RGB_EN_FLC|DIP_X_REG_CTL_RGB_EN_FLC2|DIP_X_REG_CTL_RGB_EN_G2G2|DIP_X_REG_CTL_RGB_EN_GGM2;//DIP_X_REG_CTL_RGB_EN_UDM;
    MUINT32 dip_rgb2_tuning_tag = DIP_X_REG_CTL_RGB2_EN_ADBS2|DIP_X_REG_CTL_RGB2_EN_DCPN2|DIP_X_REG_CTL_RGB2_EN_CPN2;

    MBOOL isCropG1En = MFALSE;
    MUINT32  CropGroup;
    MUINT32 WdmaoCropGroup=0, WrotoCropGroup=0;

    UFDG_META_INFO *pUfdParam = NULL;
    MBOOL isUfoEn = MFALSE;

    dip_x_reg_t *pTuningDipReg = NULL;

    ispDipPipe.isSecureFra = pFrameParams.mSecureFra;
    ispDipPipe.crzPipe.conf_cdrz=MFALSE;
#if 1 //temp  //kk test default:1
    int tpipe = CAM_MODE_TPIPE;
#else   //frame mode first
    int tpipe = CAM_MODE_FRAME;
#endif
    int tdr_tcm_en = 0x00;
    int tdr_tcm2_en = DIP_X_REG_CTL_TDR_C42_TCM_EN;
    int tdr_tcm3_en = 0x0;
    int tdr_tpipe = 0x00000000;
    int tdr_ctl = 0x00;
    //////////////////////////////////////////////////////////////////////////////////////
    //judge pass type
    dip_cq_thr_ctl = DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE; //immediately mode

    ///////////////////////////////////////////////////////////////////////////////////////
    //parse the dma ports contained in the frame
    PIPE_DBG("drvScen[%d]/cq[%d]/in[%lu]/out[%lu]", pPipePackageInfo->drvScen, pPipePackageInfo->p2cqIdx, (unsigned long)pFrameParams.mvIn.size(), (unsigned long)pFrameParams.mvOut.size());
    if ((MTKCAM_LOG_LEVEL_DEFAULT >= 3) || (mtkcamLogLevel >=3))
    {
        if (dipConfigPipeLog)
            PrintPipePortInfo(pFrameParams);
    }

    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvIn[i] ) { continue; }
        //
        planeNum = pFrameParams.mvIn[i].mBuffer->getPlaneCount();

        //
        switch(pFrameParams.mvIn[i].mPortID.index)
        {
            case EPortIndex_IMGI:    //may multi-plane
                idx_imgi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGI_TCM_EN;
                portInfo_imgi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgi];
                eImgFmt_imgi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgi].mBuffer->getImgFormat());
                pixIdImgBuf = PixelIDMapping(pFrameParams.mvIn[idx_imgi].mBuffer->getColorArrangement());
                imgi_planeNum = planeNum;
                /*PIPE_INF("portInfo_imgi:[%d]:(0x%x),w(%d),h(%d),stride(%d,0x0,0x0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0)",\
                                                            i,
                                                            (NSCam::EImageFormat)(portInfo_imgi->mBuffer->getImgFormat()),
                                                            portInfo_imgi->mBuffer->getImgSize().w,
                                                            portInfo_imgi->mBuffer->getImgSize().h,
                                                            portInfo_imgi->mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            //mPortTypeMapping[pFrameParams.mvIn[i].mPortID.type].eImgIOPortType,
                                                            portInfo_imgi->mPortID.index,
                                                            portInfo_imgi->mPortID.inout,
                                                            portInfo_imgi->mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)portInfo_imgi->mBuffer->getBufVA(0),
                                                            (unsigned long)portInfo_imgi->mBuffer->getBufPA(0));*/

                // input port limitation check for FullG format
                if ((eImgFmt_imgi == eImgFmt_FG_BAYER8) || (eImgFmt_imgi == eImgFmt_FG_BAYER10) ||
                        (eImgFmt_imgi == eImgFmt_FG_BAYER12) || (eImgFmt_imgi == eImgFmt_FG_BAYER14)){
                    //TODO: size must be align 4 for FUllG format, USER TAKE CARE THIS
                }
                //
                if(planeNum >= 2)    // check plane number
                { // check imgbi
                    switch (eImgFmt_imgi)
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
                        case eImgFmt_NV12:
                        case eImgFmt_NV21:
                            imgi_uv_h_ratio = 1;
                            break;
                        case eImgFmt_YV12:
                            imgi_uv_swap = MTRUE;
                            break;
                        default:
                            break;
                    }

                    if ((eImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (eImgFmt_imgi == eImgFmt_UFO_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_BAYER12))
                    {
                        idx_ufdi = i;
                        dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_UFDI;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_UFDI_TCM_EN;
                        //if (vInPorts[idx_ufdi]->eImgFmt != eImgFmt_UFO_FG)
                        //     dip_ctl_yuv_en |=    (DIP_X_REG_CTL_YUV_EN_SRZ1|DIP_X_REG_CTL_YUV_EN_MIX1);
                        portInfo_ufdi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_ufdi];
                        eImgFmt_ufdi = eImgFmt_imgi;

                        if (planeNum ==3)
                        {
                            pUfdParam = (UFDG_META_INFO *)pFrameParams.mvIn[idx_ufdi].mBuffer->getBufVA(2);
                            PIPE_INF("UFDG_META_INFO: UFDG_BITSTREAM_OFST_ADDR(%d),UFDG_BS_AU_START(%d),UFDG_AU2_SIZE(%d),UFDG_BOND_MODE(%d)", pUfdParam->UFDG_BITSTREAM_OFST_ADDR, pUfdParam->UFDG_BS_AU_START, pUfdParam->UFDG_AU2_SIZE, pUfdParam->UFDG_BOND_MODE);
                        }
                        isUfoEn = MTRUE;
                    }
                    else
                    {
                        idx_imgbi = i;
                        dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGBI;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGBI_TCM_EN;
                        portInfo_imgbi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgbi];
                        eImgFmt_imgbi = eImgFmt_imgi;

                        if(planeNum == 3)
                        { // check imgci
                            idx_imgci = i;
                            dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGCI;
                            tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGCI_TCM_EN;
                            portInfo_imgci = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgci];
                            eImgFmt_imgci = eImgFmt_imgi;
                        }
                    }
                }

                // Parsing crop information +++++
                CropInfo_imgi.crop1.x      = 0;
                CropInfo_imgi.crop1.floatX = 0;
                CropInfo_imgi.crop1.y      = 0;
                CropInfo_imgi.crop1.floatY = 0;
                CropInfo_imgi.crop1.w      = 0;
                CropInfo_imgi.crop1.floatW = 0;
                CropInfo_imgi.crop1.h      = 0;
                CropInfo_imgi.crop1.floatH = 0;
                CropInfo_imgi.resize1.tar_w = 0;
                CropInfo_imgi.resize1.tar_h = 0;
                CropInfo_imgi.crop2.x = 0;
                CropInfo_imgi.crop2.floatX = 0;
                CropInfo_imgi.crop2.y = 0;
                CropInfo_imgi.crop2.floatY = 0;
                CropInfo_imgi.crop2.w      = 0;
                CropInfo_imgi.crop2.floatW = 0;
                CropInfo_imgi.crop2.h      = 0;
                CropInfo_imgi.crop2.floatH = 0;
                CropInfo_imgi.resize2.tar_w = 0;
                CropInfo_imgi.resize2.tar_h = 0;
                CropInfo_imgi.crop3.x = 0;
                CropInfo_imgi.crop3.floatX = 0;
                CropInfo_imgi.crop3.y = 0;
                CropInfo_imgi.crop3.floatY = 0;
                CropInfo_imgi.crop3.w      = 0;
                CropInfo_imgi.crop3.floatW = 0;
                CropInfo_imgi.crop3.h      = 0;
                CropInfo_imgi.crop3.floatH = 0;
                CropInfo_imgi.resize3.tar_w = 0;
                CropInfo_imgi.resize3.tar_h = 0;
                for(MUINT32 k=0;k<pFrameParams.mvCropRsInfo.size();k++)
                {
                    //crop1/crop2 may be different in each buffer node
                        switch(pFrameParams.mvCropRsInfo[k].mGroupID)
                        {
                            case 1:    // for CRZ
                                CropInfo_imgi.crop1.x      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop1.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop1.y      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop1.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop1.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop1.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop1.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop1.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize1.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize1.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                break;
                            case 2:    // for WDMAO
                                CropInfo_imgi.crop2.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop2.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop2.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop2.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop2.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop2.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop2.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop2.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize2.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize2.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WdmaoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            case 3: // for WROTO
                                CropInfo_imgi.crop3.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop3.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop3.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop3.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop3.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop3.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop3.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop3.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize3.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize3.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WrotoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            default:
                                PIPE_ERR("DO not support crop group (%d)",pFrameParams.mvCropRsInfo[k].mGroupID);
                            break;
                        }
                }

                // Parsing crop information -----

                PIPE_DBG("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),dupCqIdx(%d)", \
                    CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
                    CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
                    CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
                    CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
                    CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
                    CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
                    CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
                    CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
                    CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx);

                break;
            case EPortIndex_IMGBI:    //Mixing path weighting table
                idx_imgbi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGBI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGBI_TCM_EN;
                portInfo_imgbi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgbi];
                eImgFmt_imgbi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgbi].mBuffer->getImgFormat());
                break;
            case EPortIndex_IMGCI:    //Mixing path weighting table
                idx_imgci = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_IMGCI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMGCI_TCM_EN;
                portInfo_imgci = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgci];
                eImgFmt_imgci = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgci].mBuffer->getImgFormat());
                break;
            case EPortIndex_VIPI:    //may multi-plane, vipi~vip3i
                idx_vipi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIPI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIPI_TCM_EN;
                portInfo_vipi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vipi];
                eImgFmt_vipi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_vipi].mBuffer->getImgFormat());
                if(planeNum >= 2)
                { // check vip2i
                    switch (eImgFmt_vipi)
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
                        case eImgFmt_NV21:
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
                    portInfo_vip2i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip2i];
                    eImgFmt_vip2i = eImgFmt_vipi;

                    if(planeNum == 3)
                    { // check vip3i
                        idx_vip3i = i;
                        dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIP3I;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIP3I_TCM_EN;
                        portInfo_vip3i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip3i];
                        eImgFmt_vip3i = eImgFmt_vipi;
                    }
                }
                break;
            case EPortIndex_VIP3I: //only vip3i input(weighting table), there are only vipi+vip3i without vip2i for mfb mixing path
                idx_vip3i = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_VIP3I;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_VIP3I_TCM_EN;
                portInfo_vip3i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip3i];
                eImgFmt_vip3i = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_vip3i].mBuffer->getImgFormat());
                break;
            case EPortIndex_LCEI:
                idx_lcei = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_LCEI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_LCEI_TCM_EN;

                if(drvScen==eDrvScenario_VFB_FB || drvScen==eDrvScenario_P2B_Bokeh){
                    dip_ctl_yuv_en |=  (DIP_X_REG_CTL_YUV_EN_SRZ2|DIP_X_REG_CTL_YUV_EN_MIX2);
                }

                portInfo_lcei = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_lcei];
                eImgFmt_lcei = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_lcei].mBuffer->getImgFormat());
                break;
            case EPortIndex_UFDI:
                idx_ufdi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_UFDI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_UFDI_TCM_EN;

                portInfo_ufdi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_ufdi];
                eImgFmt_ufdi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_ufdi].mBuffer->getImgFormat());

                if (eImgFmt_ufdi != eImgFmt_UFO_FG)
                    dip_ctl_yuv_en |=  (DIP_X_REG_CTL_YUV_EN_SRZ1|DIP_X_REG_CTL_YUV_EN_MIX1);

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
                //      dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ4);
                //}
                portInfo_depi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_depi];
                eImgFmt_depi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_depi].mBuffer->getImgFormat());
                break;
            case EPortIndex_DMGI:
                idx_dmgi = i;
                dip_ctl_dma_en |=  DIP_X_REG_CTL_DMA_EN_DMGI;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_DMGI_TCM_EN;
                //if(drvScen == eDrvScenario_P2B_Bokeh) {
                //      dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ3);
                //}
                portInfo_dmgi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_dmgi];
                eImgFmt_dmgi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_dmgi].mBuffer->getImgFormat());
                break;
            case EPortIndex_REGI:
                idx_regi = i;
                portInfo_regi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_regi];
                eImgFmt_regi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_regi].mBuffer->getImgFormat());
                break;
            default:
                PIPE_ERR("[Error]Not support this input port(%d),i(%d)",pFrameParams.mvIn[i].mPortID.index,i);
                break;
        }
    }

        //
    for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvOut[i] ) { continue; }
        //
        planeNum = pFrameParams.mvOut[i].mBuffer->getPlaneCount();

        if ((MTRUE == getCropFunctionEnable(drvScen, (MINT32)pFrameParams.mvOut[i].mPortID.index, CropGroup))
            && (CropGroup & ECropGroupIndex_1))
        {
            //Check the crop setting.
            isCropG1En = MTRUE;
        }
        switch(pFrameParams.mvOut[i].mPortID.index)
        {
            case EPortIndex_IMG2O: //may multi-plane, img2o+IMG2bo
                idx_img2o = i;
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG2O;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG2O_TCM_EN;
                portInfo_img2o = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img2o];
                eImgFmt_img2o = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_img2o].mBuffer->getImgFormat());

                if(planeNum == 2)
                { // check img2bo
                    switch (eImgFmt_img2o)
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
                    portInfo_img2bo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img2bo];
                    eImgFmt_img2bo = eImgFmt_img2o;
                }
                break;
            case EPortIndex_IMG3O:    //may multi-plane, img3o+img3bo+img3co
                idx_img3o = i;
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG3O;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG3O_TCM_EN;
                portInfo_img3o = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3o];
                eImgFmt_img3o = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_img3o].mBuffer->getImgFormat());

                if(planeNum >= 2)
                { // check img3bo
                    switch (eImgFmt_img3o)
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
                    portInfo_img3bo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3bo];
                    eImgFmt_img3bo = eImgFmt_img3o;

                    if(planeNum == 3)
                    { // check img3co
                        idx_img3co = i;
                        dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_IMG3CO;
                        tdr_tcm_en |= DIP_X_REG_CTL_TDR_IMG3CO_TCM_EN;
                        portInfo_img3co =  (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3co];
                        eImgFmt_img3co = eImgFmt_img3o;
                    }
                }
                break;
            case EPortIndex_WROTO:
                idx_wroto = i;
                if (WrotoCropGroup == 0)
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MDPCROP;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN;
                }
                else if (WrotoCropGroup == 1)
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_MDPCROP2|DIP_X_REG_CTL_RGB_EN_WSHIFT);
                    else
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_MDPCROP2|DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_G2G2|DIP_X_REG_CTL_RGB_EN_GGM2);
	            tdr_tcm3_en |= DIP_X_REG_CTL_TDR_MDPCROP2_TCM_EN;
               	}
                else
                {
                    PIPE_ERR("[Error]do not support this crop group of WROTO(%d)",WrotoCropGroup);
                    return MFALSE;
                }
                dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_C24B;
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_WROTO;
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                portInfo_wroto = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wroto];
                eImgFmt_wroto = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_wroto].mBuffer->getImgFormat());
                break;
            case EPortIndex_WDMAO:
                idx_wdmao = i;
                if (WdmaoCropGroup == 0)
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MDPCROP;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN;
                }
                else if (WdmaoCropGroup == 1)
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_MDPCROP2|DIP_X_REG_CTL_RGB_EN_WSHIFT);
                    else
                        dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_MDPCROP2|DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_G2G2|DIP_X_REG_CTL_RGB_EN_GGM2);
                    tdr_tcm3_en |= DIP_X_REG_CTL_TDR_MDPCROP2_TCM_EN;
                }
                else
                {
                    PIPE_ERR("[Error]do not support this crop group of WDMAO(%d)",WdmaoCropGroup);
                    return MFALSE;
                }
                dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_C24B;
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_WDMAO;
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                portInfo_wdmao = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wdmao];
                eImgFmt_wdmao = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_wdmao].mBuffer->getImgFormat());
                break;
            case EPortIndex_JPEGO:
                idx_jpego = i;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_JPEGO;
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                portInfo_jpego = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_jpego];
                eImgFmt_jpego = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_jpego].mBuffer->getImgFormat());
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
                    portInfo_feo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_feo];
                    eImgFmt_feo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_feo].mBuffer->getImgFormat());
                }
                else
                {
                    idx_feo = i;
                    dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_FEO;
                    dip_ctl_yuv_en |=DIP_X_REG_CTL_YUV_EN_FE;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_FEO_TCM_EN;
                    tdr_tcm3_en |= DIP_X_REG_CTL_TDR_FE_TCM_EN;
                    portInfo_feo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_feo];
                    eImgFmt_feo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_feo].mBuffer->getImgFormat());
                }
                break;
#if 0
            case EPortIndex_MFBO: // mfb blending
                idx_mfbo = i;
                portInfo_mfbo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_mfbo];
                eImgFmt_mfbo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_mfbo].mBuffer->getImgFormat());
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_MFBO;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_MFBO_TCM_EN;
                break;
#endif
            case EPortIndex_PAK2O:
                idx_pak2o = i;
                portInfo_pak2o = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_pak2o];
                eImgFmt_pak2o = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_pak2o].mBuffer->getImgFormat());
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_PAK2O;
                tdr_tcm_en |= DIP_X_REG_CTL_TDR_PAK2O_TCM_EN;
                break;
            case EPortIndex_VENC_STREAMO:
                idx_venco = i;
                portInfo_venco = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_venco];
                eImgFmt_venco = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_venco].mBuffer->getImgFormat());
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_MDPCROP |DIP_X_REG_CTL_YUV_EN_C24B);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN |DIP_X_REG_CTL_TDR_C24B_TCM_EN;
                dip_ctl_dma_en |= DIP_X_REG_CTL_DMA_EN_VENCO;
                break;
            default:
                PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",pFrameParams.mvOut[i].mPortID.index,i);
                break;
        };
    }

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

                    PIPE_INF("SRZ1:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz1Cfg.inout_size.in_w,ispDipPipe.srz1Cfg.inout_size.in_h,\
                        ispDipPipe.srz1Cfg.crop.x, ispDipPipe.srz1Cfg.crop.floatX, ispDipPipe.srz1Cfg.crop.y, ispDipPipe.srz1Cfg.crop.floatY,\
                        ispDipPipe.srz1Cfg.crop.w, ispDipPipe.srz1Cfg.crop.h, ispDipPipe.srz1Cfg.inout_size.out_w,ispDipPipe.srz1Cfg.inout_size.out_h,\
                        ispDipPipe.srz1Cfg.h_step, ispDipPipe.srz1Cfg.v_step);
                }
                break;
            case EDipModule_SRZ2:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    dip_ctl_yuv_en |=DIP_X_REG_CTL_YUV_EN_SRZ2;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_SRZ2_TCM_EN;
                    this->configSrz(&ispDipPipe.srz2Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ2:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz2Cfg.inout_size.in_w,ispDipPipe.srz2Cfg.inout_size.in_h,\
                        ispDipPipe.srz2Cfg.crop.x, ispDipPipe.srz2Cfg.crop.floatX, ispDipPipe.srz2Cfg.crop.y, ispDipPipe.srz2Cfg.crop.floatY,\
                        ispDipPipe.srz2Cfg.crop.w, ispDipPipe.srz2Cfg.crop.h, ispDipPipe.srz2Cfg.inout_size.out_w,ispDipPipe.srz2Cfg.inout_size.out_h,\
                        ispDipPipe.srz2Cfg.h_step, ispDipPipe.srz2Cfg.v_step);
                }
                break;

            case EDipModule_SRZ3:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ3);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ3_TCM_EN;
                    this->configSrz(&ispDipPipe.srz3Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ3:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz3Cfg.inout_size.in_w,ispDipPipe.srz3Cfg.inout_size.in_h,\
                        ispDipPipe.srz3Cfg.crop.x, ispDipPipe.srz3Cfg.crop.floatX, ispDipPipe.srz3Cfg.crop.y, ispDipPipe.srz3Cfg.crop.floatY,\
                        ispDipPipe.srz3Cfg.crop.w, ispDipPipe.srz3Cfg.crop.h, ispDipPipe.srz3Cfg.inout_size.out_w,ispDipPipe.srz3Cfg.inout_size.out_h,\
                        ispDipPipe.srz3Cfg.h_step, ispDipPipe.srz3Cfg.v_step);
                }
                break;
            case EDipModule_SRZ4:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ4);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ4_TCM_EN;
                    this->configSrz(&ispDipPipe.srz4Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ4:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz4Cfg.inout_size.in_w,ispDipPipe.srz4Cfg.inout_size.in_h,\
                        ispDipPipe.srz4Cfg.crop.x, ispDipPipe.srz4Cfg.crop.floatX, ispDipPipe.srz4Cfg.crop.y, ispDipPipe.srz4Cfg.crop.floatY,\
                        ispDipPipe.srz4Cfg.crop.w, ispDipPipe.srz4Cfg.crop.h, ispDipPipe.srz4Cfg.inout_size.out_w,ispDipPipe.srz4Cfg.inout_size.out_h,\
                        ispDipPipe.srz4Cfg.h_step, ispDipPipe.srz4Cfg.v_step);
                }
                break;
            case EDipModule_SRZ5:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= (DIP_X_REG_CTL_YUV2_EN_SRZ5);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ5_TCM_EN;

                    this->configSrz(&ispDipPipe.srz5Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ5:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz5Cfg.inout_size.in_w,ispDipPipe.srz5Cfg.inout_size.in_h,\
                        ispDipPipe.srz5Cfg.crop.x, ispDipPipe.srz5Cfg.crop.floatX, ispDipPipe.srz5Cfg.crop.y, ispDipPipe.srz5Cfg.crop.floatY,\
                        ispDipPipe.srz5Cfg.crop.w, ispDipPipe.srz5Cfg.crop.h, ispDipPipe.srz5Cfg.inout_size.out_w,ispDipPipe.srz5Cfg.inout_size.out_h,\
                        ispDipPipe.srz5Cfg.h_step, ispDipPipe.srz5Cfg.v_step);
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
        switch( eImgFmt_ufdi )
        {
            //case eImgFmt_FG_BAYER8:      //TODO, revise for ufdi format
            case eImgFmt_UFO_FG:
            case eImgFmt_UFO_FG_BAYER10:
            case eImgFmt_UFO_FG_BAYER12:
            case eImgFmt_UFO_BAYER10:
            case eImgFmt_UFO_BAYER12:
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
                PIPE_ERR("[Error] ufdi NOT Support this format(0x%x)",eImgFmt_ufdi);
                return MFALSE;
        }
    }
    //
    vipxi_en = (int)((dip_ctl_dma_en&0x00000070)>>4);
    if (vipxi_en!=0)
    {
        switch( eImgFmt_vipi )
        {
            case eImgFmt_NV21:        //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //= 0x00000040,   //420 format, 2 plane (UV)
                vipi_fmt = DIP_VIPI_FMT_YUV420_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_C02);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02_TCM_EN;
                if(vipxi_en != 3)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:        //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //= 0x00000400,   //420 format, 3 plane(YUV)
                vipi_fmt = DIP_VIPI_FMT_YUV420_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_C02);
                tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C02_TCM_EN;
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i+vip3i when 3-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:        //= 0x00001000,   //422 format, 1 plane (YUYV)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:        //= 0x00008000,   //422 format, 1 plane (UYVY)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:        //= 0x00002000,   //422 format, 1 plane (YVYU)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:        //= 0x00004000,   //422 format, 1 plane (VYUY)
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane
            case eImgFmt_I422:        //422 format, 3 plane
                vipi_fmt = DIP_VIPI_FMT_YUV422_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR1);
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV16:        //422 format, 2 plane
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
            case eImgFmt_ARGB8888:     //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
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
                dmai_swap = 0;    //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;
            default:
                PIPE_ERR("[Error] vipxi NOT Support this format(0x%x)",eImgFmt_vipi);
                return MFALSE;
        }
    }
        //
    imgxi_en = (int)((dip_ctl_dma_en&0x00000007));
    if (imgxi_en!=0)
    {
        switch( eImgFmt_imgi )
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
            case eImgFmt_FG_BAYER10:      //= 0x02000000,    //FG Bayer format, 10-bit
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
            case eImgFmt_FG_BAYER12:      //= 0x04000000,    //FG Bayer format, 12-bit
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
                if (eImgFmt_BAYER8 == eImgFmt_imgi)
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
            case eImgFmt_BAYER10:    //= 0x00000002,   //Bayer format, 10-bit
            case eImgFmt_BAYER10_UNPAK:
            case eImgFmt_UFO_BAYER10:
                if ((eImgFmt_BAYER10 == eImgFmt_imgi) || (eImgFmt_UFO_BAYER10 == eImgFmt_imgi))
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
            case eImgFmt_BAYER12:    //= 0x00000004,   //Bayer format, 12-bit
            case eImgFmt_BAYER12_UNPAK:
            case eImgFmt_UFO_BAYER12:
                if ((eImgFmt_BAYER12 == eImgFmt_imgi) || (eImgFmt_UFO_BAYER12 == eImgFmt_imgi))
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
            case eImgFmt_BAYER14:    //= 0x00000008,   //Bayer format, 14-bit
            case eImgFmt_BAYER14_UNPAK:
                if (eImgFmt_BAYER14 == eImgFmt_imgi)
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
            case eImgFmt_BAYER10_MIPI:     //= 0x00000002,   //Bayer format, 10-bit
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
            case eImgFmt_NV21:        //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //= 0x00000040,   //420 format, 2 plane (UV)
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
            case eImgFmt_YV12:        //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //= 0x00000400,   //420 format, 3 plane(YUV)
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
            case eImgFmt_YUY2:        //= 0x00001000,   //422 format, 1 plane (YUYV)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:        //= 0x00008000,   //422 format, 1 plane (UYVY)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:        //= 0x00002000,   //422 format, 1 plane (YVYU)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:        //= 0x00004000,   //422 format, 1 plane (VYUY)
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                if(nbc_sel ==0) //vipi rgb input
                {
                    dip_ctl_yuv_en |=(DIP_X_REG_CTL_YUV_EN_C24);
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_C24_TCM_EN;
                }
                g2c_sel=1;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane
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
            case eImgFmt_NV16:        //422 format, 2 plane
                imgi_fmt = DIP_IMGI_FMT_YUV422_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNR2);
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
            case eImgFmt_ARGB8888:     //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
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
                dmai_swap = 0;    //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;
            case eImgFmt_RGB48:   //= 0x10000000,    //RGB121212 (48-bit), 1 plane
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
                dmai_swap = 0;    //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;

            case eImgFmt_STA_BYTE:
                break;
            case eImgFmt_NV21_BLK:    //= 0x00000200,   //420 format block mode, 2 plane (UV)
            case eImgFmt_NV12_BLK:    //= 0x00000080,   //420 format block mode, 2 plane (VU)
            case eImgFmt_JPEG:        //= 0x00000010,   //JPEG format
            default:
                PIPE_ERR("[Error] imgxi NOT Support this format(0x%x)",eImgFmt_imgi);
                return MFALSE;
        }
        //Check Image Buffer PixId is equal to NormalStream query
        if ((imgi_fmt>=DIP_IMGI_FMT_BAYER8) &&(imgi_fmt<=DIP_IMGI_FMT_BAYER10_MIPI))
        {
            if (pixIdImgBuf != pPipePackageInfo->pixIdP2)
            {
                //PIPE_ERR("[Error] ImgBuf PixelID(%d) is not the same as NormalStream Query(%d)!!",pixIdImgBuf, pPipePackageInfo->pixIdP2);
            }
        }
    }
    //
    img2xo_en = (int)((dip_ctl_dma_en&0x00001800)>>11);
    if (img2xo_en !=0)
    {
        switch( eImgFmt_img2o )
        {
            case eImgFmt_YUY2:        //= 0x00001000,   //422 format, 1 plane (YUYV)
                img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:        //= 0x00008000,   //422 format, 1 plane (UYVY)
                img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:        //= 0x00002000,   //422 format, 1 plane (YVYU)
                img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:        //= 0x00004000,   //422 format, 1 plane (VYUY)
                img2o_fmt = DIP_IMG2O_FMT_YUV422_1P;
                dmai_swap = 2;
                break;
            case eImgFmt_NV16:        //422 format, 2 plane
                img2o_fmt = DIP_IMG2O_FMT_YUV422_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW2);
                if(img2xo_en != 3)
                {
                    PIPE_ERR("should enable img2o+img2bo when 2-plane input (0x%x)",img2xo_en);
                    return MFALSE;
                }
                break;
            default:
                PIPE_ERR("[Error] img2o NOT Support this format(0x%x)",eImgFmt_img2o);
                return MFALSE;
        }
    }
    //
    img3xo_en = (int)((dip_ctl_dma_en&0x0000e000)>>13);
    if (img3xo_en !=0)
    {
        switch( eImgFmt_img3o )
        {
            case eImgFmt_NV21:        //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //= 0x00000040,   //420 format, 2 plane (UV)
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
            case eImgFmt_YV12:        //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //= 0x00000400,   //420 format, 3 plane(YUV)
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
            case eImgFmt_YUY2:        //= 0x00001000,   //422 format, 1 plane (YUYV)
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:        //= 0x00008000,   //422 format, 1 plane (UYVY)
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:        //= 0x00002000,   //422 format, 1 plane (YVYU)
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:        //= 0x00004000,   //422 format, 1 plane (VYUY)
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane
            case eImgFmt_I422:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_3P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
                if(img3xo_en != 7)
                {
                    PIPE_ERR("should enable img3o+img3bo+img3co when 3-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV16:        //422 format, 2 plane
                img3o_fmt= DIP_IMG3O_FMT_YUV422_2P;
                dip_ctl_yuv_en |= (DIP_X_REG_CTL_YUV_EN_PLNW1);
                if(img3xo_en != 3)
                {
                    PIPE_ERR("should enable img3o+img3bo when 2-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            default:
                PIPE_ERR("[Error] img3o NOT Support this format(0x%x)",eImgFmt_img3o);
                return MFALSE;
        }
    }


    pak2o = (int)((dip_ctl_dma_en&0x00000400)>>10);
    if (pak2o !=0)
    {
        if (drvScen != eDrvScenario_MFB_Blending)
        {
            dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_RCP2 | DIP_X_REG_CTL_RGB_EN_PAK2);
        }

        switch( eImgFmt_pak2o )
        {
            case eImgFmt_BAYER8:    //= 0x00000001,   //Bayer format, 8-bit
                pak2o_fmt = DIP_IMGI_FMT_BAYER8;
                break;
            case eImgFmt_BAYER8_UNPAK:
                pak2o_fmt = DIP_IMGI_FMT_BAYER8_2BYTEs;
                break;
            case eImgFmt_BAYER10:    //= 0x00000002,   //Bayer format, 10-bit
                pak2o_fmt = DIP_IMGI_FMT_BAYER10;
                break;
            case eImgFmt_BAYER10_UNPAK:
                pak2o_fmt = DIP_IMGI_FMT_BAYER10_2BYTEs;
                break;
            case eImgFmt_BAYER12:    //= 0x00000004,   //Bayer format, 12-bit
                pak2o_fmt = DIP_IMGI_FMT_BAYER12;
                break;
            case eImgFmt_BAYER12_UNPAK:
                pak2o_fmt = DIP_IMGI_FMT_BAYER12_2BYTEs;
                break;
            case eImgFmt_BAYER14:    //= 0x00000008,   //Bayer format, 14-bit
                pak2o_fmt = DIP_IMGI_FMT_BAYER14;
                break;
            case eImgFmt_BAYER14_UNPAK:
                pak2o_fmt = DIP_IMGI_FMT_BAYER14_2BYTEs;
                break;
            case eImgFmt_STA_BYTE:
                pak2o_fmt = DIP_IMGI_FMT_BAYER8;
                break;
            default:
                PIPE_ERR("[Error] pak2o NOT Support this format(0x%x)",eImgFmt_pak2o);
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
            if (-1 != idx_pak2o)
            {
                pak2o_sel = 0x1; //0:MFB weighting output, 1:from after PAK2, 2:FM output, 3:from after G2G
                if ( eDrvScenario_DeNoise == drvScen )
                {
                    // TODO: check with algo and FPM
                    //g2g2_sel = 1;
                    //wuv_mode = 1;
                    //dip_ctl_rgb_en  |= (DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_WSYNC);
                    rcp2_sel = 0x0; // 0 : from after PGN,      1 : from after LSC2,        2 : from before LSC2
                }
                else
                {
                    rcp2_sel = 0x1; // 0 : from after PGN,      1 : from after LSC2,        2 : from before LSC2
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
            pak2o_sel = 0x0; //0:MFB weighting output, 1:from after PAK2, 2:FM output, 3:from after G2G
            sram_mode = 2;
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
            //imgi_fmt = DIP_IMGI_FMT_MFB_MIX_MODE;
            dip_ctl_path_sel = 0x00000040;
            if (bMix4En)
            {
                dip_ctl_yuv2_en |= DIP_X_REG_CTL_YUV2_EN_MIX4;
            }
            else
            {
                dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_MIX3;
            }
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
            pak2o_sel = 0x2; //0:MFB weighting output, 1:from after PAK2, 2:FM output, 3:from after G2G
            sram_mode = 3;
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
        case eDrvScenario_WUV:
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
            - g2g2_sel=1;
            */
            dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_WSYNC);
            dip_ctl_path_sel = 0x01016040;
            g2g2_sel = 1;
            wuv_mode = 1;
            if(pPipePackageInfo->pTuningQue != NULL)
            {
                dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_WSYNC);
            }
            else
            {
                dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_WSHIFT|DIP_X_REG_CTL_RGB_EN_WSYNC|DIP_X_REG_CTL_RGB_EN_G2G2|DIP_X_REG_CTL_RGB_EN_GGM2);
            }
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
    //Handle Smart Tile.
    //Smart Tile1 Enable
    if ((bSmartTileEn) && (pPipePackageInfo->pTuningQue != NULL))
    {
         pTuningBuf = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
         if ((pTuningBuf->DIP_X_CTL_RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_UFD) == DIP_X_REG_CTL_RGB_EN_UFD)
         {
             switch (imgi_fmt)
             {
                 case DIP_IMGI_FMT_BAYER8:
                 case DIP_IMGI_FMT_BAYER10:
                 case DIP_IMGI_FMT_BAYER12:
                 case DIP_IMGI_FMT_BAYER8_2BYTEs:
                 case DIP_IMGI_FMT_BAYER10_2BYTEs:
                 case DIP_IMGI_FMT_BAYER12_2BYTEs:
                     pakg2_fmt = imgi_fmt;
                     dip_ctl_rgb_en |= (DIP_X_REG_CTL_RGB_EN_SMX1|DIP_X_REG_CTL_RGB_EN_PAKG2);
                     dip_ctl_dma_en |= (DIP_X_REG_CTL_DMA_EN_SMX1I|DIP_X_REG_CTL_DMA_EN_SMX1O);
                     tdr_tcm3_en |= (DIP_X_REG_CTL_TDR_SMX1_TCM_EN|DIP_X_REG_CTL_TDR_SMX1I_TCM_EN|DIP_X_REG_CTL_TDR_SMX1O_TCM_EN);
                     break;
                 default:
                     break;
             }
         }
         if (((dip_ctl_yuv_en & DIP_X_REG_CTL_YUV_EN_MDPCROP) == DIP_X_REG_CTL_YUV_EN_MDPCROP) ||
             ((pTuningBuf->DIP_X_CTL_YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_NR3D) == DIP_X_REG_CTL_YUV_EN_NR3D))
         {
             dip_ctl_yuv2_en |= DIP_X_REG_CTL_YUV2_EN_SMX3;
             dip_ctl_dma_en |= (DIP_X_REG_CTL_DMA_EN_SMX3I|DIP_X_REG_CTL_DMA_EN_SMX3O);
             tdr_tcm3_en |= (DIP_X_REG_CTL_TDR_SMX3_TCM_EN|DIP_X_REG_CTL_TDR_SMX3I_TCM_EN|DIP_X_REG_CTL_TDR_SMX3O_TCM_EN);
         }
         if ((pTuningBuf->DIP_X_CTL_RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_MDPCROP2) == DIP_X_REG_CTL_RGB_EN_MDPCROP2)
         {
             dip_ctl_rgb2_en |= DIP_X_REG_CTL_RGB2_EN_SMX4;
             dip_ctl_dma_en |= (DIP_X_REG_CTL_DMA_EN_SMX4I|DIP_X_REG_CTL_DMA_EN_SMX4O);
             tdr_tcm3_en |= (DIP_X_REG_CTL_TDR_SMX4_TCM_EN|DIP_X_REG_CTL_TDR_SMX4I_TCM_EN|DIP_X_REG_CTL_TDR_SMX4O_TCM_EN);
         }
         if ((pTuningBuf->DIP_X_CTL_YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_NBC2) == DIP_X_REG_CTL_YUV_EN_NBC2)
         {
             dip_ctl_yuv2_en |= DIP_X_REG_CTL_YUV2_EN_SMX2;
             dip_ctl_dma_en |= (DIP_X_REG_CTL_DMA_EN_SMX2I|DIP_X_REG_CTL_DMA_EN_SMX2O);
             tdr_tcm3_en |= (DIP_X_REG_CTL_TDR_SMX2_TCM_EN|DIP_X_REG_CTL_TDR_SMX2I_TCM_EN|DIP_X_REG_CTL_TDR_SMX2O_TCM_EN);
         }
    }

    ///////////////////////////////////////////;//////////////////
    //information collection
    ispDipPipe.moduleIdx=DIP_HW_A; //TODO, need pass from iopipe
    PIPE_DBG("moduleIdx(%d),dip_ctl_dma_en(0x%x)",ispDipPipe.moduleIdx,dip_ctl_dma_en);
    if(tpipe == CAM_MODE_TPIPE) {
        ispDipPipe.isDipOnly=MFALSE;
        tdr_ctl |= DIP_X_REG_CTL_TDR_RN | DIP_X_REG_CTL_CTL_EXTENSION_EN|DIP_X_REG_CTL_TDR_SOF_RST_EN; //enable tpipe extension in default
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
        rgb2_en = ispDipPipe.pTuningIspReg->DIP_X_CTL_RGB2_EN.Raw;
        b12bitMode = ispDipPipe.pTuningIspReg->DIP_X_CTL_FMT_SEL.Bits.LP_MODE;
        nbc_gmap_ltm_mode = ispDipPipe.pTuningIspReg->DIP_X_CTL_MISC_SEL.Bits.NBC_GMAP_LTM_MODE;

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
        tdr_tcm_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_NDG2) << 21; //NDG2
        tdr_tcm_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_NDG) >>6; //NDG

        tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_SL2) << 19; //SL2
        tdr_tcm_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_SL2B|DIP_X_REG_CTL_YUV_EN_SL2C|DIP_X_REG_CTL_YUV_EN_SL2D|DIP_X_REG_CTL_YUV_EN_SL2E)) << 12; //SL2B~SL2E
        tdr_tcm_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_LCE) << 26; //LCE
        //tdr_tcm2_en
        tdr_tcm2_en |= (yuv_en & DIP_X_REG_CTL_YUV_EN_NR3D) >> 8; //NR3D
        tdr_tcm2_en |= (yuv_en & DIP_X_REG_CTL_YUV_EN_COLOR) >> 7; //COLOR (If COLOR turn on, need to turn on NR3D_TCM_EN as well.)
        tdr_tcm2_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_PCA|DIP_X_REG_CTL_YUV_EN_SEEE)) << 4; //PCA, SEEE
        tdr_tcm2_en |= (yuv_en & (DIP_X_REG_CTL_YUV_EN_NBC|DIP_X_REG_CTL_YUV_EN_NBC2)) << 8; //NBC, NBC2
        tdr_tcm2_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_DBS2) << 5; //DBS2
        tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_RMG2|DIP_X_REG_CTL_RGB_EN_BNR2|DIP_X_REG_CTL_RGB_EN_RMM2)) << 4; //RMG2, BNR2, RMM2
        tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_RNR|DIP_X_REG_CTL_RGB_EN_SL2G|DIP_X_REG_CTL_RGB_EN_SL2H)) << 5; //RNR, SL2G, SL2H
        tdr_tcm2_en |= (rgb_en & (DIP_X_REG_CTL_RGB_EN_SL2K)) << 7; //HLR2, SL2K
        tdr_tcm2_en |= (dip_ctl_dma_en & (DIP_X_REG_CTL_DMA_EN_ADL2)) << 13; //ADL2
        tdr_tcm2_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_HFG) << 23;
        tdr_tcm2_en |= (yuv2_en & DIP_X_REG_CTL_YUV2_EN_SL2I) << 21;
        tdr_tcm2_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_FLC) << 8; //FLC
        tdr_tcm2_en |= (rgb_en & DIP_X_REG_CTL_RGB_EN_FLC2) >> 23; //FLC2
        //tdr_tcm3_en
        tdr_tcm3_en |= (yuv_en & DIP_X_REG_CTL_YUV_EN_COLOR) >> 5; //Color
        tdr_tcm3_en |= (rgb2_en & DIP_X_REG_CTL_RGB2_EN_ADBS2) << 13; //ADBS2
        tdr_tcm3_en |= (rgb2_en & (DIP_X_REG_CTL_RGB2_EN_DCPN2|DIP_X_REG_CTL_RGB2_EN_CPN2)) << 14; //DCPN2, CPN2

        ispDipPipe.isp_top_ctl.YUV_EN.Raw = (dip_ctl_yuv_en &(~dip_yuv_tuning_tag))|(yuv_en & dip_yuv_tuning_tag);
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = (dip_ctl_yuv2_en &(~dip_yuv2_tuning_tag))|(yuv2_en & dip_yuv2_tuning_tag);
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = (dip_ctl_rgb_en &(~dip_rgb_tuning_tag))|(rgb_en & dip_rgb_tuning_tag);
        ispDipPipe.isp_top_ctl.RGB2_EN.Raw = (dip_ctl_rgb2_en &(~dip_rgb2_tuning_tag))|(rgb2_en & dip_rgb2_tuning_tag);

        PIPE_DBG("pIspPhyReg(0x%lx)",(long)ispDipPipe.pTuningIspReg);
    } else {
        ispDipPipe.isApplyTuning = MFALSE;

        ispDipPipe.isp_top_ctl.YUV_EN.Raw = dip_ctl_yuv_en;
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = dip_ctl_yuv2_en;
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = dip_ctl_rgb_en;
        ispDipPipe.isp_top_ctl.RGB2_EN.Raw = dip_ctl_rgb2_en;

        PIPE_WRN("[Warning]p2 tuning not be passed via imageio");
    }

    ispMdpPipe.drvScenario = drvScen;
    ispDipPipe.dupCqIdx = dupCQIdx;
    ispDipPipe.burstQueIdx = burstQIdx;
    ispDipPipe.RingBufIdx = m_pDipWorkingBuf->m_BufIdx;
    ispMdpPipe.lastframe = lastFrame;
    ispMdpPipe.isWaitBuf = isWaitBuf;
    ispDipPipe.dipTh=p2CQ;
    ispDipPipe.dip_cq_thr_ctl=dip_cq_thr_ctl;

    ispDipPipe.isp_top_ctl.DMA_EN.Raw=dip_ctl_dma_en;
    //ispDipPipe.isp_top_ctl.MISC_EN.Raw=dip_misc_en;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.APB_CLK_GATE_BYPASS = 0;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.SRAM_MODE = sram_mode;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.PAK2O_SEL = pak2o_sel;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.PAK2_FMT = pak2o_fmt;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.PAKG2_FMT = pakg2_fmt;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.PAKG2_FULL_G = fg_mode;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.PAKG2_FULL_G_BUS = fg_mode;
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.NBC_GMAP_LTM_MODE = nbc_gmap_ltm_mode;
    //hw default 1, you need to modify it to zero when init and lce enable will the value chnage to 1, and lce disalbe chnage the value to 0
    ispDipPipe.isp_top_ctl.MISC_EN.Bits.WUV_MODE = wuv_mode;

    ispDipPipe.isp_top_ctl.FMT_SEL.Raw=0x0;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMGI_FMT=imgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.VIPI_FMT=vipi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.UFDI_FMT=ufdi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DMGI_FMT=dmgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DEPI_FMT=depi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.FG_MODE=fg_mode;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMG3O_FMT=img3o_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.IMG2O_FMT=img2o_fmt;
    //ispDipPipe.isp_top_ctl.FMT_SEL.Bits.PIX_ID=pPipePackageInfo->pixIdP2;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.PIX_ID=pixIdImgBuf;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.LP_MODE = b12bitMode;
    ispDipPipe.isp_top_ctl.PATH_SEL.Raw=dip_ctl_path_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.G2G_SEL=g2g_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.G2C_SEL=g2c_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.PGN_SEL=pgn_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.NBC_SEL=nbc_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.RCP2_SEL=rcp2_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.FEO_SEL=feo_sel;
    ispDipPipe.isp_top_ctl.PATH_SEL.Bits.G2G2_SEL=g2g2_sel;

    //TODO items
    ispDipPipe.tdr_tcm_en=tdr_tcm_en;
    ispDipPipe.tdr_tcm2_en=tdr_tcm2_en;
    ispDipPipe.tdr_tcm3_en=tdr_tcm3_en;

    ispDipPipe.tdr_tpipe=tdr_tpipe;
    ispDipPipe.tdr_ctl=tdr_ctl;

    ispDipPipe.pUfdParam = pUfdParam;
    //in order to print the log, temp solution.
    if (m_Img3oCropInfo != NULL)
    {
        Img3oCrspTmp.m_CrspInfo.p_integral.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
        Img3oCrspTmp.m_CrspInfo.p_integral.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
    }
    if (pPipePackageInfo->pTuningQue != NULL)
    {
        pTuningDipReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),dupCqIdx(%d),[vipi]ofst(%d),rW/H(%d/%d)\n, \
            isDipOnly(%d), moduleIdx(%d), dipCQ/dup/burst/RingBufIdx(%d/%d/%d/%d), drvSc(%d), isWtf(%d),tdr_tpipe(%d),en_yuv/yuv2/rgb/rgb2/dma/fmt_sel/ctl_sel/misc (0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x/0x%x)\nlast(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),NBC_ANR_CON1(0x%x),BNR2_PDC_CON(0x%x),userName(%s),PIX_ID(%d),ImgBuf_PIX_ID(%d)\n, \
            pDipWBuf(0x%x), cqPa(0x%lx)-Va(0x%lx),tpipePa(0x%lx)-Va(0x%lx),IspVirBufPa(0x%lx)-Va(0x%lx)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx, \
            pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.dupCqIdx,ispDipPipe.burstQueIdx,ispDipPipe.RingBufIdx,\
            ispMdpPipe.drvScenario,ispMdpPipe.isWaitBuf,\
            ispDipPipe.tdr_tpipe,\
            ispDipPipe.isp_top_ctl.YUV_EN.Raw,ispDipPipe.isp_top_ctl.YUV2_EN.Raw,ispDipPipe.isp_top_ctl.RGB_EN.Raw,ispDipPipe.isp_top_ctl.RGB2_EN.Raw, ispDipPipe.isp_top_ctl.DMA_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw,ispDipPipe.isp_top_ctl.PATH_SEL.Raw,ispDipPipe.isp_top_ctl.MISC_EN.Raw,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en, ispDipPipe.tdr_tcm3_en,    lastFrame, m_CrzEn, WdmaoCropGroup, WrotoCropGroup, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y,\
            pTuningDipReg->DIP_X_NBC_ANR_CON1.Raw, pTuningDipReg->DIP_X_BNR2_PDC_CON.Raw, szCallerName, pPipePackageInfo->pixIdP2, pixIdImgBuf,\
            m_pDipWorkingBuf, m_pDipWorkingBuf->m_pIspDescript_phy, m_pDipWorkingBuf->m_pIspDescript_vir, m_pDipWorkingBuf->tpipeTablePa, m_pDipWorkingBuf->tpipeTableVa, m_pDipWorkingBuf->m_pIspVirRegAddr_pa, m_pDipWorkingBuf->m_pIspVirRegAddr_va);

    }
    else
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),dupCqIdx(%d)\n, \
            isDipOnly(%d), moduleIdx(%d), dipCQ/dup/burst/RingBufIdx(%d/%d/%d/%d), drvSc(%d), isWtf(%d),tdr_tpipe(%d),en_yuv/yuv2/rgb/rgb2/dma/fmt_sel/ctl_sel/misc (0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x/0x%x)\nlast(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),userName(%s),PIX_ID(%d),ImgBuf_PIX_ID(%d)\n, \
            pDipWBuf(0x%x), cqPa(0x%lx)-Va(0x%lx),tpipePa(0x%lx)-Va(0x%lx),IspVirBufPa(0x%lx)-Va(0x%lx)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->dupCqIdx, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.dupCqIdx,ispDipPipe.burstQueIdx,ispDipPipe.RingBufIdx,\
            ispMdpPipe.drvScenario,ispMdpPipe.isWaitBuf,\
            ispDipPipe.tdr_tpipe,\
            ispDipPipe.isp_top_ctl.YUV_EN.Raw,ispDipPipe.isp_top_ctl.YUV2_EN.Raw,ispDipPipe.isp_top_ctl.RGB_EN.Raw,ispDipPipe.isp_top_ctl.RGB2_EN.Raw, ispDipPipe.isp_top_ctl.DMA_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw,ispDipPipe.isp_top_ctl.PATH_SEL.Raw,ispDipPipe.isp_top_ctl.MISC_EN.Raw,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en, ispDipPipe.tdr_tcm3_en,     lastFrame, m_CrzEn, WdmaoCropGroup, WrotoCropGroup,\
            Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y, szCallerName, pPipePackageInfo->pixIdP2, pixIdImgBuf,
            m_pDipWorkingBuf, m_pDipWorkingBuf->m_pIspDescript_phy, m_pDipWorkingBuf->m_pIspDescript_vir, m_pDipWorkingBuf->tpipeTablePa, m_pDipWorkingBuf->tpipeTableVa, m_pDipWorkingBuf->m_pIspVirRegAddr_pa, m_pDipWorkingBuf->m_pIspVirRegAddr_va);
    }

    /////////////////////////////////////////////////////////////
    //config ports
    if (-1 != idx_imgi )
    {
        this->configInDmaPort(portInfo_imgi, ispDipPipe.DMAImgi.dma_cfg, (MUINT32)1, (MUINT32)dmai_swap, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgi);

        PIPE_DBG("[imgi]size[%lu,%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImgi.dma_cfg.size.w,ispDipPipe.DMAImgi.dma_cfg.size.h,ispDipPipe.DMAImgi.dma_cfg.size.xsize,
                ispDipPipe.DMAImgi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAImgi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgi.dma_cfg.size.stride);

        if ((imgi_planeNum >= 2) && (isUfoEn == MFALSE))
        {
            if (-1 != idx_imgbi )
            {    //multi-plane input image, imgi/imgbi/imgci
                this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_imgbi);
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
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_imgci);
                    ispDipPipe.DMAImgci.dma_cfg.size.w /= imgi_uv_h_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.h /= imgi_uv_v_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.xsize /= imgi_uv_h_ratio;
                    if(imgi_uv_swap)
                    {
                        //MUINTPTR tmp_base_vAddr = ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr;
                        //MUINTPTR tmp_base_pAddr = ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr;
                        //ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr = ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr;
                        //ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr;
                        //ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr = tmp_base_vAddr;
                        //ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr = tmp_base_pAddr;
                        doUVSwap(&ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr);
                        doUVSwap(&ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr);
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
                    this->configInDmaPort(portInfo_imgci,ispDipPipe.DMAImgci.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    //ispDipPipe.DMAImgci.dma_cfg.size.stride = portInfo_imgci.u4Stride[ESTRIDE_1ST_PLANE];
                }
            }
        }
        else
        {
            if (eDrvScenario_MFB_Blending == drvScen)
            {
                //
                //Blending
                if (-1 != idx_imgbi )
                {    //multi-plane input image, imgi/imgbi/imgci
                    this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)2, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgbi);
                    PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                        ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                        (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
                }


                if (-1 != idx_imgci )
                {
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",
                            ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }

                ispDipPipe.mfb_cfg.out_xofst=0;

                //TODO under check?
                if(idx_imgci != -1)
                {    //other blending sequences
                    ispDipPipe.mfb_cfg.bld_mode=1;
                }
                else
                {    //first blending
                    ispDipPipe.mfb_cfg.bld_mode=0;
                }
                //m_camPass2Param.mfb_cfg.bld_mode=0;
                ispDipPipe.mfb_cfg.bld_ll_db_en=0;
                ispDipPipe.mfb_cfg.bld_ll_brz_en=1;
            }
            else  //General Case, origian: only mixing mode. add pure raw case.
            {
                if (-1 != idx_imgbi )
                {
                    this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgbi);
                    PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                        ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                        (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
                }

                if (-1 != idx_imgci )
                {
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    PIPE_DBG("[imgci]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }
            }

        }


        if (MTRUE == m_CrzEn)
        {
            MBOOL crzResult = MTRUE;   // MTRUE: success. MFALSE: fail.

            ispDipPipe.crzPipe.conf_cdrz = MTRUE;;
            ispDipPipe.crzPipe.crz_out.w = CropInfo_imgi.resize1.tar_w;
            ispDipPipe.crzPipe.crz_out.h = CropInfo_imgi.resize1.tar_h;
            ispDipPipe.crzPipe.crz_in.w = ispDipPipe.DMAImgi.dma_cfg.size.w;
            ispDipPipe.crzPipe.crz_in.h = ispDipPipe.DMAImgi.dma_cfg.size.h;
            ispDipPipe.crzPipe.crz_crop.x = CropInfo_imgi.crop1.x;
            ispDipPipe.crzPipe.crz_crop.floatX = CropInfo_imgi.crop1.floatX;
            //ispDipPipe.crzPipe.crz_crop.floatX = ((CropInfo_imgi.crop1.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
            ispDipPipe.crzPipe.crz_crop.y = CropInfo_imgi.crop1.y;
            ispDipPipe.crzPipe.crz_crop.floatY = CropInfo_imgi.crop1.floatY;
            //ispDipPipe.crzPipe.crz_crop.floatY = ((CropInfo_imgi.crop1.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
            ispDipPipe.crzPipe.crz_crop.w = CropInfo_imgi.crop1.w;
            ispDipPipe.crzPipe.crz_crop.h = CropInfo_imgi.crop1.h;

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
        ispDipPipe.pRegiAddr  = portInfo_regi->mBuffer->getBufVA(ESTRIDE_1ST_PLANE);
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
        this->configInDmaPort(portInfo_vipi, ispDipPipe.DMAVipi.dma_cfg, (MUINT32)1, (MUINT32)1, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_vipi);

        PIPE_DBG("[vipi]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAVipi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAVipi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVipi.dma_cfg.memBuf.alignment,ispDipPipe.DMAVipi.dma_cfg.size.stride);

        if (-1 != idx_vip2i )
        {    //multi-plane input image, vipi/vip2i/vip3i
            this->configInDmaPort(portInfo_vip2i, ispDipPipe.DMAVip2i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_vip2i);
            ispDipPipe.DMAVip2i.dma_cfg.size.w /= vipi_uv_h_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.h /= vipi_uv_v_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
            PIPE_DBG("[vip2i]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAVip2i.dma_cfg.size.w,ispDipPipe.DMAVip2i.dma_cfg.size.h,
                    ispDipPipe.DMAVip2i.dma_cfg.memBuf.size,ispDipPipe.DMAVip2i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip2i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip2i.dma_cfg.size.stride);

            if (-1 != idx_vip3i )
            {
                this->configInDmaPort(portInfo_vip3i, ispDipPipe.DMAVip3i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_vip3i);
                ispDipPipe.DMAVip3i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.h /= vipi_uv_v_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
                if(vipi_uv_swap) {
                    //ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_2ND_PLANE];
                    //ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE];
                    doUVSwap(&ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr);
                    doUVSwap(&ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr);
                }
                PIPE_DBG("[vip3i]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAVip3i.dma_cfg.size.w,ispDipPipe.DMAVip3i.dma_cfg.size.h,
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip3i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip3i.dma_cfg.size.stride,vipi_uv_swap);
            }
        }
        // for NR3D set vipi information +++++
        if (pPipePackageInfo->pTuningQue != NULL)
        {
            pTuningDipReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;// check tuning enable bit on isp_function_dip
            if (pTuningDipReg->DIP_X_CTL_YUV_EN.Bits.NR3D_EN & pTuningDipReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN)
            {
                switch(eImgFmt_vipi)
                {
                    case eImgFmt_YUY2:
                        pixelInByte=2;
                        ispDipPipe.DMAVipi.dma_cfg.offset.x = (pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw % ispDipPipe.DMAVipi.dma_cfg.size.stride)/pixelInByte;
                        ispDipPipe.DMAVipi.dma_cfg.offset.y = pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw / ispDipPipe.DMAVipi.dma_cfg.size.stride;
                        ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVipi.dma_cfg.offset.y * ispDipPipe.DMAVipi.dma_cfg.size.stride + ispDipPipe.DMAVipi.dma_cfg.offset.x * pixelInByte);
                        break;
                    case eImgFmt_YV12:
                    default:
                        pixelInByte=1;
                        ispDipPipe.DMAVipi.dma_cfg.offset.x = (pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw % ispDipPipe.DMAVipi.dma_cfg.size.stride)/pixelInByte;
                        ispDipPipe.DMAVipi.dma_cfg.offset.y = pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw / ispDipPipe.DMAVipi.dma_cfg.size.stride;
                        ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVipi.dma_cfg.offset.y * ispDipPipe.DMAVipi.dma_cfg.size.stride + ispDipPipe.DMAVipi.dma_cfg.offset.x * pixelInByte);
                        ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVip2i.dma_cfg.offset.y/2 * ispDipPipe.DMAVip2i.dma_cfg.size.stride + ispDipPipe.DMAVip2i.dma_cfg.offset.x/2 * pixelInByte);
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVip3i.dma_cfg.offset.y/2 * ispDipPipe.DMAVip3i.dma_cfg.size.stride + ispDipPipe.DMAVip3i.dma_cfg.offset.x/2 * pixelInByte);

                        //portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] +
                        //    (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip2i.xoffset * pixelInByte);
                        //portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] +
                        //    (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);
                        //portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] +
                        //    (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);

                        //portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] +
                        //    (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                        //portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] +
                        //    (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                        //portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] +
                        //    (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                        break;
                }
                ispDipPipe.DMAVipi.dma_cfg.size.w = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVipi.dma_cfg.size.h = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip2i.dma_cfg.size.w = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVip2i.dma_cfg.size.h = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip3i.dma_cfg.size.w = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVip3i.dma_cfg.size.h = pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip2i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip2i.dma_cfg.size.h /= vipi_uv_v_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.h /= vipi_uv_v_ratio;
                PIPE_DBG("[vipi]ofst(%d),rW/H(%d/%d)",pTuningDipReg->DIP_X_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->DIP_X_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT);
            }
        }
        // for NR3D set vipi information -----
    }

    //
    if (-1 != idx_ufdi ) {
        PIPE_DBG("config ufdi");
        if ((eImgFmt_ufdi == eImgFmt_UFO_FG_BAYER10) || (eImgFmt_ufdi == eImgFmt_UFO_FG_BAYER12) || (eImgFmt_ufdi == eImgFmt_UFO_BAYER10) || (eImgFmt_ufdi == eImgFmt_UFO_BAYER12))
        {
            this->configInDmaPort(portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_2ND_PLANE, eImgFmt_ufdi);
            //ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_2ND_PLANE];
            ispDipPipe.DMAUfdi.dma_cfg.size.xsize = (((ispDipPipe.DMAUfdi.dma_cfg.size.w+63)/64)+7)/8*8;
        }
        else
        {
            this->configInDmaPort(portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_ufdi);
            //ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_1ST_PLANE];
        }
    }
    //
    if (-1 != idx_dmgi ) {
        //PIPE_DBG("config dmgi");
        this->configInDmaPort(portInfo_dmgi,ispDipPipe.DMADmgi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_dmgi);
        //ispDipPipe.DMADmgi.dma_cfg.size.stride = portInfo_dmgi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_depi ) {
        //PIPE_DBG("config ufdi");
        this->configInDmaPort(portInfo_depi,ispDipPipe.DMADepi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_depi);
        //ispDipPipe.DMADepi.dma_cfg.size.stride = portInfo_depi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lcei )
    {
        //PIPE_DBG("config lcei");
        this->configInDmaPort(portInfo_lcei,ispDipPipe.DMALcei.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_lcei);
        //ispDipPipe.DMALcei.dma_cfg.size.stride = portInfo_lcei.u4Stride[ESTRIDE_1ST_PLANE];
    }

    //
    if ( -1 != idx_wroto) {
        //PIPE_DBG("config wroto");
        //support different view angle
        ispMdpPipe.wroto_out.enSrcCrop=true;
        ispMdpPipe.wroto_out.srcCropX = CropInfo_imgi.crop3.x;
        ispMdpPipe.wroto_out.srcCropFloatX = CropInfo_imgi.crop3.floatX;
        //ispMdpPipe.wroto_out.srcCropFloatX = \
        //    ((CropInfo_imgi.crop3.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wroto_out.srcCropY= CropInfo_imgi.crop3.y;
        ispMdpPipe.wroto_out.srcCropFloatY = CropInfo_imgi.crop3.floatY;
        //ispMdpPipe.wroto_out.srcCropFloatY =  \
        //    ((CropInfo_imgi.crop3.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wroto_out.srcCropW = CropInfo_imgi.crop3.w;
        ispMdpPipe.wroto_out.srcCropFloatW = CropInfo_imgi.crop3.floatW;
        ispMdpPipe.wroto_out.srcCropH = CropInfo_imgi.crop3.h;
        ispMdpPipe.wroto_out.srcCropFloatH = CropInfo_imgi.crop3.floatH;
        //
        ispMdpPipe.wroto_out.crop_group = WrotoCropGroup;
        this->configMdpOutPort(portInfo_wroto,ispMdpPipe.wroto_out,eImgFmt_wroto);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_wdmao) {
        //PIPE_DBG("config wdmao");
        //
        // for digital zoom crop
        ispMdpPipe.wdmao_out.enSrcCrop=true;
        ispMdpPipe.wdmao_out.srcCropX = CropInfo_imgi.crop2.x;
        ispMdpPipe.wdmao_out.srcCropFloatX = CropInfo_imgi.crop2.floatX;
        //ispMdpPipe.wdmao_out.srcCropFloatX = \
        //        ((CropInfo_imgi.crop2.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wdmao_out.srcCropY= CropInfo_imgi.crop2.y;
        ispMdpPipe.wdmao_out.srcCropFloatY = CropInfo_imgi.crop2.floatY;
        //ispMdpPipe.wdmao_out.srcCropFloatY =  \
        //    ((CropInfo_imgi.crop2.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        ispMdpPipe.wdmao_out.srcCropW = CropInfo_imgi.crop2.w;
        ispMdpPipe.wdmao_out.srcCropFloatW = CropInfo_imgi.crop2.floatW;
        ispMdpPipe.wdmao_out.srcCropH = CropInfo_imgi.crop2.h;
        ispMdpPipe.wdmao_out.srcCropFloatH = CropInfo_imgi.crop2.floatH;
        //
        ispMdpPipe.wdmao_out.crop_group = WdmaoCropGroup;
        this->configMdpOutPort(portInfo_wdmao,ispMdpPipe.wdmao_out,eImgFmt_wdmao);
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
        ispMdpPipe.venco_out.srcCropX = CropInfo_imgi.crop2.x;
        ispMdpPipe.venco_out.srcCropFloatX = CropInfo_imgi.crop2.floatX;
        ispMdpPipe.venco_out.srcCropY= CropInfo_imgi.crop2.y;
        ispMdpPipe.venco_out.srcCropFloatY =  CropInfo_imgi.crop2.floatY;
        ispMdpPipe.venco_out.srcCropW = CropInfo_imgi.crop2.w;
        ispMdpPipe.venco_out.srcCropFloatW = CropInfo_imgi.crop2.floatW;
        ispMdpPipe.venco_out.srcCropH = CropInfo_imgi.crop2.h;
        ispMdpPipe.venco_out.srcCropFloatH = CropInfo_imgi.crop2.floatH;
        //
        this->configMdpOutPort(portInfo_venco,ispMdpPipe.venco_out,eImgFmt_venco);
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
        //PIPE_DBG("img2o u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_img2o]->u4BufSize[0],(unsigned long)vOutPorts[idx_img2o]->u4BufVA[0],(unsigned long)vOutPorts[idx_img2o]->u4BufPA[0]);
        this->configOutDmaPort(portInfo_img2o,ispDipPipe.DMAImg2o.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE,eImgFmt_img2o);
        //
        //ispDipPipe.DMAImg2o.dma_cfg.size.stride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        PIPE_DBG("[img2o]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2o.dma_cfg.size.w,ispDipPipe.DMAImg2o.dma_cfg.size.h,
                ispDipPipe.DMAImg2o.dma_cfg.memBuf.size,ispDipPipe.DMAImg2o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2o.dma_cfg.size.stride);

        if (-1 != idx_img2bo )
        {    //multi-plane input image, img2o/img2bo
            this->configOutDmaPort(portInfo_img2bo, ispDipPipe.DMAImg2bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE,eImgFmt_img2bo);
            ispDipPipe.DMAImg2bo.dma_cfg.size.w /=img2o_uv_h_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.h /= img2o_uv_v_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.xsize /= img2o_uv_h_ratio;
            PIPE_DBG("[img2bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2bo.dma_cfg.size.w,ispDipPipe.DMAImg2bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg2bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2bo.dma_cfg.size.stride);
        }

        if(isSetMdpOut == MFALSE)
        {
            this->configMdpOutPort(portInfo_img2o,ispMdpPipe.imgxo_out,eImgFmt_img2o);
            mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_IMG2O;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_img3o )
    {
        this->configOutDmaPort(portInfo_img3o, ispDipPipe.DMAImg3o.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_img3o);

        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(portInfo_img3o,ispMdpPipe.imgxo_out,eImgFmt_img3o);
            mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_IMG3O;
            isSetMdpOut = MTRUE;
        }
        PIPE_DBG("[img3o]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImg3o.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3o.dma_cfg.size.stride);

        if (-1 != idx_img3bo )
        {
            this->configOutDmaPort(portInfo_img3bo, ispDipPipe.DMAImg3bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_img3bo);
            ispDipPipe.DMAImg3bo.dma_cfg.size.w /= img3o_uv_h_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.h /= img3o_uv_v_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.xsize /= img3o_uv_h_ratio;
            PIPE_DBG("[img3bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg3bo.dma_cfg.size.w,ispDipPipe.DMAImg3bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg3bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3bo.dma_cfg.size.stride);

            if (-1 != idx_img3co )
            {
                this->configOutDmaPort(portInfo_img3co, ispDipPipe.DMAImg3co.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_img3co);
                ispDipPipe.DMAImg3co.dma_cfg.size.w /= img3o_uv_h_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.h /= img3o_uv_v_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.xsize /= img3o_uv_h_ratio;
                if(img3o_uv_swap) {
                    //ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_2ND_PLANE];
                    //ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_2ND_PLANE];
                    doUVSwap(&ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr);
                    doUVSwap(&ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr);
                }
                PIPE_DBG("[img3co]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),img3o_uv_swap(%d)",ispDipPipe.DMAImg3co.dma_cfg.size.w,ispDipPipe.DMAImg3co.dma_cfg.size.h,
                        ispDipPipe.DMAImg3co.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3co.dma_cfg.size.stride,img3o_uv_swap);
            }
        }
        if (m_Img3oCropInfo != NULL)
        {
            //support different view angle
            ispDipPipe.DMAImg3o.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x; //img3o offset must be even
            ispDipPipe.DMAImg3o.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            ispDipPipe.DMAImg3co.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            ispDipPipe.DMAImg3co.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            PIPE_DBG("IMG3O cpoft(%d/%d)",ispDipPipe.DMAImg3o.dma_cfg.offset.x,ispDipPipe.DMAImg3o.dma_cfg.offset.y);
        }
        else
        {
            //support different view angle
            ispDipPipe.DMAImg3o.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x & (~1); //img3o offset must be even
            ispDipPipe.DMAImg3o.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y & (~1);
            ispDipPipe.DMAImg3bo.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y;
            ispDipPipe.DMAImg3co.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x;
            ispDipPipe.DMAImg3co.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y;
            PIPE_DBG("IMG3O cpoft(%d/%d)",ispDipPipe.DMAImg3o.dma_cfg.offset.x,ispDipPipe.DMAImg3o.dma_cfg.offset.y);
        }

    }
    //
#if 0
    if (-1 != idx_mfbo)
    {
        this->configOutDmaPort(portInfo_mfbo, ispDipPipe.DMAMfbo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_mfbo);
        //if(isSetMdpOut == MFALSE){
        //      this->configMdpOutPort(vOutPorts[idx_mfbo],ispMdpPipe.imgxo_out);
        //      mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_MFBO;
        //      isSetMdpOut = MTRUE;
        //}
    }
#endif
    if (-1 != idx_pak2o)
    {
        this->configOutDmaPort(portInfo_pak2o, ispDipPipe.DMAPak2o.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_pak2o);
        //if(isSetMdpOut == MFALSE){
        //      this->configMdpOutPort(vOutPorts[idx_mfbo],ispMdpPipe.imgxo_out);
        //      mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_MFBO;
        //      isSetMdpOut = MTRUE;
        //}
    }

    if (-1 != idx_feo )
    {
        //PIPE_DBG("feo u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_feo]->u4BufSize[0],(unsigned long)vOutPorts[idx_feo]->u4BufVA[0],(unsigned long)vOutPorts[idx_feo]->u4BufPA[0]);
        this->configOutDmaPort(portInfo_feo, ispDipPipe.DMAFeo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_feo);

        PIPE_DBG("[feo]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x)",
            ispDipPipe.DMAFeo.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAFeo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAFeo.dma_cfg.memBuf.alignment);

         if(isSetMdpOut == MFALSE)
         {
             this->configMdpOutPort(portInfo_feo, ispMdpPipe.imgxo_out, eImgFmt_feo);
             mdp_imgxo_p2_en = DIP_X_REG_CTL_DMA_EN_FEO;
             isSetMdpOut = MTRUE;
         }

    }
    ispMdpPipe.mdp_imgxo_p2_en = mdp_imgxo_p2_en;

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
                        ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAImg2o.dma_cfg.size.w;
                        ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAImg2o.dma_cfg.size.h;
                        ispMdpPipe.p2MdpSrcFmt = eImgFmt_img2o;
                        switch(eImgFmt_img2o){
                            case eImgFmt_NV16:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImg2o.dma_cfg.size.h * ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImg2bo.dma_cfg.size.h * ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = 0;
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImg2o.dma_cfg.size.h * ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImg2bo.dma_cfg.size.h * ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = 0;
                                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_IMG3O:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_IMG3O){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAImg3o.dma_cfg.size.w;
                        ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAImg3o.dma_cfg.size.h;
                        ispMdpPipe.p2MdpSrcFmt = eImgFmt_img3o;
                        switch(eImgFmt_img3o){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImg3bo.dma_cfg.size.h * ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImg3co.dma_cfg.size.h * ispDipPipe.DMAImg3co.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImg3bo.dma_cfg.size.h * ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImg3co.dma_cfg.size.h * ispDipPipe.DMAImg3co.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = 0;
                                ispMdpPipe.p2MdpSrcVSize = 0;
                                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_FEO:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_FEO){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAFeo.dma_cfg.size.w;
                        ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAFeo.dma_cfg.size.h;
                        ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAFeo.dma_cfg.size.stride;
                        ispMdpPipe.p2MdpSrcUVStride = 0; // only one plane
                        ispMdpPipe.p2MdpSrcFmt = eImgFmt_imgi; //kk test
                    }
                    break;
                case EPortIndex_VIPI:
                    if(dip_ctl_dma_en & DIP_X_REG_CTL_DMA_EN_VIPI){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        ispMdpPipe.p2MdpSrcFmt = eImgFmt_vipi;
                        ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAVipi.dma_cfg.size.w;
                        ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAVipi.dma_cfg.size.h;
                        switch(eImgFmt_vipi){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            case eImgFmt_YV16:
                            case eImgFmt_I422:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                                break;
                            default:
                                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcUVStride = 0;
                                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
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
            ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAImgi.dma_cfg.size.w;
            ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAImgi.dma_cfg.size.h;
            ispMdpPipe.p2MdpSrcFmt = eImgFmt_imgi;
            switch(eImgFmt_imgi){
                case eImgFmt_NV21:
                case eImgFmt_NV12:
                case eImgFmt_NV16:
                    ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcPlaneNum = 2;
                    break;
                case eImgFmt_YV12:
                case eImgFmt_I420:
                    ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcPlaneNum = 3;
                    break;
                case eImgFmt_YV16:
                case eImgFmt_I422:
                    ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcPlaneNum = 3;
                    break;
                default:
                    ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcUVStride = 0;
                    ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    ispMdpPipe.p2MdpSrcPlaneNum = 1;
                    break;
            }
        }
    } else {
        //set mdp src info. equal to imgi
        ispMdpPipe.p2MdpSrcW = ispDipPipe.DMAImgi.dma_cfg.size.w;
        ispMdpPipe.p2MdpSrcH = ispDipPipe.DMAImgi.dma_cfg.size.h;
        ispMdpPipe.p2MdpSrcFmt = eImgFmt_imgi;
        switch(eImgFmt_imgi){
            case eImgFmt_NV21:
            case eImgFmt_NV12:
            case eImgFmt_NV16:
                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcPlaneNum = 2;
                break;
            case eImgFmt_YV12:
            case eImgFmt_I420:
                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                break;
            case eImgFmt_YV16:
            case eImgFmt_I422:
                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcPlaneNum = 3;
                break;
            default:
                ispMdpPipe.p2MdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcUVStride = 0;
                ispMdpPipe.p2MdpSrcSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcCSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcVSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                ispMdpPipe.p2MdpSrcPlaneNum = 1;
                break;
            }
    }

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
    //ispDipPipe.DMAMfbo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.MFBO_EN)? 0 : 1;
    //dma_pak2o
    ispDipPipe.DMAPak2o.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.PAK2O_EN)? 0 : 1;
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
    //PmQos
    if (drvScen == eDrvScenario_FM)
    {
        ispMdpPipe.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        ispMdpPipe.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        ispMdpPipe.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMAPak2o.dma_cfg.size.xsize*ispDipPipe.DMAPak2o.dma_cfg.size.h);
    }
    else
    {
        ispMdpPipe.framemode_total_in_w = 0;
        ispMdpPipe.framemode_h = 0;
        ispMdpPipe.total_data_size = 0;
    }
    //
    ispMdpPipe.bypass = ispDipPipe.isDipOnly;
    ispMdpPipe.dipCtlDmaEn = ispDipPipe.isp_top_ctl.DMA_EN.Raw; //duplicate parameter
    ispMdpPipe.dip_dma_en = ispDipPipe.isp_top_ctl.DMA_EN.Raw;
    ispMdpPipe.imgi_dma_cfg = ispDipPipe.DMAImgi.dma_cfg;
    ispMdpPipe.yuv2_en = ispDipPipe.isp_top_ctl.YUV2_EN.Raw;

    PIPE_DBG("[ispMdpPipe]p2MdpSrcFmt(0x%x),w(%d),h(%d),stride(%d,%d),size(%d,%d,%d),planeN(%d)", \
        ispMdpPipe.p2MdpSrcFmt,ispMdpPipe.p2MdpSrcW,ispMdpPipe.p2MdpSrcH,ispMdpPipe.p2MdpSrcYStride,ispMdpPipe.p2MdpSrcUVStride,ispMdpPipe.p2MdpSrcSize,ispMdpPipe.p2MdpSrcCSize,ispMdpPipe.p2MdpSrcVSize,ispMdpPipe.p2MdpSrcPlaneNum);
    PIPE_DBG("ispDipPipe.isp_top_ctl.YUV_EN.Bits.CRZ_EN %d/%d",ispDipPipe.isp_top_ctl.YUV_EN.Bits.CRZ_EN, m_CrzEn);

    ispMdpPipe.dipTh = ispDipPipe.dipTh;
    ispMdpPipe.dupCqIdx = ispDipPipe.dupCqIdx;
    ispMdpPipe.burstQueIdx = ispDipPipe.burstQueIdx;
    ispMdpPipe.RingBufIdx = ispDipPipe.RingBufIdx;
    ispMdpPipe.pTuningIspReg = ispDipPipe.pTuningIspReg;
    ispMdpPipe.isDipOnly = ispDipPipe.isDipOnly;
    ispMdpPipe.isSecureFra = ispDipPipe.isSecureFra;

    ret= ret & ispDipPipe.ConfigDipPipe(m_pDipWorkingBuf);
    ret = ret & ispMdpPipe.configMDP(m_pDipWorkingBuf);

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
    if (data == NULL)
    {
        PIPE_ERR("malloc data fail");
        ret = MFALSE;
        return ret;
    }
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
PostProcPipe::configInDmaPort(NSCam::NSIoPipe::Input const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt)
{
    ISP_QUERY_RST queryRst;
    MINT32 pixelDivNum=0, pixelMulNum=0;
    E_ISP_PIXMODE e_PixMode = ISP_QUERY_UNKNOWN_PIX_MODE;
    //
    a_dma.memBuf.size        = portInfo->mBuffer->getBufSizeInBytes(planeNum);
    a_dma.memBuf.base_vAddr  = portInfo->mBuffer->getBufVA(planeNum);
    if ((portInfo->mSecureTag != 0) & (portInfo->mSecHandle != 0))	// For tuning secure check
    {
        a_dma.memBuf.base_pAddr  = portInfo->mSecHandle;
    }
    else
    {
        a_dma.memBuf.base_pAddr  = portInfo->mBuffer->getBufPA(planeNum);
    }
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->mBuffer->getImgSize().w;
    a_dma.size.h            = portInfo->mBuffer->getImgSize().h;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->mBuffer->getBufStridesInBytes(planeNum);

    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
    switch(eImgFmt)
    {
        case eImgFmt_YUY2:
            pixelDivNum=2;
            pixelMulNum=1;
            break;
        case eImgFmt_BAYER10:
            pixelDivNum=4;
            pixelMulNum=5;
            break;
        case eImgFmt_BAYER12:
            pixelDivNum=2;
            pixelMulNum=3;
            break;
        case eImgFmt_BAYER8:
        case eImgFmt_Y8:
        case eImgFmt_YV12:
        default:
            pixelDivNum=1;
            pixelMulNum=1;
            break;
    }
    //dma starting offset
    if (planeNum == ESTRIDE_1ST_PLANE)
       {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % a_dma.size.stride)/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / a_dma.size.stride;
    }
    else
    {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % (portInfo->mBuffer->getBufStridesInBytes(0)))/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / (portInfo->mBuffer->getBufStridesInBytes(0));
    }
    //
    #if 0
    a_dma.format = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->mPortID.capbility;

    a_dma.secureTag=portInfo->mSecureTag;

    //input xsize unit:byte

    ISP_QuerySize(portInfo->mPortID.index,ISP_QUERY_XSIZE_BYTE,(EImageFormat)eImgFmt,a_dma.size.w,queryRst,e_PixMode);

    a_dma.size.xsize        =  queryRst.xsize_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    // no need check crop information for DMA port
    //a_dma.crop.x            = portInfo->crop1.x;
    //a_dma.crop.floatX        = portInfo->crop1.floatX;
    //a_dma.crop.y            = portInfo->crop1.y;
    //a_dma.crop.floatY        = portInfo->crop1.floatY;
    //a_dma.crop.w            = portInfo->crop1.w;
    //a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( a_dma.crop.y ? (a_dma.crop.y- 1) : 0 )*(a_dma.size.w)+a_dma.crop.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;
    //
    ISP_QueryBusSize((EImageFormat)eImgFmt, a_dma.bus_size);

    switch( eImgFmt ) {
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
        case eImgFmt_BAYER10_MIPI:      /*!< Bayer format, 10-bit (MIPI) */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            //a_dma.bus_size=1;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV61:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in ISP port",eImgFmt);
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
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configOutDmaPort(NSCam::NSIoPipe::Output const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt)
{
    ISP_QUERY_RST queryRst;
    MINT32 pixelDivNum=0, pixelMulNum=0;
    E_ISP_PIXMODE e_PixMode = ISP_QUERY_UNKNOWN_PIX_MODE;
    //
    a_dma.memBuf.size        = portInfo->mBuffer->getBufSizeInBytes(planeNum);
    a_dma.memBuf.base_vAddr  = portInfo->mBuffer->getBufVA(planeNum);
    a_dma.memBuf.base_pAddr  = portInfo->mBuffer->getBufPA(planeNum);
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->mBuffer->getImgSize().w;
    a_dma.size.h            = portInfo->mBuffer->getImgSize().h;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->mBuffer->getBufStridesInBytes(planeNum);

    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
    switch(eImgFmt)
    {
        case eImgFmt_YUY2:
            pixelDivNum=2;
            pixelMulNum=1;
            break;
        case eImgFmt_BAYER10:
            pixelDivNum=4;
            pixelMulNum=5;
            break;
        case eImgFmt_BAYER12:
            pixelDivNum=2;
            pixelMulNum=3;
            break;
        case eImgFmt_BAYER8:
        case eImgFmt_Y8:
        case eImgFmt_YV12:
        default:
            pixelDivNum=1;
            pixelMulNum=1;
            break;
    }
    //dma starting offset
    if (planeNum == ESTRIDE_1ST_PLANE)
       {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % a_dma.size.stride)/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / a_dma.size.stride;
    }
    else
    {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % (portInfo->mBuffer->getBufStridesInBytes(0)))/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / (portInfo->mBuffer->getBufStridesInBytes(0));
    }
    //
    #if 0
    a_dma.format = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->mPortID.capbility;//portInfo->capbility;

    a_dma.secureTag=portInfo->mSecureTag;

    //input xsize unit:byte

    ISP_QuerySize(portInfo->mPortID.index,ISP_QUERY_XSIZE_BYTE,(EImageFormat)eImgFmt,a_dma.size.w,queryRst,e_PixMode);

    a_dma.size.xsize        =  queryRst.xsize_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    // no need check crop information for DMA port
    //a_dma.crop.x            = portInfo->crop1.x;
    //a_dma.crop.floatX        = portInfo->crop1.floatX;
    //a_dma.crop.y            = portInfo->crop1.y;
    //a_dma.crop.floatY        = portInfo->crop1.floatY;
    //a_dma.crop.w            = portInfo->crop1.w;
    //a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( a_dma.crop.y ? (a_dma.crop.y- 1) : 0 )*(a_dma.size.w)+a_dma.crop.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;
    //
    ISP_QueryBusSize((EImageFormat)eImgFmt, a_dma.bus_size);

    switch( eImgFmt ) {
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
        case eImgFmt_BAYER10_MIPI:      /*!< Bayer format, 10-bit (MIPI) */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            //a_dma.bus_size=1;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV61:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in ISP port",eImgFmt);
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
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configMdpOutPort(NSCam::NSIoPipe::Output const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::EImageFormat eImgFmt)
{
    MUINT32 plane_num = 1;
    MUINT32 uv_resample = 1;
    MUINT32 uv_h_ratio = 1;
    MUINT32 uv_v_ratio = 1;
    MUINT32 y_plane_size = 0;
    MUINT32 u_plane_size = 0;

    //PIPE_DBG("[oImgInfo]w(%d),h(%d),stride(%d/%d/%d),crop(%d,%d,%d,%d)_f(0x%x, 0x%x),rot(%d) eImgFmt(%d)",
    //    oImgInfo->u4ImgWidth,oImgInfo->u4ImgHeight,oImgInfo->u4Stride[ESTRIDE_1ST_PLANE],oImgInfo->u4Stride[ESTRIDE_2ND_PLANE],oImgInfo->u4Stride[ESTRIDE_3RD_PLANE], \
    //    oImgInfo->crop1.x,oImgInfo->crop1.y,oImgInfo->crop1.w,oImgInfo->crop1.h,oImgInfo->crop1.floatX,oImgInfo->crop1.floatY, \
    //    oImgInfo->eImgRot,oImgInfo->eImgFmt );
    //
    a_rotDma.uv_plane_swap = 0;
    //
    switch( eImgFmt ) {
       case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
       case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
           a_rotDma.Format = eImgFmt_YV12;
           a_rotDma.Plane = 2;
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 2;
           a_rotDma.uv_plane_swap = (eImgFmt_YV12==eImgFmt)?1:0;
           break;
        case eImgFmt_I422:      //422 format, 3 plane(YUV)
           a_rotDma.Format = eImgFmt_I422;
           a_rotDma.Plane = 2;
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 1;
           break;
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
            a_rotDma.Format = eImgFmt_NV21;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 2;
            break;
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            a_rotDma.Format = eImgFmt_NV12;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_rotDma.Format = eImgFmt_YUY2;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_UYVY;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_YVYU:		//= 0x080000,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_YVYU;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_VYUY:      //= 0x100000,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_VYUY;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
            a_rotDma.Format = eImgFmt_YV16;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            a_rotDma.Format = eImgFmt_NV16;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            break;
        case eImgFmt_NV61:      //422 format, 2 plane
            a_rotDma.Format = eImgFmt_NV61;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_rotDma.Format = eImgFmt_RGB565;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_rotDma.Format = eImgFmt_RGB888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_rotDma.Format = eImgFmt_ARGB8888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_RGBA8888:
            a_rotDma.Format = eImgFmt_RGBA8888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_Y800:        //= 0x040000, //Y plane only
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y16:
            a_rotDma.Format = eImgFmt_STA_BYTE;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_JPG_I420:
            a_rotDma.Format = eImgFmt_JPG_I420;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_JPG_I422:
            a_rotDma.Format = eImgFmt_JPG_I422;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit     // for imgo
            a_rotDma.Format = eImgFmt_BAYER8;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit    // for imgo
            a_rotDma.Format = eImgFmt_BAYER10;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit    // for imgo
            a_rotDma.Format = eImgFmt_BAYER12;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        case eImgFmt_UFO_FG:
        default:
            PIPE_ERR("vOutPorts[]->eImgFmt:Format(%d) NOT Support",eImgFmt);
            break;
    }
    //
    //ROTATION, stride is after, others are before
    a_rotDma.Rotation = (MUINT32)(mTransformMapping[oImgInfo->mTransform].eImgRot);
    a_rotDma.Flip = (eImgFlip_ON == (mTransformMapping[oImgInfo->mTransform].eImgFlip))?MTRUE:MFALSE;
    //dma port capbility
    a_rotDma.capbility=oImgInfo->mPortID.capbility;
    //Y
    a_rotDma.memBuf.base_pAddr = oImgInfo->mBuffer->getBufPA(0);
    a_rotDma.memBuf.base_vAddr = oImgInfo->mBuffer->getBufVA(0);
    a_rotDma.memBuf.size = oImgInfo->mBuffer->getBufSizeInBytes(0);
    a_rotDma.memBuf.ofst_addr = 0;
    a_rotDma.memBuf.alignment = 0;
    //after ROT
    a_rotDma.size.w = oImgInfo->mBuffer->getImgSize().w;
    a_rotDma.size.h = oImgInfo->mBuffer->getImgSize().h;
    //stride info after ROT
    a_rotDma.size.stride = oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE);
    //
    y_plane_size = oImgInfo->mBuffer->getBufSizeInBytes(ESTRIDE_1ST_PLANE);
    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}
    //
    if ( 2<=plane_num) {
        //U
        a_rotDma.memBuf_c.base_pAddr = oImgInfo->mBuffer->getBufPA(ESTRIDE_2ND_PLANE);
        a_rotDma.memBuf_c.base_vAddr = oImgInfo->mBuffer->getBufVA(ESTRIDE_2ND_PLANE);
        a_rotDma.memBuf_c.ofst_addr = 0;
        a_rotDma.memBuf_c.alignment = 0;
        //after ROT
        a_rotDma.size_c.w =         a_rotDma.size.w/uv_h_ratio;
        a_rotDma.size_c.h =         a_rotDma.size.h/uv_v_ratio;
        //stride info after ROT
        a_rotDma.size_c.stride =    oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE);
        //
        u_plane_size = oImgInfo->mBuffer->getBufSizeInBytes(ESTRIDE_2ND_PLANE);
        //tpipemain lib need image info before ROT. stride info after ROT
        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //    a_rotDma.size_c.w = a_rotDma.size.h/uv_v_ratio;
        //    a_rotDma.size_c.h = a_rotDma.size.w/uv_h_ratio;
        //}
        //
        if ( 3 == plane_num ) {
            //V
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->mBuffer->getBufPA(ESTRIDE_3RD_PLANE);
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->mBuffer->getBufVA(ESTRIDE_3RD_PLANE);
            a_rotDma.memBuf_v.ofst_addr = 0;
            a_rotDma.memBuf_v.alignment = 0;
            //after ROT
            a_rotDma.size_v.w =        a_rotDma.size_c.w;
            a_rotDma.size_v.h =        a_rotDma.size_c.h;
            //stride info after ROT
            a_rotDma.size_v.stride =   oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE);
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

    a_rotDma.secureTag=oImgInfo->mSecureTag;

    PIPE_DBG("[a_rotDma]w(%lu),h(%lu),stride(pxl)(%lu),pa(0x%lx),va(0x%lx),size(%d)",a_rotDma.size.w,a_rotDma.size.h,a_rotDma.size.stride,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf.size);
    PIPE_DBG("[a_rotDma]crop(%d,%d,%lu,%lu)_f(0x%x,0x%x)",a_rotDma.crop.x,a_rotDma.crop.y,a_rotDma.crop.w,a_rotDma.crop.h,a_rotDma.crop.floatX,a_rotDma.crop.floatY);
    PIPE_DBG("[a_rotDma]rot(%d),fmt(%d),plane(%d),pxlByte((0x%x))",a_rotDma.Rotation,a_rotDma.Format,a_rotDma.Plane,a_rotDma.pixel_byte);
    PIPE_DBG("[a_rotDma]c_ofst_addr(0x%x),v_ofst_addr(0x%x),uv_plane_swap(%d)",a_rotDma.memBuf_c.ofst_addr,a_rotDma.memBuf_v.ofst_addr,a_rotDma.uv_plane_swap);
    PIPE_DBG("[a_rotDma]va[0x%lx,0x%lx,0x%lx]--pa[0x%lx,0x%lx,0x%lx]",
        (unsigned long)a_rotDma.memBuf.base_vAddr,(unsigned long)a_rotDma.memBuf_c.base_vAddr,(unsigned long)a_rotDma.memBuf_v.base_vAddr,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf_c.base_pAddr,(unsigned long)a_rotDma.memBuf_v.base_pAddr);


    return MTRUE;
}



MUINT32
PostProcPipe::
queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr)
{
    MUINT32 CQset=0x0;
    MINT32 p2CQ=0;
    MUINT32 dupCQIdx=0;
    MUINT32 testsensorDev = sensorDev;
    pVirIspAddr = 0x0;
    //p2CQ=static_cast<MUINT32>(geDevScenCQMapping[sDrvScen][sensorDev]);

    if (sDrvScen == eDrvScenario_VSS)
    {
        p2CQ=static_cast<MUINT32>(ISP_DRV_DIP_CQ_THRE1);
    }
    else
    {
        p2CQ=static_cast<MUINT32>(ISP_DRV_DIP_CQ_THRE0);
    }
    PIPE_DBG("sDrvScen:%d\n", sDrvScen);

   #if 0
    PIPE_DBG("sDrvScen (%d), sensorDev(0x%x), p2CQ = 0x%x", sDrvScen, sensorDev, p2CQ);
    _debug_info_ debug;
    debug.eDrvScen=static_cast<MUINT32>(sDrvScen);
    debug.senDev=sensorDev;
    dupCQIdx = m_dipCmdQMgr->getDupCQIdx(p2CQ, p2burstQIdx, frameNum, debug);
   #endif
    //
   while(!m_pDipRingBuf->checkDipWorkingBufferAvaiable((E_ISP_DIP_CQ)p2CQ))
   {
       PIPE_INF("Block Enqueue Here, P2CQ (%d), eDrvScen/SenDev(0x%x/0x%x)", p2CQ, sDrvScen);
   }

   //Acquire the buffer from ring buffer
   m_pDipWorkingBuf = m_pDipRingBuf->AcquireDipWorkingBuffer((E_ISP_DIP_CQ)p2CQ);
   if(m_pDipWorkingBuf == NULL)
   {
       PIPE_ERR("AcquireDipWorkingBuffer error!!\n");
       return MFALSE; //error command  enque
   }
   pVirIspAddr = (MUINTPTR)m_pDipWorkingBuf->m_pIspVirRegAddr_va;
   RingBufIndex = m_pDipWorkingBuf->m_BufIdx;

   //dupCQIdx = m_pDipWorkingBuf->m_BufIdx;
   dupCQIdx = mp2CQDupIdx;

   //reset the list if all frames in package are enqued done
   if(p2burstQIdx == (frameNum-1))
   {
       mp2CQDupIdx = (mp2CQDupIdx+1)  % (MAX_DUP_CQ_NUM); //update at last frame in the same package
   }

   PIPE_DBG("sDrvScen:%d, dupCQIdx:%d, RingBufIndex:%d\n", sDrvScen, dupCQIdx, RingBufIndex);

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

MBOOL
PostProcPipe::
doUVSwap(MUINTPTR *uv1, MUINTPTR *uv2)
{
    MUINTPTR tmpUV;
    tmpUV = *uv1;
    *uv1 = *uv2;
    *uv2 = tmpUV;
    return MTRUE;
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

