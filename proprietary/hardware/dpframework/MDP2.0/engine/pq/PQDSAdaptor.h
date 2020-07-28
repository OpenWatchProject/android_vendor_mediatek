#ifndef __PQDSADAPTOR_H__
#define __PQDSADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSession.h"
#include "PQDSImpl.h"
#include "PQAlgorithmAdaptor.h"

#define DS_ENGINE_MAX_NUM (2)

class PQDSAdaptor: public PQAlgorithmAdaptor
{
public:
    PQDSAdaptor(uint32_t identifier);
    ~PQDSAdaptor();

    static PQDSAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    bool calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config);

private:
    void onConfigTDSHP(PQSession* pPQSession, DpCommand &command, DpConfig &config, bool enable);
    void onCalcTDSHP(PQSession* pPQSession, DpCommand &command, DpConfig &config, bool enable);
    void ConfigDSParamByScenario(DSInput *inParam, DpPqParam *PQParam);
    void initDSFWparam(DS_CONFIG_T *dsConfig, DpConfig &config, DSInput *inParam, bool enable,
                        DpPqParam *PQParam);
    void translateToDSTuningSW(void);
    void translateFromDSTuningSW(void);
    void onCalculate(const DSInput *input, DSOutput *output);
    void tuningDSInput(DSInput *input, int32_t scenario);
    bool tuningDSOutput(DSOutput *output, int32_t scenario);
    void tuningDSSWReg(int32_t scenario);
    bool getVideoSourceSize(DSInput *inParam, DpPqParam *PQParam);
    void dumpFWReg(DpPqParam *PQParam, DSInput *input, DSOutput *output, DSReg *swReg);
    void dumpFwRegInFw(DpPqParam *PQParam, DSInput *input, DSOutput *output, DSReg *swReg);
    bool getDSTable();

public:
    enum TDSHP_CONFIG_ENUM
    {
        TDSHP_CONFIG_NORMAL = 0,
        TDSHP_CONFIG_TUNING = 1,
        TDSHP_CONFIG_INIT_FAIL = 2,
        TDSHP_CONFIG_PQSERVICE_NOT_READY = 3
    };

private:
    static PQDSAdaptor *s_pInstance[DS_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    CPQDSFW*        m_pDSFW;
    uint32_t        m_identifier;
    uint32_t        m_ispPqDebug;
    DS_CONFIG_T     *m_dsConfig;
};

#endif //__PQDSADAPTOR_H__