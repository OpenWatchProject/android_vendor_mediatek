#ifndef __PQDCADAPTOR_H__
#define __PQDCADAPTOR_H__

#include "PQMutex.h"
#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "PQSessionManager.h"
#include "PQDCImpl.h"
#include "PQAlgorithmAdaptor.h"

#define ADL_RESULT_OK   (0)
#define ADL_ID_IMAGE    (0xFFFF)

#define DC_ENGINE_MAX_NUM (2)

class PQDCAdaptor: public PQAlgorithmAdaptor
{
public:
    PQDCAdaptor(uint32_t identifier);
    ~PQDCAdaptor();

    static PQDCAdaptor* getInstance(uint32_t identifier);
    static void destroyInstance();
    bool calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config, int32_t *pFrameConfigLabel, bool LumaCurveOnly = false);

private:
    void onCalcADL(PQSession* pPQSession, DpCommand &command, DpConfig &config, int32_t enableFlag, int32_t *pFrameConfigLabel, bool LumaCurveOnly = false);
    bool isADLVideoIDValid(int32_t adl_id);
    bool isADLEnable(int32_t enableFlag, int32_t scenario);
    void ADL_writeLumaCurve(DpCommand &command, const ADLOutput &output, int32_t *pFrameConfigLabel);
    void ADL_updateLumaCurve(DpCommand &command, const ADLOutput &output, int32_t *pFrameConfigLabel);
    int32_t onCalculate(PQSession* pPQSession, int32_t LmtFullSwitch, int32_t width, int32_t height, bool enable, ADLOutput *output);
    int32_t onCalculateImage(PQSession* pPQSession, int32_t LmtFullSwitch, const DpImageParam &param, ADLOutput *output);
    void tuningDCInput(ADLInput *input, int32_t scenario);
    bool tuningDCOutput(ADLOutput *output, int32_t scenario);
    void tuningDCSWReg(ADLReg* SWReg, int32_t scenario);
    void checkAndResetUnusedADLFW(PQSession* pPQSession, dcHandle* currDC_p, uint64_t id);
    bool getDCTable();

public:
    enum ADL_ENABLE_FLAG_ENUM
    {
        ADL_ENABLE_FLAG_OFF = (0),
        ADL_ENABLE_FLAG_ON = (1),
        ADL_ENABLE_FLAG_ON_DISABLE = (2)
    };

private:
    static PQDCAdaptor *s_pInstance[DC_ENGINE_MAX_NUM];
    static PQMutex  s_ALMutex;
    uint32_t m_identifier;
    DpMutex  m_mutex;
    bool     m_isSWregBufferInit;
    CPQDCFW* m_pADLFW;
    uint32_t m_debugFlag;
};
#endif //__PQDCADAPTOR_H__
