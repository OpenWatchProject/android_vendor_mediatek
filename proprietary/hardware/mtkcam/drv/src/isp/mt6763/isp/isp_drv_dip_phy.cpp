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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IspDrvDipPhy"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>

#include "camera_isp.h"
#include "isp_drv_dip_phy.h"


#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv_dip_phy);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_dip_phy_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_dip_phy_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


IspDrvDipPhy::IspDrvDipPhy()
{
    DBG_LOG_CONFIG(drv, isp_drv_dip_phy);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_UserCnt = 0;

    m_pIspDrvImp = NULL;
    m_hwModule = DIP_A;
    mpTempIspDipHWRegValues = NULL;

}


static IspDrvDipPhy    gDipDrvObj[ISP_DIP_MODULE_IDX_MAX];

static volatile MBOOL gIsEverDumpBuffer = MTRUE;

IspDrv* IspDrvDipPhy::createInstance(ISP_HW_MODULE hwModule)
{
    MUINT32 moduleIdx;

    if(hwModule<DIP_MAX && hwModule>=DIP_A){
        moduleIdx = hwModule - DIP_START;
        LOG_DBG("moduleIdx(%d),hwModule(%d)",moduleIdx,hwModule);
    } else {
        LOG_ERR("[Error]hwModule(%d) out of the range",hwModule);
        return NULL;
    }

    gDipDrvObj[moduleIdx].m_pIspDrvImp = (IspDrvImp*)IspDrvImp::createInstance(hwModule);
    gDipDrvObj[moduleIdx].m_hwModule = hwModule;
    //
    return (IspDrv*)&gDipDrvObj[moduleIdx];
}



MBOOL IspDrvDipPhy::init(const char* userName)
{
    MBOOL Result = MTRUE;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_INF("+,m_UserCnt(%d),curUser(%s).", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt > 0)
    {
        android_atomic_inc(&this->m_UserCnt);
        LOG_INF("-,m_UserCnt(%d)", this->m_UserCnt);
        return Result;
    }

    if(this->m_pIspDrvImp->init("isp_drv_dip_phy") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }
    // load dip hw init setting
    if(this->loadInitSetting() == MFALSE){
        Result = MFALSE;
        LOG_ERR("load dip hw init setting fail\n");
        goto EXIT;
    }

    //check wheather kernel is existed or not
    if (this->m_Fd < 0) {    // 1st time open failed.
        LOG_ERR("ISP kernel is not existed\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    android_atomic_inc(&this->m_UserCnt);

    mpTempIspDipHWRegValues = (dip_x_reg_t*)malloc(sizeof(dip_x_reg_t));    //always allocate this(be used to get register offset in macro)

EXIT:

    LOG_DBG("-,ret(%d),mInitCount(%d)", Result, this->m_UserCnt);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvDipPhy::uninit(const char* userName)
{
    MBOOL Result = MTRUE;

    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_INF("+,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in isp_drv_dip_phy , curUser(%s)",userName);
        goto EXIT;
    }
    // More than one user
    android_atomic_dec(&this->m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;
    //
    if(this->m_pIspDrvImp->uninit("isp_drv_dip_phy") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }

    if(mpTempIspDipHWRegValues != NULL)
    {
        free((MUINT32*)mpTempIspDipHWRegValues);
        mpTempIspDipHWRegValues = NULL;
    }
EXIT:

    LOG_DBG("-,ret(%d),m_UserCnt(%d)", Result, this->m_UserCnt);
    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no isp device");\
        Ret = -1;\
    }\
    Ret;\
})

void IspDrvDipPhy::destroyInstance(void)
{
    this->m_pIspDrvImp = NULL;
}

MBOOL IspDrvDipPhy::waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    return this->m_pIspDrvImp->waitIrq(pWaitIrq);
}

MBOOL IspDrvDipPhy::clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq)
{
    return this->m_pIspDrvImp->clearIrq(pClearIrq);
}

MBOOL IspDrvDipPhy::registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    return this->m_pIspDrvImp->registerIrq(pRegIrq);
}

MBOOL IspDrvDipPhy::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    return this->m_pIspDrvImp->readRegs(pRegIo, Count, caller);
}

MUINT32 IspDrvDipPhy::readReg(MUINT32 Addr,MINT32 caller)
{
    return this->m_pIspDrvImp->readReg(Addr, caller);
}

MBOOL IspDrvDipPhy::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    return this->m_pIspDrvImp->writeRegs(pRegIo, Count, caller);
}

MBOOL IspDrvDipPhy::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    return this->m_pIspDrvImp->writeReg(Addr, Data, caller);
}

MBOOL IspDrvDipPhy::signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    LOG_INF("IRQ SIGNAL:hwModule:0x%x, userKey:0x%x, status:0x%x",this->m_hwModule,pWaitIrq->UserKey,pWaitIrq->Status);
    ISP_WAIT_IRQ_STRUCT wait;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(ISP_WAIT_IRQ_ST));
    switch(this->m_hwModule){
        case DIP_A:
            wait.Type = ISP_IRQ_TYPE_INT_DIP_A_ST;
            break;
        default:
            LOG_ERR("unsupported hw hwModule:0x%x\n",this->m_hwModule);
            return MFALSE;
            break;
    }

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_FLUSH_IRQ_REQUEST,&wait) < 0){
        LOG_ERR("signal IRQ fail(irq:0x%x,status:0x%x)",wait.Type,wait.EventInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}


MBOOL IspDrvDipPhy::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;

    switch(eCmd){
        case _GET_DIP_DBG_INFO:
            //dumpDbgLog((IspDumpDbgLogDipPackage*)pData);
            break;
        case _GET_SOF_CNT:
        case _GET_DMA_ERR:
        case _GET_INT_ERR:
        case _GET_DROP_FRAME_STATUS:
        default:
            LOG_ERR("unsupported cmd:0x%x, pData Address: 0x%lx",eCmd, (unsigned long)pData);
            return MFALSE;
        break;
    }
    return rst;
}

MBOOL IspDrvDipPhy::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    ISP_P2_BUFQUE_STRUCT p2bufQ;

    switch(eCmd){
        case _SET_DBG_INT:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DEBUG_FLAG,(unsigned char*)pData) < 0){
                LOG_ERR("kernel log enable error\n");
                return MFALSE;
            }
            break;
        case _SET_DIP_BUF_INFO:
            p2bufQ.ctrl = static_cast<ISP_P2_BUFQUE_CTRL_ENUM>(pData[0]);
            p2bufQ.property = static_cast<ISP_P2_BUFQUE_PROPERTY>(pData[1]);
            p2bufQ.processID = 0;//static_cast<MUINT32>(pData[2]);
            p2bufQ.callerID = static_cast<MUINT32>(pData[3]) | (static_cast<MUINT32>(pData[4])<<8) | (static_cast<MUINT32>(pData[5])<<16) | (static_cast<MUINT32>(pData[6])<<24);
            p2bufQ.frameNum = static_cast<MINT32>(pData[7]);
            p2bufQ.cQIdx = static_cast<MINT32>(pData[8]);
            p2bufQ.dupCQIdx = static_cast<MINT32>(pData[9]);
            p2bufQ.burstQIdx = static_cast<MINT32>(pData[10]);
            p2bufQ.timeoutIns = static_cast<MUINT32>(pData[11]);
            LOG_DBG("p2bufQ(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",p2bufQ.ctrl,p2bufQ.property,p2bufQ.processID,p2bufQ.callerID,\
                p2bufQ.frameNum,p2bufQ.cQIdx ,p2bufQ.dupCQIdx,p2bufQ.burstQIdx,p2bufQ.timeoutIns);
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_P2_BUFQUE_CTRL,&p2bufQ) < 0){
                LOG_ERR("ISP_P2_BUFQUE_CTRL(%d) error\n",p2bufQ.ctrl);
                return MFALSE;
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return MTRUE;
}


MBOOL IspDrvDipPhy::loadInitSetting(void)
{
    LOG_INF("loadInitSetting size(%d)",ISP_DIP_INIT_SETTING_COUNT);


    this->m_pIspDrvImp->writeRegs(mIspDipInitReg, ISP_DIP_INIT_SETTING_COUNT, DIP_A);

#if 0
    //debug check
    LOG_INF("DIP_X_IMGI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x41C,DIP_A));
    LOG_INF("DIP_X_IMGI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x420,DIP_A));
    LOG_INF("DIP_X_IMGI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x424,DIP_A));

    LOG_INF("DIP_X_IMGBI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x44C,DIP_A));
    LOG_INF("DIP_X_IMGBI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x450,DIP_A));
    LOG_INF("DIP_X_IMGBI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x454,DIP_A));

    LOG_INF("DIP_X_IMGCI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x47C,DIP_A));
    LOG_INF("DIP_X_IMGCI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x480,DIP_A));
    LOG_INF("DIP_X_IMGCI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x484,DIP_A));

    LOG_INF("DIP_X_UFDI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x5CC,DIP_A));
    LOG_INF("DIP_X_UFDI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x5D0,DIP_A));
    LOG_INF("DIP_X_UFDI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x5D4,DIP_A));

    LOG_INF("DIP_X_LCEI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x59C,DIP_A));
    LOG_INF("DIP_X_LCEI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x5A0,DIP_A));
    LOG_INF("DIP_X_LCEI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x5A4,DIP_A));

    LOG_INF("DIP_X_VIPI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x4AC,DIP_A));
    LOG_INF("DIP_X_VIPI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x4B0,DIP_A));
    LOG_INF("DIP_X_VIPI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x4B4,DIP_A));

    LOG_INF("DIP_X_VIP2I_CON(0x%x)",this->m_pIspDrvImp->readReg(0x4DC,DIP_A));
    LOG_INF("DIP_X_VIP2I_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x4E0,DIP_A));
    LOG_INF("DIP_X_VIP2I_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x4E4,DIP_A));

    LOG_INF("DIP_X_VIP3I_CON(0x%x)",this->m_pIspDrvImp->readReg(0x50C,DIP_A));
    LOG_INF("DIP_X_VIP3I_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x510,DIP_A));
    LOG_INF("DIP_X_VIP3I_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x514,DIP_A));

    LOG_INF("DIP_X_DMGI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x53C,DIP_A));
    LOG_INF("DIP_X_DMGI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x540,DIP_A));
    LOG_INF("DIP_X_DMGI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x544,DIP_A));

    LOG_INF("DIP_X_DEPI_CON(0x%x)",this->m_pIspDrvImp->readReg(0x56C,DIP_A));
    LOG_INF("DIP_X_DEPI_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x570,DIP_A));
    LOG_INF("DIP_X_DEPI_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x574,DIP_A));

    LOG_INF("DIP_X_MFBO_CON(0x%x)",this->m_pIspDrvImp->readReg(0x36C,DIP_A));
    LOG_INF("DIP_X_MFBO_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x370,DIP_A));
    LOG_INF("DIP_X_MFBO_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x374,DIP_A));

    LOG_INF("DIP_X_IMG3O_CON(0x%x)",this->m_pIspDrvImp->readReg(0x2AC,DIP_A));
    LOG_INF("DIP_X_IMG3O_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x2B0,DIP_A));
    LOG_INF("DIP_X_IMG3O_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x2B4,DIP_A));

    LOG_INF("DIP_X_IMG3BO_CON(0x%x)",this->m_pIspDrvImp->readReg(0x2DC,DIP_A));
    LOG_INF("DIP_X_IMG3BO_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x2E0,DIP_A));
    LOG_INF("DIP_X_IMG3BO_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x2E4,DIP_A));

    LOG_INF("DIP_X_IMG3CO_CON(0x%x)",this->m_pIspDrvImp->readReg(0x30C,DIP_A));
    LOG_INF("DIP_X_IMG3CO_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x310,DIP_A));
    LOG_INF("DIP_X_IMG3CO_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x314,DIP_A));

    LOG_INF("DIP_X_FEO_CON(0x%x)",this->m_pIspDrvImp->readReg(0x33C,DIP_A));
    LOG_INF("DIP_X_FEO_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x340,DIP_A));
    LOG_INF("DIP_X_FEO_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x344,DIP_A));

    LOG_INF("DIP_X_IMG2O_CON(0x%x)",this->m_pIspDrvImp->readReg(0x24C,DIP_A));
    LOG_INF("DIP_X_IMG2O_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x250,DIP_A));
    LOG_INF("DIP_X_IMG2O_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x254,DIP_A));

    LOG_INF("DIP_X_IMG2BO_CON(0x%x)",this->m_pIspDrvImp->readReg(0x27C,DIP_A));
    LOG_INF("DIP_X_IMG2BO_CON2(0x%x)",this->m_pIspDrvImp->readReg(0x280,DIP_A));
    LOG_INF("DIP_X_IMG2BO_CON3(0x%x)",this->m_pIspDrvImp->readReg(0x284,DIP_A));

    LOG_INF("loadInitSetting size -");
#endif    
    return MTRUE;
}

dip_x_reg_t* IspDrvDipPhy::getCurHWRegValues()
{
    MUINT32 size=sizeof(dip_x_reg_t);
    MUINT32* startAddr=getMMapRegAddr();
    //startAddr+=0x4000;
    //LOG_INF("isp_reg_t size(0x%x),starting Addr(0x%x),size/sizeof(MUINT32) (0x%x),0x4000Addr(0x%x)\n",size,getMMapRegAddr(),size/sizeof(MUINT32),startAddr);

    for(MUINT32 i=0;i<size/sizeof(MUINT32);i++)
    {
         //LOG_INF("addr(0x%x) value (0x%x)\n",((MUINT32*)startAddr+i),*((MUINT32*)startAddr+i));
         *((MUINT32*)mpTempIspDipHWRegValues+i)=*((MUINT32*)startAddr+i);
    }
    return (mpTempIspDipHWRegValues);
}

MBOOL IspDrvDipPhy::dumpDipHwReg(IspDumpDbgLogDipPackage* pP2Package){
    dip_x_reg_t dipReg;
    MUINT32 i;
    MUINT32 dumpTpipeLine;
    MUINT32 regTpipePA;
    ISP_DUMP_BUFFER_STRUCT dumpBufStruct;
    MUINT32 size=sizeof(dip_x_reg_t);
    ISP_GET_DUMP_INFO_STRUCT get_dumpinfo;

    //dump top regs
    LOG_INF("DIP_X_CTL_START=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_START-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_YUV_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_YUV_EN-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_YUV2_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_YUV2_EN-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_RGB_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_RGB_EN-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_DMA_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_DMA_EN-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_FMT_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_FMT_SEL-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_PATH_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_PATH_SEL-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_MISC_SEL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_MISC_SEL-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_TDR_CTL=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_CTL-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_TDR_TILE=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TILE-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_TDR_TCM_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TCM_EN-(MUINT8*)&dipReg)), DIP_A));
	LOG_INF("DIP_X_CTL_TDR_TCM2_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&dipReg.DIP_X_CTL_TDR_TCM2_EN-(MUINT8*)&dipReg)), DIP_A));

	//dump tpipe
    regTpipePA = readReg((MUINT32)((MUINT8*)&dipReg.DIP_X_TDRI_BASE_ADDR-(MUINT8*)&dipReg));
    LOG_INF("[Tpipe]va(0x%lx),pa(0x%08x),regPa(0x%08x)",(unsigned long)pP2Package->tpipeTableVa,pP2Package->tpipeTablePa,regTpipePA);
    dumpTpipeLine = DUMP_TPIPE_SIZE / DUMP_TPIPE_NUM_PER_LINE;
    for(i=0;i<dumpTpipeLine;i++){
        LOG_INF("[Tpipe](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",i,
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+1],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+2],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+3],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+4],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+5],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+6],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+7],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+8],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+9]);
    }

    //dump all dip registers
    for(i=0x0;i<=0x3000;i+=20){
        LOG_INF("(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)",
            0x15022000+i,readReg(i, DIP_A),0x15022000+i+4,readReg(i+4, DIP_A),0x15022000+i+8,readReg(i+8, DIP_A),0x15022000+i+12,readReg(i+12, DIP_A),0x15022000+i+16,readReg(i+16, DIP_A));
    }

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_DUMP_INFO,&get_dumpinfo) < 0){
        LOG_ERR("get dump info fail \n");
    }
    //0xffff0000 chip dependent, sizeof = 256x256 = 0x10000
    if ((get_dumpinfo.tdri_baseaddr & 0xffff0000) == (pP2Package->tpipeTablePa & 0xffff0000))
    {
        if ((get_dumpinfo.imgi_baseaddr != 0xffffffff) && (get_dumpinfo.imgi_baseaddr == pP2Package->IspVirRegAddrVa[256]))
            {
                gIsEverDumpBuffer = MFALSE;
            LOG_INF("imgi baseaddr is the same as kernel imgi base address, get_dumpinfo.imgi_baseaddr:0x%x, pP2Package->IspVirRegAddrVa[256]:0x%x",get_dumpinfo.imgi_baseaddr ,pP2Package->IspVirRegAddrVa[256]);
            }
        else
        {
            LOG_INF("imgi baseaddr is not the same as kernel imgi base address, get_dumpinfo.imgi_baseaddr:0x%x, pP2Package->IspVirRegAddrVa[256]:0x%x",get_dumpinfo.imgi_baseaddr ,pP2Package->IspVirRegAddrVa[256]);
        }
        gIsEverDumpBuffer = MFALSE;
    }

    LOG_INF("QQ Tunning buffer address : 0x%lx, tpipeTablePa: 0x%x, tpipeTableVa: 0x%lx, gIsEverDumpBuffer:%d, tdri(0x%x), imgi(0x%x), dmgi(0x%x)",(unsigned long)pP2Package->pTuningQue, pP2Package->tpipeTablePa, (unsigned long)pP2Package->tpipeTableVa, gIsEverDumpBuffer, get_dumpinfo.tdri_baseaddr, get_dumpinfo.imgi_baseaddr, get_dumpinfo.dmgi_baseaddr);

    if (MFALSE == gIsEverDumpBuffer)
    {
        dumpBufStruct.DumpCmd = ISP_DUMP_TPIPEBUF_CMD;
        dumpBufStruct.BytesofBufferSize = MAX_ISP_TILE_TDR_HEX_NO;
        dumpBufStruct.pBuffer = pP2Package->tpipeTableVa;
        if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump tpipe buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

        dumpBufStruct.DumpCmd = ISP_DUMP_ISPVIRBUF_CMD;
        dumpBufStruct.BytesofBufferSize = size;
        dumpBufStruct.pBuffer = pP2Package->IspVirRegAddrVa;
        if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump vir isp buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

        dumpBufStruct.DumpCmd = ISP_DUMP_CMDQVIRBUF_CMD;
        dumpBufStruct.BytesofBufferSize = (GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;
        dumpBufStruct.pBuffer = pP2Package->IspDescriptVa;
        if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DUMP_BUFFER,&dumpBufStruct) < 0){
            LOG_ERR("dump cmdq buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
        }

    }

	if (pP2Package->pTuningQue != NULL)
	{
		LOG_INF("QQ count : %u", (MUINT32)(size/sizeof(MUINT32)));
        unsigned int *pTuningBuf = (unsigned int *)pP2Package->pTuningQue;
        if (MFALSE == gIsEverDumpBuffer)
        {
            dumpBufStruct.DumpCmd = ISP_DUMP_TUNINGBUF_CMD;
            dumpBufStruct.BytesofBufferSize = size;
            dumpBufStruct.pBuffer = (unsigned int *)pP2Package->pTuningQue;
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DUMP_BUFFER,&dumpBufStruct) < 0){
                LOG_ERR("dump tuning buffer fail, size:0x%x\n", dumpBufStruct.BytesofBufferSize);
            }
        }
        MUINT32 dip_size=size;
        if ((dip_size & 0x7) > 0)
        {
            size = ((size>>3)<<3);
        }

        for(i=0x0;i<size;i+=32){
            LOG_INF("QQ (0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)(0x%08x,0x%08x)",
                0x15022000+i,pTuningBuf[(i>>2)],0x15022000+i+4,pTuningBuf[(i>>2)+1],0x15022000+i+8,pTuningBuf[(i>>2)+2],0x15022000+i+12,pTuningBuf[(i>>2)+3],0x15022000+i+16,pTuningBuf[(i>>2)+4],0x15022000+i+20,pTuningBuf[(i>>2)+5],0x15022000+i+24,pTuningBuf[(i>>2)+6],0x15022000+i+28,pTuningBuf[(i>>2)+7]);
        }
        if ((dip_size-size)>0)
        {
            for(i=size;i<dip_size;i=i+4){
                LOG_INF("res QQ (0x%08x,0x%08x)",
                    0x15022000+i,pTuningBuf[(i>>2)]);
            }
        }


		for(i=0;i<size/sizeof(MUINT32);i++)
			pP2Package->pTuningQue[i] = readReg(i*4,DIP_A);

	}
    gIsEverDumpBuffer = MTRUE;

    return MTRUE;
}

MBOOL IspDrvDipPhy::setMemInfo(unsigned long tpipeTablePa, unsigned int *tpipeTableVa, unsigned int MemSizeDiff)
{
    ISP_MEM_INFO_STRUCT TpipeMemInfoStruct;

    TpipeMemInfoStruct.MemInfoCmd = ISP_MEMORY_INFO_TPIPE_CMD;
    TpipeMemInfoStruct.MemPa = tpipeTablePa;
    TpipeMemInfoStruct.MemVa = tpipeTableVa;
    TpipeMemInfoStruct.MemSizeDiff = MemSizeDiff;

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_SET_MEM_INFO,&TpipeMemInfoStruct) < 0){
        LOG_ERR("set tpipe mem info fail, cmd:0x%x, memPa:0x%lx, memVa:0x%lx, memSizeDiff:0x%x\n",
            TpipeMemInfoStruct.MemInfoCmd, (unsigned long)TpipeMemInfoStruct.MemPa, (unsigned long)TpipeMemInfoStruct.MemVa, TpipeMemInfoStruct.MemSizeDiff);
    }

    return MTRUE;
}


