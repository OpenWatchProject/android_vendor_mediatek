#ifndef _MTK_GF_H
#define _MTK_GF_H

#include "MTKGFType.h"
#include "MTKGFErrCode.h"

#define GF_ENABLE_INK                      (0)
#define GF_DYN_CR_ENABLE
#define GF_MAX_IN_BUFFER_NUM               (6)
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
    GF_IMAGE_YUY2,
    GF_IMAGE_I422,
    GF_IMAGE_NV12
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
    GF_BUFFER_TYPE_OCC,
    GF_BUFFER_TYPE_CONF_IN,
}GF_BUFFER_TYPE_ENUM;

typedef enum
{
    GF_MODE_VR,//Video record, camera preview
    GF_MODE_DPP,//Depth post-processing
    GF_MODE_CP,//Capture
    GF_MODE_CP_2X,
    GF_MODE_CP_4X,
    GF_MODE_MAX,
}GF_MODE_ENUM;

typedef enum GF_BLUR_MAPPING_MODE
{
    DAC_LEVEL,
    DISP_CTRL_PT,
    MIXTURE,
};

typedef enum CLR_CURVE_TYPE
{
    CLR_TABLE,
    CLR_CURVE,
};

typedef enum GAIN_CURVE_TYPE
{
    BG_TABLE,
    BG_CURVE,
    FROM_CLR_CURVE,
};

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
    MINT32                  ctrlPointNum;
    MINT32*                 dispCtrlPoints;
    MINT32*                 blurGainTable;
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
    GFInitInfo();
} GFInitInfo;

typedef struct GFBufferInfo
{
    GF_BUFFER_TYPE_ENUM       type;
    GF_IMAGE_FMT_ENUM         format;
    MUINT32                    width;
    MUINT32                    stride;
    MUINT32                    height;
    PEL*                    planeAddr0;
    PEL*                    planeAddr1;
    PEL*                    planeAddr2;
    PEL*                    planeAddr3;
    GFBufferInfo();
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

typedef enum
{
    GF_AF_NONE = 0, /* for preview start only*/ 
    GF_AF_AP,
    GF_AF_OT,
    GF_AF_FD,
    GF_AF_CENTER,
    GF_AF_DEFAULT,
}GF_AF_TYPE_ENUM;

typedef struct GFAfInfo
{
    GF_AF_TYPE_ENUM afType;
    MINT32 x1;
    MINT32 y1;
    MINT32 x2;
    MINT32 y2;
}GFAfInfo;

typedef struct GFProcInfo
{
    MUINT32 touchTrigger;
    MUINT32 touchX;
    MUINT32 touchY;
    MUINT32 depthValue;
    MUINT32 dof;
    MUINT32 clearRange;
    MFLOAT  cOffset;
    MFLOAT fb;
    MUINT32 numOfBuffer;
    GFBufferInfo   bufferInfo[GF_MAX_IN_BUFFER_NUM];
    #ifdef GF_DYN_CR_ENABLE
    GFDacInfo dacInfo;
    GFFdInfo fdInfo;
    #endif
    GFAfInfo afInfo;
} GFProcInfo;

typedef struct GFResultInfo
{
    MUINT32 numOfBuffer;
    GFBufferInfo bufferInfo[GF_MAX_OUT_BUFFER_NUM];
    MUINT32 dof_m;
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



