#ifndef __FLASHALG_H__
#define __FLASHALG_H__

#define FLASHALG_H_REVISION    7521001

#include <dbg_aaa_param.h>
#include "dbg_flash_param.h"

namespace NS3A
{
    struct FlashAlgStrobeProfile
    {
    public:
        float iso;
        int exp;
        float pfEng;
        float distance;
        int dutyNum;
        int stepNum;
        int dutyTickNum;
        int stepTickNum;
        int* dutyTick;
        int* stepTick;
        float* engTab;

        //int dutyNumL;
        //int* dutyTickL;

        AWB_GAIN_T *pFlWbCalTbl;
    };

    struct FlashAlgStrobeLim
    {
        int minStep;
        int maxStep;
        int minDuty;
        int maxDuty;
    };

    struct FlashAlgExpPara
    {
        int exp;
        int iso;
        int isFlash;
        int step;
        int duty;

        //int dutyL;  // modify for dual-flash
    };

    struct FlashAlgStaData
    {
        int row;
        int col;
        int dig_row;
        int dig_col;
        short* data;
        double normalizeFactor;
        int bit;
    };

    struct FlashAlgPLineNode
    {
        int iso;
        int exp;
    };

    typedef struct	evSetting
    {
        MUINT32 u4Eposuretime;   //!<: Exposure time in ms
        MUINT32 u4AfeGain;       //!<: raw gain
        MUINT32 u4IspGain;       //!<: sensor gain
        MUINT8  uIris;           //!<: Iris
        MUINT8  uSensorMode;     //!<: sensor mode
        MUINT8  uFlag;           //!<: flag to indicate hysteresis ...
                                 //    MUINT8  uLV;                        //!<: LV avlue , in ISO 100 condition  LV=TV+AV
    }evSetting;


    typedef struct	PLine
    {
        MUINT32		u4TotalIndex;      //preview table Tatal index
        MINT32		i4StrobeTrigerBV;  // Strobe triger point in strobe auto mode
        MINT32		i4MaxBV;
        MINT32		i4MinBV;
        evSetting	*pCurrentTable;   //point to current table	
    }PLine;


    enum
    {
        FlashAlg_Err_Ok = 0,
        FlashAlg_Err_NoMem = -100,
        FlashAlg_Err_Div0 = -101,
        FlashAlg_Err_Para = -102,
        FlashAlg_Err_Other = -103,
        FlashAlg_Err_CaliDataNotSet = -104,
        FlashAlg_Stage_Af = 1,
        FlashAlg_Stage_Capture = 0,
    };

    typedef enum
    {
       FLASH_CAMERA_REAR_SENSOR,
       FLASH_CAMERA_FRONT_SENSOR,
       FLASH_CAMERA_DEFAULT_SENSOR,
    } FLASH_CAMERA_TYPE;

    typedef struct
    {
        AWB_GAIN_T NonPreferencePrvAWBGain;
        AWB_GAIN_T PreferencePrvAWBGain;
        MINT32 i4AWBCCT;
        AWB_ROTATION_MATRIX_T AwbRotationMatrix;
        MINT32 i4RotationMatrixUnit; // Rotation matrix unit
    }FlashAlgInputInfoStruct;

    typedef struct
    {
        double LumaY;    // outpur for flash AE on NVRAM
        double R;
        double G;
        double B;
        int Rgain;
        int Ggain;
        int Bgain;
        int duty;    // X
        int dutyLT; // Y, low color temporature
    }CalData;

    typedef struct
    {
        int duty;
        int dutyLT; // Y, low color temporature
    }ChooseResult;

    // CFG_Mask_4_cfgFlashPolicy
    enum
    {
        CFG_FL_PF_CONV = 0, // 0:default, 1:smooth
        CFG_FL_PF_CYC = 2, // 0:default, 1:smooth
        CFG_FL_EstDecEv = 4, // 0:default, 1:yDecTarget	
        CFG_FL_HYB = 6, // 0:default, 1:Force, 2:Auto	
    };

    typedef struct {
        int x1;
        int y1;
        int x2;
        int y2;
    }Position;

    typedef struct {
        Position facePosition;
        float weight;
        double num;
        double sum;
        double mean;
        int inValid;
    }BlockInfo;

    typedef struct {
        BlockInfo block[9];
        int faceFlashSt;
        int cnt;
        double sumWeight;
        double yMainFace;
        double yAvgFace;
        int inValid;
    }FlashAlgFacePos;

    enum
    {
        FACE_FLASH = 0, // 0: disable, 1:enable
        MAIN_FACE_Y = 1, // decided by Main Face (Big Face)
        AVERAGE_FACE_Y = 2, // decide by Multi-Face 
    };

    typedef enum
    {
       FLASH_SINGLE = 1,
       FLASH_DUAL,
    }FLASH_TYPE;

    class FlashAlg
    {
    public:
        virtual ~FlashAlg();
        static FlashAlg* getInstance(MINT32 const eFlashDev);
        static FlashAlg* getInstanceLite(MINT32 eFlashDev);

        //procedure (must call)
        virtual int init(int flashType) = 0;
        virtual void Estimate(FlashAlgExpPara* exp, FlashAlgFacePos* pFaceInfo, int* isLowRef) = 0;
        virtual int Reset() = 0;
        virtual int ResetIntermediate() = 0;
        virtual int ResetReport() = 0;
        virtual int AddStaData10(FlashAlgStaData* data, FlashAlgExpPara* exp, int* isNeedNext, FlashAlgExpPara* expNext, bool last) = 0;
        virtual int CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp) = 0;
        virtual int setStrobeMaxDutyStep(int PreEquDuty, int PreMaxStep, int CapEquDuty, int CapMaxStep, int PreEquDutyL = -1, int CapEquDutyL = -1) = 0;
        virtual int setStrobeMinDutyStep(int CapMinDuty, int CapMinStep, int CapMinDutyL = -1) = 0;
        virtual int setFlashProfile(FlashAlgStrobeProfile* pr) = 0;
        virtual int setCapturePLine(int num, FlashAlgPLineNode* nodes) = 0;
        virtual int setPreflashPLine(int num, FlashAlgPLineNode* nodes) = 0;

        virtual int setCapturePLine(PLine* p, int isoAtGain1x) = 0;
        virtual int setPreflashPLine(PLine* p, int isoAtGain1x) = 0;

        virtual int setEVComp(float ev_comp) = 0;
        virtual int setEVCompEx(float ev_comp, float tar, float evLevel) = 0;
        virtual float calFlashEng(int duty, int rStep) = 0;
        //checkInputParaError
        //setDebugDataSize

        //strobe cali
        virtual int setCaliData(int caliNum, int* caliStep, int* caliVBat_mV, int* caliIFlash_mA, int refIRange, int extrapIRange) = 0;
        virtual int calStepDuty(int Bat_mV, int peak_mA, int ave_mA, int* step, int* duty) = 0;

        //preference (optional setting)
        virtual int setWTable256(int w, int h, short* tab) = 0;

        virtual int setDefaultPreferences() = 0;
        virtual int setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv) = 0;

        virtual MVOID setFlashInfo(FlashAlgInputInfoStruct *pFlashInfo) = 0;

        //debug
        virtual int setIsSaveSimBinFile(int isSaveBin) = 0;
        virtual int setDebugDir(const char* DirName, const char* PrjName) = 0;
        virtual void getLastErr(int* time, int* type, int* reserve) = 0; //max 3
        virtual void getLastWarning(int* time, int* type, int* reserve) = 0; //max 3
        //virtual void fillDebugData2(void* data) = 0; //500 bytes
        virtual void getDbgData(FLASH_DEBUG_INFO_T *a_rFlashDbgInfo) = 0;

        virtual int checkInputParaError(int* num, int* errBuf) = 0; //num: input and output
        virtual void setDebugDataSize(int sz) = 0; //should be set initially for check the size in the alg.

                                                   // Flash AWB
        virtual MVOID Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit) = 0;
        virtual MVOID Flash_Awb_Algo(MUINT32 *FlashResultWeight) = 0;
        //virtual void SimAWB(void *FlashAWBInput, void *FlashAWBOutput, bool bSimMapFromExif) = 0;

        // Dual flash decider
        virtual MRESULT DecideCalFlashComb(int CalNum, short *yTab, int totalStep, int totalStepLT, ChooseResult *pChoose, bool dualFlashEnable) = 0;
        virtual MRESULT InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI, CalData *pCalData, short *yTab, AWB_GAIN_T *pGoldenWBGain, int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab, bool dualFlashEnable) = 0;
    };
};
#endif // __FLASHALG_H__
