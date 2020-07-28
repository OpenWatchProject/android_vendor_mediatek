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
    this module support only CAM + DIP only.
    other module like FDVT CAMSV UNI... etc. need to re-pack its owner module via inheriting this module
*/
#ifndef _ISP_DRV_H_
#define _ISP_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase

#include "utils/Mutex.h"    // android mutex
#include "dip_reg.h"
#include "camera_dip.h"
#include "isp_drv_dip_platform.h"

/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/**
    max user number supported by this module.
*/
#define MAX_USER_NUMBER     128
#define MAX_USER_NAME_SIZE  32

/**
    kernel device name
*/
#define ISP_DRV_DEV_NAME            "/dev/camera-dip"

/**
    Register R/W IO ctrl
*/
typedef enum{
    ISP_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    ISP_DRV_RW_MMAP     = 1,    //RW register via MMAP
    ISP_DRV_R_ONLY      = 2,    //R only
    ISP_DRV_RW_CQ       = 3,    //RW via CQ
}ISP_DRV_RW_MODE;


////////////////////////////////////////////////////////////////////////////////////////////////
/**
    Set/Get device information
*/
typedef enum{
    _SET_DBG_INT            = (1L<<0),      //data size: 2 words, 1st for RAW-path, 2nd for mask
    _SET_DIP_BUF_INFO       = (1L<<4),      //data size:
}E_DEVICE_INFO;



class IspDumpDbgLogDipPackage{
public:
    unsigned int *tpipeTableVa;
    unsigned int tpipeTablePa;
    unsigned int *pTuningQue;
    unsigned int *IspDescriptVa;
    unsigned int *IspVirRegAddrVa;
};


/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
class DipDrv
{
    protected:
        virtual         ~DipDrv(){}
    public:
//DipDrv Interface
        static DipDrv*  createInstance(DIP_HW_MODULE module);
        static DipDrv*  getVirDipDrvInstance(DIP_HW_MODULE module, void* ispVirRegMap);

        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(DIP_WAIT_IRQ_ST* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(DIP_CLEAR_IRQ_ST* pClearIrq) = 0;
        virtual MBOOL   signalIrq(DIP_WAIT_IRQ_ST* pWaitIrq) = 0;

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(MUINT32 DstOffsetAddr,MUINT32 Count,MUINT32* SrcAddr, MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData) = 0;

    public:
        void*                   m_pIspRegMap;    //this mem is for saving data from R/W reg macro, main purpose: calculating reg offset


        //static MINT32       m_Fd;

        //for RW reg marco,coding convenience
        //the life cycle of this memory space is combined with ISP kernel.
        //size of this memory space is 0x1000
        //static void*    m_pIspRegMap;    //this mem is for saving data from R/W reg macro, main purpose: calculating reg offset
                                        //normally, this par. have a clearly type, but because of cam & uni have dependency, so using abstract type.
        //static android::Mutex  RegRWMacro;     //avoid RW racing  when using macro
        //for R/W marco
};

//class for modules those have CmdQ
class VirDipDrv : public DipDrv
{
    protected:
        virtual           ~VirDipDrv(){}
    public:
//VirDipDrv Interface
        virtual MBOOL     cqAddModule(MUINT32 moduleId) = 0;
        virtual MBOOL     cqDelModule(MUINT32 moduleId) = 0;
        virtual MBOOL     getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);


        virtual MBOOL     setCQDescBufPhyAddr(MUINT32* pIspDescript_phy) = 0;       
        virtual MBOOL     setCQDescBufVirAddr(ISP_DRV_CQ_CMD_DESC_STRUCT* pIspDescript_vir) = 0;
        virtual MUINT32*  getCQDescBufPhyAddr(void) = 0;
        virtual MUINT32*  getCQDescBufVirAddr(void) = 0;
        virtual MBOOL     setIspVirRegPhyAddr(MUINT32* pIspVirRegAddr_pa) = 0;
        virtual MBOOL     setIspVirRegVirAddr(MUINT32* pIspVirRegAddr_va) = 0;
        virtual MUINT32*  getIspVirRegPhyAddr(void) = 0;
        virtual MUINT32*  getIspVirRegVirAddr(void) = 0;
        virtual MBOOL     setCQDescBufOft(MUINT32 IspDescript_oft) = 0;
        virtual MBOOL     setIspVirRegOft(MUINT32 IspVirRegAddr_oft) = 0;
        virtual MUINT32   getCQDescBufOft(void) = 0;
        virtual MUINT32   getIspVirRegOft(void) = 0;

//VirDipDrv Debug
        virtual MBOOL     dumpCQTable(void) = 0;
};

class PhyDipDrv : public DipDrv
{
    protected:
        virtual           ~PhyDipDrv(){}
    public:
//PhyDipDrv Interface
        virtual MBOOL   loadInitSetting(void)  = 0;
        virtual MBOOL   setMemInfo(unsigned int meminfocmd, unsigned long tpipeTablePa, unsigned int *tpipeTableVa, unsigned int MemSizeDiff) = 0;
        virtual MUINT32 getRWMode(void) = 0;
        virtual MBOOL   setRWMode(ISP_DRV_RW_MODE rwMode) = 0;
        virtual MUINT32* getMMapRegAddr(void) = 0;

//PhyDipDrv Debug
        virtual dip_x_reg_t* getCurHWRegValues() = 0;
        virtual MBOOL getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr) = 0;
        virtual MBOOL dumpDipHwReg(IspDumpDbgLogDipPackage* pP2Package) = 0;

};


class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }
};


#endif  // _ISP_DRV_H_
