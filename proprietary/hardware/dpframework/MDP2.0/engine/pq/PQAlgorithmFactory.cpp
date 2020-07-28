#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQAlgorithmFactory.h>

PQAlgorithmFactory* PQAlgorithmFactory::s_pInstance = NULL;
PQMutex   PQAlgorithmFactory::s_ALMutex;

PQAlgorithmFactory* PQAlgorithmFactory::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQAlgorithmFactory();
        atexit(PQAlgorithmFactory::destroyInstance);
    }

    return s_pInstance;
}

void PQAlgorithmFactory::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQAlgorithmFactory::PQAlgorithmFactory()
{
    PQ_LOGD("[PQAlgorithmFactory] PQAlgorithmFactory()... ");


};

PQAlgorithmFactory::~PQAlgorithmFactory()
{
    PQ_LOGD("[PQAlgorithmFactory] ~PQAlgorithmFactory()... ");
};

PQDSAdaptor* PQAlgorithmFactory::getDynamicSharpness(uint32_t identifier)
{
    return PQDSAdaptor::getInstance(identifier);
}

PQDCAdaptor* PQAlgorithmFactory::getDynamicContrast(uint32_t identifier)
{
    return PQDCAdaptor::getInstance(identifier);
}

#ifdef MDP_COLOR_ENABLE
PQColorAdaptor* PQAlgorithmFactory::getColor(uint32_t identifier)
{
    return PQColorAdaptor::getInstance(identifier);
}
#endif

#ifdef RSZ_2_0
PQRSZAdaptor* PQAlgorithmFactory::getRSZ(uint32_t identifier)
{
    return PQRSZAdaptor::getInstance(identifier);
}
#endif

#ifdef SUPPORT_HDR
PQHDRAdaptor* PQAlgorithmFactory::getHDR(uint32_t identifier)
{
    return PQHDRAdaptor::getInstance(identifier);
}
#endif

#if 0
#ifdef SUPPORT_CCORR
PQCcorrAdaptor* PQAlgorithmFactory::getCcorr(uint32_t identifier)
{
    return PQCcorrAdaptor::getInstance(identifier);
}
#endif
#endif

#ifdef SUPPORT_DRE
PQDREAdaptor* PQAlgorithmFactory::getDRE(uint32_t identifier)
{
    return PQDREAdaptor::getInstance(identifier);
}
#endif

#ifdef SUPPORT_HFG
PQHFGAdaptor* PQAlgorithmFactory::getHFG(uint32_t identifier)
{
    return PQHFGAdaptor::getInstance(identifier);
}
#endif