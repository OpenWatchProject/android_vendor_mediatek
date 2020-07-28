#ifndef __PQCOLORADAPTOR_H__
#define __PQCOLORADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSession.h"
#include "PQAlgorithmAdaptor.h"

#define COLOR_ENGINE_MAX_NUM (1)

class PQColorAdaptor: public PQAlgorithmAdaptor
{
public:
    PQColorAdaptor(uint32_t identifier);
    ~PQColorAdaptor();

    static PQColorAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    void calRegs(PQSession* pPQSession, DpCommand &command);

private:
    void onConfigLumaEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command, int32_t scenario);
    void onConfigSatEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command);
    void onConfigHueEngine(COLOR_CONFIG_T* colorConfig, DpCommand &command);
    void onConfigLocal3DLUT(COLOR_CONFIG_T* colorConfig, DpCommand &command);
    bool tuningColorOutput(COLOR_CONFIG_T *output, int32_t scenario);

private:
    static PQColorAdaptor *s_pInstance[COLOR_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    uint32_t m_identifier;
    COLOR_CONFIG_T *m_colorConfig;
};

#endif //__PQCOLORADAPTOR_H__
