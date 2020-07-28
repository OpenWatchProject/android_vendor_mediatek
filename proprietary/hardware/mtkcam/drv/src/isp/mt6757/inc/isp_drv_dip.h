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
#include "isp_drv_dip_com.h"
#include <imem_drv.h>

///////////////////////////////////////////////////////////////////////////
/**
    RW marco, for coding convenience

    note: dip module only
*/


//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_READ_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,(_dip_));\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_WRITE_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,(_dip_));\
    __ret;\
})

#define DIP_READ_REG_NOPROTECT(_dip_,IspDrvPtr,RegName,...)    DIP_READ_REG(_dip_,IspDrvPtr,RegName)

#define DIP_READ_REG(_dip_,IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,(_dip_));\
})

#define DIP_READ_BITS_NOPROTECT(_dip_,IspDrvPtr,RegName,FieldName,...)    DIP_READ_BITS(_dip_,IspDrvPtr,RegName,FieldName)

#define DIP_READ_BITS(_dip_,IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define DIP_WRITE_REG(_dip_,IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value,(_dip_));\
}while(0);

#define DIP_WRITE_BITS(_dip_,IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,(_dip_));\
}while(0);


//////////////////////////////////////////////////////////////////
/**
    vir cq definition
*/
#define CmdQ_Cache  0   //CmdQ using by Cache is 1

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
// for isp dip thread control
class IspDrvDip : public IspDrvVir, public IspDrv
{
    public:
        IspDrvDip(){}
        IspDrvDip(ISP_HW_MODULE hwModule, E_ISP_DIP_CQ dipCq, MUINT32 burstQueIdx, MUINT32 dupCqIdx);
    public:

        static  IspDrvDip*  createInstance(ISP_HW_MODULE hwModule, E_ISP_DIP_CQ dipCq, MUINT32 burstQueIdx, MUINT32 dupCqIdx, const char* userName);

        virtual void    destroyInstance(void);


        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        //virtual MBOOL   start(void);
        //virtual MBOOL   stop(void);

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);

        virtual MBOOL   cqAddModule(MUINT32 moduleId);
        virtual MBOOL   cqDelModule(MUINT32 moduleId);
        virtual MBOOL   cqNopModule(MUINT32 moduleId); // skip this description
        virtual MBOOL   cqApbModule(MUINT32 moduleId);
        virtual MBOOL   dumpCQTable(void);


        virtual MUINT32*    getCQDescBufPhyAddr(void);
        virtual MUINT32*    getCQDescBufVirAddr(void);
        virtual MUINT32*    getIspVirRegPhyAddr(void);
        virtual MUINT32*    getIspVirRegVirAddr(void);
        virtual MBOOL       getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);
        virtual MBOOL       flushCmdQ(void);

        virtual IspDrv*     getPhyObj(void);

    public:   // not inherit from IspDrvVir or IspDrv
        virtual MBOOL dumpDbgLog(IspDumpDbgLogDipPackage *pDumpPackage);
        virtual MBOOL dumpTuningDebugLog(void);

        //virtual IspDrvVir* getCQInstance(ISP_HW_MODULE hwModule, MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx);

    private:
        volatile MINT32         m_UserCnt;
        static MUINT32          m_TotalUserCnt;
        mutable android::Mutex           IspRegMutex;
        //
        MUINT32*        m_pIspVirRegAddr_va;
        MUINT32*        m_pIspVirRegAddr_pa;

        //
        ISP_DRV_CQ_CMD_DESC_STRUCT*         m_pIspDescript_vir;
        MUINT32*                            m_pIspDescript_phy;

        //
        //MUINT32         m_FSM;
        //imem
        static IMemDrv* m_pMemDrv;
        //IMEM_BUF_INFO   m_ispVirRegBufInfo;
        //IMEM_BUF_INFO   m_ispCQDescBufInfo;
        IMEM_BUF_INFO   m_ispAllCqBufInfo;

        E_ISP_DIP_CQ    m_dipCq;
        ISP_HW_MODULE   m_hwModule;
        ISP_DIP_MODULE_IDX  m_dipModuleIdx;
        MUINT32         m_burstQueIdx;
        MUINT32         m_dupCqIdx;
        //MUINT32         m_pageIdx;
        char            m_useName[MAX_USER_NAME_SIZE];

        static IspDrvDip*****  m_PageTbl;

        //static MBOOL    m_bInit[ISP_DIP_MODULE_IDX_MAX];   //flag to avoid re-init DIP obj
        static IspDrv*    m_pIspDrvImp;       //ptr for access DIP_A/DIP_B ...etc. phy
};





#endif //_ISP_DRV_DIP_H_

