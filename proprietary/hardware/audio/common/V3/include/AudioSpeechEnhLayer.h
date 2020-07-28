
#ifndef _AUDIO_SPEECH_ENH_LAYER_H
#define _AUDIO_SPEECH_ENH_LAYER_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include <system/audio.h>
#include <hardware/audio.h>
#include <utils/SortedVector.h>
#include "AudioType.h"
#include <AudioLock.h>

extern "C" {
#include "enh_api.h"
}

namespace android {
/*
typedef unsigned short uWord16;
typedef signed short Word16;
typedef signed long Word32;
typedef unsigned long uWord32;

typedef struct {

    uWord32 enhance_pars[28];
//  uWord32 error_flag;
    Word32 App_table;
    Word32 Fea_Cfg_table;
    Word32 MIC_DG;
    Word32 sample_rate;
    Word32 frame_rate;
    Word32 MMI_ctrl;
    Word32 RCV_DG;      // for VoIP, 0xE3D
    Word16 DMNR_cal_data[76];
    Word16 Compen_filter[270];
//  Word16 ne_out[960];
//    Word16 fe_out[960];
    Word16 PCM_buffer[1920];
    Word16 EPL_buffer[4160];
    Word32 Device_mode; //routing path
    Word32 MMI_MIC_GAIN;    //Uplink path total gain
    Word32 Near_end_vad;
    Word32 *SCH_mem; // caster to (SCH_mem_struct*) in every alloc function
} SPH_ENH_ctrl_struct;

*/

#define EnhanceModeParasExtNum 32
#define EnhanceParasNum  (28+EnhanceModeParasExtNum)
#define DMNRCalDataNum  76
#define CompenFilterNum  270

#define Rec48KUL2BufStartAddr  960
#define Rec16KUL2BufStartAddr  320

#define RecBufSize48K20ms  960
#define RecBufSize48K10ms  480
#define RecBufSize48K5ms  240
#define RecBufSize48K3ms  144
#define RecBufSize48K2ms   96
#define RecBufSize48K1ms   48



#define RecBufSize16K20ms  320
#define RecBufSize16K10ms  160
#define RecBufSize16K5ms  80
#define RecBufSize16K3ms  48
#define RecBufSize16K2ms  32
#define RecBufSize16K1ms  16




#define EPLBufSize 4800
#define MaxVMSize   1922    //960*2 +2 for 48K
#define VMAGC1  3847
#define VMAGC2  3848

#define VM_DUMP_FILE_NAME_SIZE (128)


enum SPE_MMI_CONTROL_TABLE {
    UL_MUTE     = (1 << 0), // Bit 0, UL mute
    NORMAL_DMNR = (1 << 1), // Bit 1, normal DMNR
    VCE         = (1 << 2), // Bit 2, VCE
    BWE         = (1 << 3), // Bit 3, BWE
    UL_NR       = (1 << 4), // Bit 4, UL NR
    DL_NR       = (1 << 5), // Bit 5, DL NR
    HANDSFREE_DMNR = (1 << 6), // Bit 6, handsfree mode  DMNR
    HANDSFREE_DMNR_ASR = (1 << 15), // Bit 15, handsfree mode  ASR DMNR
    MMI_CONTROL_MASK_ALL       = 0xFFFFFFFF
};

typedef enum {
    DMNR_DISABLE = 0,
    DMNR_NORMAL = 1,
    DMNR_HANDSFREE
} DMNR_TYPE;

typedef enum {
    SPE_MODE_NONE = 0,
    SPE_MODE_REC = 1,
    SPE_MODE_VOIP = 2,
    SPE_MODE_AECREC
} SPE_MODE;


typedef enum {
    SPE_STATE_IDLE = 0,
    SPE_STATE_START = 1,
    SPE_STATE_RUNNING = 2,
    SPE_STATE_CLEANING
} SPE_STATE;


typedef enum {
    NB_VOIP = 0x1,
    WB_VOIP = 0x2,
    MONO_RECORD = 0x4,
    STEREO_RECORD = 0x8,
    SPEECH_RECOGNITION = 0x10,
    MONO_AEC_RECORD = 0x20,
    STEREO_AEC_RECORD = 0x40,
    LOW_LATENCY_RECORD = 0x80
} SPE_APP_TABLE;

/*
typedef enum
{
   AGC = 0x1,
   AEC = 0x2,
   DMNR = 0x4
} SPE_FEA_TABLE;
*/

typedef enum {
    UPLINK = 0,
    DOWNLINK
} SPE_DATA_DIRECTION;

typedef enum {
    ROUTE_NONE  = -1,
    ROUTE_NORMAL    = 0,
    ROUTE_HEADSET   = 1,
    ROUTE_SPEAKER   = 2,
    ROUTE_BT        = 3,
    ROUTE_EARPHONE   = 4,
    ROUTE_BT_NSEC_OFF_PATH  = 5,
    ROUTE_MAX  = 6
} SPE_ROUTE;

typedef struct _SphEnhOps {
    void    *handle;
    Word32(*ENH_API_Get_Memory)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
    Word16(*ENH_API_Alloc)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word32 *mem_ptr);
    Word16(*ENH_API_Rst)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
    void (*ENH_API_Process)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
    Word16(*ENH_API_Free)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
    Word16(*ENH_API_Get_Version)(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
} SphEnhOps;

struct BufferInfo {
    short *pBufBase;
    int BufLen;
    short *pRead;   //short
    short *pWrite;  //short
    int BufLen4Delay;
    short *pRead4Delay; //short
    short *pWrite4Delay;    //short
    bool DLfirstBuf;
    struct timespec time_stamp_queued;      //buffer queue time
    struct timespec time_stamp_estimate;    //estimate buffer consume start time
    struct timespec time_stamp_process;
};


struct InBufferInfo {
    short *pBufBase;
    uint32_t BufLen;
    struct timespec time_stamp_queued;      //buffer queue time
    bool bHasRemainInfo;
    struct timespec time_stamp_predict;     //predict time output from hardware
};

struct StreamAttribute {
    int mSampleRate;
    int mChannels;
    int mBufferSize;
};

class SPELayer {
public:
    SPELayer();
    ~SPELayer();

    ////can remove below parameter setting function+++
    bool    SetEnhPara(SPE_MODE mode, uWord32 *pEnhance_pars);
    bool    SetDMNRPara(SPE_MODE mode, Word16 *pDMNR_cal_data);
    bool    SetCompFilter(SPE_MODE mode, Word16 *pCompen_filter);
    bool    SetMICDigitalGain(SPE_MODE mode, Word32 gain);
    bool    SetUpLinkTotalGain(SPE_MODE mode, uint8_t gain);
    ////can remove below parameter setting function---
    bool    SetSampleRate(SPE_MODE mode, long sample_rate);
    bool    SetFrameRate(SPE_MODE mode, long frame_rate);
    bool    SetAPPTable(SPE_MODE mode, SPE_APP_TABLE App_table);
    bool    SetFeaCfgTable(SPE_MODE mode, Word32 Fea_Cfg_table);
    bool    SetPlatfromTimeOffset(int ms);
    bool    SetChannel(int channel);
    //      bool    SetMode(SPE_MODE mode);
    bool    SetRoute(SPE_ROUTE route);
    bool GetUPlinkIntrStartTime(void);
    bool SetUPLinkIntrStartTime(struct timespec UPlinkStartTime);
    void SetUPLinkDropTime(uint32_t droptime);
    void SetDownLinkLatencyTime(uint32_t latencytime);

    bool GetDownlinkIntrStartTime(void);
    bool    SetDynamicFuncCtrl(const SPE_MMI_CONTROL_TABLE func, const bool enable);

    bool    Start(SPE_MODE mode);
    int    Process(InBufferInfo *InBufinfo);
    bool    Stop();
    bool    Standby();


    int GetLatencyTime();
    SPE_MODE    GetMode();
    SPE_ROUTE   GetRoute();
    long    GetSampleRate(SPE_MODE mode);
    long    GetFrameRate(SPE_MODE mode);
    Word32    GetMICDigitalGain(SPE_MODE mode);
    Word32    GetAPPTable(SPE_MODE mode);
    Word32    GetFeaCfgTable(SPE_MODE mode);
    int GetChannel();
    bool    GetEnhPara(SPE_MODE mode, uWord32 *pEnhance_pars);
    bool    GetDMNRPara(SPE_MODE mode, Word16 *pDMNR_cal_data);
    bool    GetCompFilter(SPE_MODE mode, Word16 *pCompen_filter);
    bool    IsSPERunning();
    void SetStreamAttribute(bool direct, StreamAttribute SA);   //direct=0 =>downlink stream info, direct=1 =>uplink stream info


    bool MutexLock(void);
    bool MutexUnlock(void);
    bool DumpMutexLock(void);
    bool DumpMutexUnlock(void);
    void dump(void);

    void Dump_Enalbe_Check(void);
    void Dump_PCM_In(SPE_DATA_DIRECTION dir, void *buffer, int bytes);
    void Dump_PCM_Process(SPE_DATA_DIRECTION dir, void *buffer, int bytes);
    void Dump_PCM_Out(SPE_DATA_DIRECTION dir, void *buffer, int bytes);
    void Dump_EPL(void *buffer, int bytes);
    bool CreateDumpThread();
    void DumpBufferClear(void);
    bool HasBufferDump();
    void EPLTransVMDump();
    void SetVMDumpEnable(bool bEnable);
    void SetVMDumpFileName(const char *VMFileName);

    //void WriteReferenceBuffer(short *inBuf, int  inBufLength);
    void WriteReferenceBuffer(struct InBufferInfo *Binfo);
    void SetOutputStreamRunning(bool bRunning, bool bFromOutputStart = 0);
    void EnableNormalModeVoIP(bool bSet);
    void SetEchoRefStartTime(struct timespec EchoRefStartTime);

    Vector<BufferInfo *> mDumpDLInBufferQ, mDumpDLOutBufferQ, mDumpULOutBufferQ, mDumpULInBufferQ, mDumpEPLBufferQ;
#if defined(PC_EMULATION)
    HANDLE hDumpThread;
#else
    pthread_t hDumpThread;
#endif

    FILE *mfpInDL;
    FILE *mfpInUL;
    FILE *mfpOutDL;
    FILE *mfpOutUL;
    FILE *mfpProcessedDL;
    FILE *mfpProcessedUL;
    FILE *mfpEPL;
    FILE *mfpVM;
    static int DumpFileNum;
    static bool EPLDebugEnable;

    AudioLock mDumpExitMutex;

private:

    void    ReStart();
    void    Clear();
    //void AddtoInputBuffer(SPE_DATA_DIRECTION dir, short *inBuf, int  inBufLength, bool prequeue=0);
    void AddtoInputBuffer(SPE_DATA_DIRECTION dir, struct InBufferInfo *BInputInfo, bool prequeue = 0);
    void AddUplinkBuffer(struct InBufferInfo *BInputInfo);
    void AddDownlinkBuffer(struct InBufferInfo *BInputInfo, bool prequeue = 0);
    int    Process_Record(short *inBuf, int  inBufLength);
    int    Process_VoIP(short *inBuf, int  inBufLength);
    void CompensateBuffer(size_t BufLength, struct timespec CompenStartTime);

    bool    WaitforDownlinkData(void);
    bool    InsertDownlinkData(void);

    bool PrepareProcessData(void);
    int GetVoIPJitterTime(void);
    int GetVoIPLatencyTime(void);
    void FlushBufferQ(void);
    void CalPreQNumber(void);
    void CalPrepareCount(void);
    void BypassDLBuffer(void);
    void ReSync(void);

    //inBufLength is time2 buffer's Sample frame count
    unsigned long long TimeStampDiff(BufferInfo *BufInfo1, BufferInfo *BufInfo2);
    bool TimeStampCompare(BufferInfo *BufInfo1, BufferInfo *BufInfo2, bool Endtime);
    bool TimeCompare(struct timespec time1, struct timespec time2);
    unsigned long long TimeDifference(struct timespec time1, struct timespec time2);

    void DropDownlinkData(uint32_t dropsample);

    SphEnhOps mSphEnhOps;
    SPE_MODE mMode;
    SPE_ROUTE   mRoute;
    SPE_STATE mState;
    Word32 mMMI_ctrl_mask;
    //Record settings
    Word32 mRecordSampleRate;
    Word32 mRecordFrameRate;
    Word32 mRecordMICDigitalGain;   //MIC_DG
    Word32 mRecordULTotalGain;  //uplink total gain
    uWord32 mRecordEnhanceParas[EnhanceParasNum];
    Word16 mRecordDMNRCalData[DMNRCalDataNum];
    Word16 mRecordCompenFilter[CompenFilterNum];
    Word32 mRecordApp_table;
    Word32 mRecordFea_Cfg_table;

    //VoIP settings
    int mLatencyTime;
    int mPlatformOffsetTime;
    int mLatencySampleCount;
    Word32 mVoIPSampleRate;
    Word32 mVoIPFrameRate;
    Word32 mVoIPMICDigitalGain; //MIC_DG
    Word32 mVoIPULTotalGain;    //uplink total gain
    uWord32 mVoIPEnhanceParas[EnhanceParasNum];
    Word16 mVoIPDMNRCalData[DMNRCalDataNum];
    Word16 mVoIPCompenFilter[CompenFilterNum];
    Word32 mVoIPApp_table;
    Word32 mVoIPFea_Cfg_table;
    bool    mNeedDelayLatency;
    bool mLatencyDir;

    bool mNeedJitterBuffer;
    int mJitterBufferTime;
    int mDefaultJitterBufferTime;
    int mJitterSampleCount;

    struct timespec mUplinkIntrStartTime;
    struct timespec mDownlinkIntrStartTime;
    struct timespec mPreUplinkEstTime;
    struct timespec mPreDownlinkEstTime;
    struct timespec mPreDownlinkQueueTime;

    uint32_t mULDropTime;
    uint32_t mDLLatencyTime;
    bool mFirstVoIPUplink;
    bool mFirstVoIPDownlink;
    int mPreULBufLen;
    int mPreDLBufLen;
    bool mDLNewStart;
    bool mPrepareProcessDataReady;
    int mDLPreQnum;
    bool mDLPreQLimit;
    unsigned long long mNsecPerSample;
    struct timespec mULIntrDeltaTime;

    SPH_ENH_ctrl_struct mSph_Enh_ctrl;
    int *mSphCtrlBuffer;
    int mSPEProcessBufSize;
    /*      BufferInfo *mpSPEBufferUL1;
            BufferInfo *mpSPEBufferUL2;
            BufferInfo *mpSPEBufferDL;
            BufferInfo *mpSPEBufferDLDelay;
    */
    short *mpSPEBufferUL1;
    short *mpSPEBufferUL2;
    short *mpSPEBufferDL;
    short *mpSPEBufferDLDelay;
    short *mpSPEBufferNE;
    short *mpSPEBufferFE;

    int mULInBufQLenTotal, mDLInBufQLenTotal, mULOutBufQLenTotal, mDLOutBufQLenTotal, mDLDelayBufQLenTotal;

    Vector<BufferInfo *> mDLInBufferQ, mDLOutBufferQ, mULOutBufferQ, mULInBufferQ, mDLDelayBufferQ;
    //Vector<BufferInfo>  mULInBufferQ;

    Mutex mLock, mDumpLock, mBufMutexWantLock;
    bool mError;
    bool mVoIPRunningbefore;

    AudioLock mBufMutex;

    bool mOutputStreamRunning;
    bool mNormalModeVoIP;
    size_t mCompensatedBufferSize;
    int DLdataPrepareCount;
    StreamAttribute mDLStreamAttribute;
    bool mNewReferenceBufferComes;

    //VM
    Word16 mVM[MaxVMSize];
    bool mVMDumpEnable;
    char mVMDumpFileName[VM_DUMP_FILE_NAME_SIZE];
};
}
#endif
