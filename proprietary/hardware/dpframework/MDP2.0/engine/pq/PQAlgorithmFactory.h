#ifndef __PQALGORITHMFACTORY_H__
#define __PQALGORITHMFACTORY_H__

#include "PQMutex.h"
#include "PQDCAdaptor.h"
#include "PQDSAdaptor.h"
#include "PQColorAdaptor.h"
#include "PQRSZAdaptor.h"
#ifdef SUPPORT_HDR
#include "PQHDRAdaptor.h"
#endif
#if 0
#ifdef SUPPORT_CCORR
#include "PQCcorrAdaptor.h"
#endif
#endif
#ifdef SUPPORT_DRE
#include "PQDREAdaptor.h"
#endif
#ifdef SUPPORT_HFG
#include "PQHFGAdaptor.h"
#endif

class PQAlgorithmFactory
{
public:
    PQAlgorithmFactory();
    ~PQAlgorithmFactory();

    static PQAlgorithmFactory* getInstance();
    static void destroyInstance();
    PQDSAdaptor* getDynamicSharpness(uint32_t identifier);
    PQDCAdaptor* getDynamicContrast(uint32_t identifier);

#ifdef MDP_COLOR_ENABLE
    PQColorAdaptor* getColor(uint32_t identifier);
#endif

#ifdef RSZ_2_0
    PQRSZAdaptor* getRSZ(uint32_t identifier);
#endif

#ifdef SUPPORT_HDR
    PQHDRAdaptor* getHDR(uint32_t identifier);
#endif

#if 0
#ifdef SUPPORT_CCORR
    PQCcorrAdaptor* getCcorr(uint32_t identifier);
#endif
#endif

#ifdef SUPPORT_DRE
    PQDREAdaptor* getDRE(uint32_t identifier);
#endif

#ifdef SUPPORT_HFG
    PQHFGAdaptor* getHFG(uint32_t identifier);
#endif

private:
    static PQAlgorithmFactory *s_pInstance;
    static PQMutex  s_ALMutex;
};
#endif //__PQALGORITHMFACTORY_H__