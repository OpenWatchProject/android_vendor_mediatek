#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_hdr.h"
#include "mmsys_config.h"
#include "DpDataType.h"
#include "tile_mdp_reg.h"
#include "DpPlatform.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"
#endif

#if CONFIG_FOR_OS_ANDROID
#include "PQSessionManager.h"
#include "PQAlgorithmFactory.h"
#endif
#define  HDR_CONFIG_FRAME_THRES (1)

//------------------------------------------------------------
// Dummy HDR0 driver engine
//-------------------------------------------------------------
class DpEngine_HDR: public DpTileEngine
{
public:
    DpEngine_HDR(uint32_t identifier)
        : DpTileEngine(identifier),
          m_prevPABufferIndex(0),
          m_prevReadbackCount(0),
          m_cropWidth(0),
          m_outHistXLeft(0),
          m_lastOutHorizontal(0),
          m_firstTile(0),
          m_isRelay(m_data.m_isRelay)
    {
        memset(m_regLabel, -1, sizeof(m_regLabel));
        m_pData = &m_data;
    }

    ~DpEngine_HDR()
    {
    }

private:
    MDP_HDR_DATA   m_data;
    int32_t        m_regLabel[MAX_NUM_READBACK_PA_BUFFER];

    uint32_t       m_prevPABufferIndex;
    uint32_t       m_prevReadbackCount;

    int32_t      m_cropWidth;
    int32_t      m_outHistXLeft;
    int32_t      m_lastOutHorizontal;

    uint32_t     m_firstTile;

    bool         &m_isRelay;

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onReconfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onPostProc(DpCommand &command);

    DP_STATUS_ENUM onAdvanceTile(DpCommand&, struct TILE_PARAM_STRUCT*);

    int64_t onQueryFeature()
    {
        return eHDR;
    }

    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);
};

// register factory function
static DpEngineBase* HDR0Factory(DpEngineType type)
{
    if (tHDR0 == type)
    {
        return new DpEngine_HDR(0);
    }
    return NULL;
};

#ifdef tHDR1
static DpEngineBase* HDR1Factory(DpEngineType type)
{
    if (tHDR1 == type)
    {
        return new DpEngine_HDR(1);
    }
    return NULL;
};
#endif

// register factory function
EngineReg HDR0Reg(HDR0Factory);
#ifdef tHDR1
EngineReg HDR1Reg(HDR1Factory);
#endif

DP_STATUS_ENUM DpEngine_HDR::onInitEngine(DpCommand &command)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_HDR::onDeInitEngine(DpCommand &command)
{
    // Disable HDR
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onConfigFrame(DpCommand &command, DpConfig &config)
{
    m_firstTile = 1;
#ifdef HW_SUPPORT_10BIT_PATH
    // 10 bit format
    if (DP_COLOR_GET_10BIT_PACKED(config.outFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.outFormat) ||
        DP_COLOR_GET_10BIT_PACKED(config.inFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.inFormat) ||
        config.scenario == STREAM_ISP_IC ||
        config.scenario == STREAM_ISP_VR ||
        config.scenario == STREAM_ISP_ZSD ||
        config.scenario == STREAM_ISP_IP ||
        config.scenario == STREAM_ISP_VSS ||
        config.scenario == STREAM_ISP_ZSD_SLOW ||
        config.scenario == STREAM_WPE ||
        config.scenario == STREAM_WPE2)
    {
        MM_REG_WRITE(command, MDP_HDR_TOP, 3 << 28, 0x30000000);
    }
    else
    {
        MM_REG_WRITE(command, MDP_HDR_TOP, 1 << 28, 0x30000000);
    }
#endif // HW_SUPPORT_10BIT_PATH

    // if input/crop width < 16, force to relay mode
    if (HDR_MIN_WIDTH > config.inWidth)
    {
        m_outHistXLeft      = 0;
        m_lastOutHorizontal = 0;
        m_isRelay = true;
        m_bypassEngine = true;
        // Set MDP_HDR relay mode
        MM_REG_WRITE(command, MDP_HDR_RELAY, 1, 0x1);
        // Always set MDP_HDR enable to 1
        MM_REG_WRITE(command, MDP_HDR_TOP, 1, 0x1);
        return DP_STATUS_RETURN_SUCCESS;
    }

    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;

    m_isRelay = true;

    DPLOGI("DpEngine_HDR: onConfigFrame");
#ifdef SUPPORT_HDR
    DP_TIMER_GET_CURRENT_TIME(begin);

    if (m_pInDataPort->getHDRStatus())
    {
        PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);
        if (pPQSession != NULL)
        {
            PQHDRAdaptor* pPQHDRAdaptor = PQAlgorithmFactory::getInstance()->getHDR(m_identifier);
            dcHandle* currDC_p = pPQSession->getDCHandle();

            m_isRelay = pPQHDRAdaptor->calRegs(pPQSession, command, config, m_frameConfigLabel, false);
            DPLOGI("DpEngine_HDR: configFrame() m_isRelay = %d\n", m_isRelay);
        }
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

    if (diff > HDR_CONFIG_FRAME_THRES)
    {
        DPLOGD("DpEngine_HDR: configFrame() time %d ms\n", diff);
    }
#endif

    if (m_isRelay == true)
    {
        m_bypassEngine = true;
        // Set MDP_HDR relay mode
        MM_REG_WRITE(command, MDP_HDR_RELAY, 1, 0x1);
    }
    else
    {
        m_bypassEngine = false;
        // Set MDP_HDR not relay mode
        MM_REG_WRITE(command, MDP_HDR_RELAY, 0, 0x1);
    }

    // Always set MDP_HDR enable to 1
    MM_REG_WRITE(command, MDP_HDR_TOP, 1, 0x1);

    m_outHistXLeft      = 0;
    m_lastOutHorizontal = 0;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    // if input/crop width < 16, force to relay mode
    if (HDR_MIN_WIDTH > config.inWidth)
    {
        m_outHistXLeft      = 0;
        m_lastOutHorizontal = 0;
        m_isRelay = true;
        return DP_STATUS_RETURN_SUCCESS;
    }

    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;

    DPLOGI("DpEngine_HDR: onReconfigFrame");
#ifdef SUPPORT_HDR
    DP_TIMER_GET_CURRENT_TIME(begin);

    if (m_pInDataPort->getHDRStatus())
    {
        PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);
        if (pPQSession != NULL)
        {
            PQHDRAdaptor* pPQHDRAdaptor = PQAlgorithmFactory::getInstance()->getHDR(m_identifier);
            dcHandle* currDC_p = pPQSession->getDCHandle();

            m_isRelay = pPQHDRAdaptor->calRegs(pPQSession, command, config, m_frameConfigLabel, true);

            DPLOGI("DpEngine_HDR: reconfigFrame() m_isRelay = %d\n", m_isRelay);
        }
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin, end, diff);

    if (diff > HDR_CONFIG_FRAME_THRES)
    {
        DPLOGD("DpEngine_HDR: reconfigFrame() time %d ms\n", diff);
    }
#endif

    m_outHistXLeft      = 0;
    m_lastOutHorizontal = 0;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onConfigTile(DpCommand &command)
{
    uint32_t HDR_tile_h;
    uint32_t HDR_tile_v;
    uint32_t HDR_hsize;
    uint32_t HDR_vsize;
    uint32_t HDR_out_start_x;
    uint32_t HDR_out_end_x;
    uint32_t HDR_out_end_y;
    uint32_t HDR_hist_left;
    uint32_t HDR_hist_begin_x;
    uint32_t HDR_hist_end_x;
    uint32_t HDR_first_tile;
    uint32_t HDR_last_tile;

    // Set tile position
    HDR_tile_h = m_outTileXLeft;
    HDR_tile_v = m_outTileYTop;
    MM_REG_WRITE(command, MDP_HDR_TILE_POS, HDR_tile_v << 16 |
                                            HDR_tile_h <<  0, 0x1FFF1FFF);

    // Set source size
    HDR_hsize      = m_inTileXRight - m_inTileXLeft + 1;
    HDR_vsize      = m_inTileYBottom - m_inTileYTop + 1;
    MM_REG_WRITE(command, MDP_HDR_SIZE_0, (HDR_vsize << 16) |
                                          (HDR_hsize <<  0), 0x1FFF1FFF);

    // Set crop region
    HDR_out_start_x = m_outTileXLeft - m_inTileXLeft;
    HDR_out_end_x =  m_outTileXRight - m_inTileXLeft;
    HDR_out_end_y = m_inTileYBottom - m_inTileYTop;
    MM_REG_WRITE(command, MDP_HDR_SIZE_1, HDR_out_end_x     << 16 |
                                          HDR_out_start_x   <<  0, 0x1FFF1FFF);
    MM_REG_WRITE(command, MDP_HDR_SIZE_2, HDR_out_end_y << 16 |
                                          0             <<  0, 0x1FFF1FFF);

    // Set histogram window
    HDR_hist_left = (m_outTileXLeft > m_outHistXLeft) ? m_outTileXLeft : m_outHistXLeft;
    HDR_hist_begin_x = HDR_hist_left - m_inTileXLeft;
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_0, HDR_hist_begin_x, 0x00001FFF); // only config x in tile

    HDR_hist_end_x = m_outTileXRight - m_inTileXLeft;
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_1, HDR_hist_end_x, 0x00001FFF); //only config x in tile

    if (m_firstTile == 1)
    {
        HDR_first_tile = 1;
    }
    else
    {
        HDR_first_tile = 0;
    }
    m_firstTile = 0;

    if ((m_outTileXRight + 1) >= m_inFrameWidth)
    {
        HDR_last_tile = 1;
    }
    else
    {
        HDR_last_tile = 0;
    }
    MM_REG_WRITE(command, MDP_HDR_TOP, (HDR_first_tile << 5) |
                                       (HDR_last_tile << 6), 0x00000060);
    MM_REG_WRITE(command, MDP_HDR_HIST_ADDR, (HDR_first_tile << 9), 0x00000200); //enable hist_clr_en

    DPLOGI("DpEngine_HDR: m_cropWidth = %u, m_outTileXRight = %u, m_inTileXRight = %u\n", m_cropWidth, m_outTileXRight, m_inTileXRight);
    DPLOGI("DpEngine_HDR: m_inTileXLeft = %u, m_outTileXLeft = %u\n", m_inTileXLeft, m_outTileXLeft);
    DPLOGI("DpEngine_HDR: source h: %d, v: %d\n", HDR_hsize, HDR_vsize);
    DPLOGI("DpEngine_HDR: HDR_hist_end_x = %u, HDR_hist_begin_x = %u\n", HDR_hist_end_x, HDR_hist_begin_x);
    DPLOGI("DpEngine_HDR: HDR_first_tile = %u, HDR_last_tile = %u\n", HDR_first_tile, HDR_last_tile);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onAdvanceTile(DpCommand&, struct TILE_PARAM_STRUCT*)
{
    // FIXME: check tile order
    if (m_lastOutHorizontal != m_outHorizontal)
    {
        m_outHistXLeft      = m_outHorizontal ? (m_outTileXRight + 1) : 0;
        m_lastOutHorizontal = m_outHorizontal;
    }

    DPLOGI("DpEngine_HDR: m_outHistXLeft %d\n", m_outHistXLeft);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onPostProc(DpCommand &command)
{
    if (command.getSyncMode()) //old version pq readback must remove
    {
        DPLOGI("DpEngine_HDR::onPostProc : SyncMode do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(hdr_readback))
    {
        DPLOGI("DpEngine_HDR::onPostProc : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_HDR::onPostProc : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }
    MM_REG_READ_BEGIN(command);

#ifdef SUPPORT_HDR
    if (hdr_readback)
    {
        DPLOGI("DpEngine_HDR::onPostProc:hdr_readback\n");

        for (int i = 0; i < 57; i++)
        {
            MM_REG_READ(command, MDP_HDR0_BASE + 0xC8, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
            counter++;
        }
        MM_REG_READ(command, MDP_HDR0_BASE + 0xE8, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
        counter++;
    }
#endif

    MM_REG_READ_END(command);

    command.setNumReadbackPABuffer(counter, tHDR0);

    m_prevPABufferIndex = readbackPABufferIndex;
    m_prevReadbackCount= counter;

    DPLOGI("DpEngine_HDR::onPostProc: counter:%d\n", counter);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_HDR::onReconfigTiles(DpCommand &command)
{
    if (command.getSyncMode()) //old version pq readback must remove
    {
        DPLOGI("DpEngine_HDR::onReconfigTiles : SyncMode do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(hdr_readback))
    {
        DPLOGI("DpEngine_HDR::onReconfigTiles : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);
    command.setNumReadbackPABuffer(m_prevReadbackCount, tHDR0);
    if (readbackPABufferIndex == m_prevPABufferIndex)
    {
        DPLOGI("DpEngine_HDR::onReconfigTiles : PABufferIndex no change do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_HDR::onReconfigTiles : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }

#ifdef SUPPORT_HDR
    if (hdr_readback)
    {
        DPLOGI("DpEngine_HDR::onReconfigTiles:hdr_readback\n");

        for (int i = 0; i < 57; i++)
        {
            MM_REG_READ(command, MDP_HDR0_BASE + 0xC8, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
            counter++;
        }
        MM_REG_READ(command, MDP_HDR0_BASE + 0xE8, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
        counter++;
    }
#endif

    m_prevPABufferIndex = readbackPABufferIndex;

    return DP_STATUS_RETURN_SUCCESS;
}
