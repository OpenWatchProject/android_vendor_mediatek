#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpWrapper_ISP.h"
#include "mmsys_reg_base.h"

//--------------------------------------------------------
// IMG2O driver engine
//--------------------------------------------------------
class DpEngine_IMG2O: public DpTileEngine
{
public:
    DpEngine_IMG2O()
        : DpTileEngine(0),
          m_pISPWrapper(0)
    {
        // Because this is the dummy engine
        //m_outputDisable = true;
        //m_tileEngGroup = TILE_GROUP_CROP_EN;
    }

    ~DpEngine_IMG2O()
    {
    }

private:
    DpWrapper_ISP *m_pISPWrapper;

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
static DpEngineBase* IMG2OFactory(DpEngineType type)
{
    if (tIMG2O == type)
    {
        return new DpEngine_IMG2O();
    }
    return NULL;
};

// register factory function
EngineReg IMG2OReg(IMG2OFactory);


DP_STATUS_ENUM DpEngine_IMG2O::onConfigFrame(DpCommand &, DpConfig &)
{
    m_pISPWrapper = static_cast<DpWrapper_ISP*>(getPrevEngine());
    assert(0 != m_pISPWrapper);

    m_pISPWrapper->setISPOnly(true);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_IMG2O::onInitTileCalc(struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_IMG2O::onRetrieveTileParam(struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}
