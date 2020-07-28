#ifndef __PQISPTUNING_H__
#define __PQISPTUNING_H__

#include "PQMutex.h"
#include "AdaptiveCALTM.h"
#include "PQRszImpl.h"
#include "PQDSImpl.h"

class PQIspTuning
{
public:
    PQIspTuning();
    ~PQIspTuning();

    static PQIspTuning* getInstance();
    static void destroyInstance();

    bool loadIspTuningFile(void);
    bool getIspTuningFileStatus(void);
    void getDreTuningValue(DREToolReg *appliedDreReg, TAdaptiveCALTMReg *appliedAdaptiveCALTM, TAdaptiveCALTMFace_Dump *appliedAdaptiveCALTMFace_dump);
    void getRszTuningValue(RszReg *appliedRszReg, RszInput *appliedRszInput);
    void getDSTuningValue(DSReg *appliedDSReg, DSInput *appliedDSInput);
    void getHFGTuningValue(HFGInput_frame *appliedHFGInput);

private:
    bool getTuningValue(const char *str, uint32_t &tuningAddress, uint32_t &tuningValue);

    static PQIspTuning *s_pInstance;
    static PQMutex  s_ALMutex;

    bool m_loadFileSuccess;
    DREToolReg *m_pTuningDreReg;
    TAdaptiveCALTMReg *m_pTuningAdaptiveCaltmReg;
    TAdaptiveCALTMFace_Dump *m_pTuningAdaptiveCaltmFaceReg;
    RszReg *m_pTuningRszReg;
    DSReg *m_pTuningDSReg;
    RszInput *m_pTuningRszInput;
    DSInput *m_pTuningDSInput;
    HFGInput_frame *m_pTuningHFGInput;
};
#endif //__PQISPTUNING_H__
