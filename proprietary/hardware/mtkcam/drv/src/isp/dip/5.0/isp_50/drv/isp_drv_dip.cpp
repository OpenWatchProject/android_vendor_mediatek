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
//#include <isp_drv_stddef.h>
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

DipDrv* VirDipDrvImp::createInstance(DIP_HW_MODULE hwModule, void* ispVirRegMap)
{
    static VirDipDrvImp singleton[DIP_HW_MAX];
    singleton[hwModule].m_hwModule = hwModule;
    singleton[hwModule].m_pIspRegMap = ispVirRegMap;
    return &singleton[hwModule];
}

void VirDipDrvImp::destroyInstance(void)
{

}

VirDipDrvImp::VirDipDrvImp()
{
    m_pIspDescript_vir = NULL;
    m_pIspDescript_phy = NULL;
    m_IspDescript_oft  = 0;
    //
    m_pIspVirRegAddr_va = NULL;
    m_pIspVirRegAddr_pa = NULL;
    m_IspVirRegAddr_oft = 0;

    m_DipRegLegalRange = DIP_REG_RANGE;
    m_hwModule = DIP_HW_A;

    LOG_DBG("getpid[0x%08x],gettid[0x%08x]\n", getpid() ,gettid());
}



MBOOL VirDipDrvImp::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    if ((m_pIspVirRegAddr_va == NULL) || (pRegIo == NULL)){
        LOG_ERR("[0x%x]. Virtual ISP Addr:0x%x or pRegIo:0x%x is NULL!! Count:0x%x, caller:0x%x",this->m_hwModule, m_pIspVirRegAddr_va, pRegIo, Count, caller);
        return MFALSE;
    }  
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= m_DipRegLegalRange){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        pRegIo[i].Data = this->m_pIspVirRegAddr_va[pRegIo[i].Addr >>2];
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,pRegIo[i].Addr,pRegIo[i].Data);
    }
    return MTRUE;
}


MUINT32 VirDipDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    if (m_pIspVirRegAddr_va == NULL){
        LOG_ERR("[0x%x]. Virtual ISP Addr:0x%x is NULL!! Addr:0x%x, caller:0x%x",this->m_hwModule, m_pIspVirRegAddr_va, Addr, caller);
        return MFALSE;
    }
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    if(Addr >= m_DipRegLegalRange){
        LOG_ERR("over range(0x%x)\n",Addr);
        return MFALSE;
    }
    LOG_DBG("Data:0x%x",this->m_pIspVirRegAddr_va[Addr>>2]);

    return this->m_pIspVirRegAddr_va[Addr>>2];
}

MBOOL VirDipDrvImp::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    if ((m_pIspVirRegAddr_va == NULL) || (pRegIo == NULL)){
        LOG_ERR("[0x%x]. Virtual ISP Addr:0x%x or pRegIo:0x%x is NULL!! Count:0x%x, caller:0x%x",this->m_hwModule, m_pIspVirRegAddr_va, pRegIo, Count, caller);
        return MFALSE;
    } 
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= m_DipRegLegalRange){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        this->m_pIspVirRegAddr_va[pRegIo[i].Addr>>2] = pRegIo[i].Data;
        LOG_DBG("this->m_pIspVirRegAddr_va:0x%x, %d:addr:0x%x,data:0x%x",this->m_pIspVirRegAddr_va, i,pRegIo[i].Addr,this->m_pIspVirRegAddr_va[pRegIo[i].Addr>>2]);
    }
    return MTRUE;
}


MBOOL VirDipDrvImp::writeRegs(MUINT32 OffsetAddr,MUINT32 Count,MUINT32* SrcAddr, MINT32 caller)
{
    MUINT32 copysize = 0;
    if ((m_pIspVirRegAddr_va == NULL) || (SrcAddr == NULL)){
        LOG_ERR("[0x%x]. Virtual ISP Addr:0x%x or SrcAddr:0x%x is NULL!! Count:0x%x, caller:0x%x",this->m_hwModule, m_pIspVirRegAddr_va, SrcAddr, Count, caller);
        return MFALSE;
    } 
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    copysize = Count<<2;
    if((OffsetAddr+copysize) >= m_DipRegLegalRange){
        LOG_ERR("over range m_DipRegLegalRange(0x%x), OffsetAddr(0x%x), Count(0x%x)\n",m_DipRegLegalRange, OffsetAddr, Count);
        copysize = m_DipRegLegalRange - OffsetAddr;
    }
    ::memcpy(&m_pIspVirRegAddr_va[OffsetAddr>>2], &SrcAddr[OffsetAddr>>2], copysize);
    LOG_DBG("caller:%d, OffsetAddr:0x%x,Count:0x%x",caller, OffsetAddr, Count);

    return MTRUE;
}


MBOOL VirDipDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    if (m_pIspVirRegAddr_va == NULL){
        LOG_ERR("[0x%x]. Virtual ISP Addr:0x%x is NULL!! Data:0x%x, caller:0x%x",this->m_hwModule, m_pIspVirRegAddr_va, Data, caller);
        return MFALSE;
    } 
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    if(Addr >= m_DipRegLegalRange){
        LOG_ERR("over range(0x%x)\n",Addr);
        return MFALSE;
    }
    this->m_pIspVirRegAddr_va[Addr>>2] = Data;
    LOG_DBG("m_pIspVirRegAddr_va:0x%x, addr:0x%x,Data:0x%x, m_pIspVirRegAddr_va[0x%x]:0x%x",this->m_pIspVirRegAddr_va, Addr, Data, (Addr>>2), this->m_pIspVirRegAddr_va[Addr>>2]);

    return MTRUE;
}


// set max descriptor size for HW
MBOOL VirDipDrvImp::cqAddModule(MUINT32 moduleId)
{
    int cmd;
    MUINTPTR dummyaddr;
    LOG_DBG("[0x%x]moduleID:0x%x",this->m_hwModule,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if ((m_pIspVirRegAddr_pa == NULL) || (m_pIspDescript_vir == NULL)){
        LOG_ERR("[0x%x]. Virtual ISP Phy Addr:0x%x or CMDQ Addr:0x%x is NULL!! moduleID:0x%x",this->m_hwModule, m_pIspVirRegAddr_pa, m_pIspDescript_vir, moduleId);
        return MFALSE;
    }
    dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspDipCQModuleInfo[moduleId].sw_addr_ofst);
    //
    cmd = ((mIspDipCQModuleInfo[moduleId].sw_addr_ofst+DIP_OFFSET)&0xffff)  |  \
            (((mIspDipCQModuleInfo[moduleId].reg_num-1)&0x3ff)<<16)  |  \
            (((ISP_CQ_APB_INST)<<26)&0x1C000000)  | \
            (((mIspDipCQModuleInfo[moduleId].sw_addr_ofst+DIP_OFFSET)&0x70000)<<(29-16));

    this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
    this->m_pIspDescript_vir[moduleId].u.cmd = cmd;
    LOG_DBG("[0x%x]. this->m_pIspDescript_vir:0x%x, addr:0x%x cmd:0x%x, moduleID:0x%x",this->m_hwModule, this->m_pIspDescript_vir, this->m_pIspDescript_vir[moduleId].v_reg_addr, this->m_pIspDescript_vir[moduleId].u.cmd, moduleId);

    return MTRUE;
}


MBOOL VirDipDrvImp::cqDelModule(MUINT32 moduleId)
{
    android::Mutex::Autolock lock(this->IspRegMutex);
    if (m_pIspDescript_vir == NULL){
        LOG_ERR("[0x%x]. Virtual CMDQ Addr:0x%x is NULL!! moduleID:0x%x",this->m_hwModule, m_pIspDescript_vir, moduleId);
        return MFALSE;
    }

    this->m_pIspDescript_vir[moduleId].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
    LOG_DBG("cmd(0x%x)",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}
MBOOL VirDipDrvImp::setCQDescBufPhyAddr(MUINT32* pIspDescript_phy)
{
    this->m_pIspDescript_phy = pIspDescript_phy;
    return MTRUE;
}

MBOOL VirDipDrvImp::setCQDescBufVirAddr(ISP_DRV_CQ_CMD_DESC_STRUCT* pIspDescript_vir)
{
    this->m_pIspDescript_vir = pIspDescript_vir;
    return MTRUE;
}
MUINT32* VirDipDrvImp::getCQDescBufPhyAddr(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspDescript_phy:0x%x!!",this->m_hwModule, (unsigned long)this->m_pIspDescript_phy);
    return (MUINT32*)this->m_pIspDescript_phy;
}

MUINT32* VirDipDrvImp::getCQDescBufVirAddr(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspDescript_vir:0x%x!!",this->m_hwModule, (unsigned long)this->m_pIspDescript_vir);
    return (MUINT32*)this->m_pIspDescript_vir;
}

MBOOL VirDipDrvImp::setIspVirRegPhyAddr(MUINT32* pIspVirRegAddr_pa)
{
    this->m_pIspVirRegAddr_pa = pIspVirRegAddr_pa;
    return MTRUE;
}

MBOOL VirDipDrvImp::setIspVirRegVirAddr(MUINT32* pIspVirRegAddr_va)
{
    this->m_pIspVirRegAddr_va = pIspVirRegAddr_va;
    return MTRUE;
}

MUINT32* VirDipDrvImp::getIspVirRegPhyAddr(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspVirRegAddr_pa:0x%x!!",this->m_hwModule, (unsigned long)this->m_pIspVirRegAddr_pa);
    return (MUINT32*)this->m_pIspVirRegAddr_pa;
}

MUINT32* VirDipDrvImp::getIspVirRegVirAddr(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspVirRegAddr_va:0x%x!!",this->m_hwModule, (unsigned long)this->m_pIspVirRegAddr_va);
    return (MUINT32*)this->m_pIspVirRegAddr_va;
}

MBOOL VirDipDrvImp::setCQDescBufOft(MUINT32 IspDescript_oft)
{
    this->m_IspDescript_oft = IspDescript_oft;
    return MTRUE;
}

MBOOL VirDipDrvImp::setIspVirRegOft(MUINT32 IspVirRegAddr_oft)
{
    this->m_IspVirRegAddr_oft = IspVirRegAddr_oft;
    return MTRUE;
}

MUINT32 VirDipDrvImp::getCQDescBufOft(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspDescript_phy:0x%x!!",this->m_hwModule, (unsigned long)this->m_IspDescript_oft);
    return (MUINT32)this->m_IspDescript_oft;
}

MUINT32 VirDipDrvImp::getIspVirRegOft(void)
{
    LOG_DBG("this->m_hwModule:0x%x, this->m_pIspVirRegAddr_pa:0x%x!!",this->m_hwModule, (unsigned long)this->m_IspVirRegAddr_oft);
    return (MUINT32)this->m_IspVirRegAddr_oft;
}


MBOOL VirDipDrvImp::getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize)
{
    LOG_DBG("moduleId(%d),[0x%x], addrOffset:0x%x, moduleSize:0x%x",moduleId,this->m_hwModule, addrOffset, moduleSize);
    addrOffset = mIspDipCQModuleInfo[moduleId].sw_addr_ofst;
    moduleSize = mIspDipCQModuleInfo[moduleId].reg_num;
    return MTRUE;
}

MBOOL VirDipDrvImp::dumpCQTable(void)
{
    LOG_DBG("[0x%x_0x%x_0x%x_0x%x]+",this->m_hwModule);
    char _str[4096] = {"\0"};
    char _tmp[16] = {"\0"};
    if (m_pIspDescript_vir == NULL){
        LOG_ERR("[0x%x]. Virtual CMDQ Addr:0x%x is NULL!!",this->m_hwModule, m_pIspDescript_vir);
        return MFALSE;
    }
    for(int i=0;i<DIP_A_END_;i++){
        if(this->m_pIspDescript_vir[i].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN) {
            LOG_INF("[%d]:[0x%08x]:",i,this->m_pIspDescript_vir[i].u.cmd);
            for(unsigned int j=0;j<mIspDipCQModuleInfo[i].reg_num;j++){
                snprintf (_tmp, 16, "0x%08x-", this->m_pIspVirRegAddr_va[(mIspDipCQModuleInfo[i].sw_addr_ofst >>2) + j]);
                strncat (_str, _tmp, strlen(_tmp));
            }
            LOG_INF(" %s\n",_str);
            _str[0] = '\0';
        }
    }

    return MTRUE;
}

