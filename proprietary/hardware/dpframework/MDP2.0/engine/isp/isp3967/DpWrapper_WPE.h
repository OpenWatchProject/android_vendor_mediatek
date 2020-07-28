#ifndef __DP_WRAPPER_WPE_H__
#define __DP_WRAPPER_WPE_H__

#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "DpWriteBMP.h"
#include "tile_driver.h"
#include "tile_param.h"

typedef enum WPE_TILE_FUNC_ENUM
{
    WPE_TILE_FUNC_WPE =  (0),
    WPE_TILE_FUNC_NUM ,
};

//--------------------------------------------------------
// WPE tile driver wrapper
//--------------------------------------------------------
class DpWrapper_WPE: public DpTileEngine
{
public:
    DpWrapper_WPE(uint32_t identifier)
        : DpTileEngine(identifier),
          m_pTDRIBufMem(NULL),
          m_TDRIBufPA(0),
          m_pTDRIBufVA(0),
          m_TDRITileID(0),
          m_pTileInfo(NULL),
          m_pTdrFlag(NULL),
          m_pIrqFlag(NULL),
          m_pTileDesc(NULL),
          m_pTileFunc(NULL),
          m_pVECI(NULL),
          m_pMDPCrop(NULL),
          m_pISPCrop(NULL),
          //m_ISPPassType(ISP_DRV_DIP_CQ_THRE0),
          m_dumpIndex(0),
          m_onlyWPE(false),
          m_wpeDataSize(0)
    {
        for (int i = 0; i < WPE_TILE_FUNC_NUM; i++)
        {
            m_pWPEDMA[i] = NULL;
        }
        DPLOGI("DpEngineISP: initialize ISP object\n");

#if CONFIG_FOR_VERIFY_FPGA
        m_pTDRIBufMem = DpMemory::Factory(DP_MEMORY_ION, -1, MAX_ISP_TILE_TDR_HEX_NO_WPE);
        assert(NULL != m_pTDRIBufMem);
#endif // CONFIG_FOR_VERIFY_FPGA

        memset(m_TDRITileLabel, -1, sizeof(m_TDRITileLabel));

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

    virtual ~DpWrapper_WPE()
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

    inline void setWPEOnly(bool only)
    {
        m_onlyWPE = only;
        if (only)
        {
            DPLOGD("DpWpe: WPE only\n");
        }
    }

    inline bool isWPEOnly()
    {
        return m_onlyWPE;
    }

    DP_STATUS_ENUM initTilePath(TILE_PARAM_STRUCT *p_tile_param);

private:

    DpMemory                     *m_pTDRIBufMem;
    uint32_t                     m_TDRIBufPA;
    uint32_t                     *m_pTDRIBufVA;
    uint32_t                     m_TDRITileID;
    int32_t                      m_TDRITileLabel[MAX_TILE_TOT_NO];

    ISP_TPIPE_INFORMATION_STRUCT *m_pTileInfo;
    uint32_t                     *m_pTdrFlag;
    uint32_t                     *m_pIrqFlag;
    ISP_TPIPE_DESCRIPTOR_STRUCT  *m_pTileDesc;
    FUNC_DESCRIPTION_STRUCT      *m_pTileFunc;
    TILE_FUNC_BLOCK_STRUCT       *m_pVECI;
    TILE_FUNC_BLOCK_STRUCT       *m_pMDPCrop;
    TILE_FUNC_BLOCK_STRUCT       *m_pISPCrop;
    TILE_FUNC_BLOCK_STRUCT       *m_pWPEDMA[WPE_TILE_FUNC_NUM];

    //E_ISP_DIP_CQ                 m_ISPPassType;
    ISP_TPIPE_CONFIG_STRUCT      m_ISPConfig;
    uint32_t                     m_dumpIndex;
    bool                         m_onlyWPE;
    uint32_t                     m_wpeDataSize;

    //DP_STATUS_ENUM setupFrameInfo(STREAM_TYPE_ENUM  scenario,
    //                             DpColorFormat     colorFormat)
    //{
       // return DP_STATUS_RETURN_SUCCESS;
    //}

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onReconfigTiles(DpCommand&);

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    DP_STATUS_ENUM onUpdatePMQOS(uint32_t&, uint32_t&, struct timeval&);

    DP_STATUS_ENUM onFlushBuffer(FLUSH_TYPE_ENUM);

    int64_t onQueryFeature()
    {
        return eRMEM | eISP;
    }
};

#endif  // __DP_WRAPPER_WPE_H__
