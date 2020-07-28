#ifndef __DP_DRIVER_ANDROID_H__
#define __DP_DRIVER_ANDROID_H__

#include "DpDataType.h"
#include "DpMutex.h"
#include "ddp_drv.h"
#include "cmdq_mdp_pmqos.h"
#if defined(CMDQ_V3)
#include "cmdq_v3_def.h"
#elif defined(CMDQ_K414)
#include "cmdq_k414_def.h"
#else
#include "cmdq_def.h"
#endif
#include <cutils/properties.h>
#ifndef ISP_MAX_OUTPUT_PORT_NUM
#define ISP_MAX_OUTPUT_PORT_NUM (4)
#endif

class DpPathBase;

class DpCommandBase
{
public:
    enum EVENT_TYPE_ENUM
    {
        RDMA0_FRAME_DONE = CMDQ_EVENT_MDP_RDMA0_EOF,
        RDMA1_FRAME_DONE = CMDQ_EVENT_MDP_RDMA1_EOF,
        WDMA_FRAME_START = CMDQ_EVENT_MDP_WDMA_SOF,
        WDMA_FRAME_DONE  = CMDQ_EVENT_MDP_WDMA_EOF,
        WROT1_FRAME_START = CMDQ_EVENT_MDP_WROT1_SOF,
        WROT1_FRAME_DONE = CMDQ_EVENT_MDP_WROT1_WRITE_EOF,
        SYNC_WROT0_SRAM_READY = CMDQ_SYNC_RESOURCE_WROT0,
        SYNC_WROT1_SRAM_READY = CMDQ_SYNC_RESOURCE_WROT1,
#ifdef CMDQ_6765_EVENT
        IMG_DL_RELAY_FRAME_START = CMDQ_EVENT_IMG_DL_RELAY_SOF,
#endif
        RDMA0_FRAME_START = CMDQ_EVENT_MDP_RDMA0_SOF,
        RDMA1_FRAME_START = CMDQ_EVENT_MDP_RDMA1_SOF,
        RSZ0_FRAME_START = CMDQ_EVENT_MDP_RSZ0_SOF,
        RSZ1_FRAME_START = CMDQ_EVENT_MDP_RSZ1_SOF,
#ifdef CMDQ_6797_EVENT
        WROT0_FRAME_START = CMDQ_EVENT_MDP_WROT0_SOF,
        WROT0_FRAME_DONE = CMDQ_EVENT_MDP_WROT0_WRITE_EOF,
        ISP_P2_0_DONE = CMDQ_EVENT_DIP_CQ_THREAD0_EOF,
        ISP_P2_1_DONE = CMDQ_EVENT_DIP_CQ_THREAD1_EOF,
        ISP_P2_2_DONE = CMDQ_EVENT_DIP_CQ_THREAD2_EOF,
        ISP_P2_3_DONE = CMDQ_EVENT_DIP_CQ_THREAD3_EOF,
        ISP_P2_4_DONE = CMDQ_EVENT_DIP_CQ_THREAD4_EOF,
        ISP_P2_5_DONE = CMDQ_EVENT_DIP_CQ_THREAD5_EOF,
        ISP_P2_6_DONE = CMDQ_EVENT_DIP_CQ_THREAD6_EOF,
        ISP_P2_7_DONE = CMDQ_EVENT_DIP_CQ_THREAD7_EOF,
        ISP_P2_8_DONE = CMDQ_EVENT_DIP_CQ_THREAD8_EOF,
        ISP_P2_9_DONE = CMDQ_EVENT_DIP_CQ_THREAD9_EOF,
        ISP_P2_10_DONE = CMDQ_EVENT_DIP_CQ_THREAD10_EOF,
        ISP_P2_11_DONE = CMDQ_EVENT_DIP_CQ_THREAD11_EOF,
        ISP_P2_12_DONE = CMDQ_EVENT_DIP_CQ_THREAD12_EOF,
        ISP_P2_13_DONE = CMDQ_EVENT_DIP_CQ_THREAD13_EOF,
        ISP_P2_14_DONE = CMDQ_EVENT_DIP_CQ_THREAD14_EOF,
#elif defined(CMDQ_6739_EVENT)
        WROT0_FRAME_START = CMDQ_EVENT_MDP_WROT0_SOF,
        WROT0_FRAME_DONE = CMDQ_EVENT_MDP_WROT0_WRITE_EOF,
        ISP_P2_0_DONE = CMDQ_EVENT_ISP_PASS2_0_EOF,
        ISP_P2_1_DONE = CMDQ_EVENT_ISP_PASS2_1_EOF,
        ISP_P2_2_DONE = CMDQ_EVENT_ISP_PASS2_2_EOF,
#else
        WROT0_FRAME_START  = CMDQ_EVENT_MDP_WROT_SOF,
        WROT0_FRAME_DONE = CMDQ_EVENT_MDP_WROT_WRITE_EOF,
        ISP_P2_0_DONE = CMDQ_EVENT_ISP_PASS2_0_EOF,
        ISP_P2_1_DONE = CMDQ_EVENT_ISP_PASS2_1_EOF,
        ISP_P2_2_DONE = CMDQ_EVENT_ISP_PASS2_2_EOF,
#endif
#if defined(CMDQ_V3) || defined(CMDQ_K414)
        WPE_FRAME_DONE = CMDQ_EVENT_WPE_A_EOF,
        WPE_B_FRAME_DONE = CMDQ_EVENT_WPE_B_FRAME_DONE,
        TDSHP0_FRAME_START = CMDQ_EVENT_MDP_TDSHP_SOF,
#endif
#if defined(CMDQ_6885_EVENT)
        ISP_P2_15_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_15,
        ISP_P2_16_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_16,
        ISP_P2_17_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_17,
        ISP_P2_18_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_18,
        ISP_P2_19_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_19,
        ISP_P2_20_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_20,
        ISP_P2_MFB_FRAME_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_21,
        ISP_P2_22_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_22,
        ISP_P2_MSS_FRAME_DONE = CMDQ_EVENT_IMG1_EVENT_TX_FRAME_DONE_23,
        ISP_P2_2_0_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_0,
        ISP_P2_2_1_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_1,
        ISP_P2_2_2_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_2,
        ISP_P2_2_3_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_3,
        ISP_P2_2_4_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_4,
        ISP_P2_2_5_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_5,
        ISP_P2_2_6_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_6,
        ISP_P2_2_7_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_7,
        ISP_P2_2_8_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_8,
        ISP_P2_2_9_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_9,
        ISP_P2_2_10_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_10,
        ISP_P2_2_11_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_11,
        ISP_P2_2_12_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_12,
        ISP_P2_2_13_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_13,
        ISP_P2_2_14_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_14,
        ISP_P2_2_15_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_15,
        ISP_P2_2_16_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_16,
        ISP_P2_2_17_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_17,
        ISP_P2_2_18_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_18,
        ISP_P2_2_19_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_19,
        ISP_P2_2_20_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_20,
        ISP_P2_2_MFB_FRAME_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_21,
        ISP_P2_2_22_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_22,
        ISP_P2_2_MSS_FRAME_DONE = CMDQ_EVENT_IMG2_EVENT_TX_FRAME_DONE_23,
#endif
        JPEGENC_FRAME_DONE  = CMDQ_EVENT_JPEG_ENC_EOF,
        JPEGDEC_FRAME_DONE  = CMDQ_EVENT_JPEG_DEC_EOF,
        SYNC_TOKEN_VENC_EOF = CMDQ_SYNC_TOKEN_VENC_EOF,
        SYNC_TOKEN_VENC_INPUT_READY = CMDQ_SYNC_TOKEN_VENC_INPUT_READY,
        // Display driver
        DISP_RDMA0_FRAME_START = CMDQ_EVENT_DISP_RDMA0_SOF,
        DISP_RDMA0_FRAME_DONE = CMDQ_EVENT_DISP_RDMA0_EOF,
        DISP_OVL0_FRAME_DONE = CMDQ_EVENT_DISP_OVL0_EOF,
        DISP_WDMA0_FRAME_DONE = CMDQ_EVENT_DISP_WDMA0_EOF,
        // GPR tokens
        SYNC_TOKEN_GPR_READ = CMDQ_SYNC_TOKEN_GPR_SET_0,
        SYNC_TOKEN_GPR_WRITE = CMDQ_SYNC_TOKEN_GPR_SET_1,
        SYNC_TOKEN_GPR_POLL = CMDQ_SYNC_TOKEN_GPR_SET_1,
        SYNC_TOKEN_GPR_WRITE_FROM_MEM = CMDQ_SYNC_TOKEN_GPR_SET_3,
        SYNC_TOKEN_GPR_WRITE_FROM_REG = CMDQ_SYNC_TOKEN_GPR_SET_1,
    };
};

typedef struct DpReadbackRegs_t
{
    uint32_t m_regs[MAX_NUM_READBACK_REGS];
    uint32_t m_values[MAX_NUM_READBACK_REGS];
    uint32_t m_num;
    uint64_t m_engineFlag;
    uint32_t m_jpegEnc_filesize;
} DpReadbackRegs;

typedef struct DpFrameInfo_t
{
    DpColorFormat m_srcFormat;
    int32_t       m_srcWidth;
    int32_t       m_srcHeight;
    int32_t       m_srcYPitch;
    int32_t       m_srcUVPitch;
    uint32_t      m_srcMemAddr[3];
    uint32_t      m_srcMemSize[3];
    DpSecure      m_srcSecMode;

    DpColorFormat m_dstFormat[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstYPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_dstUVPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_outXStart[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t       m_outYStart[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t      m_dstMemAddr[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t      m_dstMemSize[ISP_MAX_OUTPUT_PORT_NUM][3];
    DpSecure      m_dstSecMode[ISP_MAX_OUTPUT_PORT_NUM];
} DpFrameInfo;

class DpCommandBlock
{
public:
    DpCommandBlock()
    {
    }

    virtual ~DpCommandBlock()
    {
    }

    virtual uint32_t getScenario() = 0;

    virtual uint32_t getPriority() = 0;

    virtual uint64_t getEngineFlag() = 0;

    virtual uint64_t getSecurePortFlag() = 0;

    virtual void* getBlockBaseSW() = 0;

    virtual uint32_t getBlockSize() = 0;

    virtual void* getSecureAddrMD() = 0;

    virtual uint32_t getSecureAddrCount() = 0;

    virtual DpFrameInfo getFrameInfo() = 0;

    virtual void setRegDstNum(int32_t regNum) = 0;

    virtual int32_t getRegDstNum() = 0;

    virtual uint32_t* getReadbackRegs(uint32_t& numReadRegs) = 0;

    virtual uint32_t* getReadbackValues(uint32_t& numValues) = 0;

    virtual uint32_t getISPDebugDumpRegs() = 0;

    virtual char* getFrameInfoToCMDQ() = 0;

    virtual mdp_pmqos* getMdpPmqos();

#if defined(CMDQ_V3) || defined(CMDQ_K414)
    virtual bool hasIspSecMeta();

    virtual cmdqSecIspMeta getSecIspMeta();
#endif
};

class DpDriver
{
public:
    typedef uint32_t EngUsages[CMDQ_MAX_ENGINE_COUNT];

    static DpDriver* getInstance();

    static void destroyInstance();

    DP_STATUS_ENUM getTDSHPGain(DISPLAY_TDSHP_T *pSharpness,
                                uint32_t        *pCurLevel);

    DP_STATUS_ENUM requireMutex(int32_t *pMutex);

    DP_STATUS_ENUM releaseMutex(int32_t mutex);

    DP_STATUS_ENUM waitFramedone(DpJobID pFrame, DpReadbackRegs &readBackRegs);

    DP_STATUS_ENUM execCommand(DpCommandBlock &block);

    DP_STATUS_ENUM submitCommand(DpCommandBlock &block, DpJobID* pRet, uint32_t extRecorderFlag = 0, char** pFrameInfo = NULL);

    DP_STATUS_ENUM queryEngUsages(EngUsages &engUsages);

    void addRefCnt(DpEngineType &sourceEng);

    void removeRefCnt(uint64_t pathFlags);

    DP_STATUS_ENUM allocatePABuffer(uint32_t slotCount, uint32_t *slotIDs);

    DP_STATUS_ENUM releasePABuffer(uint32_t slotCount, uint32_t *slotIDs);

    int32_t getEnableLog();

    int32_t getEnableSystrace();

    int32_t getEnableDumpBuffer();

    int32_t getEnableMMpath();

    int32_t getEnableMet();

    char* getdumpBufferFolder();

    int32_t getEnableDumpRegister();

    int32_t getDisableReduceConfig();

    int32_t getPQSupport();

    int32_t getGlobalPQSupport();

    int32_t getMdpColor();

    int32_t getDisableFrameChange();

    DP_STATUS_ENUM notifyEngineWROT();

    int32_t getEventValue(int32_t event);

    uint32_t getMMSysMutexBase();

    DP_STATUS_ENUM setFrameInfo(DpFrameInfo frameInfo, int32_t portNum, char *frameInfoToCMDQ);

private:
    DP_STATUS_ENUM checkHandle();
    DP_STATUS_ENUM queryDeviceTreeInfo();

    static DpDriver *s_pInstance;
    static DpMutex  s_instMutex;

    int32_t         m_driverID;
    DpMutex         m_instMutex;
    int32_t         m_enableLog;
    int32_t         m_enableSystrace;
    int32_t         m_enableDumpBuffer;
    char            m_dumpBufferFolder[PROPERTY_VALUE_MAX];
    int32_t         m_enableCheckDumpReg;
    int32_t         m_enableDumpRegister;
    int32_t         m_enableMMpath;
    int32_t         m_enableCheckMet;
    int32_t         m_enableMet;
    int32_t         m_reduceConfigDisable;
    int32_t         m_pq_support;
    int32_t         m_mdpColor;
    int32_t         m_disableFrameChange;

    int32_t         m_refCntRDMA0;
    int32_t         m_refCntRDMA1;

    int32_t         m_supportGlobalPQ;

    cmdqDTSDataStruct m_cmdqDts;

    DpDriver();

    DpDriver(DpDriver &other);

    ~DpDriver();
};

#endif  // __DP_DRIVER_CLIENT_ANDROID_H__
