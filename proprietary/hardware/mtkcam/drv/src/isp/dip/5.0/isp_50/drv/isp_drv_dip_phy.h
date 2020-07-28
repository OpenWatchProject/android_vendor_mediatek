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

#include <dip_reg.h>
#include <camera_dip.h>
#include "isp_drv.h"
#include "isp_drv_dip_platform.h"


/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////
/**
    RW marco, for coding convenience

    note: dip module only
*/

//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_READ_PHY_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
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

//user no need to write reg address, drv will fill the address automatically
#define DIP_BURST_WRITE_PHY_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount);\
    __ret;\
})

#define DIP_READ_REG_PHY_NOPROTECT(IspDrvPtr,RegName,...)    DIP_READ_REG(IspDrvPtr,RegName)

#define DIP_READ_PHY_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset);\
})

#define DIP_READ_PHY_BITS_NOPROTECT(IspDrvPtr,RegName,FieldName,...)    DIP_READ_BITS(IspDrvPtr,RegName,FieldName)

#define DIP_READ_PHY_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define DIP_WRITE_PHY_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value);\
}while(0);


#define DIP_WRITE_PHY_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset);\
    ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((dip_x_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw);\
}while(0);


class PhyDipDrvImp : public PhyDipDrv
{
    public:
                         ~PhyDipDrvImp(void){}
                         PhyDipDrvImp(void);

    public:
//DipDrv Interface
        static DipDrv*   createInstance(DIP_HW_MODULE module);

        virtual void     destroyInstance(void){};

        virtual MBOOL    init(const char* userName="");
        virtual MBOOL    uninit(const char* userName="");

        virtual MBOOL    waitIrq(DIP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL    clearIrq(DIP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL    signalIrq(DIP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL    readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32  readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL    writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL    writeRegs(MUINT32 DstOffsetAddr,MUINT32 Count,MUINT32* SrcAddr, MINT32 caller=0);
        virtual MBOOL    writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL    setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);// temp for hwsync use
//PhyDipDrv Interface
        virtual MBOOL    loadInitSetting(void);
        virtual MBOOL    setMemInfo(unsigned int meminfocmd, unsigned long tpipeTablePa, unsigned int *tpipeTableVa, unsigned int MemSizeDiff);
        virtual MUINT32  getRWMode(void);
        virtual MBOOL    setRWMode(ISP_DRV_RW_MODE rwMode);    //for EP test code only, will be removed when seninf drv ready.
        virtual MUINT32* getMMapRegAddr(void) {return (mpIspDipHwRegAddr);}
//PhyDipDrv Debug
        virtual MBOOL getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr);
        virtual MBOOL    dumpDipHwReg(IspDumpDbgLogDipPackage* pP2Package);
        virtual dip_x_reg_t* getCurHWRegValues();

    private:

        MINT32                  m_Fd;

        volatile MINT32         m_UserCnt;
        ISP_DRV_RW_MODE         m_regRWMode;
        //for dbg convenience,dump current user when uninit()
        char                    m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.

        android::Mutex          m_IspInitMutex;
        mutable android::Mutex  IspRegMutex;
        DIP_HW_MODULE           m_HWmodule;

        volatile MBOOL          m_IsEverDumpBuffer;

        MUINT32*                mpIspDipHwRegAddr;//register map from mmap 
        dip_x_reg_t*            mpTempIspDipHWRegValues;

};


#endif  // _ISP_DRV_DIP_PHY_
