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

/**
    this module support only DIP phy only.
*/
#ifndef _ISP_DRV_DIP_PHY_
#define _ISP_DRV_DIP_PHY_


#include <utils/Mutex.h>    // android mutex

#include <isp_reg.h>
#include <camera_isp.h>
#include "isp_drv_dip_com.h"


/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////
/**
    RW marco, for coding convenience

    note: dip module only
*/

#if 0
#define DIP_BURST_READ_PHY_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
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
#define DIP_BURST_WRITE_PHY_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
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

#define DIP_READ_REG_PHY_NOPROTECT(_dip_,IspDrvPtr,RegName,...)    DIP_READ_REG(_dip_,IspDrvPtr,RegName)

#define DIP_READ_PHY_REG(_dip_,IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,(_dip_));\
})

#define DIP_READ_PHY_BITS_NOPROTECT(_dip_,IspDrvPtr,RegName,FieldName,...)    DIP_READ_BITS(_dip_,IspDrvPtr,RegName,FieldName)

#define DIP_READ_PHY_BITS(_dip_,IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define DIP_WRITE_PHY_REG(_dip_,IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    MUINT32 _moduleIdx_;\
    if(_dip_<DIP_MAX && hwModule>=DIP_A){\
        _moduleIdx_ = hwModule - DIP_START;\
    } else {\
        LOG_ERR("[Error]hwModule(%d) out of the range",hwModule);\
        return NULL;\
    }\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(gDipDrvObj[_moduleIdx_].m_pIspDrvImp->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(gDipDrvObj[_moduleIdx_].m_pIspDrvImp->m_pIspRegMap);\
    gDipDrvObj[_moduleIdx_].m_pIspDrvImp->writeReg(__offset, Value,(_dip_));\
}while(0);


#define DIP_WRITE_PHY_BITS(_dip_,IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,(_dip_));\
}while(0);

#else
//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_READ_PHY_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
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
#define DIP_BURST_WRITE_PHY_REGS(_dip_,IspDrvPtr,RegName,RegStruct,RegCount,...)({\
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

#define DIP_READ_REG_PHY_NOPROTECT(_dip_,IspDrvPtr,RegName,...)    DIP_READ_REG(_dip_,IspDrvPtr,RegName)

#define DIP_READ_PHY_REG(_dip_,IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,(_dip_));\
})

#define DIP_READ_PHY_BITS_NOPROTECT(_dip_,IspDrvPtr,RegName,FieldName,...)    DIP_READ_BITS(_dip_,IspDrvPtr,RegName,FieldName)

#define DIP_READ_PHY_BITS(_dip_,IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define DIP_WRITE_PHY_REG(_dip_,IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value,(_dip_));\
}while(0);


#define DIP_WRITE_PHY_BITS(_dip_,IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,(_dip_));\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,(_dip_));\
}while(0);
#endif

#define GET_MAX_CQ_DESCRIPTOR_SIZE()({\
    MUINT32 __size = 0;\
    __size = ((sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*DIP_A_MODULE_MAX));\
    __size;\
})

#define ISP_DIP_CQ_DUMMY_BUFFER 0x1000


// for p2 tpipe dump information
#define DUMP_TPIPE_SIZE         100
#define DUMP_TPIPE_NUM_PER_LINE 10

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
/**
    for Dip control
*/
class IspDrvDipPhy : public IspDrvImp
{
    public:
                        ~IspDrvDipPhy(void){}
                        IspDrvDipPhy(void);

    public:

        static IspDrv*  createInstance(ISP_HW_MODULE hwModule);
        virtual void    destroyInstance(void);

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");
        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=DIP_A);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=DIP_A);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=DIP_A);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=DIP_A);
        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setMemInfo(unsigned long tpipeTablePa, unsigned int *tpipeTableVa, unsigned int MemSizeDiff);
    public:   // not inherit from IspDrv
        virtual MBOOL loadInitSetting(void);
        virtual dip_x_reg_t* getCurHWRegValues();
        virtual MBOOL dumpDipHwReg(IspDumpDbgLogDipPackage* pP2Package);

    private:
        volatile MINT32     m_UserCnt;

        mutable android::Mutex  IspRegMutex;
        //MUINT32*     m_pIspHwRegAddr;

        IspDrvImp*  m_pIspDrvImp;

        ISP_HW_MODULE m_hwModule;

        dip_x_reg_t*      mpTempIspDipHWRegValues;

};


#endif  // _ISP_DRV_DIP_PHY_
