#include "DpEngineBase.h"
#include "DpEngineType.h"
#include "DpTileScaler.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"

inline DP_STATUS_ENUM DpEngineBase::configFrameISP(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameRead(DpCommand &command, DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameRingBuf(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameScale(DpConfig &config)
{
    if (m_identifier == 0)
    {
        config.pEngine_cfg = &cfg.MDP_PRZ0_en;
    }
    else if (m_identifier == 1)
    {
        config.pEngine_cfg = &cfg.MDP_PRZ1_en;
    }
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameSharpness(DpConfig &config)
{
    config.pEngine_cfg = &cfg.MDP_TDSHP0_tds_en;
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameColor(DpConfig &config)
{
    config.pEngine_cfg = &cfg.MDP_COLOR0_color_en;
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameWrite(DpCommand &command, DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

#endif // CONFIG_FOR_VERIFY_FPGA
