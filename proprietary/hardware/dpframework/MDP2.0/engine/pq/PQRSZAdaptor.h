#ifndef __PQRSZADAPTOR_H__
#define __PQRSZADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSessionManager.h"
#include "PQRszImpl.h"
#include "PQAlgorithmAdaptor.h"

#define RSZ_ENGINE_MAX_NUM (3)

class PQRSZAdaptor:  public PQAlgorithmAdaptor
{
public:
    PQRSZAdaptor(uint32_t identifier);
    ~PQRSZAdaptor();

    static PQRSZAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    bool calRegs(PQSession* pPQSession, DpCommand &command,
                     RszInput* inParam, RszOutput* outParam);
    void dumpFWReg(DpPqParam *PQParam, RszInput *input, RszOutput *output, RszReg *swReg);
    void dumpFwRegInFw(DpPqParam *PQParam, RszInput *input, RszOutput *output, RszReg *swReg);

private:
    void tuningRSZInput(RszInput *input, int32_t scenario);
    bool tuningRSZOutput(RszOutput *output, int32_t scenario);
    void tuningRSZSWReg(int32_t scenario);
    bool getRSZTable();

private:
    static PQRSZAdaptor *s_pInstance[RSZ_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    CPQRszFW*       m_pRszFW;
    uint32_t        m_identifier;
};
#endif //__PQRSZADAPTOR_H__
