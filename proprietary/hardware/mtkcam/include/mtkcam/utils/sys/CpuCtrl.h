#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_CPUCTRL_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_CPUCTRL_H_

#include <mtkcam/def/common.h>

/******************************************************************************
 *
 ******************************************************************************/
class CpuCtrl
{
    protected:
        virtual ~CpuCtrl() {};
    //
    public:
        typedef struct
        {
            MINT bigCore;
            MINT bigFreq;
            MINT littleCore;
            MINT littleFreq;
            MINT timeout;
        }CPU_CTRL_INFO;
        //
        typedef struct
        {
            MINT bigCoreMin;
            MINT littleCoreMin;
            //
            MINT bigCoreMax;
            MINT littleCoreMax;
            //
            MINT bigFreqMin;
            MINT littleFreqMin;
            //
            MINT bigFreqMax;
            MINT littleFreqMax;
            //
            MINT timeout;
        }CPU_CTRL_RANGE_INFO;
        //
        static CpuCtrl* createInstance(void);
        virtual MVOID   destroyInstance(void) = 0;
        virtual MBOOL   init(void) = 0;
        virtual MBOOL   uninit(void) = 0;
        virtual MBOOL   enable(CPU_CTRL_INFO &cpuInfo) = 0;
        virtual MBOOL   enable(CPU_CTRL_RANGE_INFO &cpuRangeInfo) = 0;
        virtual MBOOL   enableBoost(MINT timeout) = 0;
        virtual MBOOL   enablePreferIdle(void) = 0;
        virtual MBOOL   disable(void) = 0;
        virtual MBOOL   cpuPerformanceMode(MINT timeOut) = 0;
        virtual MBOOL   resetTimeout(MINT timeOut) = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_CPUCTRL_H_
