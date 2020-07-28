#ifndef __PQHFGCONFIG_H__
#define __PQHFGCONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

typedef struct{
    uint32_t ENABLE;
    uint32_t ispTuningFlag;
} HFG_CONFIG_T;

class PQHFGConfig
{
public:
    PQHFGConfig();
    ~PQHFGConfig();

    int32_t isEnabled(void);
    bool getHFGConfig(HFG_CONFIG_T* HFGConfig);

private:
    uint32_t getIspTuningFlag(void);
};
#endif //__PQHFGCONFIG_H__
