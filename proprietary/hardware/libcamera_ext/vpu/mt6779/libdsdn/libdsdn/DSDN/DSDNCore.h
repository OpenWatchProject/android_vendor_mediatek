#ifndef _DSDNCORE_H
#define _DSDNCORE_H
#include "MTKDSDN.h"
#include <vpu.h>                        // interface for vpu stream

#define PC_DUMP_PATH            "./bit_true_test/"
#define PHONE_DUMP_PATH         "/sdcard/DSDN/"

//debug Level 0: no buffer dump 1: dump output 2: dump input + output 3: dump in + out + intermediate
#define DSDN_DEBUG_LEVEL     (0)
#define DSDN_LOG_LEVEL       (0)
#define DSDN_VPU_LOW_OPPSTEP            (9)
#define DSDN_VPU_HIGH_OPPSTEP           (5)
#define DSDN_VPU_OPPSTEP_THRESHOLD      (6291456)       // 6M
#define DSDN_VPU_FPS_THRESHOLD          (30)            // 30FPS

#define DSDN_MAX_TUNING   (256)


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(ANDROID)
#include <sys/time.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <pthread.h>
#define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PROPERTY_VALUE_MAX PROP_VALUE_MAX
#else
#define LOGD(fmt, ...) printf("[%s] " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[%s] Error(%d) " fmt, LOG_TAG, __LINE__, ##__VA_ARGS__)
#define PROPERTY_VALUE_MAX (255)
#define __system_property_set(...) (0)
#define __system_property_get(...) (0)
#endif

typedef struct
{
    MUINT32 xScale; 
    MUINT32 yScale; 
    MUINT32 xScale_uv; 
    MUINT32 yScale_uv; 
    MUINT32 xShift; 
    MUINT32 yShift; 
    MUINT32 xShift_uv; 
    MUINT32 yShift_uv; 
    MUINT32 format;         /* small image, NV12 = 0, NV21 = 1*/
    MUINT32 Bformat;        /* big image,   NV12 = 0, NV21 = 1*/ 
}DSDN_VPU_PROPERTY;

class DSDNCore
{
public:
    int mLogLevel;

    DSDNCore(void);
    ~DSDNCore(void);

    int getBufferSizeNeeded(void);
    MRESULT initialize(DSDNInitInfo* pInitInfo);
    MRESULT reset(void);
    MRESULT setupBuffer(DSDNInitInfo* initInfo);
    MRESULT setProcInfo(DSDNProcInfo* procInfo);
    MRESULT doDSDNMain(void);

private:
    int debugLevel;
    char   m_dumpPath[64];
    DSDNInitInfo initInfo;
    DSDNTuningInfo tuningInfo;
    DSDNProcInfo procInfo;
    DSDNBufferInfo* bufInfoIn1;
    DSDNBufferInfo* bufInfoIn2;
    DSDNBufferInfo* bufInfoIn3;
    DSDNBufferInfo* bufInfoOut1;

    MUINT32 bufferSizeNeeded;
    MUINT32 workingBufSize;
    MUINT8* workingBufAddr;

    int profiling[15];      //for porfiling

    MUINT32 dsdnMode;
    MUINT32 maxWidth;
    MUINT32 maxHeight;
    MUINT32 dstWidth;
    MUINT32 dstHeight;
    MUINT32 srcWidth;
    MUINT32 srcHeight;
    MUINT32 currentFPS;

    //vpu related data structure
    VpuStream  *vpu;
    VpuAlgo    *algo;
    VpuRequest *request;
    MINT32    forcedVpuOppStep;
    MUINT32   vpuOppStep;
    MUINT32   finalVpuOppStep;
    

    int calBufferSizeNeeded(DSDNInitInfo* pInitInfo);
    void initializeTuningParam(DSDNInitInfo* pInitInfo);
    void initializeParseTuningParam(DSDNTuningInfo* pTuningInfo);
    MRESULT printBufferInfo(DSDNBufferInfo* bufferInfo);
    MRESULT initializeVPU(void);
    void setupVPUBuffer(unsigned int portId, VpuBuffer* buf, DSDNBufferInfo* bufInfo);
    void setupVPUProperty(DSDN_VPU_PROPERTY* property);
    MUINT32 getVpuOppStep(void);
};

#endif

