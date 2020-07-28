#ifndef _MTK_DSDN_H
#define _MTK_DSDN_H

#include "MTKDSDNType.h"
#include "MTKDSDNErrCode.h"

#define DSDN_MAX_BUFFER_NUM               (4)

//////////////////////////////////////////////////////////////////////////////////////////////

typedef enum DRVDSDNObject_s {
    DRV_DSDN_OBJ_NONE = 0,
    DRV_DSDN_OBJ_SW,
    DRV_DSDN_OBJ_UNKNOWN = 0xFF,
} DrvDSDNObject_e;


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    DSDN_FEATURE_BEGIN,              // minimum of feature id
    DSDN_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    DSDN_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    DSDN_FEATURE_SET_PROC_INFO,      // feature id to set proc info
    DSDN_FEATURE_MAX                 // maximum of feature id
}   DSDN_FEATURE_ENUM;

typedef struct DSDNTuningParam
{
    const char* key;
    MINT32 value;
}DSDNTuningParam;

typedef struct DSDNTuningInfo
{
    MUINT32 NumOfParam;
    DSDNTuningParam* params;
} DSDNTuningInfo;

typedef struct DSDNInitInfo
{
    MUINT32                 dsdnMode;                       //0:vpu, 1:cpu
    MUINT32                 maxWidth;
    MUINT32                 maxHeight;
    MUINT8*                 workingBuffAddr;                // Working buffer start address
    MUINT32                 workingBuffSize;                // Working buffer size
    DSDNTuningInfo          *pTuningInfo;                  // Tuning info
} DSDNInitInfo;

typedef enum
{
    DSDN_BUFFER_TYPE_IN1,           //input big image   (NV12 or NV21)
    DSDN_BUFFER_TYPE_IN2,           //input small image (YUYV)
    DSDN_BUFFER_TYPE_IN3,           //input small image (NV12 or NV21)
    DSDN_BUFFER_TYPE_OUT1           //output big image  (same format with input big image)
}DSDN_BUFFER_TYPE_ENUM;

typedef enum
{
    DSDNFormatData,
    DSDNFormatImageY8,
    DSDNFormatImageYV12,
    DSDNFormatImageNV21,
    DSDNFormatImageYUY2,
    DSDNFormatImageNV12,
    DSDNFormatReserved2,
    DSDNFormatReserved3,
}DSDN_BUFFER_FMT_ENUM;

typedef struct {
    MUINT32 fd;
    MUINT32 offset;
    MUINT32 stride;
    MUINT32 length;
    MUINT8* va;
} DSDNPlaneInfo;

typedef struct DSDNBufferInfo
{
    DSDN_BUFFER_TYPE_ENUM       type;
    DSDN_BUFFER_FMT_ENUM        format;
    MUINT32                     planeCount;
    DSDNPlaneInfo               planes[3];
    MUINT32                     width;
    MUINT32                     height;
}DSDNBufferInfo;

typedef struct DSDNProcInfo
{
    MUINT32                     fps;
    MUINT32                     numOfBuffer;
    DSDNBufferInfo              bufferInfo[DSDN_MAX_BUFFER_NUM];
} DSDNProcInfo;

/*******************************************************************************
*
********************************************************************************/
class MTKDSDN {
public:
    static MTKDSDN* createInstance(DrvDSDNObject_e eobject);
    virtual void   destroyInstance(MTKDSDN* obj) = 0;

    virtual ~MTKDSDN(){}
    // Process Control
    virtual MRESULT Init(void* InitInData, void* InitOutData);    // Env/Cb setting
    virtual MRESULT Main(void);                                         // START
    virtual MRESULT Reset(void);                                        // RESET

    // Feature Control
    virtual MRESULT FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppDSDNTmp : public MTKDSDN {
public:
    static MTKDSDN* getInstance();
    virtual void destroyInstance(){};
    AppDSDNTmp() {};
    virtual ~AppDSDNTmp() {};
};

#endif


