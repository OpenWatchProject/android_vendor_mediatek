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

#include <vector>
#include <string>
#include <iostream>
#include "utils/Mutex.h"    // android mutex
#include "isp_reg.h"
#include "camera_isp.h"
#include "isp_drv_stddef.h"

using namespace std;

/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/

/**
    max user number supported by this module.
*/
#define MAX_USER_NUMBER     128
#define MAX_USER_NAME_SIZE  32

/**
    kernel device name
*/
#define ISP_DRV_DEV_NAME            "/dev/camera-isp"

/**
    type of timestamp, ref to kernel's header
    compile option:GLOBAL_BT is for supported type of timestamp
*/
#ifdef TS_BOOT_T
#define GLOBAL_BT
#else
#undef GLOBAL_BT
#endif

typedef enum{
    _e_mono_    = 0,
#ifdef GLOBAL_BT
    _e_boot_    = 1,
#endif
    _e_TS_max,
}E_TS;


/**
    Register R/W IO ctrl
*/
typedef enum{
    ISP_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    ISP_DRV_RW_MMAP     = 1,    //RW register via MMAP
    ISP_DRV_R_ONLY      = 2,    //R only
    ISP_DRV_RW_CQ       = 3,    //RW via CQ
}ISP_DRV_RW_MODE;

/**
    Reg R/W target HW module.
    note:
        this module support only CAM , CAMSV and DIP only
*/
typedef enum{
    CAM_A   = 0,
    CAM_B,
    CAM_C,
    PHY_CAM,
    CAM_A_TWIN_B = PHY_CAM,    //CAMA use this virtual hw for CAM_B's CQ-operation
    CAM_A_TWIN_C,    //CAMA use this virtual hw for CAM_C's CQ-operation
    CAM_B_TWIN_C,    //CAMB use this virtual hw for CAM_C's CQ-operation
    CAM_A_TRIPLE_B, //CAMA use this virtual hw for CAM_B's CQ-operation with 3 raws to 1 tg
    CAM_A_TRIPLE_C, //CAMA use this virtual hw for CAM_C's CQ-operation with 3 raws to 1 tg
    CAM_MAX,
    CAMSV_START = CAM_MAX,
    CAMSV_0 = CAMSV_START,
    CAMSV_1,
    CAMSV_2,
    CAMSV_3,
    CAMSV_4,
    CAMSV_5,
    CAMSV_MAX,
    MAX_ISP_HW_MODULE = CAMSV_MAX
}ISP_HW_MODULE;
//////////////////////////////////////////////////////////////////////////////////////////////

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}ISP_DRV_REG_IO_STRUCT;

////////////////////////////////////////////////////////////////////////////////////////////////
/**
    Set/Get device information
*/
typedef enum{
    _SET_DBG_INT            = (1L<<0),      //data size: 2 words, 1st for RAW-path, 2nd for mask
    _SET_VF_ON              = (1L<<1),      //start isp: log only
    _SET_VF_OFF             = (1L<<2),      //stop isp: log only
    _SET_WAKE_LOCK          = (1L<<3),      //data size: 1 word, for lock/unlock wake_lock
    _SET_DIP_BUF_INFO          = (1L<<4),      //data size:
    _SET_BUF_CTRL           = (1L<<5),
    _RESET_VSYNC_CNT        = (1L<<6),
    _SET_RESET_HW_MOD       = (1L<<8),
    _SET_LABR_MMU           = (1L<<9),
    _SET_ION_HANDLE         = (1L<<10),
    _SET_ION_FREE           = (1L<<11),
    _SET_ION_FREE_BY_HWMODULE = (1L<<12),
    _SET_CQ_SW_PATCH        = (1L<<13),
    _SET_DFS_UPDATE            = (1L<<14),

    _GET_GLOBAL_TIME       = (1L<<15),     //data size:
    _GET_SOF_CNT            = (1L<<16),     //data size: 2 words, 1st for RAW-path, 2nd for index
    _GET_DMA_ERR            = (1L<<17),     //data size:
    _GET_INT_ERR            = (1L<<18),     //data size: 5 words. 1st for RAW-path, 2 to 5 mapping to corresponding RAW-PATH int_status
    _GET_DROP_FRAME_STATUS  = (1L<<19),     //data size: 2 words. 1st for RAW-path, 2nd for status
    _GET_START_TIME         = (1L<<20),     //data size: 2 words. in:1st word for module. out: 1st word:sec, 2nd word:usec.
    _GET_VSYNC_CNT          = (1L<<21),     //data size: 1 word.  out: current vsync counter
    _GET_SUPPORTED_ISP_CLOCKS = (1L<<22),
    _GET_CUR_ISP_CLOCK = (1L<<23),
    _SET_PM_QOS_INFO        = (1L<<24),
    _SET_PM_QOS_ON          = (1L<<25),
    _SET_PM_QOS_RESET       = (1L<<26),
    _SET_SEC_DAPC_REG       = (1L<<27),
}E_DEVICE_INFO;


//////////////////////////////////////////////////////
/**
    CmdQ Cmd structure
*/
typedef struct cq_desc_t{
    union {
        struct {
            MUINT32 osft_addr_lsb   :16;
            MUINT32 cnt             :10;
            MUINT32 inst            :3;
            MUINT32 osft_addr_msb   :3;
        } token;
        MUINT32 cmd;
    } u;
    MUINT32 v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_STRUCT;

typedef struct {
    unsigned int id;
    unsigned int addr_ofst;
    unsigned int sw_addr_ofst;
    unsigned int reg_num;
}ISP_DRV_CQ_MODULE_INFO_STRUCT;

///////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
//class for modules those have CmdQ
class IspDrvVir
{
    protected:
        virtual        ~IspDrvVir(){}
    public:
        //virtual IspDrvVir* getCQInstance(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx) = 0;

        //virtual MBOOL   setVirBuffer(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx) = 0;

        virtual MBOOL   cqAddModule(MUINT32 moduleId) = 0;
        virtual MBOOL   cqDelModule(MUINT32 moduleId) = 0;
        virtual MBOOL   cqNopModule(MUINT32 moduleId) = 0;
        virtual MBOOL   cqApbModule(MUINT32 moduleId) = 0;

        virtual MBOOL   dumpCQTable(void) = 0;


        virtual MUINT32*    getCQDescBufPhyAddr(void) = 0;
        //virtual MUINT32*    getCQDescBufVirAddr(void) = 0;
        //virtual MUINT32*    getCQVirBufVirAddr(void) = 0;
        virtual MBOOL   getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize) = 0;

        //virtual MBOOL   setCQTriggerMode(ISP_DRV_CQ_ENUM cq,ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src) = 0;

        virtual MBOOL   flushCmdQ(void) = 0;
};

class IspDrv
{
    protected:
        virtual         ~IspDrv(){}
    public:
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq) = 0;
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq) = 0;
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq) = 0;

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData) = 0;
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData) = 0;

    public:
        static MINT32       m_Fd;

        //for RW reg marco,coding convenience
        //the life cycle of this memory space is combined with ISP kernel.
        //size of this memory space is 0x1000
        static void*    m_pIspRegMap;    //this mem is for saving data from R/W reg macro, main purpose: calculating reg offset
                                        //normally, this par. have a clearly type, but because of cam & uni have dependency, so using abstract type.
        static android::Mutex  RegRWMacro;     //avoid RW racing  when using macro
        //for R/W marco
        //cannon ep temp p2
        #define MAX_MODULE_SIZE()   ({\
        MUINT32 __max = REG_SIZE;\
            __max = (CAM_BASE_RANGE_SPECIAL > __max)?(CAM_BASE_RANGE_SPECIAL):(__max);\
            __max = (UNI_BASE_RANGE > __max)?(UNI_BASE_RANGE):(__max);\
            __max = (CAMSV_BASE_RANGE > __max)?(CAMSV_BASE_RANGE):(__max);\
            __max = (FDVT_BASE_RANGE > __max)?(FDVT_BASE_RANGE):(__max);\
            __max = (WPE_BASE_RANGE > __max)?(WPE_BASE_RANGE):(__max);\
            __max = (RSC_BASE_RANGE > __max)?(RSC_BASE_RANGE):(__max);\
            __max = (DPE_BASE_RANGE > __max)?(DPE_BASE_RANGE):(__max);\
            __max;})
};

/**
    default accessible module: CAM/DIP only
*/
class IspDrvImp : public IspDrv
{
    public:
                        ~IspDrvImp(void){}
                        IspDrvImp(void);

    public:

        static IspDrv*  createInstance(ISP_HW_MODULE module);
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq){(void)pWaitIrq;return MFALSE;}
        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        //virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData){(void)eCmd;(void)pData;return MFALSE;}
        //virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData){(void)eCmd;(void)pData;return MFALSE;}
        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData); // temp for hwsync use
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);// temp for hwsync use

        virtual MUINT32 getRWMode(void);

                  MBOOL setDapcReg(vector<MUINT32> reg_list);

                  MBOOL setRWMode(ISP_DRV_RW_MODE rwMode);    //for EP test code only, will be removed when seninf drv ready.
    private:

        static volatile MINT32     m_UserCnt;
        static ISP_DRV_RW_MODE     m_regRWMode;

        MUINT32*     m_pIspHwRegAddr;
        static android::Mutex   m_IspInitMutex;
        mutable android::Mutex  IspRegMutex;
        ISP_HW_MODULE m_HWmodule;

        //for dbg convenience,dump current user when uninit()
        static vector<string> m_User;

        //for secure cam
        static vector<MUINT32> m_DapcReg;
        static MBOOL m_SecOn;
};

#endif  // _ISP_DRV_H_
