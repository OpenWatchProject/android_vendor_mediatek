#ifndef __DP_WRAPPER_ISP_H__
#define __DP_WRAPPER_ISP_H__

#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "DpWriteBMP.h"
#include "tile_driver.h"
#include "tile_param.h"
#include "DpPlatform.h"

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
          m_ISPPassType(TPIPE_PASS2_CMDQ_1),
          m_dumpIndex(0),
          m_onlyISP(false),
          m_tileXEndFlag(false),
          m_tileYEndFlag(false),
          m_ispDataSize(0)
    {
        for (int i = 0; i < TILE_FUNC_DMA_NUM; i++)
        {
            m_pISPDMA[i] = NULL;
        }

        DPLOGI("DpEngineISP: initialize ISP object\n");

#if CONFIG_FOR_VERIFY_FPGA
        m_pTDRIBufMem = DpMemory::Factory(DP_MEMORY_ION, -1, MAX_ISP_TILE_TDR_HEX_NO);
        assert(NULL != m_pTDRIBufMem);
#endif // CONFIG_FOR_VERIFY_FPGA

        char *buf = (char*)calloc(1, (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)) + // tpipe_info
                                     (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)) +   // tdr_disable_flag
                                     (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)));   // last_irq_flag
        assert(NULL != buf);
        m_pTileInfo = (ISP_TPIPE_INFORMATION_STRUCT*)(buf);
        m_pTdrFlag  = (unsigned int*)(buf + (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)));
        m_pIrqFlag  = (unsigned int*)(buf + (MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT)) + (((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int)));

        m_pTileDesc = new ISP_TPIPE_DESCRIPTOR_STRUCT();
        assert(NULL != m_pTileDesc);

        memset(&m_ISPConfig, 0x0, sizeof(ISP_TPIPE_CONFIG_STRUCT));

        //m_tileEngGroup = TILE_GROUP_CROP_EN;
    }

    virtual ~DpWrapper_ISP()
    {
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

    DpMemory                     *m_pTDRIBufMem;
    uint32_t                     m_TDRIBufPA;
    uint32_t                     *m_pTDRIBufVA;
    uint32_t                     m_TDRITileID;

    ISP_TPIPE_INFORMATION_STRUCT *m_pTileInfo;
    uint32_t                     *m_pTdrFlag;
    uint32_t                     *m_pIrqFlag;
    ISP_TPIPE_DESCRIPTOR_STRUCT  *m_pTileDesc;
    FUNC_DESCRIPTION_STRUCT      *m_pTileFunc;
    TILE_FUNC_BLOCK_STRUCT       *m_pIMGI;
    TILE_FUNC_BLOCK_STRUCT       *m_pMDPCrop;
    TILE_FUNC_BLOCK_STRUCT       *m_pISPDMA[TILE_FUNC_DMA_NUM];
    TPIPE_Pass2CmdqNum_ENUM      m_ISPPassType;
    ISP_TPIPE_CONFIG_STRUCT      m_ISPConfig;
    uint32_t                     m_dumpIndex;
    bool                         m_onlyISP;
    bool                         m_tileXEndFlag;
    bool                         m_tileYEndFlag;
    uint32_t                     m_ispDataSize;

    DP_STATUS_ENUM setupFrameInfo(STREAM_TYPE_ENUM,
                                  DpColorFormat)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

#if PMQOS_SETTING
    DP_STATUS_ENUM onUpdatePMQOS(uint32_t&, uint32_t&, struct timeval&);
#endif // PMQOS_SETTING

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    DP_STATUS_ENUM onFlushBuffer(FLUSH_TYPE_ENUM);

    int64_t onQueryFeature()
    {
        return eRMEM | eISP;
    }
};

#endif  // __DP_WRAPPER_ISP_H__
