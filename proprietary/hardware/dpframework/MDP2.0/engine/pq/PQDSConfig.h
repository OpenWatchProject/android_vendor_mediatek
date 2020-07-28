#ifndef __PQDSCONFIG_H__
#define __PQDSCONFIG_H__

#include "PQMutex.h"
#include "cust_color.h"
#include "cust_tdshp.h"
#include "PQConfig.h"

typedef struct{

    unsigned int  TDSHP[THSHP_PARAM_MAX];
    unsigned char ENABLE;
    unsigned char DS_ENABLE;
    unsigned char ISO_ENABLE;
    unsigned char UR_ENABLE;
    unsigned int  ispTuningFlag;
} DS_CONFIG_T;

class PQDSConfig
{
public:
    PQDSConfig();
    ~PQDSConfig();

    int32_t isEnabled(GlobalPQParam &globalPQParam);
    int32_t isDShpEnabled(void);
    int32_t isISOShpEnabled(void);
    int32_t isUREnabled(void);
    bool getDSConfig(DS_CONFIG_T** DSConfig, int32_t scenarioi, GlobalPQParam &globalPQParam);

private:
    bool initTDSHPTable(void);
    bool composeDSParameter(int32_t scenario, GlobalPQParam &globalPQParam);
    uint32_t getIspTuningFlag(void);

public:
    static DISPLAY_TDSHP_T      m_TDSHPTable;
    static bool                 m_TDSHPTableInit;

private:
    bool                m_DSParameterInit;
    DISP_PQ_PARAM       m_TDSHPTableIndex; //seperate tdshp index from pq index in the future
    DS_CONFIG_T         m_DSConfig;
    uint32_t            m_PQParameterSN;
};
#endif //__PQDSCONFIG_H__
