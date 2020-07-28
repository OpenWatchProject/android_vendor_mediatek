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

#include "utils/Mutex.h"    // android mutex
#include "ccu_drv.h"
#include "ccu_udrv_ipc.h"
#include "ccu_ext_interface/ccu_ext_interface.h"

#include <semaphore.h>

//For ION functions
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

//For AEE function
#include <dlfcn.h>
#include "aee.h"
#include "ccu_aee_mgr.h"
#include "kd_ccu_i2c.h"
#include "kd_camera_feature.h"/*for IMGSENSOR_SENSOR_IDX*/
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

/**
    max user number supported by this module.
*/
#define MAX_USER_NUMBER     128
#define MAX_USER_NAME_SIZE  32

/**
    kernel device name
*/
#define CCU_DRV_DEV_NAME            "/dev/ccu"

#define OFFSET_CCU_A_INFO20      0xD0//ccu log enable statue

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
    //CCU_CACHE,
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

/**
    Load CCU/Sensor driver binary
*/
typedef enum{
    BIN_CCU     = 0,
    BIN_SENSOR_BIN_1,
    BIN_SENSOR_BIN_2
}CCU_BIN_SLOT;


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

struct CcuUdrvBuffer
{
    char *va;
    uint32_t mva;
    uint32_t size;
    int32_t fd;
    ion_user_handle_t ionHandle;
};

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
        virtual MBOOL   init(const char* userName="", 
            uint32_t log_buf_mva_0=0, uint32_t log_buf_mva_1=0,
            MINT8 *log_buf_va_0=0, MINT8 *log_buf_va_1=0,
            uint32_t ddr_buf_mva=0, MINT8 *ddr_buf_va=0) = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;
        virtual MBOOL   shutdown() = 0;

        virtual MBOOL   waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(CCU_CLEAR_IRQ_ST* pClearIrq) = 0;

        virtual MBOOL   waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq, MUINT32 user_tg) = 0;
        virtual MBOOL   clearAFIrq(CCU_CLEAR_IRQ_ST* pClearIrq) = 0;

        virtual MBOOL   registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq) = 0;
        virtual MBOOL   signalIrq(CCU_WAIT_IRQ_ST* pWaitIrq) = 0;

        virtual MUINT32 readInfoReg(MUINT32 regNo) = 0;

    public:
        static MINT32       m_Fd;
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

        virtual MBOOL   init(const char* userName="", 
            uint32_t log_buf_mva_0=0, uint32_t log_buf_mva_1=0,
            MINT8 *log_buf_va_0=0, MINT8 *log_buf_va_1=0,
            uint32_t ddr_buf_mva=0, MINT8 *ddr_buf_va=0);
        virtual MBOOL   uninit(const char* userName="");
        virtual MBOOL   shutdown();
        virtual MBOOL   powerOff();

        virtual MBOOL   waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(CCU_CLEAR_IRQ_ST* pClearIrq);

        virtual MBOOL   waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq, MUINT32 user_tg);
        virtual MBOOL   clearAFIrq(CCU_CLEAR_IRQ_ST* pClearIrq);

        virtual MBOOL   registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(CCU_WAIT_IRQ_ST* pWaitIrq){(void)pWaitIrq;return MFALSE;}

        virtual MBOOL   sendCmdIpc(struct ccu_msg *msg);
        virtual MBOOL   getIpcIOBuf(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu);
        virtual MBOOL   waitCmd( ccu_cmd_st *pCMD );
        virtual MBOOL   loadCCUBin();
        virtual MBOOL   loadSensorBin(uint32_t sensor_idx, uint8_t sensor_slot, bool *isSpSensor);
        virtual MBOOL   initI2cController(uint32_t i2c_id);
        virtual MBOOL   getI2CDmaBufferAddr(struct ccu_i2c_buf_mva_ioarg *ioarg);
        virtual MBOOL   getCurrentFps(int32_t *current_fps);
        virtual MBOOL   getSensorI2CInfo(struct ccu_i2c_info *sensorI2CInfo);
        virtual MBOOL   checkSensorSupport(uint32_t sensor_idx);
        virtual struct shared_buf_map *getSharedBufMap();
        virtual void *ccuAddrToVa(MUINT32 ccuAddr);

        virtual MUINT32 readInfoReg(MUINT32 regNo);

        void createThread();
        void destroyThread();

        int dequeueAFO_A();
        int dequeueAFO_B();

		static void* ApTaskWorkerThreadLoop(void *arg);

        static void* AFApTaskWorkerThreadLoop(void *arg);
        static void* AFApTaskWorkerThread(void *arg);
        static void* AFBApTaskWorkerThread(void *arg);

        virtual void    PrintReg();
        virtual void    DumpSramLog();
        virtual void    DumpDramLog();
        virtual void    TriggerAee(char *msg);

        MBOOL   importmem(import_mem_t mempool);
    private:
        virtual MBOOL   setCcuPower(int powerCtrlType);
        virtual MBOOL   setCcuRun();
        virtual MBOOL   OpenCcuKdrv();
        virtual MBOOL   CloseCcuKdrv();
        virtual MBOOL   GetSensorName(MINT32 kdrvFd, uint32_t sensor_idx, char *sensorName);
        virtual MBOOL   RealUninit(const char* userName="ccuDrvinternal");
        virtual MBOOL   UnMapHwRegs();
        virtual MBOOL   OnlypowerOff();

        static volatile MINT32     m_UserCnt;
        static CCU_DRV_RW_MODE     m_regRWMode;

        MUINT32*    m_pCcuHwRegAddr;
        MUINT32     m_CcuLogBaseOffset;
        CcuDrvIpc *m_ccuDrvIpc;
        struct shared_buf_map *m_shared_buf_map_ptr;
        android::Mutex   m_CcuSendCmdMutex;
        static android::Mutex   m_CcuInitMutex;
        mutable android::Mutex  CcuRegMutex;
        CCU_HW_MODULE m_HWmodule;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.

	    pthread_t       mThread;
	    sem_t           mSem;

        pthread_t       AFmThread;
        sem_t           AFmSem;

        pthread_t       AFBmThread;
        sem_t           AFBmSem;

		ccu_power_t m_power;
        uint32_t m_ddr_buf_mva;
        MINT8 *m_ddr_buf_va;
        ion_user_handle_t m_logBufIonHandle[MAX_LOG_BUF_NUM];

        volatile MBOOL       m_taskWorkerThreadReady;
        volatile MBOOL       m_taskWorkerThreadEnd;

        volatile MBOOL       AFm_taskWorkerThreadReady;
        volatile MBOOL       AFm_taskWorkerThreadEnd;

        volatile MBOOL       AFBm_taskWorkerThreadReady;
        volatile MBOOL       AFBm_taskWorkerThreadEnd;

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
        MBOOL mmapMVA(int buf_share_fd, ion_user_handle_t *p_ion_handle, unsigned int *mva, MUINT32 upper_bound, MUINT32 lower_bound);
        MBOOL munmapMVA(ion_user_handle_t ion_handle);
    public:
        int                     gIonDevFD;
    private:
        //MTKM4UDrv               m4u_client;

};


#endif  // _CCU_DRV_H_
