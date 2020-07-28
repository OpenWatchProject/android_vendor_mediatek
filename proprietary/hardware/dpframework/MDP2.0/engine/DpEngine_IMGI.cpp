#include "DpWrapper_ISP.h"

//--------------------------------------------------------
// IMGI driver engine
//--------------------------------------------------------
class DpEngine_IMGI: public DpWrapper_ISP
{
public:
    DpEngine_IMGI()
    {
    }

    ~DpEngine_IMGI()
    {
    }
};


// register factory function
static DpEngineBase* IMGIFactory(DpEngineType type)
{
    if (tIMGI == type)
    {
        return new DpEngine_IMGI();
    }
    return NULL;
};

// register factory function
EngineReg IMGIReg(IMGIFactory);
