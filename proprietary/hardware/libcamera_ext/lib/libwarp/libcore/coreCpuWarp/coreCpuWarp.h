#ifndef _CORE_CPU_WARP_H_
#define _CORE_CPU_WARP_H_
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) > (B) ? (B) : (A))

#include "MTKCoreType.h"

#define WARP_MAX_IMG_NUM            (50)        // maximum image number
#define MAX_GB_NUM 50
#define MAX_WARP_MAP_NUM 50
#define MAX_WARP_MATRIX_NUM 50

typedef enum CPU_WARP_IMAGE_ENUM
{
    CPU_WARP_IMAGE_RGB565=1,
    CPU_WARP_IMAGE_BGR565,
    CPU_WARP_IMAGE_RGB888,
    CPU_WARP_IMAGE_BGR888,
    CPU_WARP_IMAGE_ARGB888,
    CPU_WARP_IMAGE_ABGR888,
    CPU_WARP_IMAGE_BGRA8888,
    CPU_WARP_IMAGE_RGBA8888,
    CPU_WARP_IMAGE_YUV444,
    CPU_WARP_IMAGE_YUV422,
    CPU_WARP_IMAGE_YUV420,
    CPU_WARP_IMAGE_YUV411,
    CPU_WARP_IMAGE_YUV400,
    CPU_WARP_IMAGE_PACKET_UYVY422,
    CPU_WARP_IMAGE_PACKET_YUY2,
    CPU_WARP_IMAGE_PACKET_YVYU,
    CPU_WARP_IMAGE_NV21,
    CPU_WARP_IMAGE_YV12,

    CPU_WARP_IMAGE_RAW8=100,
    CPU_WARP_IMAGE_RAW10,
    CPU_WARP_IMAGE_EXT_RAW8,
    CPU_WARP_IMAGE_EXT_RAW10,
    CPU_WARP_IMAGE_JPEG=200
} CPU_WARP_IMAGE_ENUM;


// WARP_FEATURE_ADD_IMAGE,
// Input    : WarpImageInfo
// Output   : NONE
typedef struct CPU_WARP_IMG_INFO
{
    MUINT32*                 ImgAddr[WARP_MAX_IMG_NUM];  // input image address array
    //MUINT32                 ImgNum;                     // input image number
    CPU_WARP_IMAGE_ENUM     ImgFmt;                     // input image format
    MUINT16                 Width;                      // input image width
    MUINT16                 Height;                     // input image height
    MFLOAT                  Hmtx[WARP_MAX_IMG_NUM][9];  // input homography matrix for MAV
    //MUINT32                 Flength;                    // input focal length for Autorama
    //MTK_WARP_DIRECTION_ENUM Direction;                  // input capture direction for Autorama
    MUINT32                 WorkingBuffAddr;            // Working buffer start address

    MUINT32                 ClipX[WARP_MAX_IMG_NUM];    // image offset X
    MUINT32                 ClipY[WARP_MAX_IMG_NUM];    // image offset Y
    MUINT32                 ClipWidth;                  // image result width
    MUINT32                 ClipHeight;                 // image result height
}CPU_WARP_IMG_INFO;

typedef struct CPU_WARP_IMG_EXT_INFO : CPU_WARP_IMG_INFO
{
        MUINT32 Features;                                // Current feature combination
        void* SrcBuffer;                                // input GB
        void* DstBuffer;                                // output GB
        CPU_WARP_IMAGE_ENUM     OutImgFmt;              // output image format
        MUINT32* WarpMapAddr[WARP_MAX_IMG_NUM][2];        // input warpMap address array, 0:X, 1:Y
        MUINT32 WarpMapSize[WARP_MAX_IMG_NUM][2];        // input warpMap size array, 0:width, 1: height
        MUINT32 WarpMapNum;                                // input warp map number
        MUINT32 MaxWarpMapSize[2];                        // input max warp map size (mesh)
        MUINT32 WarpMatrixNum;                            // input warp matrix number
        MUINT32 WarpLevel;                                // warping ratio
        //MUINT32 InputGBNum;
        //MUINT32 OutputGBNum;
        //GLES_GPU_TUNING TuningPara;
        //MINT32 demo;                                    //1: demo mode, 0: mp mode
}CPU_WARP_IMG_EXT_INFO;

// WARP_FEATURE_GET_RESULT
// Input    : NONE
// Output   : WarpResultInfo
typedef struct CPU_WARP_RESULT
{
    MUINT32                 Width;                      // output image width for Autorama
    MUINT32                 Height;                     // output image hieght for Autorama
    MINT32                  ElapseTime[3];              // record execution time
    CORE_ERRCODE_ENUM       RetCode;                    // returned status
}CPU_WARP_RESULT;

class CoreCpuWarp {
public:
    void CpuWarpingInit(void);
    void CpuWarpingMain(void);
    void CpuWarpingReset(void);
    CPU_WARP_IMG_EXT_INFO core_info;

private:
    bool CpuWarping();
    //int test;
};

/* GPU warping */


#endif