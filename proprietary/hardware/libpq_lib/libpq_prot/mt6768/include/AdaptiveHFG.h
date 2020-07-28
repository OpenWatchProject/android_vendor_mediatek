#ifndef __ADAPTIVEHFG_H__
#define __ADAPTIVEHFG_H__

#include "feature_custom_hfg_nvram.h"
#include "stdlib.h"
#include "PQDSImpl.h"
#include "feature_Adaptive.h"
#include "feature_cmparam.h"

#if 0
MVOID SmoothHFG(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_HFG_T const& rUpperHFG,   // HFG settings for upper ISO
                ISP_NVRAM_HFG_T const& rLowerHFG,   // HFG settings for lower ISO
                ISP_NVRAM_HFG_T& rSmoothHFG);   // Output
#endif

struct TAdaptiveHFG_Exif{
    int HFG_ISO;
    int HFG_LV;
};

struct TAdaptiveHFG_Dump{
    TAdaptiveHFG_Exif adaptiveHFG_Exif;
};

struct TAdaptiveHFGReg {
    int Enabled;
    int AdaptiveMethod; //Not Define yet
    int DebugFlag;
    int DebugTrace;
};

class TAdaptiveHFG
{
private:
    CPQDSFW *PQDSFW;
    HFGInput_frame *Prev_HFGInputFrame;
    HFGOutput_frame *Prev_HFGOutputFrame;

    int ApplyNvramTuningRegisterToHFGFrameInput(FEATURE_NVRAM_HFG_TUNINGREG_T *TuningReg, HFGInput_frame *HFGInputFrame);
    int GetCustomParameters(uint32_t RealISO, uint32_t UpperISO, uint32_t LowerISO, const void *UpperHFG, const void *LowerHFG, HFGInput_frame *HFGInputFrame);
    int AdaptiveRegister(const void *SLKInfo, HFGInput_frame *HFGInputFrame);
    int DumpInfo(int DumpCondition, char* DumbBuffer, int& Count, int MaxEXIFSize);
public:
    TAdaptiveHFG(CPQDSFW *InputPQDSFW);
    ~TAdaptiveHFG();
    TAdaptiveHFGReg *AdaptiveHFGReg;
    TAdaptiveHFG_Exif *AdaptiveHFG_exif;
    TAdaptiveHFG_Dump *AdaptiveHFG_dump;
    int onCalculateTile(const HFGInput_tile *HFGInputTile, HFGOutput_tile *HFGOutputTile);
    int onCalculateFrame(uint32_t RealISO,  // Real ISO
        uint32_t UpperISO, // Upper ISO
        uint32_t LowerISO, // Lower ISO
        const void *UpperHFG,   // HFG settings for upper ISO
        const void *LowerHFG,   // HFG settings for lower ISO
        const void *SLKInfo,    // HFG SLK Information
        HFGInput_frame *HFGInputFrame,
        HFGOutput_frame *HFGOutputFrame,
        const unsigned int ispTuningFlag = 0
    );
    int DumpExif(char* EXIFBuffer, int& EXIFCount, int MaxEXIFSize);
    int DumpMDP(char* MDPDumpBuffer, int& MDPCount, int MaxMDPSize);
};


#endif
