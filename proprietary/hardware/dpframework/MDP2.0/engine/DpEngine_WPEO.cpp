#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpWrapper_WPE.h"
#include "mmsys_reg_base.h"

//--------------------------------------------------------
// IMG2O driver engine
//--------------------------------------------------------
class DpEngine_WPEO: public DpTileEngine
{
public:
    DpEngine_WPEO(uint32_t identifier)
        : DpTileEngine(identifier),
          m_pWPEWrapper(0)
    {
        // Because this is the dummy engine
        //m_outputDisable = true;
        //m_tileEngGroup = TILE_GROUP_CROP_EN;
    }

    ~DpEngine_WPEO()
    {
    }

private:
    DpWrapper_WPE *m_pWPEWrapper;

    DP_STATUS_ENUM onDumpDebugInfo()
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onConfigFrame(DpCommand &, DpConfig &);

    DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT*);

    int64_t onQueryFeature()
    {
        return eWMEM | eISP;
    }
};

// register factory function
static DpEngineBase* WPEOFactory(DpEngineType type)
{
    if (tWPEO == type)
    {
        return new DpEngine_WPEO(0);
    }
    return NULL;
};

// register factory function
EngineReg WPEOReg(WPEOFactory);

#ifdef tWPEO2
// register factory function
static DpEngineBase* WPEO2Factory(DpEngineType type)
{
    if (tWPEO2 == type)
    {
        return new DpEngine_WPEO(1);
    }
    return NULL;
};

// register factory function
EngineReg WPEO2Reg(WPEO2Factory);
#endif

DP_STATUS_ENUM DpEngine_WPEO::onConfigFrame(DpCommand &, DpConfig &)
{
    m_pWPEWrapper = static_cast<DpWrapper_WPE*>(getPrevEngine());
    assert(0 != m_pWPEWrapper);

    m_pWPEWrapper->setWPEOnly(true);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WPEO::onInitTileCalc(struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WPEO::onRetrieveTileParam(struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}
