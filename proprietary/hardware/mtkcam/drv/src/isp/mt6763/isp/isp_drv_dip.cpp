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
#define LOG_TAG "ispDrvDip"

#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>
#include <isp_drv_stddef.h>
#include "isp_drv_dip.h"
#include "isp_drv_dip_phy.h"


#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv_dip);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_dip_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_dip_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_dip_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_dip_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_dip_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_dip_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


typedef enum{
    IspDrv_UNKNONW      = 0,
    IspDrv_Create       = 1,
    IspDrv_Init         = 2,
    IspDrv_Start        = 3,
    IspDrv_Stop         = 4,
    IspDrv_Uninit       = 5,
}E_IspDrvStatus;

typedef enum{
    OP_IspDrv_Destroy,
    OP_IspDrv_sig,         //wait/clr/reg signal
    OP_IspDrv_sig_sig,     //signal signal
    OP_IspDrv_CQ,          //CQ operation
    OP_IspDrv_Device,      //device driver ctrl
    OP_IspDrv_init,
    OP_IspDrv_start,
    OP_IspDrv_stop,
    OP_IspDrv_Uninit,
}E_IspDrvOP;



MUINT32         IspDrvDip::m_TotalUserCnt = 0;
//MBOOL           IspDrvDip::m_bInit[ISP_DIP_MODULE_IDX_MAX] = {MFALSE};
IspDrvDip*****  IspDrvDip::m_PageTbl = NULL;
IspDrv*         IspDrvDip::m_pIspDrvImp = NULL;
IMemDrv*        IspDrvDip::m_pMemDrv = NULL;

#define ISP_DRV_OPTIMIZE	// If you don't want to optimize, just remove this line.
IMEM_BUF_INFO  gAllCqBufInfo;

//------------------------------------------------------------------------------------------------------

IspDrvDip::IspDrvDip(ISP_HW_MODULE hwModule, E_ISP_DIP_CQ dipCq, MUINT32 burstQueIdx, MUINT32 dupCqIdx)
{
    DBG_LOG_CONFIG(drv, isp_drv_dip);
    //
    LOG_DBG("hwModule(%d),dipCq(%d),burstQueIdx(%d),dupCqIdx(%d)",hwModule,dipCq,burstQueIdx,dupCqIdx);
    //
    m_UserCnt = 0;
    //m_FSM = IspDrv_UNKNONW;
    //imem
    //m_pMemDrv = NULL;
    //
    m_pIspDescript_vir = NULL;
    m_pIspDescript_phy = NULL;
    //
    m_pIspVirRegAddr_va = NULL;
    m_pIspVirRegAddr_pa = NULL;

    m_hwModule = hwModule;
    m_dipModuleIdx = (ISP_DIP_MODULE_IDX)(hwModule - DIP_START);
    m_dipCq = dipCq;
    m_burstQueIdx = burstQueIdx;
    m_dupCqIdx = dupCqIdx;
    //m_pageIdx = pageIdx;
    //
    m_TotalUserCnt++;


    LOG_DBG("getpid[0x%08x],gettid[0x%08x],m_TotalUserCnt(%d)\n", getpid() ,gettid(), m_TotalUserCnt);
}


IspDrvDip* IspDrvDip::createInstance(ISP_HW_MODULE hwModule, E_ISP_DIP_CQ dipCq, MUINT32 burstQueIdx, MUINT32 dupCqIdx, const char* userName)
{
    MUINT32 moduleIdx;

    LOG_DBG("hwModule(%d),dipCq(%d),burstQueIdx(%d),dupCqIdx(%d),userName:%s",hwModule,dipCq,burstQueIdx,dupCqIdx,userName);
    if(userName == NULL){
        LOG_ERR("userName can't be NULL (hwModule:0x%x)",hwModule);
        return NULL;
    }

    if(hwModule<DIP_MAX && hwModule>=DIP_A){
        moduleIdx = hwModule - DIP_START;
        LOG_DBG("moduleIdx(%d)",moduleIdx);
    } else {
        LOG_ERR("[Error]hwModule(%d) out of the range",hwModule);
        return NULL;
    }

    if(m_TotalUserCnt == 0){
        m_PageTbl = (IspDrvDip*****)malloc(sizeof(IspDrvDip****)*(ISP_DIP_MODULE_IDX_MAX));
        for(int i=0;i<ISP_DIP_MODULE_IDX_MAX;i++){
            m_PageTbl[i] = (IspDrvDip****)malloc(sizeof(IspDrvDip***)*ISP_DRV_DIP_CQ_NUM);
            for(int j=0;j<ISP_DRV_DIP_CQ_NUM;j++) {
                m_PageTbl[i][j] = (IspDrvDip***)malloc(sizeof(IspDrvDip**)*MAX_BURST_QUE_NUM);
                for(int k=0;k<MAX_BURST_QUE_NUM;k++) {
                    m_PageTbl[i][j][k] = (IspDrvDip**)malloc(sizeof(IspDrvDip*)*MAX_DUP_CQ_NUM);
                    for(int l=0;l<MAX_DUP_CQ_NUM;l++) {
                        m_PageTbl[i][j][k][l] = NULL;
                    }
                }
            }
        }
    }


    if(m_PageTbl[moduleIdx][dipCq][burstQueIdx][dupCqIdx] == NULL){
        m_PageTbl[moduleIdx][dipCq][burstQueIdx][dupCqIdx] = new IspDrvDip(hwModule,dipCq,burstQueIdx,dupCqIdx);

        if(sizeof(userName) < MAX_USER_NAME_SIZE) {
            strncpy(m_PageTbl[moduleIdx][dipCq][burstQueIdx][dupCqIdx]->m_useName,userName,strlen(userName));
        } else {
            LOG_ERR("[Error]the size of user name(%lx) exceed defaul value(%d)",(unsigned long)sizeof(userName),MAX_USER_NAME_SIZE);
            return NULL;
        }
        m_PageTbl[moduleIdx][dipCq][burstQueIdx][dupCqIdx]->m_pIspDrvImp = IspDrvImp::createInstance(hwModule);
    }
    else{
        LOG_INF("this obj already created ,dipCq(%d),burstQueIdx(%d),dupCqIdx(%d),userNmae(%s)",dipCq,burstQueIdx,dupCqIdx,userName);
    }

    return (IspDrvDip*)m_PageTbl[moduleIdx][dipCq][burstQueIdx][dupCqIdx];

}


void IspDrvDip::destroyInstance(void)
{
    //if(this->m_UserCnt <=0)
    //    delete this;

    if(m_PageTbl[this->m_dipModuleIdx][this->m_dipCq][this->m_burstQueIdx][this->m_dupCqIdx]) {
        this->m_TotalUserCnt--;
        m_PageTbl[this->m_dipModuleIdx][this->m_dipCq][this->m_burstQueIdx][this->m_dupCqIdx] = NULL;
        //delete this;
    } else {
        /* do nothing */
    }

    LOG_DBG("+ hwModule(%d),m_dipModuleIdx(%d),cq(%d),userName:%s,m_TotalUserCnt(%d),m_UserCnt(%d)\n",
        this->m_hwModule,this->m_dipModuleIdx,this->m_dipCq,this->m_useName,this->m_TotalUserCnt,this->m_UserCnt);


    if(this->m_TotalUserCnt == 0){
        for(int i=0;i<ISP_DIP_MODULE_IDX_MAX;i++){
            for(int j=0;j<ISP_DRV_DIP_CQ_NUM;j++) {
                for(int k=0;k<MAX_BURST_QUE_NUM;k++) {
                    //for(int l=0;l<MAX_DUP_CQ_NUM;l++) {
                    //    m_PageTbl[i][j][k][l] = NULL;
                    //}
                    free(m_PageTbl[i][j][k]);
                }
                free(m_PageTbl[i][j]);
            }
            free(m_PageTbl[i]);
        }
        free(m_PageTbl);
        //
        m_PageTbl = NULL;
    }
    delete this;

}

/*
IspDrvVir* IspDrvDip::getCQInstance(ISP_HW_MODULE hwModule, MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx)
{
    MUINT32 moduleIdx;

    if(hwModule<DIP_MAX && hwModule>=DIP_A){
        moduleIdx = DIP_MAX - hwModule;
        LOG_DBG("hwModule(%d),cq(%d),burstQueIdx(%d),dupCqIdx(%d)",hwModule,cq,burstQueIdx,dupCqIdx);
    } else {
        LOG_ERR("[Error]hwModule(%d) out of the range",hwModule);
        return NULL;
    }

    return (IspDrvVir*)m_PageTbl[moduleIdx][cq][burstQueIdx][dupCqIdx];
}
*/



MBOOL IspDrvDip::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;
    MUINT32 cqDesAllocSize;
    MUINT32 bufIndex;
    MUINT32 bufOffset;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_DBG("+,m_TotalUserCnt(%d),m_UserCnt(%d),curUser(%s),hwModule(%d),m_dipModuleIdx(%d),m_dipCq(%d),m_burstQueIdx(%d),m_dupCqIdx(%d)",
            this->m_TotalUserCnt,this->m_UserCnt,userName,this->m_hwModule,this->m_dipModuleIdx,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
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

    // init ispdrvimp
    //to avoid iIspDrv userCnt over upper bound
    if(this->m_TotalUserCnt==1 && this->m_UserCnt==0) {
        if(this->m_pIspDrvImp->init(userName) == MFALSE){
            Result = MFALSE;
            LOG_ERR("isp drv init fail\n");
            goto EXIT;
        }
        // create & init imem
        this->m_pMemDrv = IMemDrv::createInstance();
        if(this->m_pMemDrv->init() == MFALSE){
            Result = MFALSE;
            LOG_ERR("imem fail\n");
            goto EXIT;
        }
        //
        //this->m_bInit[this->m_dipModuleIdx] = MTRUE;
    }

    #if 1
    //cq ptr chk,if not NULL, means imem was not freed also
    if(this->m_pIspDescript_vir || this->m_pIspDescript_phy || this->m_pIspVirRegAddr_va || this->m_pIspVirRegAddr_pa){
        LOG_ERR("[Error]cq is not null (%lx)-(%lx)-(%lx)-(%lx)",(unsigned long)this->m_pIspDescript_vir,
            (unsigned long)this->m_pIspDescript_phy,(unsigned long)this->m_pIspVirRegAddr_va,(unsigned long)this->m_pIspVirRegAddr_pa);
        Result = MFALSE;
        goto EXIT;
    }

#ifdef ISP_DRV_OPTIMIZE
    //memory allocation for all of cq, including cq descriptor and cq virtual
    cqDesAllocSize = (GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;
    if ((this->m_dipCq == 0) && (this->m_burstQueIdx == 0) && (this->m_dupCqIdx == 0))
    {
        ISP_MEM_INFO_STRUCT CmdqMemInfoStruct;
        gAllCqBufInfo.size = (((cqDesAllocSize+DIP_BASE_RANGE_SPECIAL+0x3) & (~0x3)))*(ISP_DRV_DIP_CQ_NUM*MAX_BURST_QUE_NUM*MAX_DUP_CQ_NUM); //4byte alignment,round up
        gAllCqBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
        if ( this->m_pMemDrv->allocVirtBuf(&gAllCqBufInfo) ){
            LOG_ERR("[Error]m_pMemDrv->allocVirtBuf fail");
            Result = MFALSE;
            goto EXIT;
        }
        else{
            if ( this->m_pMemDrv->mapPhyAddr(&gAllCqBufInfo) ) {
                LOG_ERR("[ERROR]m_pIMemDrv->mapPhyAddr");
                Result = MFALSE;
                goto EXIT;
            }
        }
        //va/pa 0 init
        memset((MUINT8*)gAllCqBufInfo.virtAddr,0x0,gAllCqBufInfo.size);
        CmdqMemInfoStruct.MemInfoCmd = ISP_MEMORY_INFO_CMDQ_CMD;
        CmdqMemInfoStruct.MemPa = (unsigned int)gAllCqBufInfo.phyAddr;
        CmdqMemInfoStruct.MemVa = (unsigned int *)gAllCqBufInfo.virtAddr;
        CmdqMemInfoStruct.MemSizeDiff = cqDesAllocSize;

        if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_SET_MEM_INFO,&CmdqMemInfoStruct) < 0){
            LOG_ERR("set cmdq mem info fail, cmd:0x%x, memPa:0x%x, memVa:0x%lx, memSizeDiff:0x%x\n",
                CmdqMemInfoStruct.MemInfoCmd, CmdqMemInfoStruct.MemPa, (unsigned long)CmdqMemInfoStruct.MemVa, CmdqMemInfoStruct.MemSizeDiff);
        }
    }
    bufIndex = (this->m_dipCq * MAX_BURST_QUE_NUM * MAX_DUP_CQ_NUM) + (this->m_burstQueIdx * MAX_DUP_CQ_NUM) + this->m_dupCqIdx;
    bufOffset = ((cqDesAllocSize+DIP_BASE_RANGE_SPECIAL+0x3) & (~0x3))*bufIndex;
    //
    //alignment chk
    if(( (_tmpVirAddr=gAllCqBufInfo.virtAddr+bufOffset) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem virRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(gAllCqBufInfo.virtAddr+bufOffset));
        _tmpVirAddr = (gAllCqBufInfo.virtAddr+bufOffset + 0x3) & (~0x3);
    }
    if(( (_tmpPhyAddr=gAllCqBufInfo.phyAddr+bufOffset) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem phyRegAddr not 4 bytes alignment(0x%lx), round up",(unsigned long)(gAllCqBufInfo.phyAddr+bufOffset));
        _tmpPhyAddr = (gAllCqBufInfo.phyAddr+bufOffset + 0x3) & (~0x3);
    }
#else
    //memory allocation for all of cq, including cq descriptor and cq virtual
    cqDesAllocSize = (GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;
    this->m_ispAllCqBufInfo.size = ((cqDesAllocSize+DIP_BASE_RANGE_SPECIAL+0x3) & (~0x3)); //4byte alignment,round up
    this->m_ispAllCqBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
    if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispAllCqBufInfo) ){
        LOG_ERR("[Error]m_pMemDrv->allocVirtBuf fail");
        Result = MFALSE;
        goto EXIT;
    }
    else{
        if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispAllCqBufInfo) ) {
            LOG_ERR("[ERROR]m_pIMemDrv->mapPhyAddr");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    //alignment chk
    if(( (_tmpVirAddr=this->m_ispAllCqBufInfo.virtAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem virRegAddr not 4 bytes alignment(0x%x), round up",this->m_ispAllCqBufInfo.virtAddr);
        _tmpVirAddr = (this->m_ispAllCqBufInfo.virtAddr + 0x3) & (~0x3);
    }
    if(( (_tmpPhyAddr=this->m_ispAllCqBufInfo.phyAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem phyRegAddr not 4 bytes alignment(0x%x), round up",this->m_ispAllCqBufInfo.phyAddr);
        _tmpPhyAddr = (this->m_ispAllCqBufInfo.phyAddr + 0x3) & (~0x3);
    }

    //va/pa 0 init
    memset((MUINT8*)_tmpVirAddr,0x0,this->m_ispAllCqBufInfo.size);
#endif    
    this->m_pIspDescript_vir = (ISP_DRV_CQ_CMD_DESC_STRUCT*)_tmpVirAddr;
    this->m_pIspDescript_phy = (MUINT32*)_tmpPhyAddr;

    #if 0
    this->m_pIspVirRegAddr_va = (MUINT32*)(this->m_pIspDescript_vir+GET_MAX_CQ_DESCRIPTOR_SIZE()+ISP_DIP_CQ_DUMMY_BUFFER);
    this->m_pIspVirRegAddr_pa = (MUINT32*)(this->m_pIspDescript_phy+GET_MAX_CQ_DESCRIPTOR_SIZE()+ISP_DIP_CQ_DUMMY_BUFFER);
    #else
    this->m_pIspVirRegAddr_va = (MUINT32*)(this->m_pIspDescript_vir+(cqDesAllocSize/sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)));
    this->m_pIspVirRegAddr_pa = (MUINT32*)(this->m_pIspDescript_phy+(cqDesAllocSize/sizeof(MUINT32)));
    #endif
    //
    //initiate cq descriptor buffer
    for(int m=0;m<(DIP_A_MODULE_MAX - 1);m++){
        this->m_pIspDescript_vir[m].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
        this->m_pIspDescript_vir[m].v_reg_addr = 0;
    }
    this->m_pIspDescript_vir[DIP_A_END_].u.cmd = ISP_DRV_CQ_END_TOKEN;
    this->m_pIspDescript_vir[DIP_A_END_].v_reg_addr = 0;

    //LOG_INF("virtIspAddr:virt[0x%08x]/phy[0x%08x]",this->m_pIspVirRegAddr_va, this->m_pIspVirRegAddr_pa);
    //
    LOG_DBG("[CQ]m_TotalUserCnt(%d)--des,va(0x%lx),pa(0x%lx),size(0x%x)/(0x%x)-vir,va(0x%lx),pa(0x%lx),size(0x%x)",this->m_TotalUserCnt,
        (unsigned long)this->m_pIspDescript_vir,(unsigned long)this->m_pIspDescript_phy,GET_MAX_CQ_DESCRIPTOR_SIZE(),cqDesAllocSize,
        (unsigned long)this->m_pIspVirRegAddr_va,(unsigned long)this->m_pIspVirRegAddr_pa,DIP_BASE_RANGE_SPECIAL);


    #endif

    #if 0
    // allocate virCmdQ
    //va ptr chk, if not NULL, means imem was not freed also
    if(this->m_pIspVirRegAddr_va){
        LOG_ERR("va of VirReg is not null(0x%x)",this->m_pIspVirRegAddr_va);
        Result = MFALSE;
        goto EXIT;
    }
    //pa ptr chk, if not NULL, means imem was not freed also
    if(this->m_pIspVirRegAddr_pa){
        LOG_ERR("pa of VirReg is not null(0x%x)",this->m_pIspVirRegAddr_pa);
        Result = MFALSE;
        goto EXIT;
    }

    //imem allocate
    this->m_ispVirRegBufInfo.size = ((DIP_BASE_RANGE_SPECIAL+0x3) & (~0x3)); //4byte alignment,round up
    this->m_ispVirRegBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
    if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispVirRegBufInfo) ){
        LOG_ERR("m_pMemDrv->allocVirtBuf fail");
        Result = MFALSE;
        goto EXIT;
    }
    else{
        if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispVirRegBufInfo) ) {
            LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    //alignment chk
    if(( (_tmpVirAddr=this->m_ispVirRegBufInfo.virtAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem virRegAddr not 4 bytes alignment(0x%x), round up",this->m_ispVirRegBufInfo.virtAddr);
        _tmpVirAddr = (this->m_ispVirRegBufInfo.virtAddr + 0x3) & (~0x3);
    }
    if(( (_tmpPhyAddr=this->m_ispVirRegBufInfo.phyAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem phyRegAddr not 4 bytes alignment(0x%x), round up",this->m_ispVirRegBufInfo.phyAddr);
        _tmpPhyAddr = (this->m_ispVirRegBufInfo.phyAddr + 0x3) & (~0x3);
    }
    //va/pa 0 init
    memset((MUINT8*)_tmpVirAddr,0x0,DIP_BASE_RANGE_SPECIAL);
    this->m_pIspVirRegAddr_va = (MUINT32*)_tmpVirAddr;
    this->m_pIspVirRegAddr_pa = (MUINT32*)_tmpPhyAddr;
    //LOG_INF("virtIspAddr:virt[0x%08x]/phy[0x%08x]",this->m_pIspVirRegAddr_va, this->m_pIspVirRegAddr_pa);

    //CQ descriptor
    //descriptor vir ptr chk,if not NULL, means imem was not freed also
    if(this->m_pIspDescript_vir){
        LOG_ERR("vir list of descriptor is not null(0x%x)",this->m_pIspDescript_vir);
        Result = MFALSE;
        goto EXIT;
    }

    //descriptor phy ptr chk,if not NULL, means imem was not freed also
    if(this->m_pIspDescript_phy){
        LOG_ERR("phy list of descriptor is not null(0x%x)",this->m_pIspDescript_phy);
        goto EXIT;
    }

    //imem allcoate
    this->m_ispCQDescBufInfo.size = ((GET_MAX_CQ_DESCRIPTOR_SIZE() + 0x3) & (~0x3)); //4byte alignment,round up
    this->m_ispCQDescBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
    if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispCQDescBufInfo) ){
        LOG_ERR("m_pMemDrv->allocVirtBuf fail");
        Result = MFALSE;
        goto EXIT;
    }
    else{
        if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispCQDescBufInfo) ) {
            LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //alignment chk
    if(( (_tmpVirAddr=this->m_ispCQDescBufInfo.virtAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem virDescriptAddr not 4 bytes alignment(0x%x), round up",this->m_ispCQDescBufInfo.virtAddr);
        _tmpVirAddr = (this->m_ispCQDescBufInfo.virtAddr + 0x3) & (~0x3);
    }
    if(( (_tmpPhyAddr=this->m_ispCQDescBufInfo.phyAddr) % 0x4) != 0 ){
        LOG_WRN("[Warning]iMem phyDescriptAddr not 4 bytes alignment(0x%x), round up",this->m_ispCQDescBufInfo.phyAddr);
        _tmpPhyAddr = (this->m_ispCQDescBufInfo.phyAddr + 0x3) & (~0x3);
    }
    this->m_pIspDescript_vir = (ISP_DRV_CQ_CMD_DESC_STRUCT*)_tmpVirAddr;
    this->m_pIspDescript_phy = (MUINT32*)_tmpPhyAddr;
    //
    //descriptor initial
    for(int m=0;m<(DIP_A_MODULE_MAX - 1);m++){
        this->m_pIspDescript_vir[m].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
        this->m_pIspDescript_vir[m].v_reg_addr = 0;
    }
    this->m_pIspDescript_vir[DIP_A_END_].u.cmd = ISP_DRV_CQ_END_TOKEN;
    this->m_pIspDescript_vir[DIP_A_END_].v_reg_addr = 0;
    //
    LOG_DBG("[CQ]vir,va(0x%x),pa(0x%x),size(0x%x)-des,va(0x%x),pa(0x%x),size(0x%x)",
        this->m_ispVirRegBufInfo.virtAddr,this->m_ispVirRegBufInfo.phyAddr,this->m_ispVirRegBufInfo.size,
        this->m_ispCQDescBufInfo.virtAddr,this->m_ispCQDescBufInfo.phyAddr,this->m_ispCQDescBufInfo.size);
    //
    #endif
    //
    android_atomic_inc(&this->m_UserCnt);
    //
    //this->m_FSM = IspDrv_Init;
EXIT:

    //
    LOG_DBG("-,ret(%d),m_UserCnt(%d)", Result, this->m_UserCnt);
    return Result;
}


MBOOL IspDrvDip::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bClr = 0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_DBG("+,m_TotalUserCnt(%d),m_UserCnt(%d),curUser(%s)",this->m_TotalUserCnt, this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }
    // More than one user
    android_atomic_dec(&this->m_UserCnt);
    if(this->m_UserCnt > 0)
    {
        goto EXIT;
    }

    #if 1
#ifdef ISP_DRV_OPTIMIZE
        if ((this->m_dipCq == ISP_DRV_DIP_CQ_NUM-1) && (this->m_burstQueIdx == MAX_BURST_QUE_NUM -1) && (this->m_dupCqIdx == MAX_DUP_CQ_NUM - 1))
        {
            //free all of cq buffer
            if(gAllCqBufInfo.phyAddr != 0){
                if(this->m_pMemDrv->unmapPhyAddr(&gAllCqBufInfo) == 0){
                    if(this->m_pMemDrv->freeVirtBuf(&gAllCqBufInfo) != 0){
                        LOG_ERR("free all of cq buffer fail");
                        Result = MFALSE;
                        goto EXIT;
                    }
                }
                else{
                    LOG_ERR("unmap all of cq buffer fail");
                    Result = MFALSE;
                    goto EXIT;
                }
            }
            else{
                LOG_INF("all of cq PA is already NULL");
            }
        }
#else
    //free all of cq buffer
    if(this->m_ispAllCqBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispAllCqBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispAllCqBufInfo) != 0){
                LOG_ERR("free all of cq buffer fail");
                Result = MFALSE;
                goto EXIT;
            }
        }
        else{
            LOG_ERR("unmap all of cq buffer fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else{
        LOG_INF("all of cq PA is already NULL");
    }
#endif
    this->m_pIspDescript_vir = NULL;
    this->m_pIspDescript_phy = NULL;
    this->m_pIspVirRegAddr_va = NULL;
    this->m_pIspVirRegAddr_pa = NULL;

    #else

    //free cq descriptor
    if(this->m_ispCQDescBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispCQDescBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispCQDescBufInfo) != 0){
                LOG_ERR("free descriptor fail");
                Result = MFALSE;
                goto EXIT;
            }
        }
        else{
            LOG_ERR("unmap descriptor fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else{
        LOG_INF("cq descriptor PA is already NULL");
    }
    this->m_pIspDescript_vir = NULL;
    this->m_pIspDescript_phy = NULL;

    //free virtual isp
    if(this->m_ispVirRegBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispVirRegBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispVirRegBufInfo) != 0){
                LOG_ERR("free virRegister fail");
                Result = MFALSE;
                goto EXIT;
            }
        }
        else{
            LOG_ERR("unmap virRegister fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else{
        LOG_INF("cq virRegister PA is already NULL");
    }

    this->m_pIspVirRegAddr_va = NULL;
    this->m_pIspVirRegAddr_pa = NULL;
    #endif

    //
    //IspDrvImp
    if(this->m_TotalUserCnt==1 && this->m_UserCnt==0) {
        this->m_pIspDrvImp->uninit(userName);
        //destroy need to be paired with createinstance
        this->m_pIspDrvImp->destroyInstance();
        this->m_pIspDrvImp = NULL;
        //IMEM
        this->m_pMemDrv->uninit();
        this->m_pMemDrv->destroyInstance();
        this->m_pMemDrv = NULL;
        //this->m_bInit[this->m_hwModule] = MFALSE;
    }
    //


EXIT:

    LOG_DBG("-,ret(%d),m_UserCnt(%d)", Result, this->m_UserCnt);
    return Result;
}

/*
MBOOL IspDrvDip::start(void)
{
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]start -",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //this->m_FSM = IspDrv_Start;
    return MTRUE;
}

MBOOL IspDrvDip::stop(void)
{
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]stop -",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //this->m_FSM = IspDrv_Stop;
    return MTRUE;
}
*/


MBOOL IspDrvDip::waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    return this->m_pIspDrvImp->waitIrq(pWaitIrq);
}

MBOOL IspDrvDip::clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq)
{
    return this->m_pIspDrvImp->clearIrq(pClearIrq);
}

MBOOL IspDrvDip::registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    return this->m_pIspDrvImp->registerIrq(pRegIrq);
}


MBOOL IspDrvDip::signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
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

MBOOL IspDrvDip::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;

    switch(eCmd){
        case _GET_DIP_DBG_INFO:
            dumpDbgLog((IspDumpDbgLogDipPackage*)pData);
            break;
        case _GET_SOF_CNT:
        case _GET_DMA_ERR:
        case _GET_INT_ERR:
        case _GET_DROP_FRAME_STATUS:
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return rst;
}

MBOOL IspDrvDip::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{

    switch(eCmd){
        case _SET_DBG_INT:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DEBUG_FLAG,(unsigned char*)pData) < 0){
                LOG_ERR("kernel log enable error\n");
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



MBOOL IspDrvDip::dumpDbgLog(IspDumpDbgLogDipPackage *pDumpPackage)
{
    LOG_DBG("dumpDbgLog: tpipeTableVa(0x%lx), tpipeTablePa(0x%x), pTuningQue(0x%lx), IspDescriptVa(0x%lx), IspVirRegAddrVa(0x%lx)", \
		(unsigned long)pDumpPackage->tpipeTableVa, pDumpPackage->tpipeTablePa, (unsigned long)pDumpPackage->pTuningQue, (unsigned long)pDumpPackage->IspDescriptVa, (unsigned long)pDumpPackage->IspVirRegAddrVa);
    return MTRUE;
}



// set max descriptor size for HW
MBOOL IspDrvDip::cqAddModule(MUINT32 moduleId)
{
    int cmd;
    MUINTPTR dummyaddr;
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]moduleID:0x%x",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspDipCQModuleInfo[moduleId].sw_addr_ofst);
    //
    cmd = (mIspDipCQModuleInfo[moduleId].addr_ofst&0xffff)  |  \
            (((mIspDipCQModuleInfo[moduleId].reg_num-1)&0x3ff)<<16)  |  \
            (((ISP_CQ_APB_INST)<<26)&0x1C000000)  | \
            ((mIspDipCQModuleInfo[moduleId].addr_ofst&0x70000)<<(29-16));

    this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
    this->m_pIspDescript_vir[moduleId].u.cmd = cmd;
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);

    #if (CMDQ_HW_BUG==1)
    if (mIspDipCQModuleInfo[moduleId].reg_num > 1)
    {
        dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspDipCQModuleInfo[moduleId-1].sw_addr_ofst);
        //
        cmd = (mIspDipCQModuleInfo[moduleId-1].addr_ofst&0xffff)  |  \
                (((mIspDipCQModuleInfo[moduleId-1].reg_num-1)&0x3ff)<<16)  |  \
                (((ISP_CQ_APB_INST)<<26)&0x1C000000)  | \
                ((mIspDipCQModuleInfo[moduleId-1].addr_ofst&0x70000)<<(29-16));

        this->m_pIspDescript_vir[moduleId-1].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
        this->m_pIspDescript_vir[moduleId-1].u.cmd = cmd;
        LOG_DBG("bug cmd 1:0x%x",this->m_pIspDescript_vir[moduleId-1].u.cmd);

        dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspDipCQModuleInfo[moduleId+1].sw_addr_ofst);
        //
        cmd = (mIspDipCQModuleInfo[moduleId+1].addr_ofst&0xffff)  |  \
                (((mIspDipCQModuleInfo[moduleId+1].reg_num-1)&0x3ff)<<16)  |  \
                (((ISP_CQ_APB_INST)<<26)&0x1C000000)  | \
                ((mIspDipCQModuleInfo[moduleId+1].addr_ofst&0x70000)<<(29-16));

        this->m_pIspDescript_vir[moduleId+1].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
        this->m_pIspDescript_vir[moduleId+1].u.cmd = cmd;
        LOG_DBG("bug cmd 2:0x%x",this->m_pIspDescript_vir[moduleId+1].u.cmd);
    }
    else{}
    #endif
    return MTRUE;
}


MBOOL IspDrvDip::cqDelModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]moduleID:0x%x",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    //

    this->m_pIspDescript_vir[moduleId].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
    LOG_DBG("cmd(0x%x)",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MBOOL IspDrvDip::cqNopModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]moduleID:0x%x",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    this->m_pIspDescript_vir[moduleId].u.cmd &= (0xFFFFFFFF - (ISP_CQ_NOP_INST<<26));
    LOG_DBG("cmd(0x%x)",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MBOOL IspDrvDip::cqApbModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]moduleID:0x%x",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    this->m_pIspDescript_vir[moduleId].u.cmd &= (0xFFFFFFFF - (ISP_CQ_APB_INST<<26));
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MUINT32* IspDrvDip::getCQDescBufPhyAddr(void)
{
    LOG_DBG("m_hwModule(%d),m_dipCq(%d),m_burstQueIdx(%d),m_dupCqIdx(%d),va(0x%lx),pa(0x%lx)",
        this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,(unsigned long)this->m_pIspVirRegAddr_va,(unsigned long)this->m_pIspDescript_phy);


    return (MUINT32*)this->m_pIspDescript_phy;
}

MUINT32* IspDrvDip::getCQDescBufVirAddr(void)
{
    return (MUINT32*)this->m_pIspDescript_vir;
}

MUINT32* IspDrvDip::getIspVirRegPhyAddr(void)
{
    return (MUINT32*)this->m_pIspVirRegAddr_pa;
}

MUINT32* IspDrvDip::getIspVirRegVirAddr(void)
{
    return (MUINT32*)this->m_pIspVirRegAddr_va;
}


MBOOL IspDrvDip::getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize)
{
    LOG_DBG("moduleId(%d),[0x%x_0x%x_0x%x_0x%x]",moduleId,this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);

    addrOffset = mIspDipCQModuleInfo[moduleId].sw_addr_ofst;
    moduleSize = mIspDipCQModuleInfo[moduleId].reg_num;
    return MTRUE;
}

IspDrv* IspDrvDip::getPhyObj(void)
{
    #if 0
    if(this->m_bInit[this->m_dipModuleIdx] == MFALSE){
        LOG_ERR("[Error]hwModule:0x%x is not initialized yet!\n");
        return NULL;
    }
    #else
    if(this->m_TotalUserCnt <= 0){
        LOG_ERR("[Error]hwModule is not initialized yet!\n");
        return NULL;
    }
    #endif

    return this->m_pIspDrvImp;
}
#if 0
// for debug CQ virtual table
#define LOG_CQ_VIRTUAL_TABLE(_pIspVirCqVa_,_idx_,_num_) \
{   \
    switch(_num_) {   \
        case 1: \
            LOG_INF("0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2]);   \
            break;  \
        case 2: \
            LOG_INF("0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2], \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1]); \
            break;  \
        case 3: \
            LOG_INF("0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2]); \
            break;  \
        case 4: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2],   \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+3]); \
            break;  \
        case 5: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2],    \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+4]); \
            break;  \
        case 6: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2], \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+5]); \
            break;  \
        case 7: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+5],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+6]); \
            break;  \
        case 8: \
        default:    \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspDipCQModuleInfo[_idx_].addr_ofst >>2],   \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+5],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+6],  \
                _pIspVirCqVa_[(mIspDipCQModuleInfo[_idx_].addr_ofst>>2)+7]); \
            break;  \
    }   \
}
#endif
MBOOL IspDrvDip::dumpCQTable(void)
{
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]+",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    char _str[512] = {"\0"};
    char _tmp[16] = {"\0"};
    for(int i=0;i<DIP_A_END_;i++){
        if(this->m_pIspDescript_vir[i].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN) {
            LOG_INF("[%d]:[0x%08x]:",i,this->m_pIspDescript_vir[i].u.cmd);
            //LOG_CQ_VIRTUAL_TABLE(this->m_pIspVirRegAddr_va,i,mIspDipCQModuleInfo[i].reg_num);
            for(unsigned int j=0;j<mIspDipCQModuleInfo[i].reg_num;j++){
                sprintf(_tmp,"0x%08x-",this->m_pIspVirRegAddr_va[(mIspDipCQModuleInfo[i].addr_ofst >>2) + j]);
                strncat(_str,_tmp,strlen(_tmp));
            }
            LOG_INF(" %s\n",_str);
            _str[0] = '\0';
        }
    }

    return MTRUE;
}



MBOOL IspDrvDip::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]+",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //

    switch(caller){
        case DIP_A:
            shift = 0x0;
            legal_range = DIP_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        pRegIo[i].Data = this->m_pIspVirRegAddr_va[(pRegIo[i].Addr + shift)>>2];
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,pRegIo[i].Addr,pRegIo[i].Data);
    }
    return MTRUE;
}


MUINT32 IspDrvDip::readReg(MUINT32 Addr,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]Addr:0x%08X",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx,Addr);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    switch(caller){
        case DIP_A:
            shift = 0x0;
            legal_range = DIP_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    if(Addr >= legal_range){
        LOG_ERR("over range(0x%x)\n",Addr);
        return MFALSE;
    }
    LOG_DBG("Data:0x%x",this->m_pIspVirRegAddr_va[(Addr+shift)>>2]);

    return this->m_pIspVirRegAddr_va[(Addr+shift)>>2];
}

MBOOL IspDrvDip::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]+",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //

    switch(caller){
        case DIP_A:
            shift = 0x0;
            legal_range = DIP_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        this->m_pIspVirRegAddr_va[(pRegIo[i].Addr+shift)>>2] = pRegIo[i].Data;
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,pRegIo[i].Addr,this->m_pIspVirRegAddr_va[(pRegIo[i].Addr+shift)>>2]);
    }
    return MTRUE;
}

MBOOL IspDrvDip::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("IspDrvDip:[0x%x_0x%x_0x%x_0x%x]+",this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //

    switch(caller){
            case DIP_A:
                shift = 0x0;
                legal_range = DIP_BASE_RANGE_SPECIAL;
                break;
            default:
                LOG_ERR("un-supported module_0x%x\n",caller);
                return MFALSE;
                break;
        }
    if(Addr >= legal_range){
        LOG_ERR("over range(0x%x)\n",Addr);
        return MFALSE;
    }
    this->m_pIspVirRegAddr_va[(Addr+shift)>>2] = Data;
    LOG_DBG("addr:0x%x,data:0x%x",Addr,this->m_pIspVirRegAddr_va[(Addr+shift)>>2]);

    return MTRUE;
}

MBOOL IspDrvDip::flushCmdQ(void)
{
    LOG_DBG("+,m_hwModule(%d),m_dipCq(%d),m_burstQueIdx(%d),m_dupCqIdx(%d)",
        this->m_hwModule,this->m_dipCq,this->m_burstQueIdx,this->m_dupCqIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //

    LOG_DBG("+");//for flush performance tracking
    #if 1
#ifdef ISP_DRV_OPTIMIZE

#else
    if(0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispAllCqBufInfo) ){
        LOG_ERR("all of cq buffer flush fail");
        return MFALSE;
    }
#endif
    #else
    if(0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispVirRegBufInfo) ){
        LOG_ERR("virReg flush fail");
        return MFALSE;
    }
    if( 0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispCQDescBufInfo) ){
        LOG_ERR("descriptor flush fail");
        return MFALSE;
    }
    #endif
    LOG_DBG("-");//for flush performance tracking
    return MTRUE;
}

MBOOL IspDrvDip::dumpTuningDebugLog(void)
{
	IspDrvDipPhy* pDrvDipPhy = (IspDrvDipPhy*)IspDrvDipPhy::createInstance(DIP_A);
	MUINT32 regW, regR, val, cnt;
	//
	pDrvDipPhy->init("dumpTuningDebugLog");

	LOG_INF("========== DUMP DIP ==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_CTL_RGB_EN);
	LOG_INF("DIP_X_CTL_RGB_EN=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_CTL_YUV_EN);
	LOG_INF("DIP_X_CTL_YUV_EN=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_CTL_YUV2_EN);
	LOG_INF("DIP_X_CTL_YUV2_EN=0x%08x",regR);


//DBS2
	LOG_INF("========== DUMP DBS2 PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_SIGMA);
	LOG_INF("DIP_X_DBS2_SIGMA=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_BSTBL_0);
	LOG_INF("DIP_X_DBS2_BSTBL_0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_BSTBL_1);
	LOG_INF("DIP_X_DBS2_BSTBL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_BSTBL_2);
	LOG_INF("DIP_X_DBS2_BSTBL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_BSTBL_3);
	LOG_INF("DIP_X_DBS2_BSTBL_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_CTL);
	LOG_INF("DIP_X_DBS2_CTL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_DBS2_CTL_2);
	LOG_INF("DIP_X_DBS2_CTL_2=0x%08x",regR);

//OBC2
	LOG_INF("========== DUMP OBC2 PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_OFFST0);
	LOG_INF("DIP_X_OBC2_OFFST0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_OFFST1);
	LOG_INF("DIP_X_OBC2_OFFST1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_OFFST2);
	LOG_INF("DIP_X_OBC2_OFFST2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_OFFST3);
	LOG_INF("DIP_X_OBC2_OFFST3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_GAIN0);
	LOG_INF("DIP_X_OBC2_GAIN0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_GAIN1);
	LOG_INF("DIP_X_OBC2_GAIN1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_GAIN2);
	LOG_INF("DIP_X_OBC2_GAIN2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_OBC2_GAIN3);
	LOG_INF("DIP_X_OBC2_GAIN3=0x%08x",regR);

//LSC2
	LOG_INF("========== DUMP LSC2 PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_CTL1);
	LOG_INF("DIP_X_LSC2_CTL1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_CTL2);
	LOG_INF("DIP_X_LSC2_CTL2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_CTL3);
	LOG_INF("DIP_X_LSC2_CTL3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_LBLOCK);
	LOG_INF("DIP_X_LSC2_LBLOCK=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_RATIO_0);
	LOG_INF("DIP_X_LSC2_RATIO_0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_RATIO_1);
	LOG_INF("DIP_X_LSC2_RATIO_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_TPIPE_OFST);
	LOG_INF("DIP_X_LSC2_TPIPE_OFST=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_TPIPE_SIZE);
	LOG_INF("DIP_X_LSC2_TPIPE_SIZE=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LSC2_GAIN_TH);
	LOG_INF("DIP_X_LSC2_GAIN_TH=0x%08x",regR);

//PGN
	LOG_INF("========== DUMP PGN PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_SATU_1);
	LOG_INF("DIP_X_PGN_SATU_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_SATU_2);
	LOG_INF("DIP_X_PGN_SATU_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_GAIN_1);
	LOG_INF("DIP_X_PGN_GAIN_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_GAIN_2);
	LOG_INF("DIP_X_PGN_GAIN_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_OFST_1);
	LOG_INF("DIP_X_PGN_OFST_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PGN_OFST_2);
	LOG_INF("DIP_X_PGN_OFST_2=0x%08x",regR);


//G2G
	LOG_INF("========== DUMP G2G PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_1);
	LOG_INF("DIP_X_G2G_CNV_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_2);
	LOG_INF("DIP_X_G2G_CNV_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_3);
	LOG_INF("DIP_X_G2G_CNV_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_4);
	LOG_INF("DIP_X_G2G_CNV_4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_5);
	LOG_INF("DIP_X_G2G_CNV_5=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CNV_6);
	LOG_INF("DIP_X_G2G_CNV_6=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2G_CTRL);
	LOG_INF("DIP_X_G2G_CTRL=0x%08x",regR);


//G2C
	LOG_INF("========== DUMP G2C PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_0A);
	LOG_INF("DIP_X_G2C_CONV_0A=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_0B);
	LOG_INF("DIP_X_G2C_CONV_0B=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_1A);
	LOG_INF("DIP_X_G2C_CONV_1A=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_1B);
	LOG_INF("DIP_X_G2C_CONV_1B=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_2A);
	LOG_INF("DIP_X_G2C_CONV_2A=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CONV_2B);
	LOG_INF("DIP_X_G2C_CONV_2B=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_SHADE_CON_1);
	LOG_INF("DIP_X_G2C_SHADE_CON_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_SHADE_CON_2);
	LOG_INF("DIP_X_G2C_SHADE_CON_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_SHADE_CON_3);
	LOG_INF("DIP_X_G2C_SHADE_CON_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_SHADE_TAR);
	LOG_INF("DIP_X_G2C_SHADE_TAR=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_SHADE_SP);
	LOG_INF("DIP_X_G2C_SHADE_SP=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CFC_CON_1);
	LOG_INF("DIP_X_G2C_CFC_CON_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_G2C_CFC_CON_2);
	LOG_INF("DIP_X_G2C_CFC_CON_2=0x%08x",regR);

//MFB
	LOG_INF("========== DUMP MFB PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_CON);
	LOG_INF("DIP_X_MFB_CON=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_LL_CON1);
	LOG_INF("DIP_X_MFB_LL_CON1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_LL_CON2);
	LOG_INF("DIP_X_MFB_LL_CON2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_LL_CON3);
	LOG_INF("DIP_X_MFB_LL_CON3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_LL_CON4);
	LOG_INF("DIP_X_MFB_LL_CON4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MFB_EDGE);
	LOG_INF("DIP_X_MFB_EDGE=0x%08x",regR);


//MIX3
	LOG_INF("========== DUMP MIX3 PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MIX3_CTRL_0);
	LOG_INF("DIP_X_MIX3_CTRL_0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MIX3_CTRL_1);
	LOG_INF("DIP_X_MIX3_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_MIX3_SPARE);
	LOG_INF("DIP_X_MIX3_SPARE=0x%08x",regR);

//LCE
	LOG_INF("========== DUMP LCE PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_ANR2_LCE);
	LOG_INF("DIP_X_ANR2_LCE=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_CON);
	LOG_INF("DIP_X_LCE25_CON=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_ZR);
	LOG_INF("DIP_X_LCE25_ZR=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_SLM_SIZE);
	LOG_INF("DIP_X_LCE25_SLM_SIZE=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_OFST);
	LOG_INF("DIP_X_LCE25_OFST=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_BIAS);
	LOG_INF("DIP_X_LCE25_BIAS=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_IMAGE_SIZE);
	LOG_INF("DIP_X_LCE25_IMAGE_SIZE=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_BIL_TH0);
	LOG_INF("DIP_X_LCE25_BIL_TH0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_BIL_TH1);
	LOG_INF("DIP_X_LCE25_BIL_TH1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA0);
	LOG_INF("DIP_X_LCE25_TM_PARA0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA1);
	LOG_INF("DIP_X_LCE25_TM_PARA1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA2);
	LOG_INF("DIP_X_LCE25_TM_PARA2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA3);
	LOG_INF("DIP_X_LCE25_TM_PARA3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA4);
	LOG_INF("DIP_X_LCE25_TM_PARA4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA5);
	LOG_INF("DIP_X_LCE25_TM_PARA5=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_LCE25_TM_PARA6);
	LOG_INF("DIP_X_LCE25_TM_PARA6=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_ANR_LCE);
	LOG_INF("DIP_X_ANR_LCE=0x%08x",regR);


//GGM
	LOG_INF("========== DUMP GGM PART==============");
	//DIP_X_GGM_LUT_RB[144];
	//DIP_X_GGM_LUT_G[144];
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_GGM_CTRL);
	LOG_INF("DIP_X_GGM_CTRL=0x%08x",regR);


//PCA
	LOG_INF("========== DUMP PCA PART==============");
	//DIP_X_PCA_TBL[360];
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON1);
	LOG_INF("DIP_X_PCA_CON1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON2);
	LOG_INF("DIP_X_PCA_CON2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON3);
	LOG_INF("DIP_X_PCA_CON3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON4);
	LOG_INF("DIP_X_PCA_CON4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON5);
	LOG_INF("DIP_X_PCA_CON5=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON6);
	LOG_INF("DIP_X_PCA_CON6=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_PCA_CON7);
	LOG_INF("DIP_X_PCA_CON7=0x%08x",regR);


//SEEE
	LOG_INF("========== DUMP SEEE PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_CTRL);
	LOG_INF("DIP_X_SEEE_CTRL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_CLIP_CTRL_1);
	LOG_INF("DIP_X_SEEE_CLIP_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_CLIP_CTRL_2);
	LOG_INF("DIP_X_SEEE_CLIP_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_CLIP_CTRL_3);
	LOG_INF("DIP_X_SEEE_CLIP_CTRL_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_BLND_CTRL_1);
	LOG_INF("DIP_X_SEEE_BLND_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_BLND_CTRL_2);
	LOG_INF("DIP_X_SEEE_BLND_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GN_CTRL);
	LOG_INF("DIP_X_SEEE_GN_CTRL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_LUMA_CTRL_1);
	LOG_INF("DIP_X_SEEE_LUMA_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_LUMA_CTRL_2);
	LOG_INF("DIP_X_SEEE_LUMA_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_LUMA_CTRL_3);
	LOG_INF("DIP_X_SEEE_LUMA_CTRL_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_LUMA_CTRL_4);
	LOG_INF("DIP_X_SEEE_LUMA_CTRL_4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SLNK_CTRL_1);
	LOG_INF("DIP_X_SEEE_SLNK_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SLNK_CTRL_2);
	LOG_INF("DIP_X_SEEE_SLNK_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_1);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_2);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_3);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_4);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_4=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_5);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_5=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_GLUT_CTRL_6);
	LOG_INF("DIP_X_SEEE_GLUT_CTRL_6=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_OUT_EDGE_CTRL);
	LOG_INF("DIP_X_SEEE_OUT_EDGE_CTRL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_Y_CTRL);
	LOG_INF("DIP_X_SEEE_SE_Y_CTRL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_EDGE_CTRL_1);
	LOG_INF("DIP_X_SEEE_SE_EDGE_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_EDGE_CTRL_2);
	LOG_INF("DIP_X_SEEE_SE_EDGE_CTRL_2=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_EDGE_CTRL_3);
	LOG_INF("DIP_X_SEEE_SE_EDGE_CTRL_3=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_SPECL_CTRL);
	LOG_INF("DIP_X_SEEE_SE_SPECL_CTRL=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_CORE_CTRL_1);
	LOG_INF("DIP_X_SEEE_SE_CORE_CTRL_1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SEEE_SE_CORE_CTRL_2);
	LOG_INF("DIP_X_SEEE_SE_CORE_CTRL_2=0x%08x",regR);

//SL2
	LOG_INF("========== DUMP SL2 PART==============");
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_CEN);
	LOG_INF("DIP_X_SL2_CEN=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_RR_CON0);
	LOG_INF("DIP_X_SL2_RR_CON0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_RR_CON1);
	LOG_INF("DIP_X_SL2_RR_CON1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_GAIN);
	LOG_INF("DIP_X_SL2_GAIN=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_RZ);
	LOG_INF("DIP_X_SL2_RZ=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_XOFF);
	LOG_INF("DIP_X_SL2_XOFF=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_YOFF);
	LOG_INF("DIP_X_SL2_YOFF=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_SLP_CON0);
	LOG_INF("DIP_X_SL2_SLP_CON0=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_SLP_CON1);
	LOG_INF("DIP_X_SL2_SLP_CON1=0x%08x",regR);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_SL2_SIZE);
	LOG_INF("DIP_X_SL2_SIZE=0x%08x",regR);

//BNR2

//UDM

//NR3D

//NBC

//NBC2

	return 0;
}



