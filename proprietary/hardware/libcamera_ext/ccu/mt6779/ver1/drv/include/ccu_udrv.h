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
#include "ccu_platform_def.h"
#include "ccu_udrv_ipc.h"
#include "ccu_ext_interface/ccu_ext_interface.h"

#include <semaphore.h>

//For ION functions
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

//For AEE function
#include "ccu_aee_mgr.h"
#include "kd_ccu_i2c.h"


//enumerates HW reg/mem that will map to user-space in CCU driver
typedef enum{
    CCU_A   = 0,
    CCU_CAMSYS,
    CCU_PMEM,
    CCU_DMEM,
    CCU_MAX,
    MAX_CCU_HW_MODULE = CCU_MAX
}CCU_HW_MODULE;

//indicates to which mem slot senosr binary should be loaded
typedef enum{
    BIN_CCU     = 0,
    BIN_SENSOR_BIN_1,
    BIN_SENSOR_BIN_2,
    BIN_LENS_BIN_1,
    BIN_LENS_BIN_2,
    BIN_CUSTSENSOR_BIN_1,
    BIN_CUSTSENSOR_BIN_2,
    BIN_CUSTFUNC_BIN
}CCU_BIN_SLOT;

struct CcuDrvInitData
{
    uint32_t log_buf_mva_0;
    uint32_t log_buf_mva_1;
    int8_t *log_buf_va_0;
    int8_t *log_buf_va_1;
    uint32_t ddr_buf_mva;
    int8_t *ddr_buf_va;
    uint32_t cpu_ref_buf_mva;
    uint32_t bkdata_buf_mva;
};

class CcuDrvImp
{
public:
    ~CcuDrvImp(void){}
    CcuDrvImp(void);

    static void* ApTaskWorkerThreadLoop(void *arg);
    static CcuDrvImp*  createInstance();
    void    destroyInstance(void){};

    //system life-cycle control functions
    MBOOL init(struct CcuDrvInitData initData);
    MBOOL boot();
    MBOOL uninit();
    MBOOL shutdown();
    MBOOL powerOff();
    
    //binary loading functions
    //enum CCU_I2C_CHANNEL whichSesnorToI2cChannel(WHICH_SENSOR_T whichSensor);
    MBOOL loadCcuBin();
    MBOOL loadCcuDdrBin();

    MBOOL allocSensorSlot(uint32_t sensor_idx, uint8_t *sensor_slot);
    void FreeSensorSlot(uint32_t sensor_idx);
    MBOOL loadSensorBin(uint32_t sensor_idx, uint8_t sensor_slot, bool *isSpSensor);
    MBOOL loadLensBin(uint32_t sensor_idx, uint8_t lens_slot, char* lensName);
    MBOOL loadCustSensorBin(uint32_t sensor_idx, uint8_t sensor_slot);
    MBOOL loadCustFuncBin();
    
    //command related functions
    MBOOL sendCmdIpc(struct ccu_msg *msg);
    MBOOL getIpcIOBuf(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu);
    
    //memory/buffer manipulation
    struct shared_buf_map *getSharedBufMap();
    void *ccuAddrToVa(MUINT32 ccuAddr);
    MBOOL importCcuMemToKernel(import_mem_t mempool);

    //utility
    void RequestCamFreqUpdate(uint32_t freqLevel);
    void PrintReg();
    void PrintDdrBuffer();
    void CheckPm();
    void DumpSramLog();
    void DumpDramLog();
    void PrintCcuLogByIdx(uint32_t logBufIdx, MBOOL mustPrint=MFALSE);
    MUINT32 readInfoReg(MUINT32 regNo);
    void TriggerAee(char *msg);

    //for feature: AE
    MBOOL initI2cController(uint32_t i2c_id);
    MBOOL getI2CDmaBufferAddr(struct ccu_i2c_buf_mva_ioarg *ioarg);
    MBOOL getCurrentFps(int32_t *current_fps);
    MBOOL getSensorI2cSlaveAddr(int32_t *sensorI2cSlaveAddr);

    //for feature: AF
    int dequeueAFO(MUINT32 m_sensorIdx);
    int dequeueAFO_Imp(void *arg, MUINT32 m_sensorIdx);

private:
    MINT32 m_Fd;
    MUINT32* m_pHwRegAddr[MAX_CCU_HW_MODULE];
    MINT8 *m_pLogBuf[MAX_LOG_BUF_NUM];
    MUINT32 m_CcuLogBaseOffset;
    MUINT32 m_CcuBkDataMva;
    MUINT32 m_CpuRefBufMva;
    MUINT32 m_CcuSensorSlot[CCU_SENSOR_BIN_CAPACITY];
    CcuDrvIpc *m_ccuDrvIpc;
    android::Mutex m_CcuSendCmdMutex;
    android::Mutex m_CcuInitMutex;
    android::Mutex m_CcuSensorMutex;
    ccu_power_t m_power;
    struct shared_buf_map *m_shared_buf_map_ptr;
    uint32_t m_ddr_buf_mva;
    MINT8 *m_ddr_buf_va;

    pthread_t m_CcuWorkerThread;
    volatile bool isPowerOn;
    volatile MBOOL m_taskWorkerThreadReady;
    volatile MBOOL m_taskWorkerThreadEnd;

    MBOOL _setCcuPower(int powerCtrlType);
    MBOOL _setCcuRun();
    MBOOL _openCcuKdrv();
    MBOOL _closeCcuKdrv();
    MBOOL _realUninit();
    bool _mapHwRegs();
    MBOOL _unMapHwRegs();
    MBOOL _onlypowerOff();
    MBOOL _waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq);
    void _parseAndPrintCcuLog(const char *logStr, const MUINT32 *logTail, MBOOL mustPrint=MFALSE);
    bool _parsenextloginfo(MUINT32 **ccuLOGBasePtr, unsigned char &log_para_num, unsigned char &log_str_len, bool print_log_all, const MUINT32 *logTail);
    void _createCcuWorkerThread();
    void _destroyCcuWorkerThread();
    bool _ccuSwHwReset();
    void _createCcuDrvIpc();
    void _destroyCcuDrvIpc();

    //for feature: AE
    MBOOL _getSensorName(MINT32 kdrvFd, uint32_t sensor_idx, char *sensorName);

    //for feature: AF
    MBOOL _waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq, MUINT32 m_sensorIdx);
};

#endif  //_CCU_DRV_H_