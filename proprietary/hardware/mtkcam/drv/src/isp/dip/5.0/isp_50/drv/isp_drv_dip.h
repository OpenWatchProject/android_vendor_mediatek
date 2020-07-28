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

#ifndef _ISP_DRV_DIP_H_
#define _ISP_DRV_DIP_H_
/**
    this module contains DIP_A in abstract method
    note:
        DIP_A phy is not included
*/
//#include "isp_drv_dip_com.h"
#include "isp_drv.h"
#include "isp_drv_dip_platform.h"

///////////////////////////////////////////////////////////////////////////
/**
    RW marco, for coding convenience

    note: dip module only
*/


//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount);\
    ret;\
})

#define DIP_WRITE_REGS(IspDrvPtr,RegName,RegCount,SrcAddr...)({\
        MBOOL __ret=0;\
        MUINT32 __i=0;\
        MUINT32 __offset;\
        __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                    (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
        __ret=IspDrvPtr->writeRegs(__offset,RegCount,SrcAddr);\
        __ret;\
    })

#define DIP_READ_REG_NOPROTECT(IspDrvPtr,RegName,...)    DIP_READ_REG(IspDrvPtr,RegName)

#define DIP_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset);\
})

#define DIP_READ_BITS_NOPROTECT(IspDrvPtr,RegName,FieldName,...)    DIP_READ_BITS(IspDrvPtr,RegName,FieldName)

#define DIP_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define DIP_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value);\
}while(0);

#define DIP_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw);\
}while(0);

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
//-----------------------------------------------------------------------------

// for isp dip thread control
class VirDipDrvImp : public VirDipDrv
{
    public:
        VirDipDrvImp();
    public:
        static DipDrv*   createInstance(DIP_HW_MODULE hwModule, void* ispVirRegMap);
        virtual void     destroyInstance(void);

        virtual MBOOL    init(const char* userName=""){return MTRUE;}
        virtual MBOOL    uninit(const char* userName=""){return MTRUE;}

        virtual MBOOL    waitIrq(DIP_WAIT_IRQ_ST* pWaitIrq){return MTRUE;}
        virtual MBOOL    clearIrq(DIP_CLEAR_IRQ_ST* pClearIrq){return MTRUE;}
        virtual MBOOL    registerIrq(DIP_REGISTER_USERKEY_STRUCT* pRegIrq){return MTRUE;}
        virtual MBOOL    signalIrq(DIP_WAIT_IRQ_ST* pWaitIrq){return MTRUE;}

        virtual MBOOL    readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32  readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL    writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL    writeRegs(MUINT32 DstOffsetAddr,MUINT32 Count,MUINT32* SrcAddr, MINT32 caller=0);
        virtual MBOOL    writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);
        virtual MBOOL    setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData){return MTRUE;}

        virtual MBOOL    cqAddModule(MUINT32 moduleId);
        virtual MBOOL    cqDelModule(MUINT32 moduleId);
        virtual MBOOL    dumpCQTable(void);

        virtual MBOOL     setCQDescBufPhyAddr(MUINT32* pIspDescript_phy);        
        virtual MBOOL     setCQDescBufVirAddr(ISP_DRV_CQ_CMD_DESC_STRUCT* pIspDescript_vir);
        virtual MUINT32*  getCQDescBufPhyAddr(void);
        virtual MUINT32*  getCQDescBufVirAddr(void);
        virtual MBOOL     setIspVirRegPhyAddr(MUINT32* pIspVirRegAddr_pa);
        virtual MBOOL     setIspVirRegVirAddr(MUINT32* pIspVirRegAddr_va);
        virtual MUINT32*  getIspVirRegPhyAddr(void);
        virtual MUINT32*  getIspVirRegVirAddr(void);
        virtual MBOOL     setCQDescBufOft(MUINT32 IspDescript_oft);
        virtual MBOOL     setIspVirRegOft(MUINT32 IspVirRegAddr_oft);
        virtual MUINT32   getCQDescBufOft(void);
        virtual MUINT32   getIspVirRegOft(void);
        virtual MBOOL     getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);

    private:
        mutable android::Mutex       IspRegMutex;
        //
        MUINT32*                     m_pIspVirRegAddr_va;
        MUINT32*                     m_pIspVirRegAddr_pa;
        MUINT32                      m_IspVirRegAddr_oft;

        //
        ISP_DRV_CQ_CMD_DESC_STRUCT*  m_pIspDescript_vir;
        MUINT32*                     m_pIspDescript_phy;
        MUINT32                      m_IspDescript_oft;

        DIP_HW_MODULE                m_hwModule;
        MUINT32                      m_DipRegLegalRange;
};


#endif //_ISP_DRV_DIP_H_

