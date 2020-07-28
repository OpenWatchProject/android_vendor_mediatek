#ifndef __PQHFGADAPTOR_H__
#define __PQHFGADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSession.h"
#include "PQDSImpl.h"
#include "AdaptiveHFG.h"
#include "PQAlgorithmAdaptor.h"

#define HFG_ENGINE_MAX_NUM (2)

enum HFG_DEBUG_FLAG {
    HFG_DUMP_TILE = 1 << 0,
};

class PQHFGAdaptor: public PQAlgorithmAdaptor
{
public:
    PQHFGAdaptor(uint32_t identifier);
    ~PQHFGAdaptor();

    static PQHFGAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    bool calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config);
    bool calTileRegs(PQSession* pPQSession, DpCommand &command,
        const uint32_t tileWidth, const uint32_t tileHeight,
        const uint32_t tileXOffset, const uint32_t tileYOffset);

private:
    void tuningHFGInput(HFGInput_frame *input, int32_t scenario);
    bool tuningHFGOutput(HFGOutput_frame *output, int32_t scenario);
    void debugDumpTileReg(const HFGInput_tile *inTileParam, const HFGOutput_tile *outTileParam);
    void dumpFWReg(DpPqParam *PQParam, HFGInput_frame *input, HFGOutput_frame *output,
    TAdaptiveHFG_Dump *adaptiveHFG_Dump, TAdaptiveHFG_Exif *adaptiveHFG_Exif);
    void dumpFwRegInFw(DpPqParam *PQParam);

public:
    enum HFG_CONFIG_ENUM
    {
        HFG_CONFIG_NORMAL = 0,
        HFG_CONFIG_TUNING = 1,
        HFG_CONFIG_INIT_FAIL = 2,
        HFG_CONFIG_PQSERVICE_NOT_READY = 3
    };

private:
    static PQHFGAdaptor *s_pInstance[HFG_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    TAdaptiveHFG    *m_pAdaptiveHFGFW;
    uint32_t        m_identifier;
    uint32_t        m_ispPqDebug;
    HFG_CONFIG_T     m_HFGConfig;
};

#endif //__PQHFGADAPTOR_H__