#ifndef __PQREADBACKMFACTORY_H__
#define __PQREADBACKMFACTORY_H__

#include "PQMutex.h"
#ifdef SUPPORT_DRE
#include "PQDREHistogramAdaptor.h"
#endif

class PQReadBackFactory
{
public:
    PQReadBackFactory();
    ~PQReadBackFactory();

    static PQReadBackFactory* getInstance();
    static void destroyInstance();

#ifdef SUPPORT_DRE
    PQDREHistogramAdaptor* getDRE(uint32_t identifier);
#endif

private:
    static PQReadBackFactory *s_pInstance;
    static PQMutex  s_ALMutex;
};
#endif //__PQREADBACKMFACTORY_H__