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
    this module support only CAMSV phy only.
*/
#ifndef _ISP_DRV_CAMSV_H_
#define _ISP_DRV_CAMSV_H_


#include <utils/Mutex.h>    // android mutex

#include <isp_drv.h>
#include <isp_reg.h>
#include <camera_isp.h>


/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/

/**
    RW marco, for coding convenience

    note1: camsv module only
    note2: caller here don't care, CAMSV_0 ~ CAMSV_5 r all acceptable!
*/


//user no need to write reg address, drv will fill the address automatically
#define CAMSV_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAMSV_0);\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define CAMSV_BURST_WRITE_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAMSV_0);\
    __ret;\
})

#define CAMSV_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,CAMSV_0);\
})

#define CAMSV_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAMSV_0);\
    ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define CAMSV_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    printf("reg_offset:0x%08x [%s]\n", __offset, #RegName);\
    IspDrvPtr->writeReg(__offset, Value,CAMSV_0);\
}while(0);

#define CAMSV_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAMSV_0);\
    ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    printf("reg_offset:0x%08x [%s]\n", __offset, #RegName);\
    IspDrvPtr->writeReg(__offset, ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,CAMSV_0);\
}while(0);

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
/**
    for CamSV control
*/
class IspDrvCamsv : public IspDrvImp
{
    public:
                        ~IspDrvCamsv(void){}
                        IspDrvCamsv(void);

    public:

        static IspDrv*  createInstance(ISP_HW_MODULE hwModule);
        virtual void    destroyInstance(void);

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");
        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=CAMSV_0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=CAMSV_0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=CAMSV_0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=CAMSV_0);
        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        MBOOL           DumpReg(MBOOL bPhy);

    private:
        MBOOL   FSM_CHK(MUINT32 op);
    private:
        volatile MINT32     m_UserCnt;

        mutable android::Mutex  IspRegMutex;
        MUINT32*     m_pIspHwRegAddr;

        MUINT32         m_FSM;

        IspDrvImp*  m_pIspDrvImp;

    public:
        ISP_HW_MODULE m_hwModule;

};


#endif  // _ISP_DRV_CAMSV_H_