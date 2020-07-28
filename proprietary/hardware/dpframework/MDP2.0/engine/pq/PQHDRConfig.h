#ifndef __PQHDRCONFIG_H__
#define __PQHDRCONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

typedef struct{
    unsigned char ENABLE;
    unsigned char CCORR_ENABLE;
    uint32_t debugFlag;
    uint32_t externalPanelNits;
    uint32_t driverDebugFlag;
} HDR_CONFIG_T;

class PQHDRConfig
{
public:
    PQHDRConfig();
    ~PQHDRConfig();

    uint32_t isEnabled(void);
    uint32_t isCcorrEnabled(void);
    bool getHDRConfig(HDR_CONFIG_T* HDRConfig);

private:
    uint32_t getDebugFlag(void);
    uint32_t getExternalPanelNits(void);
    uint32_t getDriverDebugFlag(void);

private:
    int32_t m_HDRSupport;
};
#endif //__PQHDRCONFIG_H__
