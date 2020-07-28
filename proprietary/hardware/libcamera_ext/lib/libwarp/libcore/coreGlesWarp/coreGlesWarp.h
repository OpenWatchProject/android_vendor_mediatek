#ifndef _CORE_GLES_WARP_H_
#define _CORE_GLES_WARP_H_

#include "MTKCoreType.h"
#include "utilEGLImageIO/utilEGLImageIO.h"
#define WARP_MAX_IMG_NUM            (50)        // maximum image number
#define MAX_GB_NUM 50
#define MAX_WARP_MAP_NUM 50
#define MAX_WARP_MATRIX_NUM 50

typedef enum GLES_WARP_IMAGE_ENUM
{
    GLES_WARP_IMAGE_RGB565=1,
    GLES_WARP_IMAGE_BGR565,
    GLES_WARP_IMAGE_RGB888,
    GLES_WARP_IMAGE_BGR888,
    GLES_WARP_IMAGE_ARGB888,
    GLES_WARP_IMAGE_ABGR888,
    GLES_WARP_IMAGE_BGRA8888,
    GLES_WARP_IMAGE_RGBA8888,
    GLES_WARP_IMAGE_YUV444,
    GLES_WARP_IMAGE_YUV422,
    GLES_WARP_IMAGE_YUV420,
    GLES_WARP_IMAGE_YUV411,
    GLES_WARP_IMAGE_YUV400,
    GLES_WARP_IMAGE_PACKET_UYVY422,
    GLES_WARP_IMAGE_PACKET_YUY2,
    GLES_WARP_IMAGE_PACKET_YVYU,
    GLES_WARP_IMAGE_NV21,
    GLES_WARP_IMAGE_YV12,

    GLES_WARP_IMAGE_RAW8=100,
    GLES_WARP_IMAGE_RAW10,
    GLES_WARP_IMAGE_EXT_RAW8,
    GLES_WARP_IMAGE_EXT_RAW10,
    GLES_WARP_IMAGE_JPEG=200
} GLES_WARP_IMAGE_ENUM;

//GPU tuning
typedef struct GLES_GPU_TUNE
{
    MUINT32 GLESVersion;
    MINT32 demo;
}GLES_GPU_TUNING;

// WARP_FEATURE_ADD_IMAGE,
// Input    : WarpImageInfo
// Output   : NONE
typedef struct GLES_WARP_IMG_INFO
{
    MUINT32                 ImgAddr[WARP_MAX_IMG_NUM];  // input image address array
    //MUINT32                 ImgNum;                     // input image number
    GLES_WARP_IMAGE_ENUM     ImgFmt;                     // input image format
    MUINT16                 Width;                      // input image width
    MUINT16                 Height;                     // input image height
    MFLOAT                  Hmtx[WARP_MAX_IMG_NUM][9];  // input homography matrix for MAV
    //MUINT32                 Flength;                    // input focal length for Autorama
    //MTK_WARP_DIRECTION_ENUM Direction;                  // input capture direction for Autorama
    void*                 WorkingBuffAddr;            // Working buffer start address

    MUINT32                 ClipX[WARP_MAX_IMG_NUM];    // image offset X
    MUINT32                 ClipY[WARP_MAX_IMG_NUM];    // image offset Y
    MUINT32                 ClipWidth;                  // image result width
    MUINT32                 ClipHeight;                 // image result height
}GLES_WARP_IMG_INFO;

typedef struct GLES_WARP_IMG_EXT_INFO : GLES_WARP_IMG_INFO
{
        MUINT32 Features;                                // Current feature combination
        void* SrcGraphicBuffer;                            // input GB
        void* DstGraphicBuffer;                            // output GB
        MUINT16                 GB_Width;                      // input GB width
    MUINT16                 GB_Height;                     // input GB height
        GLES_WARP_IMAGE_ENUM     OutImgFmt;              // output image format
        void* WarpMapAddr[WARP_MAX_IMG_NUM][2];        // input warpMap address array, 0:X, 1:Y
        MUINT32 WarpMapSize[WARP_MAX_IMG_NUM][2];        // input warpMap size array, 0:width, 1: height
        MUINT32 WarpMapNum;                                // input warp map number
        MUINT32 MaxWarpMapSize[2];                        // input max warp map size (mesh)
        MUINT32 WarpMatrixNum;                            // input warp matrix number
        MUINT32 WarpLevel;                                // warping ratio
        MUINT32 InputGBNum;
        MUINT32 OutputGBNum;
        MUINT32 PreWarpMapSize[2];
        MFLOAT* FWarpMap;
        GLES_GPU_TUNING TuningPara;
        //MINT32 demo;                                    //1: demo mode, 0: mp mode
        MUINT32 DisableEGLImageInit;
        MUINT32 InitBufferCount;
        MUINT32 InitBufferType; //0:input, 1:output
        MUINT32 InputBufferHead;
        MUINT32 OutputBufferHead;
        UTIL_EGLIMAGE_COLOR_SPACE source_color_domain;
}GLES_WARP_IMG_EXT_INFO;

// WARP_FEATURE_GET_RESULT
// Input    : NONE
// Output   : WarpResultInfo
typedef struct GLES_WARP_RESULT
{
    MUINT32                 Width;                      // output image width for Autorama
    MUINT32                 Height;                     // output image hieght for Autorama
    MINT32                  ElapseTime[3];              // record execution time
    CORE_ERRCODE_ENUM       RetCode;                    // returned status
}GLES_WARP_RESULT;

class CoreGlesWarpExt {
public:
    void GlesWarpingInit(void);
    void GlesWarpingMain(void);
    void GlesWarpingReset(void);
    void InitEGLImage(void);
    GLES_WARP_IMG_EXT_INFO core_info;
    EGLDisplay gles_display;
private:
    GLuint VertShader, FragShader;        /* Used to hold the fragment and vertex shader handles */
    GLuint ProgramObject;        /* Used to hold the program handle (made out of the two previous shaders */
    GLuint uiVBO[2];                        /* Used to hold the vbo handle */
    GLuint elenum;
    UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT Src[MAX_GB_NUM];
    UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT Dst[MAX_GB_NUM];
    bool GlesWarping();//(int width[2], int height[2], unsigned char* input_file, int* map_x, int* map_y, unsigned char* output_file, int warp_level);
    bool CreateShderProgram(const char* vs, const char* fs);
    bool DestroyShaderProgram();
};

/* GPU warping */


#endif
