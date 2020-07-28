#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1

#include "DpDataType.h"
#include "PQLogger.h"
#include <PQReadBackFactory.h>

PQReadBackFactory* PQReadBackFactory::s_pInstance = NULL;
PQMutex   PQReadBackFactory::s_ALMutex;

PQReadBackFactory* PQReadBackFactory::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQReadBackFactory();
        atexit(PQReadBackFactory::destroyInstance);
    }

    return s_pInstance;
}

void PQReadBackFactory::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQReadBackFactory::PQReadBackFactory()
{
    PQ_LOGD("[PQReadBackFactory] PQReadBackFactory()... ");

};

PQReadBackFactory::~PQReadBackFactory()
{
    PQ_LOGD("[PQReadBackFactory] ~PQReadBackFactory()... ");
};

#ifdef SUPPORT_DRE
PQDREHistogramAdaptor* PQReadBackFactory::getDRE(uint32_t identifier)
{
    return PQDREHistogramAdaptor::getInstance(identifier);
}
#endif