#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "mdp_reg_tcc.h"
#include "mmsys_config.h"
#include "DpDataType.h"
#include "tile_mdp_reg.h"
#include "DpPlatform.h"

//------------------------------------------------------------
// Dummy TCC0 driver engine
//-------------------------------------------------------------
class DpEngine_TCC: public DpTileEngine
{
public:
    DpEngine_TCC(uint32_t identifier)
        : DpTileEngine(identifier)
    {
        m_pData = &m_data;
    }

    ~DpEngine_TCC()
    {
    }

private:
    MDP_TCC_DATA   m_data;

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
        return eTCC;
    }

    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);
};

#ifdef tTCC0
// register factory function
static DpEngineBase* TCC0Factory(DpEngineType type)
{
    if (tTCC0 == type)
    {
        return new DpEngine_TCC(0);
    }
    return NULL;
}
#endif
#ifdef tTCC1
static DpEngineBase* TCC1Factory(DpEngineType type)
{
    if (tTCC1 == type)
    {
        return new DpEngine_TCC(1);
    }
    return NULL;
}
#endif
#ifdef tTCC2
static DpEngineBase* TCC2Factory(DpEngineType type)
{
    if (tTCC2 == type)
    {
        return new DpEngine_TCC(2);
    }
    return NULL;
}
#endif
#ifdef tTCC3
static DpEngineBase* TCC3Factory(DpEngineType type)
{
    if (tTCC3 == type)
    {
        return new DpEngine_TCC(3);
    }
    return NULL;
}

#endif
// register factory function
#ifdef tTCC0
EngineReg TCC0Reg(TCC0Factory);
#endif
#ifdef tTCC1
EngineReg TCC1Reg(TCC1Factory);
#endif
#ifdef tTCC2
EngineReg TCC2Reg(TCC2Factory);
#endif
#ifdef tTCC3
EngineReg TCC3Reg(TCC3Factory);
#endif

DP_STATUS_ENUM DpEngine_TCC::onInitEngine(DpCommand &command)
{
    DPLOGI("DpEngine_TCC::onInitEngine relay mode\n");
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_TCC::onDeInitEngine(DpCommand &command)
{
    // Disable TCC
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onConfigFrame(DpCommand &command, DpConfig &config)
{
    DPLOGI("DpEngine_TCC::onConfigFrame relay mode\n");
    //reset engine
/*    MM_REG_WRITE(command, MDP_TCC_CTRL, (0x00000001<<30) +
                                        (0x1           )
                                        , 0x40000001);
    MM_REG_WRITE(command, MDP_TCC_CTRL, (0x00000000<<30) +
                                        (0x1           )
                                        , 0x40000001);
*/
    //enable engine
    MM_REG_WRITE(command, MDP_TCC_CTRL, 0x00000001, 0x00000001);
    //enable relay mode
    MM_REG_WRITE(command, MDP_TCC_CTRL, (0x01<<29), 0x20000000);
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onConfigTile(DpCommand &command)
{
    uint32_t src_w = m_inTileXRight - m_inTileXLeft + 1;
    uint32_t src_h = m_inTileYBottom - m_inTileYTop + 1;
    DPLOGI("DpEngine_TCC::onConfigTile 0x%08x relay mode, %d, %d, 0x%08x\n",
            MDP_TCC_CTRL, src_w, src_h,
            (0x01<<29) + ((src_h & 0x3FFF) << 15) + ((src_w & 0x3FFF) << 1) + (0x1));
    DPLOGI("DpEngine_TCC::onConfigTile  relay mode, %d, %d\n", src_w, src_h);
    MM_REG_WRITE(command, MDP_TCC_CTRL, ((src_h & 0x3FFF) << 15) +
                                        ((src_w & 0x3FFF) << 1)
                                        , 0x1FFFFFFE);
        DPLOGI("DpEngine_TCC::onConfigTile end\n");
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onAdvanceTile(DpCommand&, struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onPostProc(DpCommand &command)
{

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_TCC::onReconfigTiles(DpCommand &command)
{
    return DP_STATUS_RETURN_SUCCESS;
}
