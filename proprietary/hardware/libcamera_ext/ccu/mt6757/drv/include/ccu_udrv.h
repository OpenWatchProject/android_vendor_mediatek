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
    this module support only CCU + DIP only.
    other module like FDVT CCUSV UNI... etc. need to re-pack its owner module via inheriting this module
*/
#ifndef _CCU_DRV_H_
#define _CCU_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase



/**
    in order to avoid translation fault when NE
*/
#define TF_PROT 0


#include "utils/Mutex.h"    // android mutex
#include "ccu_reg.h"
#include "ccu_drv.h"
#include "ccu_ext_interface/ccu_ext_interface.h"

#if TF_PROT
#include "imem_drv.h"
#endif

//For AEE function
#include <dlfcn.h>
#include "aee.h"
#include "ccu_aee_mgr.h"

/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/
///////////////////////////////////////////////////////////////////////////
#define AEE_ASSERT_CCU_USER(String, ...)                            \
          do {                                                      \
            if(CcuAeeMgrDl::IsCcuAeeSysExpInitDone())               \
            {                                                       \
                CcuAeeMgrDl::AeeSystemException(                    \
                  "CCU",                                            \
                  NULL,                                             \
                  DB_OPT_DEFAULT,                                   \
                  String, ##__VA_ARGS__);                           \
            }                                                       \
            else                                                    \
            {                                                       \
                LOG_ERR("CcuAeeMgrDl not initialed correctly\n");   \
            }                                                       \
          } while(0)
///////////////////////////////////////////////////////////////////////////


#define CCU_A_BASE_HW		        m_platformInfo.ccu_hw_base
#define CCU_A_BASE_HW_ALIGN_OFFST   m_platformInfo.ccu_hw_offset
#define CCU_CAMSYS_BASE_HW	        m_platformInfo.ccu_camsys_base

#define CCU_PMEM_BASE_HW	m_platformInfo.ccu_pmem_base
#define CCU_PMEM_RANGE		m_platformInfo.ccu_pmem_size
#define CCU_PMEM_SIZE		m_platformInfo.ccu_pmem_size

#define CCU_DMEM_BASE_HW	m_platformInfo.ccu_dmem_base
#define CCU_DMEM_RANGE		m_platformInfo.ccu_dmem_size
#define CCU_DMEM_SIZE		m_platformInfo.ccu_dmem_size
#define CCU_DMEM_OFFSET     m_platformInfo.ccu_dmem_offset

#define SENSOR_PM_SIZE      m_platformInfo.ccu_sensor_pm_size
#define SENSOR_DM_SIZE      m_platformInfo.ccu_sensor_dm_size
#define SENSOR_PM_OFFSET    (CCU_PMEM_SIZE - SENSOR_PM_SIZE)
#define SENSOR_DM_OFFSET    (CCU_DMEM_SIZE - SENSOR_DM_SIZE)

#define OFFSET_CCU_A_DMA_DEBUG_SEL  -0x2B4/4
#define OFFSET_CCU_A_DMA_DEBUG      0x028/4
#define CCU_A_DUMP_RANGE            m_platformInfo.ccu_hw_dump_size

#define OFFSET_CCU_INTERNAL_LOG             m_platformInfo.ccu_log_base
#define OFFSET_CCU_INTERNAL_LOG_CHUNK_SZ    m_platformInfo.ccu_log_size
///////////////////////////////////////////////////////////////////////////
/**
    max user number supported by this module.
*/
#define MAX_USER_NUMBER     128
#define MAX_USER_NAME_SIZE  32

/**
    kernel device name
*/
#define CCU_DRV_DEV_NAME            "/dev/ccu"

/**
    Register R/W IO ctrl
*/
typedef enum{
    CCU_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    CCU_DRV_RW_MMAP     = 1,    //RW register via MMAP
    CCU_DRV_R_ONLY      = 2,    //R only
    CCU_DRV_RW_CQ       = 3,    //RW via CQ
}CCU_DRV_RW_MODE;

/**
    Reg R/W target HW module.
    note:
        this module support only CCU , CCUSV only
*/
typedef enum{
    CCU_A   = 0,
    CCU_CAMSYS,
    CCU_PMEM,
    CCU_DMEM,
    CCU_MAX,
    MAX_CCU_HW_MODULE = CCU_MAX
}CCU_HW_MODULE;

/**
    CCU interrupt state.
    note:
        this module support only CCU , CCUSV only
*/
typedef enum{
    CCU_INT_LOG   = 0,
    CCU_INT_ASSERT,
}CCU_INT_STATE;

//////////////////////////////////////////////////////////////////////////////////////////////

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}CCU_DRV_REG_IO_STRUCT;


class CcuMemImp;

///////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
class CcuDrv
{
    protected:
        virtual         ~CcuDrv(){}
    public:
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="", WHICH_SENSOR_T sensorDev=WHICH_SENSOR_NONE) = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;
        virtual MBOOL   shutdown() = 0;

        virtual MBOOL   waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(CCU_CLEAR_IRQ_ST* pClearIrq) = 0;
        virtual MBOOL   registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq) = 0;
        virtual MBOOL   signalIrq(CCU_WAIT_IRQ_ST* pWaitIrq) = 0;

        virtual MUINT32 readInfoReg(MUINT32 regNo) = 0;
/*
        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData) = 0;
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData) = 0;
*/
    public:
        static MINT32       m_Fd;

        //for RW reg marco,coding convenience
        //the life cycle of this memory space is combined with CCU kernel.
        //size of this memory space is 0x1000
        static void*    m_pCcuRegMap;    //this mem is for saving data from R/W reg macro, main purpose: calculating reg offset
                                        //normally, this par. have a clearly type, but because of cam & uni have dependency, so using abstract type.
        static android::Mutex  RegRWMacro;     //avoid RW racing  when using macro
        //for R/W marco
        #define MAX_MODULE_SIZE()   ({\
        MUINT32 __max = REG_SIZE;\
            __max;})
};

/**
    default accessible module: CCU only
*/
class CcuDrvImp : public CcuDrv
{
    public:
                        ~CcuDrvImp(void){}
                        CcuDrvImp(void);

    public:

        static CcuDrv*  createInstance(CCU_HW_MODULE module);
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="", WHICH_SENSOR_T sensorDev=WHICH_SENSOR_NONE);
        virtual MBOOL   uninit(const char* userName="");
        virtual MBOOL   shutdown();

        virtual MBOOL   waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(CCU_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(CCU_WAIT_IRQ_ST* pWaitIrq){(void)pWaitIrq;return MFALSE;}

        virtual MBOOL   sendCmd( ccu_cmd_st *pCMD_T, ccu_cmd_st *pCMD_R, MBOOL bSync );
        virtual MBOOL   waitCmd( ccu_cmd_st *pCMD );
        virtual MBOOL   loadCCUBin(WHICH_SENSOR_T sensorDev);
        virtual MBOOL   setI2CChannel(CCU_I2C_CHANNEL channel);
        virtual MBOOL   getI2CDmaBufferAddr(void *va);
        virtual MBOOL   setI2CMode(unsigned int i2c_write_id, unsigned int transfer_len);
        virtual MBOOL   getCurrentFps(int32_t *current_fps);
        virtual MBOOL   getSensorI2cSlaveAddr(int32_t *sensorI2cSlaveAddr);
        virtual MBOOL   checkSensorSupport(WHICH_SENSOR_T sensorDev);

        virtual MUINT32 readInfoReg(MUINT32 regNo);
/*
        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData); // temp for hwsync use
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);// temp for hwsync use
        virtual MUINT32 getRWMode(void);
                MBOOL setRWMode(CCU_DRV_RW_MODE rwMode);    //for EP test code only, will be removed when seninf drv ready.
*/
		void createThread();
		void destroyThread();

		static void* ApTaskWorkerThreadLoop(void *arg);

    private:
        virtual MBOOL   allocateBuffers();
        virtual void DumpCcuSram();
        virtual void PrintReg();
        virtual void DumpSramLog();
        virtual MBOOL OpenCcuKdrv();
        virtual MBOOL CloseCcuKdrv();
        virtual MBOOL GetSensorName(MINT32 kdrvFd, WHICH_SENSOR_T sensorDev, char *sensorName);
        virtual MBOOL RealUninit(const char* userName="ccuDrvinternal");

        static volatile MINT32     m_UserCnt;
        static CCU_DRV_RW_MODE     m_regRWMode;

        MUINT32*     m_pCcuHwRegAddr;
        static android::Mutex   m_CcuInitMutex;
        mutable android::Mutex  CcuRegMutex;
        CCU_HW_MODULE m_HWmodule;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.

	    pthread_t       mThread;
	    sem_t           mSem;

		ccu_power_t m_power;
        ion_user_handle_t m_logBufIonHandle[MAX_LOG_BUF_NUM];

        volatile MBOOL       m_taskWorkerThreadReady;
        volatile MBOOL       m_taskWorkerThreadEnd;

        struct ccu_platform_info m_platformInfo;
        CcuMemImp   *pCcuMem;

};

/**
    default accessible module: CCU only
*/
class CcuMemImp
{
    public:
        ~CcuMemImp(void){}
        CcuMemImp(void);
    public:
        static CcuMemImp*  createInstance();
        void    destroyInstance(void){};

        MBOOL   init(const char* userName="");
        MBOOL   uninit(const char* userName="");

        //memory
        MBOOL ccu_malloc(int drv_h, int len, int *buf_share_fd, char **buf_va);
        MBOOL ccu_free(int drv_h, int len,int buf_share_fd, char *buf_va);
        MBOOL mmapMVA( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva );
        MBOOL munmapMVA( ion_user_handle_t ion_handle );
    public:
        int                     gIonDevFD;
    private:
        //MTKM4UDrv               m4u_client;

};


#endif  // _CCU_DRV_H_
