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
    this module support only UNI phy only.
*/
#ifndef _UNI_DRV_H_
#define _UNI_DRV_H_


#include "utils/Mutex.h"    // android mutex
#include <isp_reg.h>
#include "camera_isp.h"
#include <isp_drv.h>



/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////

/**
    Reg R/W target HW module.
    note:
        this module support only CAM , and DIP only
*/
typedef enum{
    UNI_A = MAX_ISP_HW_MODULE,      //start idx is't starting from 0 because of the dependency of UNI & CAM at CQ. idx will be starting at 0 when the hw dependency is removed
    MAX_UNI_HW_MODULE
}UNI_HW_MODULE;
//////////////////////////////////////////////////////////////////////////////////////////////

//user no need to write reg address, drv will fill the address automatically
#define UNI_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,UNI_A);\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define UNI_BURST_WRITE_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,UNI_A);\
    __ret;\
})


#define UNI_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,UNI_A);\
})


#define UNI_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,UNI_A);\
    ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define UNI_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value,UNI_A);\
}while(0);

#define UNI_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,UNI_A);\
    ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((cam_uni_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,UNI_A);\
}while(0);


/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
/**
    default accessible module: uni
*/
class UniDrvImp : public IspDrv
{
    public:
                        ~UniDrvImp(void){}
                        UniDrvImp(void);

    public:

        static IspDrv*  createInstance(UNI_HW_MODULE module);
        virtual void    destroyInstance(void);

        virtual MBOOL   init(const char* userName,IspDrvImp* pConnectedObj);
        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq){(void)pWaitIrq; return MFALSE;}
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq){(void)pClearIrq;return MFALSE;}
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq){(void)pRegIrq;return MFALSE;}
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq){(void)pWaitIrq;return MFALSE;}

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData){(void)eCmd;(void)pData;return MFALSE;}
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData){(void)eCmd;(void)pData;return MFALSE;}

        UNI_HW_MODULE   getCurObjInfo(void);

        MBOOL           DumpReg(void);
    private:
        volatile MINT32     m_UserCnt;

        mutable android::Mutex  IspRegMutex;
        MUINT32*     m_pIspHwRegAddr;

        IspDrvImp*  pIspdrvimp;

        UNI_HW_MODULE m_HWmodule;

};

#endif  // _ISP_DRV_H_
