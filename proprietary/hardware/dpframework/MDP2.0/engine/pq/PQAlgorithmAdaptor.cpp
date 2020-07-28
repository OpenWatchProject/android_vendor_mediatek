#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQAlgorithmAdaptor.h>

PQAlgorithmAdaptor::PQAlgorithmAdaptor(ProxyTuningBuffer swreg, ProxyTuningBuffer input, ProxyTuningBuffer output)
{
    PQ_LOGD("[PQAlgorithmAdaptor] PQAlgorithmAdaptor()... ");
    m_swRegBuffer = new PQTuningBuffer(swreg);
    m_inputBuffer = new PQTuningBuffer(input);
    m_outputBuffer = new PQTuningBuffer(output);
    m_ReadOnlyBuffer = NULL;
};

PQAlgorithmAdaptor::PQAlgorithmAdaptor(ProxyTuningBuffer swreg, ProxyTuningBuffer input, ProxyTuningBuffer output, ProxyTuningBuffer readReg)
{
    PQ_LOGD("[PQAlgorithmAdaptor] PQAlgorithmAdaptor()... ");
    m_swRegBuffer = new PQTuningBuffer(swreg);
    m_inputBuffer = new PQTuningBuffer(input);
    m_outputBuffer = new PQTuningBuffer(output);
    m_ReadOnlyBuffer = new PQTuningBuffer(readReg);
};

PQAlgorithmAdaptor::~PQAlgorithmAdaptor()
{
    PQ_LOGD("[PQAlgorithmAdaptor] ~PQAlgorithmAdaptor()... ");
    delete m_swRegBuffer;
    delete m_inputBuffer;
    delete m_outputBuffer;
    if(m_ReadOnlyBuffer != NULL)
        delete m_ReadOnlyBuffer;
};

