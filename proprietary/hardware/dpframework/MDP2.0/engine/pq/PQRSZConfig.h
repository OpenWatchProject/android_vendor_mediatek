#ifndef __PQRSZCONFIG_H__
#define __PQRSZCONFIG_H__

#include "PQMutex.h"
#include "PQConfig.h"

typedef struct{
    unsigned char UR_ENABLE;
    unsigned int  ispTuningFlag;
} RSZ_CONFIG_T;

class PQRSZConfig
{
public:
    PQRSZConfig();
    ~PQRSZConfig();

    int32_t isUREnabled(void);
    bool getRSZConfig(RSZ_CONFIG_T* RSZConfig);

private:
    uint32_t getIspTuningFlag(void);
};
#endif //__PQRSZCONFIG_H__
