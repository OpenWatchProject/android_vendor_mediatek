#ifndef __PQDCCONFIG_H__
#define __PQDCCONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

typedef struct{
    unsigned char ENABLE;
    uint32_t debugFlag;
} DC_CONFIG_T;

class PQDCConfig
{
public:
    PQDCConfig();
    ~PQDCConfig();

    bool isEnabled(GlobalPQParam &globalPQParam);
    bool getDCConfig(DC_CONFIG_T* DCConfig, GlobalPQParam &globalPQParam);

private:
    uint32_t getDebugFlag(void);

private:
    PQMutex s_ALMutex;
};
#endif //__PQDCCONFIG_H__
