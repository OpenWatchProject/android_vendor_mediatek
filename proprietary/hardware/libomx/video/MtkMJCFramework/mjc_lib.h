#ifndef _MJC_LIB_H_
#define _MJC_LIB_H_


#ifdef __cplusplus
extern "C" {
#endif

#define FR_RATIO (10)
#define MJC_FR(a) (a*FR_RATIO)
#define REAL_FR(a) (a/FR_RATIO)

//**************************************************//
// For user
//**************************************************//
typedef enum __MJC_DRV_RESULT_T
{
    MJC_DRV_RESULT_OK = 0,           ///< OK
    MJC_DRV_RESULT_FAIL,             ///< Fail
    MJC_DRV_RESULT_RETRY,            ///< Partial fial, try to recover
    MJC_DRV_RESULT_MAX = 0x0FFFFFFF
} MJC_DRV_RESULT_T;

typedef enum __MJC_DRV_PIC_FORMAT_T
{
    MJC_DRV_BLK_FORMAT = 0,
    MJC_DRV_LINE_FORMAT_YUV420,
    MJC_DRV_LINE_FORMAT_YV12
}MJC_DRV_PIC_FORMAT_T;

typedef enum __MJC_3D_MODE_T
{
    MJC_3D_OFF = 0,
    MJC_3D_FRAME_SEQ,
    MJC_3D_SIDE_BY_SIDE,
    MJC_3D_TOP_BOTTOM,
    MJC_3D_MODE_MAX
} MJC_3D_MODE_T;

typedef enum __MJC_10Bit_MODE_T
{
    MJC_10BIT_OFF = 0,
    MJC_10BIT_HORIZONTAL = 2,   //for H264 10-bit
    MJC_10BIT_VERTICAL = 3      //for HEVC 10-bit
} MJC_10Bit_MODE_T;

typedef enum
{
    E_MJC_DEMO_OFF = 0,
    E_MJC_DEMO_COL_LEFT_STA,
    E_MJC_DEMO_COL_RIGHT_STA,
    E_MJC_DEMO_COL_LEFT_MCA,
    E_MJC_DEMO_COL_RIGHT_MCA,
    E_MJC_DEMO_COL_LEFT_INTP,
    E_MJC_DEMO_COL_RIGHT_INTP,
    E_MJC_DEMO_ROW_TOP_STA,
    E_MJC_DEMO_ROW_BOT_STA,
    E_MJC_DEMO_ROW_TOP_MCA,
    E_MJC_DEMO_ROW_BOT_MCA,
    E_MJC_DEMO_ROW_TOP_INTP,
    E_MJC_DEMO_ROW_BOT_INTP,
    E_MJC_DEMO_WINDOW_INSIDE,
    E_MJC_DEMO_WINDOW_OUTSIDE,
    E_MJC_DEMO_MAX
} E_MJC_DEMO_MODE;


typedef enum __MJC_DRV_INDEXTYPE_T
{
    // Get Part start at 0
    MJC_DRV_TYPE_HW_INFO = 0,
    MJC_DRV_TYPE_DRV_INFO,
    MJC_DRV_TYPE_DRV_REGISTER_INFO,
    MJC_DRV_TYPE_DRV_UNREGISTER_INFO
    // set Part start at 0x1000
} MJC_DRV_INDEXTYPE_T;


typedef struct __MJC_DRV_CONFIG_T
{
    unsigned int u4Width;    // (Must) real data width
    unsigned int u4Height;   // (Must) real data height
    MJC_DRV_PIC_FORMAT_T eInPicFormat; // (Must) input picture format (block or line)
    //unsigned short u2InPicBlkPitch;  // (By format) input picture width pitch with block mode (width/block size)
    //unsigned short u2InPicYPitch; // (By format) input picture Y width pitch with line mode
    //unsigned short u2InPicCbCrPitch; // (By format) input picture CbCr width pitch with line mode
    unsigned char u1InPicWidthAlign; // (Must) input picture width alignment
    unsigned char u1InPicHeightAlign; // (Must) input picture height alignment
    unsigned char u1OutPicWidthAlign; // (Must) output picture width alignment
    unsigned char u1OutPicHeightAlign; // (Must) output picture height alignment
    unsigned short u2InFrmRate; // (Must) input frame rate
    unsigned short u2OutFrmRate; // (Must) output frame rate
    unsigned char u1DFR; //(Must) Dynamic fram rate during detect region
    MJC_3D_MODE_T e3DModeIn; // (Must) input 3D format
    MJC_3D_MODE_T e3DModeOut; // (Must) output 3D format
    unsigned char u12xMode; // support 2X mode
    unsigned char u1DynamicME2ME3; // support dynamic turn 0n Me2/ME3 in 120Hz
    unsigned char u1InkModu; // (debug)ink configure
    unsigned char u1InkInfo; // (debug)ink configure
    unsigned char u1DemoMode; // (debug)demo mode ([6:0]:E_MJC_DEMO_MODE, [7]: 0 two windows 1 thress windows)
    unsigned char u1DisDSR; //(debug) disable dynamic search region
    unsigned char u1DisLPB; //(debug) disable  Letter and pillar-box
    unsigned char u1DisSCNCHG; //(debug) disable Scene change
    unsigned char u1Input10Bit; //(Optional) 10 bit input mode. 2 for H264. 3 for HEVC
    unsigned char u1DisFallback; //(debug) disable fallback
    unsigned char u1NrmFbTabIdx; //(Must) nrm fallback table index 0~255
    unsigned char u1BdrFbTabIdx; //(Must) bdr fallback table index 0~255
    unsigned char u1Effect; // (Must) default fallback table index 0~255
    unsigned short u2MaxFps; // (Must) max fps
    char *pi1CtrlNcsFile; //(debug) load control ncs setting from file
    char *pi1MemcNcsFile; //(debug) load ME/MC ncs setting from file
} MJC_DRV_CONFIG_T;

typedef struct __MJC_DRV_INIT_INFO_T
{
   unsigned char u1FRRatio; // out frame rate *10/ input frame rate
   unsigned char u1NextInputNum; // (out) input num
   unsigned char u1NextOutputNum; // (out) output num including dummy
} MJC_DRV_INIT_INFO_T;

typedef struct __MJC_DRV_FRAME_T
{
   unsigned int u4InputNum;  // (Must) input frame number (3 or 4)
   unsigned int u4InputAddress[4]; // (Must) input frame address
   unsigned int u4OutputNum; // (Must) output frame number (0, 1 or 2)
   unsigned int u4OutputAddress[2]; // (Must) input frame address
   unsigned short u2UpdateInFrmRate; // (Option) update input frame rate, defult 0
   unsigned short u2UpdateOutFrmRate; // (Option) update output frame rate, defult 0
   unsigned char u1DFR; //(Option) Dynamic fram rate during detect region
   unsigned char u1LFRDepth;  // (Option) low fram Rate detect depth
   unsigned char u1LFRPosition;  // (Option) low fram Rate detect position
   unsigned int u4SyncFrameNum; //(Option) dump frame number
} MJC_DRV_FRAME_T;

typedef struct __MJC_DRV_NEXT_INFO_T
{
    unsigned char u1IntpOutputNum;  // (out)(out) interpolated output frame number
    unsigned char u1OutputOrgFrame; // (out) 1: original frame, 0 interploation frame
    unsigned int u4OutputAddress[2]; // (out) output frame address
    unsigned char u1UpdateFrame; // (out) skip first input frame
    unsigned char u1UpdateRefTimeStamp; // (out) update reference timestamp
    unsigned char u1NextInputNum; // (out) input num
    unsigned char u1NextOutputNum; // (out) output num including dummy
    unsigned char u1CheckDFR; //(out) check whether dynamic frame rate
    unsigned char u1FRRatio; // (out)out frame rate *10/ input frame rate
    unsigned char u1NonProcessedFrame; // (out) 0: Normal MJC interpolated frame, 1: Non-Processing mode (Fake engine)
} MJC_DRV_NEXT_INFO_T;

typedef struct __MJC_DRV_RESET_CONFIG_T
{
    unsigned short u2UpdateInFrmRate; // (Option) update input frame rate, defult 0
    unsigned short u2UpdateOutFrmRate; // (Option) update output frame rate, defult 0
    unsigned char u1DFR; //(Option) Dynamic fram rate during detect region
} MJC_DRV_RESET_CONFIG_T;

typedef struct __MJC_DRV_RESET_INFO_T
{
    unsigned char u1FRRatio; // out frame rate *10/ input frame rate
    unsigned char u1NextInputNum; // (out) input num
    unsigned char u1NextOutputNum; // (out) output num including dummy
} MJC_DRV_RESET_INFO_T;

typedef struct __MJC_DRV_PARAM_HW_INFO_T
{
    unsigned int u4WidthPitch;    //  width pitch
    unsigned int u4HeightPitch;   // height pitch
} MJC_DRV_PARAM_HW_INFO_T;

typedef struct __MJC_DRV_PARAM_DRV_INFO_T
{
    unsigned char u1DetectNum; //check dynamic frame rate during n frame
    unsigned char u1SucessfullyRegisterDrv; // check if driver has been created
} MJC_DRV_PARAM_DRV_INFO_T;

MJC_DRV_RESULT_T eMjcDrvCreate(unsigned int *pu4Handle);
MJC_DRV_RESULT_T eMjcDrvRelease(unsigned int u4Handle);
MJC_DRV_RESULT_T eMjcDrvInit(unsigned int u4Handle, MJC_DRV_CONFIG_T * prConfig, MJC_DRV_INIT_INFO_T *prInitInfo);
MJC_DRV_RESULT_T eMjcDrvDeInit(unsigned int u4Handle);
MJC_DRV_RESULT_T eMjcDrvProcess(unsigned int u4Handle, MJC_DRV_FRAME_T *prFramInfo, MJC_DRV_NEXT_INFO_T *prNextInfo);
MJC_DRV_RESULT_T eMjcDrvReset(unsigned int u4Handle, MJC_DRV_RESET_CONFIG_T *prConfig, MJC_DRV_RESET_INFO_T *prRestInfo);
MJC_DRV_RESULT_T eMjcDrvGetParam(unsigned int u4Handle, MJC_DRV_INDEXTYPE_T eType, void *pvParam);
MJC_DRV_RESULT_T eMjcDrvSetParam(unsigned int u4Handle, MJC_DRV_INDEXTYPE_T eType, void *pvParam);


#ifdef __cplusplus
}
#endif

#endif //#ifndef _MJC_LIB_H_

