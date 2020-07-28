#include "DpDriver.h"
#include "DpLogger.h"
#include "DpTimer.h"
#include "DpProfiler.h"
#include "DpDataType.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#if defined(CMDQ_V3)
#include "cmdq_v3_driver.h"
#elif defined(CMDQ_K414)
#include "cmdq_k414_driver.h"
#else
#include "cmdq_driver.h"
#endif
#include "DpEngineType.h"
#include "DpEngineBase.h"
#include "cmdq_mdp_pmqos.h"
#include "ddp_drv.h"
#include "DpProperty.h"
#include "DpPlatform.h"

#define MDP_PAGE_SIZE 1024

DpDriver* DpDriver::s_pInstance = 0;

// Mutex object
DpMutex   DpDriver::s_instMutex;

DpDriver* DpDriver::getInstance()
{
    if (0 == s_pInstance)
    {
        AutoMutex lock(s_instMutex);
        if (0 == s_pInstance)
        {
            s_pInstance = new DpDriver();
            DPLOGI("DpDriver: create driver instance(0x%08x)\n", s_pInstance);

            atexit(destroyInstance);
        }
    }

    return s_pInstance;
}


void DpDriver::destroyInstance()
{
    AutoMutex lock(s_instMutex);

    delete s_pInstance;
    DPLOGI("DpDriver: driver instance(0x%08x) is deleted\n", s_pInstance);

    s_pInstance = NULL;
}


DpDriver::DpDriver()
    : m_driverID(-1),
      m_enableLog(0),
      m_enableSystrace(0),
      m_enableDumpBuffer(0),
      m_enableCheckDumpReg(0),
      m_enableDumpRegister(0),
      m_enableMMpath(0),
      m_enableCheckMet(0),
      m_enableMet(0),
      m_reduceConfigDisable(0),
      m_pq_support(0),
      m_mdpColor(0),
      m_disableFrameChange(0),
      m_refCntRDMA0(0),
      m_refCntRDMA1(0),
      m_supportGlobalPQ(0),
      m_cmdqDts({})
{
    DPLOGI("DpDriver: create DpDriver instance\n");

    m_driverID = open("/dev/mtk_cmdq", O_RDONLY, 0);
    if (-1 == m_driverID)
    {
        DPLOGE("Open disp driver failed\n!");
        assert(0);
    }
    else
    {
        queryDeviceTreeInfo();
    }

    getProperty("vendor.dp.log.enable", &m_enableLog);
    getProperty("vendor.dp.systrace.enable", &m_enableSystrace);
    getProperty("vendor.dp.dumpbuffer.enable", &m_enableDumpBuffer);
    getProperty("vendor.dp.dumpbuffer.folder", m_dumpBufferFolder);
    getProperty("persist.vendor.dp.dumpreg.check", &m_enableCheckDumpReg);
    getProperty("vendor.dp.dumpreg.enable", &m_enableDumpRegister);
    getProperty("vendor.dp.mmpath.enable", &m_enableMMpath);
    getProperty("persist.vendor.dp.met.check", &m_enableCheckMet);
    getProperty("vendor.dp.met.enable", &m_enableMet);
#ifdef MDP_REDUCE_CONFIG
    getProperty("vendor.dp.reduceconfig.disable", &m_reduceConfigDisable);
#else
    m_reduceConfigDisable = 1;
#endif
    getProperty("ro.vendor.mtk_pq_support", &m_pq_support);
    getProperty("ro.vendor.globalpq.support", &m_supportGlobalPQ);
    getProperty("vendor.dp.frameChange.disable", &m_disableFrameChange);

    int32_t pqColorMode = 0;
    getProperty("ro.vendor.mtk_pq_color_mode", &pqColorMode);
#if defined(MDP_VERSION_6885)     // for  LDVT
    pqColorMode = 3;
#endif

    if (pqColorMode == 2 || pqColorMode == 3)
    {
        m_mdpColor = 1;
    }
    else
    {
        m_mdpColor = 0;
    }

    if (strcmp(m_dumpBufferFolder, "") == 0)
    {
        sprintf(m_dumpBufferFolder, "/sdcard/mdp/");
    }
}


DpDriver::~DpDriver()
{
    DPLOGI("DpDriver: destroy DpDriver instance\n");

    if(-1 != m_driverID)
    {
        close(m_driverID);
        m_driverID = -1;
    }
}


DP_STATUS_ENUM DpDriver::checkHandle()
{
    AutoMutex lock(m_instMutex);

    if (-1 == m_driverID)
    {
        m_driverID = open("/dev/mtk_cmdq", O_RDONLY, 0);
    }

    if (-1 == m_driverID)
    {
        DPLOGE("DpDriver: can't open display driver\n");
        assert(0);

        return DP_STATUS_OPERATION_FAILED;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::getTDSHPGain(DISPLAY_TDSHP_T *pSharpness,
                                      uint32_t        *pCurLevel)
{
    int32_t       status;
    DISP_PQ_PARAM pqparam;
    int           drvID = -1;

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    drvID = open("/proc/mtk_mira", O_RDONLY, 0);
    if (-1 == drvID)
    {
        DPLOGE("DpDriver: can't open display driver\n");
        assert(0);

        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(drvID, DISP_IOCTL_GET_TDSHPINDEX, pSharpness);
    if (0 != status)
    {
        DPLOGE("DpDriver: get sharpness value failed(%d)\n", status);
        close(drvID);
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(drvID, DISP_IOCTL_GET_PQPARAM, &pqparam);
    if (0 != status)
    {
        DPLOGE("DpDriver: get sharpness level failed(%d)\n", status);
        close(drvID);
        return DP_STATUS_OPERATION_FAILED;
    }

    *pCurLevel = pqparam.u4SHPGain;
    close(drvID);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::requireMutex(int32_t *pMutex)
{
    int32_t status;

    DPLOGI("DpDriver: require the mutex with pointer %p\n", pMutex);

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(m_driverID, CMDQ_IOCTL_LOCK_MUTEX, pMutex);
    if (0 != status)
    {
        DPLOGE("DpDriver: require mutex failed(%d)\n", status);
        return DP_STATUS_OPERATION_FAILED;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::releaseMutex(int32_t mutex)
{
    int32_t status;

    DPLOGI("DpDriver: relase the specified mutex: %d\n", mutex);

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(m_driverID, CMDQ_IOCTL_UNLOCK_MUTEX, &mutex);
    if (0 != status)
    {
        DPLOGE("DpDriver: release mutex failed(%d)\n", status);
        return DP_STATUS_OPERATION_FAILED;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::queryEngUsages(EngUsages &engUsages)
{
    int32_t  status;
    cmdqUsageInfoStruct usageInfo;

    DPLOGI("DpDriver: exec CMDQ_IOCTL_QUERY_USAGE\n");

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(m_driverID, CMDQ_IOCTL_QUERY_USAGE, &usageInfo);
    if (0 != status)
    {
        DPLOGI("DpDriver: exec CMDQ_IOCTL_QUERY_USAGE failed(%d)\n", status);
        return DP_STATUS_OPERATION_FAILED;
    }

    memcpy(engUsages, usageInfo.count, sizeof(engUsages));
    return DP_STATUS_RETURN_SUCCESS;
}


void DpDriver::addRefCnt(DpEngineType &sourceEng)
{
    AutoMutex lock(m_instMutex);

    if (tRDMA0 == sourceEng)
    {
        if (m_refCntRDMA0 > m_refCntRDMA1)
        {
            DPLOGI("DpDriver: select source engine RDMA1 due to ref. count\n");
            sourceEng = tRDMA1;
            m_refCntRDMA1++;
        }
        else
        {
            m_refCntRDMA0++;
        }
    }
    else if (tRDMA1 == sourceEng)
    {
        if (m_refCntRDMA1 > m_refCntRDMA0)
        {
            DPLOGI("DpDriver: select source engine RDMA0 due to ref. count\n");
            sourceEng = tRDMA0;
            m_refCntRDMA0++;
        }
        else
        {
            m_refCntRDMA1++;
        }
    }
}


void DpDriver::removeRefCnt(uint64_t pathFlags)
{
    AutoMutex lock(m_instMutex);

    if (pathFlags & (1 << tRDMA0))
    {
        if (m_refCntRDMA0 != 0)
        {
            m_refCntRDMA0--;
        }
    }
#if tRDMA1 != tNone
    else if (pathFlags & (1 << tRDMA1))
    {
        if (m_refCntRDMA1 != 0)
        {
            m_refCntRDMA1--;
        }
    }
#endif
}


DP_STATUS_ENUM DpDriver::submitCommand(DpCommandBlock &block, DpJobID* pRet, uint32_t extRecorderFlag, char**)
{
    DP_TRACE_CALL();
    int32_t           status;
    mdp_submit        submitJob;
    DpTimeValue       begin;
    DpTimeValue       end;
    int32_t           diff;
    DpFrameInfo       frameInfo;
    int32_t           portNum;
    char              *frameInfoToCMDQ;

#if 1
    frameInfo = block.getFrameInfo();
    portNum = block.getRegDstNum();
    frameInfoToCMDQ = block.getFrameInfoToCMDQ();
    setFrameInfo(frameInfo, portNum, frameInfoToCMDQ);

    //*pFrameInfo = block.getFrameInfoToCMDQ();
#endif

    memset(&submitJob, 0, sizeof(submitJob));
#if 0
    param.userDebugStr = (unsigned long)frameInfoToCMDQ;
    param.userDebugStrLen = strlen(frameInfoToCMDQ);
#endif

#if PMQOS_SETTING
    submitJob.prop_addr = (unsigned long)block.getMdpPmqos();
    submitJob.prop_size = sizeof(mdp_pmqos);
#endif

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    if (0 != block.getSecureAddrCount())
    {
        submitJob.secData.isSecure = true;
        submitJob.secData.addrMetadataCount = block.getSecureAddrCount();
        submitJob.secData.addrMetadatas = (unsigned long)block.getSecureAddrMD();
        submitJob.secData.enginesNeedDAPC = block.getEngineFlag() | block.getSecurePortFlag();
        submitJob.secData.enginesNeedPortSecurity = block.getSecurePortFlag();

        DPLOGI("secure meta data engine flag:%#llx\n", block.getSecurePortFlag());

#if defined(CMDQ_V3) || defined(CMDQ_K414)
        if (block.hasIspSecMeta())
        {
            DPLOGI("DpDriver: Set ISP secure meta\n");
            submitJob.secData.ispMeta = block.getSecIspMeta();
        }
#endif
        DPLOGI("DpDriver: Secure meta data Ready !!!!\n");
    }

    submitJob.priority = block.getPriority();

    if (VENC_ENABLE_FLAG == (VENC_ENABLE_FLAG & extRecorderFlag))
    {
        submitJob.engine_flag = (1LL << tVENC);
    }
    else
    {
        submitJob.engine_flag = block.getEngineFlag() & (~(1LL << tVENC));
    }

#ifdef ISP_SMART_TILE_ENABLE // or MDP_VSS_ASYNC_ENABLE ?
    if (STREAM_ISP_VSS == block.getScenario())
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
#endif
#if defined(CMDQ_V3) || defined(CMDQ_K414)
#if !defined(tRDMA1) || tRDMA1 == tNone
    // Workaround conflict starving of secure videos
    if (submitJob.engine_flag & (1LL << tRDMA0))
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
    else
#endif
#ifndef MDP_VERSION_6885
    // On platforms with only one blk2scan RDMA
    if (DP_COLOR_GET_BLOCK_MODE(frameInfo.m_srcFormat))
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
#endif
#endif

    submitJob.metas = (unsigned long)block.getBlockBaseSW();
    submitJob.meta_count = block.getBlockSize() / sizeof(op_meta);

    DPLOGI("DpDriver: submit command block: start %#010llx, size %d (end %#010llx), engine flag %#010x\n",
        submitJob.pVABase, submitJob.blockSize, submitJob.pVABase + submitJob.blockSize, submitJob.engineFlag);
    DPLOGI("DpDriver: submit command block: debugRegDump: %d\n", submitJob.debugRegDump);
    if (submitJob.meta_count * sizeof(op_meta) > 0x40000)  // 256 KB
    {
        DPLOGW("DpDriver: submit command block in large count %u, engine flag %#010x\n",
            submitJob.meta_count, submitJob.engine_flag);
    }

    DP_TIMER_GET_CURRENT_TIME(begin);
    status = ioctl(m_driverID, CMDQ_IOCTL_ASYNC_EXEC, &submitJob);
    if (0 != status)
    {
        DPLOGE("DpDriver: submit command block failed(%d), JobID: %llx %d %s\n", status, submitJob.job_id, errno, strerror(errno));
        return DP_STATUS_OPERATION_FAILED;
    }
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

    *pRet = submitJob.job_id;

    if ((STREAM_BITBLT == block.getScenario()) || (STREAM_GPU_BITBLT == block.getScenario()))
    {
        removeRefCnt(submitJob.engine_flag);
    }

    if (diff > 34)
    {
        DPLOGW("DpDriver::submitCommand takes %dms", diff);
    }
    else
    {
        DPLOGI("DpDriver::submitCommand takes %dms", diff);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpDriver::waitFramedone(DpJobID pFrame, DpReadbackRegs &readBackRegs)
{
    DP_TRACE_CALL();
    int32_t         status;
    mdp_wait        waitJob = {.job_id = pFrame};
    uint32_t        index;
    DpTimeValue     begin;
    DpTimeValue     end;
    int32_t         diff;

    memset(&waitJob, 0, sizeof(waitJob));
    waitJob.job_id = pFrame;

    if (((readBackRegs.m_engineFlag >> tVENC) & 0x1) ||
        ((readBackRegs.m_engineFlag >> tCAMIN) & 0x1) ||
        ((readBackRegs.m_engineFlag >> tTDSHP0) & 0x1)
#ifdef SUPPORT_DRE
        || ((readBackRegs.m_engineFlag >> tAAL0) & 0x1)
#endif // SUPPORT_DRE
#ifdef SUPPORT_HDR
#ifdef HDR_MT6799
        || ((readBackRegs.m_engineFlag >> tRDMA0) & 0x1)
#else
        || ((readBackRegs.m_engineFlag >> tHDR0) & 0x1)
#endif // HDR_MT6799
#endif // SUPPORT_HDR

       )
    {
        waitJob.read_result.count = readBackRegs.m_num;
        waitJob.read_result.ids = (unsigned long)readBackRegs.m_regs;
        waitJob.read_result.ret_values = (unsigned long)readBackRegs.m_values;

        DPLOGI("DpDriver: read count:%d\n", readBackRegs.m_num);
        for (index = 0; index < readBackRegs.m_num; index++)
        {
            DPLOGI("DpDriver: %d, addr:%#010x\n", index, readBackRegs.m_regs[index]);
        }

        if (DP_STATUS_RETURN_SUCCESS != checkHandle())
        {
            DPLOGE("DpDriver: invalid display driver handle\n");
            return DP_STATUS_OPERATION_FAILED;
        }

        DP_TIMER_GET_CURRENT_TIME(begin);
        status = ioctl(m_driverID, CMDQ_IOCTL_ASYNC_WAIT, &waitJob);
        if (0 != status)
        {
            DPLOGE("DpDriver: waitFramedone failed(%d), JobID: %llx\n", status, pFrame);
            return DP_STATUS_OPERATION_FAILED;
        }

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);
        DPLOGI("DpDriver: read count:%d\n", readBackRegs.m_num);
        for (index = 0; index < readBackRegs.m_num; index++)
        {
            DPLOGI("DpDriver: %d, addr:%#010x, value:%#010x\n", index, readBackRegs.m_regs[index], readBackRegs.m_values[index]);
        }
    }
    else // for ISP part
    {
        //waitJob.read_v1_result.ret_values = (unsigned long)readBackRegs.m_values;
        //waitJob.read_v1_result.count = MAX_NUM_READBACK_REGS;

        if (DP_STATUS_RETURN_SUCCESS != checkHandle())
        {
            DPLOGE("DpDriver: invalid display driver handle\n");
            return DP_STATUS_OPERATION_FAILED;
        }

        DP_TIMER_GET_CURRENT_TIME(begin);
        status = ioctl(m_driverID, CMDQ_IOCTL_ASYNC_WAIT, &waitJob);
        if (0 != status)
        {
            DPLOGE("DpDriver: waitFramedone failed(%d), JobID: %llx\n", status, pFrame);
            return DP_STATUS_OPERATION_FAILED;
        }
        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

        //readBackRegs.m_num = waitJob.read_v1_result.count;
        DPLOGI("DpDriver: read count:%d engine:%#010x\n", readBackRegs.m_num, readBackRegs.m_engineFlag);
    }

#if 0
    //JPEGEnc is enabled
    if ((readBackRegs.m_engineFlag >> tJPEGENC) & 0x1)
    {
        readBackRegs.m_jpegEnc_filesize = (readBackRegs.m_values[0] + 0x200) -
            (readBackRegs.m_values[1] + readBackRegs.m_values[2]);
        DPLOGI("JPEGEnc is enabled and JPEGEnc regs read back[0..2] %X %X %X \n",
               readBackRegs.m_values[0], readBackRegs.m_values[1], readBackRegs.m_values[2]);
        DPLOGI("waitComplete: JPEGEnc file size is %d\n", readBackRegs.m_jpegEnc_filesize);
    }
#endif
    if (diff > 34)
    {
        DPLOGW("DpDriver::waitFramedone takes %dms", diff);
    }
    else
    {
        DPLOGI("DpDriver::waitFramedone takes %dms", diff);
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::execCommand(DpCommandBlock &block)
{
    DP_TRACE_CALL();
    int32_t         status;
    mdp_submit      submitJob;
    mdp_wait        waitJob;
    DpTimeValue     begin;
    DpTimeValue     end;
    int32_t         diff;
    DpFrameInfo     frameInfo;
    int32_t         portNum;
    char            *frameInfoToCMDQ;

#if 1
    frameInfo = block.getFrameInfo();
    portNum = block.getRegDstNum();
    frameInfoToCMDQ = block.getFrameInfoToCMDQ();
    setFrameInfo(frameInfo, portNum, frameInfoToCMDQ);
#endif

    memset(&submitJob, 0, sizeof(submitJob));
    memset(&waitJob, 0, sizeof(waitJob));

#if 0
    param.userDebugStr = (unsigned long)frameInfoToCMDQ;
    param.userDebugStrLen = strlen(frameInfoToCMDQ);
#endif

#if PMQOS_SETTING
    submitJob.prop_addr = (unsigned long)block.getMdpPmqos();
    submitJob.prop_size = sizeof(mdp_pmqos);
#endif

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    if (0 != block.getSecureAddrCount())
    {
        submitJob.secData.isSecure = true;
        submitJob.secData.addrMetadataCount = block.getSecureAddrCount();
        submitJob.secData.addrMetadatas = (unsigned long)block.getSecureAddrMD();
        submitJob.secData.enginesNeedDAPC = block.getEngineFlag() | block.getSecurePortFlag();
        submitJob.secData.enginesNeedPortSecurity = block.getSecurePortFlag();

        DPLOGI("secure meta data engine flag = %x !!!!\n",block.getSecurePortFlag());
        DPLOGI("DpDriver: Secure meta data Ready !!!!\n");
    }

#if 0
#ifdef CONFIG_FOR_SOURCE_PQ
    if (STREAM_COLOR_BITBLT == block.getScenario())
    {
        param.scenario = CMDQ_SCENARIO_USER_DISP_COLOR;
    }
    else
#endif
    {
        param.scenario = CMDQ_SCENARIO_USER_MDP;
    }
#endif

    submitJob.priority = block.getPriority();

    submitJob.engine_flag = block.getEngineFlag();
#ifdef ISP_SMART_TILE_ENABLE // or MDP_VSS_ASYNC_ENABLE ?
    if (STREAM_ISP_VSS == block.getScenario())
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
#endif
#if defined(CMDQ_V3) || defined(CMDQ_K414)
#if !defined(tRDMA1) || tRDMA1 == tNone
    // Workaround conflict starving of secure videos
    if (submitJob.engine_flag & (1LL << tRDMA0))
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
    else
#endif
#ifndef MDP_VERSION_6885
    // On platforms with only one blk2scan RDMA
    if (DP_COLOR_GET_BLOCK_MODE(frameInfo.m_srcFormat))
    {
        submitJob.engine_flag |= (1LL << CMDQ_ENG_INORDER);
    }
#endif
#endif

    submitJob.metas = (unsigned long)block.getBlockBaseSW();
    submitJob.meta_count = block.getBlockSize() / sizeof(op_meta);
    submitJob.hw_metas_read_v1 = (unsigned long)block.getReadbackRegs(submitJob.read_count_v1);
    waitJob.read_v1_result.ret_values = (unsigned long)block.getReadbackValues(waitJob.read_v1_result.count);

    DPLOGI("DpDriver: exec command block: start %#llx count %u engine flag %#llx\n",
        submitJob.metas, submitJob.meta_count, submitJob.engine_flag);
    DPLOGI("DpDriver: exec command block: read ids v1:%#llx count:%u values:%#llx read back ids:%#llx\n",
        submitJob.metas, submitJob.read_count_v1, waitJob.read_v1_result.ret_values,
        waitJob.readback_result.readback_ids);

    DP_TIMER_GET_CURRENT_TIME(begin);
    status = ioctl(m_driverID, CMDQ_IOCTL_ASYNC_EXEC, &submitJob);
    if (status)
    {
        DPLOGE("DpDriver: exec command submit failed(%d) %d %s\n", status, errno, strerror(errno));
        return DP_STATUS_OPERATION_FAILED;
    }
    else
    {
        waitJob.job_id = submitJob.job_id;
        status = ioctl(m_driverID, CMDQ_IOCTL_ASYNC_WAIT, &waitJob);
        if (status)
        {
            DPLOGE("DpDriver: exec command wait failed(%d) %d %s\n", status, errno, strerror(errno));
            return DP_STATUS_OPERATION_FAILED;
        }
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > 30)
    {
        DPLOGW("DpDriver::execCommand takes %d ms\n", diff);
    }

    if ((STREAM_BITBLT == block.getScenario()) || (STREAM_GPU_BITBLT == block.getScenario()))
    {
        removeRefCnt(submitJob.engine_flag);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpDriver::allocatePABuffer(uint32_t slotCount, uint32_t *slotIDs)
{
    DP_TRACE_CALL();
    int32_t status;
    uint32_t index;
    mdp_readback slot;

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    for (index = 0; index < slotCount; index++)
    {
        if((index & (MDP_PAGE_SIZE - 1)) == 0)
        {
            memset(&slot, 0, sizeof(slot));
            slot.count = MDP_PAGE_SIZE;
            status = ioctl(m_driverID, CMDQ_IOCTL_ALLOC_READBACK_SLOTS, &slot);
            if (status)
            {
                DPLOGE("DpDriver::allocatePABuffer: allocate Buffer failed(%d) %d %s\n", status, errno, strerror(errno));
                return DP_STATUS_OPERATION_FAILED;
            }
            DPLOGI("DpDriver::allocatePABuffer: start id:%u count:%u\n", slot.start_id, slotCount);
        }
        slotIDs[index] = slot.start_id + (index & (MDP_PAGE_SIZE - 1));
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpDriver::releasePABuffer(uint32_t slotCount, uint32_t *slotIDs)
{
    DP_TRACE_CALL();
    int32_t status;
    uint32_t index;
    mdp_readback slot;

    if (checkHandle() != DP_STATUS_RETURN_SUCCESS)
    {
        DPLOGE("DpDriver::releasePABuffer: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    for (index = 0 ; index < slotCount ; index += MDP_PAGE_SIZE)
    {
        memset(&slot, 0, sizeof(slot));
        slot.count = MDP_PAGE_SIZE;
        slot.start_id = slotIDs[index];
        status = ioctl(m_driverID, CMDQ_IOCTL_FREE_READBACK_SLOTS, &slot);
        if (status)
        {
            DPLOGE("DpDriver::releasePABuffer: release buffer failed(%d) %d %s\n", status, errno, strerror(errno));
            return DP_STATUS_OPERATION_FAILED;
        }
    }

    memset(slotIDs, 0, sizeof(uint32_t) * slotCount);

    return DP_STATUS_RETURN_SUCCESS;
}


int32_t DpDriver::getEnableLog()
{
    return m_enableLog || CONFIG_LOG_LEVEL;
}


int32_t DpDriver::getEnableSystrace()
{
    return m_enableSystrace;
}

int32_t DpDriver::getEnableMMpath()
{
    return m_enableMMpath;
}

int32_t DpDriver::getEnableMet()
{
    if (m_enableCheckMet)
    {
        getProperty("vendor.dp.met.enable", &m_enableMet);
    }
    return m_enableMet;
}

int32_t DpDriver::getEnableDumpBuffer()
{
    return m_enableDumpBuffer;
}

char* DpDriver::getdumpBufferFolder()
{
    return m_dumpBufferFolder;
}

int32_t DpDriver::getEnableDumpRegister()
{
    if (m_enableCheckDumpReg)
    {
        getProperty("vendor.dp.dumpreg.enable", &m_enableDumpRegister);
    }
    return m_enableDumpRegister;
}

int32_t DpDriver::getDisableReduceConfig()
{
    return m_reduceConfigDisable;
}

int32_t DpDriver::getPQSupport()
{
    return m_pq_support;
}

int32_t DpDriver::getGlobalPQSupport()
{
    return m_supportGlobalPQ;
}

int32_t DpDriver::getMdpColor()
{
    return m_mdpColor;
}

int32_t DpDriver::getDisableFrameChange()
{
    return m_disableFrameChange;
}

DP_STATUS_ENUM DpDriver::notifyEngineWROT()
{
    int status;
    uint32_t engineFlag = (1 << tWROT0)
#if tWROT1 != tNone
        | (1 << tWROT1)
#endif
        ;

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(m_driverID, CMDQ_IOCTL_NOTIFY_ENGINE, engineFlag);
    if (0 != status)
    {
        DPLOGE("DpDriver: notify WROT failed(%d)\n", status);
        return DP_STATUS_OPERATION_FAILED;
    }

    DPLOGD("notify WROT success\n");
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpDriver::queryDeviceTreeInfo()
{
    int32_t status;

    if (DP_STATUS_RETURN_SUCCESS != checkHandle())
    {
        DPLOGE("DpDriver: invalid display driver handle\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    status = ioctl(m_driverID, CMDQ_IOCTL_QUERY_DTS, &m_cmdqDts);
    if (0 != status)
    {
        DPLOGE("DpDriver: exec CMDQ_IOCTL_QUERY_DTS failed(%d)\n", status);
        return DP_STATUS_OPERATION_FAILED;
    }

    if (0 == m_cmdqDts.MDPBaseAddress[CMDQ_MDP_PA_BASE_MM_MUTEX])
    {
        DPLOGD("DpDriver: Disp_mutex value is not written in device tree.\n");
    }

    return DP_STATUS_RETURN_SUCCESS;
}


int32_t DpDriver::getEventValue(int32_t event)
{
    if (event < 0 || event >= CMDQ_SYNC_TOKEN_MAX)
    {
        DPLOGE("DpDriver: event value %d is not written in device tree.\n", event);
        return DP_STATUS_OPERATION_FAILED;
    }

#ifdef CMDQ_DVENT_FROM_DTS
    return m_cmdqDts.eventTable[event];
#else
    return event;
#endif
}


uint32_t DpDriver::getMMSysMutexBase()
{
    return m_cmdqDts.MDPBaseAddress[CMDQ_MDP_PA_BASE_MM_MUTEX];
}

DP_STATUS_ENUM DpDriver::setFrameInfo(DpFrameInfo frameInfo, int32_t portNum, char *frameInfoToCMDQ)
{
    char frameInfoOutput[200] = {0};
    uint32_t frameInfoOutputLength = 0;

    sprintf(frameInfoToCMDQ,"[MDP] Input: (%d, %d, %d, %d, C%d%s%s%s%s%s%s) sec%d\n[MDP] Buffer info:bufMVA={0x%08x,0x%08x,0x%08x} bufSize={0x%08x,0x%08x,0x%08x}\n",
        frameInfo.m_srcWidth,
        frameInfo.m_srcHeight,
        frameInfo.m_srcYPitch,
        frameInfo.m_srcUVPitch,
        DP_COLOR_GET_UNIQUE_ID(frameInfo.m_srcFormat),
        DP_COLOR_GET_SWAP_ENABLE(frameInfo.m_srcFormat) ? "s" : "",
        DP_COLOR_GET_BLOCK_MODE(frameInfo.m_srcFormat) ? "b" : "",
        DP_COLOR_GET_INTERLACED_MODE(frameInfo.m_srcFormat) ? "i" : "",
        DP_COLOR_GET_UFP_ENABLE(frameInfo.m_srcFormat) ? "u" : "",
        DP_COLOR_GET_10BIT_TILE_MODE(frameInfo.m_srcFormat) ? "t" :
        DP_COLOR_GET_10BIT_PACKED(frameInfo.m_srcFormat) ? "p" :
        DP_COLOR_GET_10BIT_LOOSE(frameInfo.m_srcFormat) ? "l" : "",
        DP_COLOR_GET_10BIT_JUMP_MODE(frameInfo.m_srcFormat) ? "j" : "",
        frameInfo.m_srcSecMode,
        frameInfo.m_srcMemAddr[0],
        frameInfo.m_srcMemAddr[1],
        frameInfo.m_srcMemAddr[2],
        frameInfo.m_srcMemSize[0],
        frameInfo.m_srcMemSize[1],
        frameInfo.m_srcMemSize[2]);

    for(int portIndex = 0 ; portIndex < portNum ; portIndex++){
        frameInfoOutputLength = sprintf(frameInfoOutput,"[MDP] Output: port%d (%d, %d, %d, %d, C%d%s%s%s%s), misc: (X:%d, Y:%d) sec%d\n[MDP] Buffer info:bufMVA={0x%08x,0x%08x,0x%08x} bufSize={0x%08x,0x%08x,0x%08x}\n",
            portIndex,
            frameInfo.m_dstWidth[portIndex],
            frameInfo.m_dstHeight[portIndex],
            frameInfo.m_dstYPitch[portIndex],
            frameInfo.m_dstUVPitch[portIndex],
            DP_COLOR_GET_UNIQUE_ID(frameInfo.m_dstFormat[portIndex]),
            DP_COLOR_GET_SWAP_ENABLE(frameInfo.m_dstFormat[portIndex]) ? "s" : "",
            DP_COLOR_GET_BLOCK_MODE(frameInfo.m_dstFormat[portIndex]) ? "b" : "",
            DP_COLOR_GET_INTERLACED_MODE(frameInfo.m_dstFormat[portIndex]) ? "i" : "",
            DP_COLOR_GET_UFP_ENABLE(frameInfo.m_dstFormat[portIndex]) ? "u" : "",
            frameInfo.m_outXStart[portIndex],
            frameInfo.m_outYStart[portIndex],
            frameInfo.m_dstSecMode[portIndex],
            frameInfo.m_dstMemAddr[portIndex][0],
            frameInfo.m_dstMemAddr[portIndex][1],
            frameInfo.m_dstMemAddr[portIndex][2],
            frameInfo.m_dstMemSize[portIndex][0],
            frameInfo.m_dstMemSize[portIndex][1],
            frameInfo.m_dstMemSize[portIndex][2]);

        strncat(frameInfoToCMDQ, frameInfoOutput, frameInfoOutputLength);
    }
    return DP_STATUS_RETURN_SUCCESS;
}
