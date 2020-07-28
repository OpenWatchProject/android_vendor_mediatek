#ifndef __PQDRECONFIG_H__
#define __PQDRECONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

struct DRE_CONFIG_T {
    uint32_t ENABLE; // CALTM enable
    uint32_t VP_ENABLE; // SCLTM enable
    uint32_t HDR_VP_ENABLE; // HDR AAL enable
    uint32_t debugFlag;
    uint32_t adaptiveCaltmFlag;
    uint32_t driverDebugFlag;
    uint32_t demoWinX;
    uint32_t driverBLKFlag;
    uint32_t ispTuningFlag;
};

class PQDREConfig
{
public:
    PQDREConfig();
    ~PQDREConfig();

    bool getDREConfig(DRE_CONFIG_T* DREConfig);

private:
    uint32_t isEnabled(void);
    uint32_t isVPEnabled(void);
    uint32_t isHDRVPEnabled(void);
    uint32_t getDebugFlag(void);
    uint32_t getAdaptiveCaltmFlag(void);
    uint32_t getDriverDebugFlag(void);
    uint32_t getDemoWindowX(void);
    uint32_t getDriverBlockFlag(void);
    uint32_t getIspTuningFlag(void);

private:
    PQMutex s_ALMutex;
};
#endif //__PQDRECONFIG_H__
