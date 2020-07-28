#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_fg.h"
#include "mmsys_config.h"
#include "DpDataType.h"
#include "tile_mdp_reg.h"
#include "DpPlatform.h"


//------------------------------------------------------------
// Dummy FG0 driver engine
//-------------------------------------------------------------
class DpEngine_FG: public DpTileEngine
{
public:
    DpEngine_FG(uint32_t identifier)
        : DpTileEngine(identifier)
    {
        m_pData = &m_data;
    }

    ~DpEngine_FG()
    {
    }

private:
    MDP_FG_DATA   m_data;

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
        return eFG;
    }

    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);
};

#ifdef tFG0
// register factory function
static DpEngineBase* FG0Factory(DpEngineType type)
{
    if (tFG0 == type)
    {
        return new DpEngine_FG(0);
    }
    return NULL;
}
#endif

#ifdef tFG1
// register factory function
static DpEngineBase* FG1Factory(DpEngineType type)
{
    if(tFG1 == type)
    {
        return new DpEngine_FG(1);
    }
    return NULL;
}
#endif
#ifdef tFG0
// register factory function
EngineReg FG0Reg(FG0Factory);
#endif
#ifdef tFG1
EngineReg FG1Reg(FG1Factory);
#endif

DP_STATUS_ENUM DpEngine_FG::onInitEngine(DpCommand &command)
{
    
    DPLOGI("DpEngine_FG::onInitEngine relay mode\n");
    MM_REG_WRITE(command, MDP_FG_TRIGGER, (0x00000001<<2) 
                                        , 0x0000002);
    MM_REG_WRITE(command, MDP_FG_TRIGGER, (0x00000000) 
                                        , 0x0000002);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_FG::onDeInitEngine(DpCommand &command)
{
    // Disable FG
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onConfigFrame(DpCommand &command, DpConfig &config)
{
    DPLOGI("DpEngine_FG::onConfigFrame  relay mode\n");
    MM_REG_WRITE(command, MDP_FG_FG_CTRL_0, (0x1), 0x1);
    MM_REG_WRITE(command, MDP_FG_FG_CK_EN, 0x7, 0x7);
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onConfigTile(DpCommand &command)
{
    DPLOGI("DpEngine_FG::onConfigTile  relay mode\n");
    uint32_t src_w = m_inTileXRight - m_inTileXLeft + 1;
    uint32_t src_h = m_inTileYBottom - m_inTileYTop + 1;
    MM_REG_WRITE(command, MDP_FG_TILE_INFO_0, (m_inTileXLeft & 0xFFFF)+ 
                                              (src_w         & 0xFFFF) << 16
                                              , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_FG_TILE_INFO_1, (m_inTileYTop& 0xFFFF)+ 
                                              (src_h         & 0xFFFF) << 16
                                              , 0xFFFFFFFF);
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onAdvanceTile(DpCommand&, struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onPostProc(DpCommand &command)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_FG::onReconfigTiles(DpCommand &command)
{
    return DP_STATUS_RETURN_SUCCESS;
}
