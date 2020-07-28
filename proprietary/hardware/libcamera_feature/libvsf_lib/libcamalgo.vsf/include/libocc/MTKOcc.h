#ifndef _MTK_OCC_H
#define _MTK_OCC_H

#include "MTKOccType.h"
#include "MTKOccErrCode.h"

typedef unsigned char PEL;

#define DVEC_HW short

#define OCC_MAX_IN_BUFFER_NUM               (5)
#define OCC_MAX_OUT_BUFFER_NUM              (4)
#define OCC_CUSTOM_PARAM

//////////////////////////////////////////////////////////////////////////////////////////////

typedef enum DRVOccObject_s {
    DRV_OCC_OBJ_NONE = 0,
    DRV_OCC_OBJ_SW,
    DRV_OCC_OBJ_UNKNOWN = 0xFF,
} DrvOccObject_e;


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    OCC_FEATURE_BEGIN,              // minimum of feature id
    OCC_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    OCC_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    OCC_FEATURE_SET_PROC_INFO,      // feature id to set proc info
    OCC_FEATURE_GET_RESULT,         // feature id to get result
    OCC_FEATURE_MAX                 // maximum of feature id
}   OCC_FEATURE_ENUM;

typedef enum OCC_STATE_ENUM
{
    OCC_STATE_STANDBY=0,            // After Create Obj or Reset
    OCC_STATE_INIT,                 // After Called Init
    OCC_STATE_PROCESS,              // After Called Main
    OCC_STATE_PROCESS_DONE,         // After Finish Main
} OCC_STATE_ENUM;

typedef enum
{
    OCC_IMAGE_YONLY,
    OCC_IMAGE_RGB888,
    OCC_IMAGE_RGBA8888,
    OCC_IMAGE_YV12,
}OCC_IMAGE_FMT_ENUM;

typedef enum
{
    OCC_MAINCAM_POS_ON_LEFT,
    OCC_MAINCAM_POS_ON_RIGHT,
}OCC_MAINCAM_POS_ENUM;

typedef enum
{
    OCC_BUFFER_TYPE_DS_L,
    OCC_BUFFER_TYPE_DS_R,
    OCC_BUFFER_TYPE_LDC,
    OCC_BUFFER_TYPE_CONF_IN,
    OCC_BUFFER_TYPE_DEPTH,
    OCC_BUFFER_TYPE_OCC,
    OCC_BUFFER_TYPE_NOC,
    OCC_BUFFER_TYPE_DS,
    OCC_BUFFER_TYPE_DS_V,
    OCC_BUFFER_TYPE_MASK_L,
    OCC_BUFFER_TYPE_MASK_R,
}OCC_BUFFER_TYPE_ENUM;

typedef enum
{
    OCC_MODE_OCC_1X,
    OCC_MODE_OCC_2X,
    OCC_MODE_OCC_ONLY_1X,
    OCC_MODE_OCC_ONLY_2X,
    OCC_MODE_MAX,
}OCC_MODE_ENUM;

#ifdef OCC_CUSTOM_PARAM
typedef struct OCCTuningParam
{
    char* key;
    MINT32 value;
}OCCTuningParam;
#endif

typedef struct OccTuningInfo
{
    OCC_MAINCAM_POS_ENUM    mainCamPos;                 // main cam on left or right?
    MUINT32                 coreNumber;                 // valid value = {1 ~ MAX_CORE_NUM}
    #ifdef OCC_CUSTOM_PARAM
    MUINT32 NumOfParam;
    OCCTuningParam* params;
    #endif
} OccTuningInfo;

typedef struct OccInitInfo
{
    MUINT32                 inputWidth;
    MUINT32                 inputHeight;
    MUINT32                 outputX;
    MUINT32                 outputY;
    MUINT32                 outputWidth;
    MUINT32                 outputHeight;
    MUINT8*                 workingBuffAddr;                // Working buffer start address
    MUINT32                 workingBuffSize;                // Working buffer size
    OccTuningInfo           *pTuningInfo;                   // Tuning info
    OCC_MODE_ENUM           occMode;
} OccInitInfo;

typedef struct OccBufferInfo
{
    OCC_BUFFER_TYPE_ENUM       type;
    OCC_IMAGE_FMT_ENUM         format;
    MUINT32                    width;
    MUINT32                    height;
    PEL*                    planeAddr0;
    PEL*                    planeAddr1;
    PEL*                    planeAddr2;
    PEL*                    planeAddr3;
}OccBufferInfo;

typedef struct OccProcInfo
{
    DVEC_HW* dispL;         //disparity L (size = sizeof(MINT16)x input Width x input Height)
    DVEC_HW* dispR;         //disparity R (size = sizeof(MINT16)x input Width x input Height)
    MFLOAT  cOffset;
    MUINT32 numOfBuffer;
    OccBufferInfo   bufferInfo[OCC_MAX_IN_BUFFER_NUM];
} OccProcInfo;

typedef struct OccResultInfo
{
    MUINT32 numOfBuffer;
    OccBufferInfo bufferInfo[OCC_MAX_OUT_BUFFER_NUM];
    MRESULT     RetCode;                        // returned status
}OccResultInfo;

/*******************************************************************************
*
********************************************************************************/
class MTKOcc {
public:
    static MTKOcc* createInstance(DrvOccObject_e eobject);
    virtual void   destroyInstance(MTKOcc* obj) = 0;

    virtual ~MTKOcc(){}
    // Process Control
    virtual MRESULT Init(void* InitInData, void* InitOutData);    // Env/Cb setting
    virtual MRESULT Main(void);                                         // START
    virtual MRESULT Reset(void);                                        // RESET

    // Feature Control
    virtual MRESULT FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppOccTmp : public MTKOcc {
public:
    static MTKOcc* getInstance();
    virtual void destroyInstance(){};
    AppOccTmp() {};
    virtual ~AppOccTmp() {};
};

#endif


