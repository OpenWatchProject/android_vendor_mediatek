#ifndef _CORE_WARP_H_
#define _CORE_WARP_H_

#include "libutility.h"
#include "MTKCoreType.h"

#define WARP_MAX_IMG_NUM        (50)        // maximum image number
#define V_WIDTH                 (1)
#define V_HEIGHT                (1)
#define WARP_MATRIX_SIZE        (9*WARP_MAX_IMG_NUM)
#define WARP_VTX_DATA_SIZE      ((V_WIDTH+1)*2*V_HEIGHT)
#define WARP_ELE_DATA_SIZE      (6*V_WIDTH*V_HEIGHT)
#define WARP_MATRIX_BIT         (20)

typedef enum MAV_WARP_DIRECTION_ENUM
{
    MAV_WARP_DIR_RIGHT=0,   // panorama direction is right
    MAV_WARP_DIR_LEFT,      // panorama direction is left
    MAV_WARP_DIR_UP,        // panorama direction is up
    MAV_WARP_DIR_DOWN,      // panorama direction is down
    MAV_WARP_DIR_NO         // panorama direction is not decided
} MAV_WARP_DIRECTION_ENUM;

typedef struct MAV_WARP_IMAGE_INFO
{
    MUINT8                     *ImgAddr[WARP_MAX_IMG_NUM];  // input image address
    MUINT32                 ImgNum;                     // input image number
    UTL_IMAGE_FORMAT_ENUM   ImgFmt;                     // input image format
    MUINT16                 Width;                      // input image width
    MUINT16                 Height;                     // input image height
    MFLOAT                  Hmtx[WARP_MAX_IMG_NUM][9];  // input homography matrix for MAV
    MUINT32                 Flength;                    // input focal length for Autorama
    MAV_WARP_DIRECTION_ENUM Direction;                  // input capture direction for Autorama
    void                    *WorkingBuffAddr;            // Working buffer start address

    MUINT32                 ClipX[WARP_MAX_IMG_NUM];    // image offset X
    MUINT32                 ClipY[WARP_MAX_IMG_NUM];    // image offset Y
    MUINT32                 ClipWidth;                  // image result width
    MUINT32                 ClipHeight;                 // image result height
} MAV_WARP_IMAGE_INFO, *P_MAV_WARP_IMAGE_INFO;

typedef struct MAV_WARP_RESULT_INFO
{
    MUINT32                 Width;                      // output image width for Autorama
    MUINT32                 Height;                     // output image hieght for Autorama
    MINT32                  ElapseTime[3];              // record execution time
    CORE_ERRCODE_ENUM       RetCode;                    // returned status
} MAV_WARP_RESULT_INFO, *P_MAV_WARP_RESULT_INFO;

typedef struct MAV_WARP_SW_CAL_STRUCT
{
    // image parameters
    void                     *ProcBufAddr;                // working buffer address
    MUINT8                  *ImgAddr[WARP_MAX_IMG_NUM]; // input image address
    MUINT32                 ImgNum;                     // input image number
    UTL_IMAGE_FORMAT_ENUM   ImgFmt;                     // input image format
    MUINT32                 ImgWidth;                   // input image width
    MUINT32                 ImgHeight;                  // input image height
    MFLOAT                  *WarpMatrix;                // warping matrix pointer
    MUINT8                  *ImgBufferAddr;             // image buffer address
    MUINT32                 ClipX[WARP_MAX_IMG_NUM];    // image offset X
    MUINT32                 ClipY[WARP_MAX_IMG_NUM];    // image offset Y
    MUINT32                 ClipWidth;                  // image result width
    MUINT32                 ClipHeight;                 // image result height
} MAV_WARP_SW_CAL_STRUCT;

typedef struct MAV_WARP_CAL_STRUCT : MAV_WARP_SW_CAL_STRUCT
{
    // image parameters
    /* inherited from MAV_WARP_SW_CAL_STRUCT*/

    // opengl parameters
    vtx_fmt                 *VtxData;               // vertex data pointer
    MUINT16                 *EleData;               // element data pointer
    GLuint                  uiProgramObject;        // program object handle
    GLuint                  uiVertShader;           // vertex shader handle
    GLuint                  uiFragShader;           // fragment shader handle
    GLuint                  uiVBO[2];               // VPE object handle
    GLuint                  fbo;                    // framebuffer object
    GLuint                  rbo;                    // renderbuffer object
} MAV_WARP_CAL_STRUCT, *P_MAV_WARP_CAL_STRUCT;

/* GPU warping */
void WarpingInit(void *pParaIn, void *pParaOut);
void Warping(MINT32 ImgIdx, void *pParaOut);
void WarpingFinish(void);

/* CPU warping */
static MUINT8 SimpleBilinear(MUINT8 *src, MINT32 x4, MINT32 y4, MINT32 stride);
void SwWarpingInit(void *pParaIn);
void SwWarping(MINT32, void *pParaOut);

#endif /* _CORE_WARP_H_ */

