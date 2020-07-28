#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_ccorr.h"
#include "mmsys_config.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"
#endif

//------------------------------------------------------------
// Dummy CCORR0 driver engine
//-------------------------------------------------------------
class DpEngine_CCORR: public DpTileEngine
{
public:
    DpEngine_CCORR(uint32_t identifier)
        : DpTileEngine(identifier)
    {
    }

    ~DpEngine_CCORR()
    {
    }

private:
    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    int64_t onQueryFeature()
    {
        return eCCORR;
    }

};

// register factory function
static DpEngineBase* CCORR0Factory(DpEngineType type)
{
    if (tCCORR0 == type)
    {
        return new DpEngine_CCORR(0);
    }
    return NULL;
};

// register factory function
EngineReg CCORR0Reg(CCORR0Factory);

DP_STATUS_ENUM DpEngine_CCORR::onInitEngine(DpCommand &command)
{
    //CCORR enable
    MM_REG_WRITE(command, MDP_CCORR_EN, 0x1, 0x1);
    command.addMetLog("CCORR__DISP_CCORR_EN", 1);

    //Relay mode
    MM_REG_WRITE(command, MDP_CCORR_CFG, 0x1, 0x1);
    command.addMetLog("CCORR__MDP_DISP_CCORR_CFG", 1);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_CCORR::onDeInitEngine(DpCommand &command)
{
    // Disable CCORR

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_CCORR::onConfigFrame(DpCommand &command, DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_CCORR::onConfigTile(DpCommand &command)
{
    uint32_t CCORR_hsize;
    uint32_t CCORR_vsize;

    CCORR_hsize      = m_inTileXRight   - m_inTileXLeft + 1;
    CCORR_vsize      = m_inTileYBottom  - m_inTileYTop  + 1;

    MM_REG_WRITE(command, MDP_CCORR_SIZE, (CCORR_hsize << 16) +
                                          (CCORR_vsize <<  0), 0x1FFF1FFF);

    return DP_STATUS_RETURN_SUCCESS;
}
