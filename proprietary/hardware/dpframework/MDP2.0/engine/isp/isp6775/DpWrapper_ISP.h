#ifndef __DP_WRAPPER_ISP_H__
#define __DP_WRAPPER_ISP_H__

#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "DpWriteBMP.h"
#include "tile_driver.h"
#include "tile_param.h"

#define CONFIG_FOR_DEBUG_ISP    0
#define CONFIG_ISP_WRAPPER_UT   0
#define ISP_PASS2_UT 0

typedef enum TILE_FUNC_DMA_ENUM
{
    TILE_FUNC_DMA_IMGI =  (0),
    TILE_FUNC_DMA_IMGBI = (1),
    TILE_FUNC_DMA_IMGCI = (2),
    TILE_FUNC_DMA_UFDI =  (3),
    TILE_FUNC_DMA_VIPI =  (4),
    TILE_FUNC_DMA_VIP2I = (5),
    TILE_FUNC_DMA_VIP3I = (6),
    TILE_FUNC_DMA_LCEI =  (7),
    TILE_FUNC_DMA_DMGI =  (8),
    TILE_FUNC_DMA_DEPI =  (9),
    TILE_FUNC_DMA_MFBO =  (10),
    TILE_FUNC_DMA_IMG2O = (11),
    TILE_FUNC_DMA_IMG2BO = (12),
    TILE_FUNC_DMA_FEO = (13),
    TILE_FUNC_DMA_IMG3O = (14),
    TILE_FUNC_DMA_IMG3BO = (15),
    TILE_FUNC_DMA_IMG3CO = (16),
    TILE_FUNC_DMA_NUM ,
};

//--------------------------------------------------------
// ISP tile driver wrapper
//--------------------------------------------------------
class DpWrapper_ISP: public DpTileEngine
{
public:
    DpWrapper_ISP()
        : DpTileEngine(0),
          m_pTDRIBufMem(NULL),
          m_TDRIBufPA(0),
          m_pTDRIBufVA(0),
          m_TDRITileID(0),
          m_pTileInfo(NULL),
          m_pTdrFlag(NULL),
          m_pIrqFlag(NULL),
          m_pTileDesc(NULL),
          m_pTileFunc(NULL),
          m_pIMGI(NULL),
          m_pMDPCrop(NULL),
          m_ISPPassType(ISP_DRV_DIP_CQ_THRE0),
          m_dumpIndex(0),
          m_onlyISP(false),
          m_tileXEndFlag(false),
          m_tileYEndFlag(false),
          m_pReadbackReg(NULL),
          m_regCount(0),
          m_ispDataSize(0)
    {
        for (int i = 0; i < TILE_FUNC_DMA_NUM; i++)
        {
            m_pISPDMA[i] = NULL;
        }

    #if CONFIG_FOR_DEBUG_ISP
        void *pTemp;

        DPLOGI("DpEngineISP: allocate IMGI input buffer: %d\n", sizeof(g_imgi_array));

        m_pIMGIBuffer  = DpMemory::Factory((void*)NULL, sizeof(g_imgi_array));
        assert(NULL != m_pIMGIBuffer);

        DPLOGI("DpEngineISP: map IMGI buffer virtual address\n");

        pTemp = m_pIMGIBuffer->mapVA();
        assert(NULL != pTemp);

        DPLOGI("DpEngineISP: copy IMGI source buffer data\n");
        memcpy(pTemp, g_imgi_array, sizeof(g_imgi_array));

        m_pIMG2OBuffer = DpMemory::Factory(-1, sizeof(g_img2o_array));
        assert(NULL != m_pIMG2OBuffer);

        DPLOGI("DpEngineISP: allocate IMG2O output buffer: %d\n", sizeof(g_img2o_array));

        memset(m_pMemHandler, 0x0, sizeof(DpMemory*) * 10);
    #endif  // CONFIG_FOR_DEBUG_ISP

        DPLOGI("DpEngineISP: initialize ISP object\n");

#if CONFIG_FOR_VERIFY_FPGA
        m_pTDRIBufMem = DpMemory::Factory(DP_MEMORY_ION, -1, MAX_ISP_TILE_TDR_HEX_NO);
        assert(NULL != m_pTDRIBufMem);
#endif // CONFIG_FOR_VERIFY_FPGA

        memset(m_TDRITileLabel, -1, sizeof(m_TDRITileLabel));
        memset(m_regLabel, -1, sizeof(m_regLabel));
        memset(m_PABuffer, 0, sizeof(m_PABuffer));

        char *buf = (char*)calloc(1, (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)) + // tpipe_info
                                     (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)) +   // tdr_disable_flag
                                     (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)));   // last_irq_flag
        assert(NULL != buf);
        m_pTileInfo = (ISP_TPIPE_INFORMATION_STRUCT*)(buf);
        m_pTdrFlag  = (unsigned int*)(buf + (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)));
        m_pIrqFlag  = (unsigned int*)(buf + (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)) + (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)));

        m_pTileDesc = new ISP_TPIPE_DESCRIPTOR_STRUCT();
        assert(NULL != m_pTileDesc);

        memset(&m_ISPConfig, 0x0, sizeof(m_ISPConfig));

        //m_tileEngGroup = TILE_GROUP_CROP_EN;
    }

    virtual ~DpWrapper_ISP()
    {
    #if CONFIG_FOR_DEBUG_ISP
        int32_t index;
        uint8_t *pTemp;

        if (NULL != m_pIMGIBuffer)
        {
            delete m_pIMGIBuffer;
            m_pIMGIBuffer = NULL;
        }

        pTemp = m_pIMG2OBuffer->mapSWAddress();

        m_pIMG2OBuffer->flushWriteBuffer(DpMemory::FLUSH_AFTER_HW_WRITE_BUFFER_RANGE);

        DPLOGI("DpWrapper_ISP: dump IMG2O output begin: 0x%p\n", pTemp);

        utilWriteBMP("./out/IMG2OOut.bmp",
                     pTemp,
                     NULL,
                     NULL,
                     DP_COLOR_YUYV,
                     3264,
                     2448,
                     6528);

        DPLOGI("DpWrapper_ISP: dump IMG2O output end\n");

        if (NULL != m_pIMG2OBuffer)
        {
            delete m_pIMG2OBuffer;
            m_pIMG2OBuffer = NULL;
        }

        for (index = 0; index < 10; index++)
        {
            if (NULL != m_pMemHandler[index])
            {
                delete m_pMemHandler[index];
                m_pMemHandler[index] = NULL;
            }
        }
    #endif  // CONFIG_FOR_DEBUG_ISP

        delete m_pTileDesc;
        m_pTileDesc = NULL;

        free(m_pTileInfo);
        m_pTileInfo = NULL;
        m_pTdrFlag = NULL;
        m_pIrqFlag = NULL;

#if CONFIG_FOR_VERIFY_FPGA
        delete m_pTDRIBufMem;
        m_pTDRIBufMem = NULL;
#endif // CONFIG_FOR_VERIFY_FPGA
    }

    inline void setISPOnly(bool only)
    {
        m_onlyISP = only;
        if (only)
        {
            DPLOGD("DpIsp: ISP Pass2 only\n");
        }
    }

    inline bool isISPOnly()
    {
        return m_onlyISP;
    }

    DP_STATUS_ENUM initTilePath(TILE_PARAM_STRUCT *p_tile_param);

private:
    enum
    {
        COMMAND_QUEUE_0 = 0,
        COMMAND_QUEUE_0B,
        COMMAND_QUEUE_1,
        COMMAND_QUEUE_2,
        COMMAND_QUEUE_3,
        COMMAND_QUEUE_NUM
    };

#if CONFIG_FOR_DEBUG_ISP
    #define CQ_SET_NUM              1000
    #define CQ_SET_BYTE_SIZE        2
    #define CQ_APB                  0x00
    #define CQ_TRIGGER_POLLING      0x3B
    #define CQ_TRIGGER_IDLE         0x3F
    #define SET_CQ_APB_ADDR(_x_)    ((_x_)&(0xFFFF))           /* bit0 -bit15 */
    #define SET_CQ_CMD_COUNT(_x_)   (((_x_)<<16)&(0x3FF0000))  /* bit16 -bit25 */
    #define SET_CQ_COMMAND(_x_)     (((_x_)<<26)&(0xFC000000)) /* bit26 -bit31 */
    #define CQ_MAPPING_ADDR_SIZE    1000

    #define SET_CQ_TABLE(_CqIdx_, _Addr_, _Cmd_, _Val_)                                                             \
    {                                                                                                               \
        *(pCqMappingAddrVir[_CqIdx_] + tableNum) = (_Val_);                                                         \
        *(pCqSetVir[_CqIdx_]+cqCount) = SET_CQ_APB_ADDR(_Addr_) | SET_CQ_CMD_COUNT(0x00) | SET_CQ_COMMAND(_Cmd_);   \
        cqCount++;                                                                                                  \
        if(CQ_TRIGGER_IDLE == _Cmd_ || CQ_TRIGGER_POLLING == _Cmd_)                                                 \
        {                                                                                                           \
            *(pCqSetVir[_CqIdx_]+cqCount) = 0x00;                                                                   \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            *(pCqSetVir[_CqIdx_]+cqCount) = (CqMappingAddrPhy[_CqIdx_] + tableNum*4);                               \
        }                                                                                                           \
        cqCount++;                                                                                                  \
        tableNum++;                                                                                                 \
    }

    DpMemory                     *m_pIMGIBuffer;
    DpMemory                     *m_pIMG2OBuffer;
    uint32_t                     m_CMDQPhysical[COMMAND_QUEUE_NUM];
    DpMemory                     *m_pMemHandler[10];
#endif  // CONFIG_FOR_DEBUG_ISP

    DpMemory                     *m_pTDRIBufMem;
    uint32_t                     m_TDRIBufPA;
    uint32_t                     *m_pTDRIBufVA;
    uint32_t                     m_TDRITileID;
    int32_t                      m_TDRITileLabel[MAX_TILE_TOT_NO];
    int32_t                      m_regLabel[MAX_NUM_READBACK_REGS];

    ISP_TPIPE_INFORMATION_STRUCT *m_pTileInfo;
    uint32_t                     *m_pTdrFlag;
    uint32_t                     *m_pIrqFlag;
    ISP_TPIPE_DESCRIPTOR_STRUCT  *m_pTileDesc;
    FUNC_DESCRIPTION_STRUCT      *m_pTileFunc;
    TILE_FUNC_BLOCK_STRUCT       *m_pIMGI;
    TILE_FUNC_BLOCK_STRUCT       *m_pMDPCrop;
    TILE_FUNC_BLOCK_STRUCT       *m_pISPDMA[TILE_FUNC_DMA_NUM];
    E_ISP_DIP_CQ                 m_ISPPassType;
    ISP_TPIPE_CONFIG_STRUCT      m_ISPConfig;
    uint32_t                     m_dumpIndex;
    bool                         m_onlyISP;
    bool                         m_tileXEndFlag;
    bool                         m_tileYEndFlag;
    uint32_t                     m_PABuffer[DP_MAX_SINGLE_PABUFFER_COUNT];
    uint32_t                     *m_pReadbackReg;
    uint32_t                     m_regCount;
    uint32_t                     m_ispDataSize;

    DP_STATUS_ENUM setupFrameInfo(STREAM_TYPE_ENUM,
                                  DpColorFormat)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onReconfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onReconfigTiles(DpCommand&);

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    DP_STATUS_ENUM onFlushBuffer(FLUSH_TYPE_ENUM);

    DP_STATUS_ENUM onUpdatePMQOS(uint32_t&, uint32_t&, struct timeval&);

    DP_STATUS_ENUM onDumpDebugInfo();

    int64_t onQueryFeature()
    {
        return eRMEM | eISP;
    }
};

#endif  // __DP_WRAPPER_ISP_H__
