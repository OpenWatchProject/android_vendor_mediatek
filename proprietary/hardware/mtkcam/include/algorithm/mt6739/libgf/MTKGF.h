#ifndef _MTK_GF_H
#define _MTK_GF_H

#include "MTKGFType.h"
#include "MTKGFErrCode.h"

#define GF_ENABLE_INK                      (0)
#define GF_DYN_CR_ENABLE
#define GF_MAX_IN_BUFFER_NUM               (5)
#define GF_MAX_OUT_BUFFER_NUM              (4)
#define GF_CUSTOM_PARAM

typedef unsigned char PEL;

typedef enum DRVOccObject_s {
    DRV_GF_OBJ_NONE = 0,
    DRV_GF_OBJ_SW,
    DRV_GF_OBJ_UNKNOWN = 0xFF,
} DrvGFObject_e;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    GF_FEATURE_BEGIN,              // minimum of feature id
    GF_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    GF_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    GF_FEATURE_SET_PROC_INFO,      // feature id to set proc info
    GF_FEATURE_GET_RESULT,         // feature id to get result
    GF_FEATURE_MAX                 // maximum of feature id
}   GF_FEATURE_ENUM;

typedef enum OCC_STATE_ENUM
{
    GF_STATE_STANDBY=0,            // After Create Obj or Reset
    GF_STATE_INIT,                 // After Called Init
    GF_STATE_PROCESS,              // After Called Main
    GF_STATE_PROCESS_DONE,         // After Finish Main
} GF_STATE_ENUM;

typedef enum
{
    GF_IMAGE_YONLY,
    GF_IMAGE_RGB888,
    GF_IMAGE_RGBA8888,
    GF_IMAGE_YV12,
    GF_IMAGE_YUV444,
}GF_IMAGE_FMT_ENUM;

typedef enum
{
    GF_BUFFER_TYPE_DEPTH,
    GF_BUFFER_TYPE_DS,
    GF_BUFFER_TYPE_DS_2X,
    GF_BUFFER_TYPE_DS_4X,
    GF_BUFFER_TYPE_BMAP,
    GF_BUFFER_TYPE_DMAP,
    GF_BUFFER_TYPE_DMAP_PV,
    GF_BUFFER_TYPE_INK,
}GF_BUFFER_TYPE_ENUM;

typedef enum
{
    GF_MODE_VR,
    GF_MODE_CP,
    GF_MODE_CP_2X,
    GF_MODE_CP_4X,
    GF_MODE_MAX,
}GF_MODE_ENUM;

#ifdef GF_CUSTOM_PARAM
typedef struct GFTuningParam
{
    char* key;
    MINT32 value;
}GFTuningParam;
#endif

typedef struct GFTuningInfo
{
    MUINT32                 coreNumber;                 // valid value = {1 ~ MAX_CORE_NUM}
    MINT32                  clrTblSize;
    MINT32*                 clrTbl;
#ifdef GF_CUSTOM_PARAM
    MUINT32 NumOfParam;
    GFTuningParam* params;
#endif
} GFTuningInfo;

typedef struct GFInitInfo
{
    MUINT32                 inputWidth;
    MUINT32                 inputHeight;
    MUINT32                 outputWidth;
    MUINT32                 outputHeight;
    MUINT8*                 workingBuffAddr;                // Working buffer start address
    MUINT32                 workingBuffSize;                // Working buffer size
    GFTuningInfo           *pTuningInfo;                   // Tuning info
    GF_MODE_ENUM            gfMode;
} GFInitInfo;

typedef struct GFBufferInfo
{
    GF_BUFFER_TYPE_ENUM       type;
    GF_IMAGE_FMT_ENUM         format;
    MUINT32                    width;
    MUINT32                    height;
    PEL*                    planeAddr0;
    PEL*                    planeAddr1;
    PEL*                    planeAddr2;
    PEL*                    planeAddr3;
}GFBufferInfo;

#ifdef GF_DYN_CR_ENABLE
typedef struct GFDacInfo
{
    MINT32 min;
    MINT32 max;
    MINT32 cur;
}GFDacInfo;

typedef struct GFFdInfo
{
    MINT32 isFd;
    float  ratio;
}GFFdInfo;
#endif

typedef struct GFProcInfo
{
    MUINT32 touchTrigger;
    MUINT32 touchX;
    MUINT32 touchY;
    MUINT32 depthValue;
    MUINT32 dof;
    MUINT32 clearRange;
    MFLOAT  cOffset;
    MUINT32 numOfBuffer;
    GFBufferInfo   bufferInfo[GF_MAX_IN_BUFFER_NUM];
    #ifdef GF_DYN_CR_ENABLE
    GFDacInfo dacInfo;
    GFFdInfo fdInfo;
    #endif
} GFProcInfo;

typedef struct GFResultInfo
{
    MUINT32 numOfBuffer;
    GFBufferInfo bufferInfo[GF_MAX_OUT_BUFFER_NUM];
    MRESULT     RetCode;                        // returned status
}GFResultInfo;

/*******************************************************************************
*
********************************************************************************/
class MTKGF {
public:
    static MTKGF* createInstance(DrvGFObject_e eobject);
    virtual void   destroyInstance(MTKGF* obj) = 0;

    virtual ~MTKGF(){}
    // Process Control
    virtual MRESULT Init(void* InitInData, void* InitOutData);    // Env/Cb setting
    virtual MRESULT Main(void);                                         // START
    virtual MRESULT Reset(void);                                        // RESET

    // Feature Control
    virtual MRESULT FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppGFTmp : public MTKGF {
public:
    //
    static MTKGF* getInstance();
  virtual void destroyInstance(){};
    //
    AppGFTmp() {};
    virtual ~AppGFTmp() {};
};

#endif



