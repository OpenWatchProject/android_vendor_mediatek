#include "DpTileEngine.h"
#include "DpEngineType.h"

//------------------------------------------------------------
// Dummy PATH_SOUT0/1 driver engine
//-------------------------------------------------------------
class DpEngine_MDPSOUT: public DpTileEngine
{
public:
    DpEngine_MDPSOUT(uint32_t identifier)
        : DpTileEngine(identifier)
    {
    }

    ~DpEngine_MDPSOUT()
    {
    }

private:
    int64_t onQueryFeature()
    {
        return eMDPSOUT;
    }

};

// register factory function
static DpEngineBase* PATH_SOUT0Factory(DpEngineType type)
{
    if (tPATH0_SOUT == type)
    {
        return new DpEngine_MDPSOUT(0);
    }
    return NULL;
};

// register factory function
static DpEngineBase* PATH_SOUT1Factory(DpEngineType type)
{
    if (tPATH1_SOUT == type)
    {
        return new DpEngine_MDPSOUT(1);
    }
    return NULL;
};

// register factory function
EngineReg PATH0_SOUTReg(PATH_SOUT0Factory);
EngineReg PATH1_SOUTReg(PATH_SOUT1Factory);
