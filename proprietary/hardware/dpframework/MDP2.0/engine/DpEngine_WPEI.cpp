#include "DpWrapper_WPE.h"

//--------------------------------------------------------
// IMGI driver engine
//--------------------------------------------------------
class DpEngine_WPEI: public DpWrapper_WPE
{
public:
    DpEngine_WPEI(uint32_t identifier)
        :DpWrapper_WPE(identifier)
    {
    }

    ~DpEngine_WPEI()
    {
    }
};


// register factory function
static DpEngineBase* WPEIFactory(DpEngineType type)
{
    if (tWPEI == type)
    {
        return new DpEngine_WPEI(0);
    }
    return NULL;
};

// register factory function
EngineReg WPEIReg(WPEIFactory);

#ifdef tWPEI2
// register factory function
static DpEngineBase* WPEI2Factory(DpEngineType type)
{
    if (tWPEI2 == type)
    {
        return new DpEngine_WPEI(1);
    }
    return NULL;
};

// register factory function
EngineReg WPEI2Reg(WPEI2Factory);
#endif
