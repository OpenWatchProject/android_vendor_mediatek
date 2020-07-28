#define LOG_TAG "coreWarp"

#define MTK_LOG_ENABLE 1
#include <string.h>
#include "coreWarp.h"

#ifdef SIM_MAIN
#include <stdio.h>
#define LOGD printf
#else
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,##__VA_ARGS__) 
#endif

#define LOAD_SHAD_STR

#if defined(SIM_MAIN) || defined(LOAD_SHAD_STR)
static char gVertShaderStr[] = {
    "// Vertex shader I/O\n"
    "attribute highp vec4 vpg_tc;\n"
    "attribute highp vec2 vpg_sc;\n"
    "\n"
    "varying highp vec2 vpe_sc;\n"
    "varying highp vec3 temp_cord;\n"
    "\n"
    "// Place any non-compiled time constant variable you want\n"
    "uniform mediump mat3 matrix;\n"
    "uniform highp vec2 target_size;\n"
    "uniform highp float origin_x;\n"
    "uniform highp float offset_x;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    //vpe source coordinate computation\n"
    "    vpe_sc = vpg_sc;\n"
    "    vpe_sc.x = vpe_sc.x - offset_x;\n"
    "\n"
    "    //test warp by texture coordinate\n"
    "    temp_cord = matrix * vec3(vpe_sc, 1.0);\n"
    "\n"
    "    //Output the target coordinate. Do not modify!!\n"
    "    gl_Position = vpg_tc;\n"
    "    gl_Position.x = (vpg_tc.x - origin_x)*0.25 + origin_x;\n"
    "\n"
    "}\n"
};

static char gFragShaderStr[] = {
    "// Fragment shader I/O\n"
    "varying highp vec2 vpe_sc;\n"
    "varying highp vec3 temp_cord;\n"
    "\n"
    "// Declare the source image label\n"
    "uniform sampler2D  SrcImg;\n"
    "\n"
    "// Place any non-compiled time constant variable you want\n"
    "uniform highp vec3 delta_x;\n"
    "\n"
    "void main (void)\n"
    "{\n"
    "    // Variables declaration\n"
    "    highp vec3 new_cord;\n"
    "\n"
    "    // Warping and ouput target color\n"
    "    new_cord = temp_cord;\n"
    "    gl_FragColor.x = texture2DProj(SrcImg, new_cord).x;\n"
    "\n"
    "    new_cord += delta_x;\n"
    "    gl_FragColor.y = texture2DProj(SrcImg, new_cord).x;\n"
    "\n"
    "    new_cord += delta_x;\n"
    "    gl_FragColor.z = texture2DProj(SrcImg, new_cord).x;\n"
    "\n"
    "    new_cord += delta_x;\n"
    "    gl_FragColor.w = texture2DProj(SrcImg, new_cord).x;\n"
    "}\n"
};
#endif /* SIM_MAIN || LOAD_SHAD_STR */

MAV_WARP_SW_CAL_STRUCT g_wap_sw_cal_data;
MAV_WARP_CAL_STRUCT g_wap_cal_data;
static UTIL_BASE_IMAGE_STRUCT g_image_data;

/******************************************************************************
Warping Main Process
******************************************************************************/
void WarpingInit(void *pParaIn, void *pParaOut)
{
    // variables
    MAV_WARP_CAL_STRUCT *warp_cal_data = &g_wap_cal_data;
    P_MAV_WARP_IMAGE_INFO ImageInfo = (MAV_WARP_IMAGE_INFO *) pParaIn;
    P_MAV_WARP_RESULT_INFO ResultInfo = (MAV_WARP_RESULT_INFO *) pParaOut;

    // opengl init
    MINT32 SourceWidth = 0;
    MINT32 SourceHeight =  0;
    GLuint fbo = 0;
    GLuint rbo = 0;
    GLuint uiFragShader = 0, uiVertShader = 0;
    GLuint uiProgramObject = 0;

    // allocate memory
    ResultInfo->RetCode = CORE_OK;
    warp_cal_data->ProcBufAddr = ImageInfo->WorkingBuffAddr;
    for (MUINT32 i=0; i<ImageInfo->ImgNum; i++)
    {
        warp_cal_data->ImgAddr[i] = (MUINT8 *)(ImageInfo->ImgAddr[i]);
    }
    warp_cal_data->WarpMatrix = (MFLOAT *)(warp_cal_data->ProcBufAddr);
    warp_cal_data->VtxData = (vtx_fmt *)((MUINT8 *)warp_cal_data->WarpMatrix + WARP_MATRIX_SIZE*sizeof(MFLOAT));
    warp_cal_data->EleData = (MUINT16 *)((MUINT8 *)warp_cal_data->VtxData + WARP_VTX_DATA_SIZE*sizeof(vtx_fmt));
    warp_cal_data->ImgBufferAddr = (MUINT8 *)((MUINT8 *)warp_cal_data->EleData + WARP_ELE_DATA_SIZE*sizeof(MUINT16));

    // image init
    warp_cal_data->ImgNum = ImageInfo->ImgNum;
    warp_cal_data->ImgFmt = ImageInfo->ImgFmt;
    warp_cal_data->ImgWidth = ImageInfo->Width;
    warp_cal_data->ImgHeight = ImageInfo->Height;

    // image fortmat conversion
    UTL_IMAGE_FORMAT_ENUM ImgFmt = warp_cal_data->ImgFmt;
    if(ImgFmt == UTL_IMAGE_FORMAT_NV21)
    {
        P_UTIL_BASE_IMAGE_STRUCT p_img = &g_image_data;
        MINT32 ImgNum = warp_cal_data->ImgNum;
        MINT32 ImgWidth = warp_cal_data->ImgWidth;
        MINT32 ImgHeight = warp_cal_data->ImgHeight;
        //MINT32 ImgSize = ImgWidth*ImgHeight*3/2;
        for(MINT32 i=0; i<ImgNum; i++)
        {
            p_img->data = warp_cal_data->ImgAddr[i];
            p_img->width = ImgWidth;
            p_img->height = ImgHeight;
            UtlNV21toI420(p_img, warp_cal_data->ImgBufferAddr);
        }
    }

    // matrix init
    memcpy(warp_cal_data->WarpMatrix, (MFLOAT *)(ImageInfo->Hmtx), WARP_MATRIX_SIZE*sizeof(MFLOAT));

    // clipping info init
    warp_cal_data->ClipWidth = ImageInfo->ClipWidth;
    warp_cal_data->ClipHeight = ImageInfo->ClipHeight;
    memcpy(warp_cal_data->ClipX, ImageInfo->ClipX, WARP_MAX_IMG_NUM*sizeof(MUINT32));
    memcpy(warp_cal_data->ClipY, ImageInfo->ClipY, WARP_MAX_IMG_NUM*sizeof(MUINT32));

    // opengl init
    SourceWidth = warp_cal_data->ImgWidth;
    SourceHeight = warp_cal_data->ImgHeight;


    /*
        Step 0 - Create and initialize the window resource.
        The image file format is ppm R8G8B8 format.
    */
    if (UTIL_OK != igCreateBufferObject(SourceWidth, SourceHeight, fbo, rbo))
    {
        return;
    }
    warp_cal_data->fbo = fbo;
    warp_cal_data->rbo = rbo;


    /*
        Step 1 - Load the shader.
        At this point everything is initialized and we're ready to use.
        We will get both the vertex shader and fragment shader handle in this stage
    */
#if !defined(SIM_MAIN) && !defined(LOAD_SHAD_STR)
    if ((UTIL_OK != igLoadShaderBinary(&uiVertShader, (const char *)shd_bin_1, sizeof(shd_bin_1), GL_VERTEX_SHADER)) ||
        (UTIL_OK != igLoadShaderBinary(&uiFragShader, (const char *)shd_bin_2, sizeof(shd_bin_2), GL_FRAGMENT_SHADER)))
    {
        return;
    }
#else
    if ((UTIL_OK != igLoadShaderString(&uiVertShader, (const char *)gVertShaderStr, GL_VERTEX_SHADER)) ||
        (UTIL_OK != igLoadShaderString(&uiFragShader, (const char *)gFragShaderStr, GL_FRAGMENT_SHADER)))
    {
        return;
    }
#endif
    warp_cal_data->uiVertShader = uiVertShader;
    warp_cal_data->uiFragShader = uiFragShader;


    /*
        Step 2 - Create the program and link with shader.
        Specify the vpg_ref data if needed. Each VPG owns a vec4 reference data.
        In this case, there is no vpg_ref. Set ref_buffer pointer to 0
    */
    if (UTIL_OK != igCreateProgram(&uiProgramObject, uiVertShader, uiFragShader, 0))
    {
        return;
    }
    warp_cal_data->uiProgramObject = uiProgramObject;
}

void Warping(MINT32 ImgIdx, void *pParaOut)
{
    // intermediate data assignment
    P_MAV_WARP_RESULT_INFO ResultInfo = (MAV_WARP_RESULT_INFO *) pParaOut;
    MAV_WARP_CAL_STRUCT *warp_cal_data = &g_wap_cal_data;
    MINT32 SourceWidth = warp_cal_data->ImgWidth;
    MINT32 SourceHeight = warp_cal_data->ImgHeight;
    MFLOAT pWrapMatrix[9];
    MINT32 SourceWidthUV = SourceWidth/2;
    MINT32 SourceHeightUV = SourceHeight/2;
    MINT32 ClipX = warp_cal_data->ClipX[ImgIdx];
    MINT32 ClipY = warp_cal_data->ClipY[ImgIdx];
    MUINT32 ClipWidth = warp_cal_data->ClipWidth;
    MUINT32 ClipHeight = warp_cal_data->ClipHeight;
    vtx_fmt *VtxData = warp_cal_data->VtxData;
    MUINT16 *EleData = warp_cal_data->EleData;

    // for opengl setup
    GLuint uiProgramObject = warp_cal_data->uiProgramObject;
    GLuint uiTextureObject[3];
    GLuint elenum;
    MINT32 i32LocMat, i32LocDeltaX, i32LocTgtSize, i32LocOrigin, i32LocOffsetX, i32LocOriginX;
    GLuint uiVBO[2];

    // image buffer assignment
    MINT32 ImgSize;
    UTL_IMAGE_FORMAT_ENUM ImgFmt = warp_cal_data->ImgFmt;
    switch(ImgFmt)
    {
    case UTL_IMAGE_FORMAT_YV12:
    case UTL_IMAGE_FORMAT_NV21:
        ImgSize = SourceWidth*SourceHeight*3/2;
        break;
    case UTL_IMAGE_FORMAT_RGB565:
        ImgSize = SourceWidth*SourceHeight*2;
        break;
    default:
        ResultInfo->RetCode = CORE_WARP_ERR_INCORRECT_IMAGE_FORMAT;
        LOGD("[MavWarpCore] Error Message: %s\n", CORE_GET_ERRCODE_NAME(ResultInfo->RetCode));
        return;
    }
    MUINT8 *SrcBuffer = (warp_cal_data->ImgAddr[ImgIdx]);
    MUINT8 *SrcBufferU = SrcBuffer + ClipWidth*ClipHeight;
    MUINT8 *SrcBufferV = SrcBufferU + ClipWidth*ClipHeight/4;

    /*
        Step 3 - Load the source image from file.
        Use the MIRRORED image address mode to sample the data,
        when the address in out of the source address range.
        Bind the source image into the texture unit 0 in fragment shader.
    */
    igLoadImage(SrcBuffer, GL_CLAMP_TO_EDGE, IG_YUV420P, SourceWidth, SourceHeight, uiTextureObject);


    /*
        Step 4 - Create the virtual processing element.
        In this case, set the VPG as a 1x1 Quad.
    */
    /* Used to hold the vbo handle */
    glGenBuffers(2, uiVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBO[1]);

    MFLOAT grid_x = 2.0f/SourceWidth;
    MFLOAT grid_y = 2.0f/SourceHeight;
    MFLOAT grid_u = 1.0f;
    MFLOAT grid_v = 1.0f;

    //Prepare vtx data
    VtxData[0].x = -1.0f + ClipX * grid_x;
    VtxData[0].y = -1.0f + ClipY * grid_y;
    VtxData[0].u =  0.0f + ClipX * grid_u;
    VtxData[0].v =  0.0f + ClipY * grid_v;
    VtxData[1].x = VtxData[0].x;
    VtxData[1].y = -1.0f + (ClipY+ClipHeight) * grid_y;
    VtxData[1].u = VtxData[0].u;
    VtxData[1].v =  0.0f + (ClipY+ClipHeight) * grid_v;
    VtxData[2].x = -1.0f + (ClipX+ClipWidth) * grid_x;
    VtxData[2].y = VtxData[0].y;
    VtxData[2].u =  0.0f + (ClipX+ClipWidth) * grid_u;
    VtxData[2].v = VtxData[0].v;
    VtxData[3].x = VtxData[2].x;
    VtxData[3].y = VtxData[1].y;
    VtxData[3].u = VtxData[2].u;
    VtxData[3].v = VtxData[1].v;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*WARP_VTX_DATA_SIZE, VtxData, GL_STATIC_DRAW);

    //Prepare element data
    EleData[0] = 0;
    EleData[1] = 1;
    EleData[2] = 2;
    EleData[3] = 2;
    EleData[4] = 1;
    EleData[5] = 3;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*WARP_ELE_DATA_SIZE, EleData, GL_STATIC_DRAW);

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));
    elenum = WARP_ELE_DATA_SIZE;
    warp_cal_data->uiVBO[0] = uiVBO[0];
    warp_cal_data->uiVBO[1] = uiVBO[1];


    /*
        Step 5 - Bind the non-compile time constant data for the shader
        In this step, use the gl[x]Uniform relative API directly.
        In OpenGL, the matrix form is column major.
        | mat[0], mat[3], mat[6] | * | vtx.x |
        | mat[1], mat[4], mat[7] |   | vtx.y |
        | mat[2], mat[5], mat[8] |   | vtx.z |
    */
    memcpy(pWrapMatrix, warp_cal_data->WarpMatrix+ImgIdx*9, 9*sizeof(float));
    utilSwap(&(pWrapMatrix[1]), &(pWrapMatrix[3]));
    utilSwap(&(pWrapMatrix[2]), &(pWrapMatrix[6]));
    utilSwap(&(pWrapMatrix[5]), &(pWrapMatrix[7]));

    pWrapMatrix[0] /= SourceWidth;     pWrapMatrix[3] /= SourceWidth;     pWrapMatrix[6] /= SourceWidth;
    pWrapMatrix[1] /= SourceHeight;    pWrapMatrix[4] /= SourceHeight;    pWrapMatrix[7] /= SourceHeight;

    i32LocMat = glGetUniformLocation(uiProgramObject, "matrix");
    glUniformMatrix3fv(i32LocMat, 1, GL_FALSE, pWrapMatrix);
    i32LocDeltaX = glGetUniformLocation(uiProgramObject, "delta_x");
    glUniform3f(i32LocDeltaX, pWrapMatrix[0], pWrapMatrix[1], pWrapMatrix[2]);
    i32LocTgtSize = glGetUniformLocation(uiProgramObject, "target_size");
    glUniform2f(i32LocTgtSize, MFLOAT(SourceWidth), MFLOAT(SourceHeight));
    i32LocOrigin = glGetUniformLocation(uiProgramObject, "origin");
    glUniform2f(i32LocOrigin, MFLOAT(ClipX), MFLOAT(ClipY));
    i32LocOffsetX = glGetUniformLocation(uiProgramObject, "offset_x");
    glUniform1f(i32LocOffsetX, 1.5f);
    i32LocOriginX = glGetUniformLocation(uiProgramObject, "origin_x");
    glUniform1f(i32LocOriginX, VtxData[0].x);


    /*
        Step 6 - Now we are ready to go to trigger GPU
    */
    // Sets the clear color.
    // The colours are passed per channel
    // (red,green,blue,alpha) as float values from 0.0 to 1.0
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //=====================================================
    // Y texture
    UTIL_CLIP_IMAGE_STRUCT y_data;
    y_data.data = SrcBuffer;
    y_data.width = SourceWidth;
    y_data.height = SourceHeight;
    y_data.clip_width = ClipWidth/4;
    y_data.clip_height = ClipHeight;
    y_data.clip_x = ClipX;
    y_data.clip_y = ClipY;
    igStartRendering(uiProgramObject, "SrcImg", uiTextureObject[0], elenum, &y_data);
    //=====================================================

    //=====================================================
    // U texture
    UTIL_CLIP_IMAGE_STRUCT uv_data;
    uv_data.data = SrcBufferU;
    uv_data.width = SourceWidthUV;
    uv_data.height = SourceHeightUV;
    uv_data.clip_width = ClipWidth/8;
    uv_data.clip_height = ClipHeight/2;
    uv_data.clip_x = ClipX/2;
    uv_data.clip_y = ClipY/2;
    igStartRendering(uiProgramObject, "SrcImg", uiTextureObject[1], elenum, &uv_data);
    //=====================================================

    //=====================================================
    // V texture
    uv_data.data = SrcBufferV;
    igStartRendering(uiProgramObject, "SrcImg", uiTextureObject[2], elenum, &uv_data);
    //=====================================================


    // image format conversion
    if(ImgFmt == UTL_IMAGE_FORMAT_NV21)
    {
        P_UTIL_BASE_IMAGE_STRUCT p_img = &g_image_data;
        p_img->data = SrcBuffer;
        p_img->width = ClipWidth;
        p_img->height = ClipHeight;
        UtlI420toNV21(p_img, warp_cal_data->ImgBufferAddr);
    }


    ResultInfo->RetCode = CORE_OK;
    ResultInfo->Width = ClipWidth;
    ResultInfo->Height = ClipHeight;
    igFreeImage(3, uiTextureObject);
}

void WarpingFinish(void)
{
    /*
        Step 7 - Finish all the GPU task
        Delete all the handle to prevent memory leakage
    */
    MAV_WARP_CAL_STRUCT *warp_cal_data = &g_wap_cal_data;
    igFreeProgram(warp_cal_data->uiProgramObject);
    igFreeShader(warp_cal_data->uiFragShader);
    igFreeShader(warp_cal_data->uiVertShader);
    igFreeVPE(warp_cal_data->uiVBO);
    igFreeBufferObject(warp_cal_data->fbo, warp_cal_data->rbo);
}

/******************************************************************************
SW Warping Main Process
******************************************************************************/
void SwWarpingInit(void *pParaIn)
{
    // allocate memory
    P_MAV_WARP_IMAGE_INFO ImageInfo = (MAV_WARP_IMAGE_INFO *)pParaIn;
    MAV_WARP_SW_CAL_STRUCT *warp_cal_data = &g_wap_sw_cal_data;
    warp_cal_data->ProcBufAddr = ImageInfo->WorkingBuffAddr;
    for (MUINT32 i=0; i<ImageInfo->ImgNum; i++)
    {
        warp_cal_data->ImgAddr[i] = (MUINT8 *)(ImageInfo->ImgAddr[i]);
    }
    warp_cal_data->WarpMatrix = (MFLOAT *)(warp_cal_data->ProcBufAddr);
    warp_cal_data->ImgBufferAddr = (MUINT8 *)((MUINT8 *)warp_cal_data->WarpMatrix + WARP_MATRIX_SIZE*sizeof(MFLOAT));

    // init
    warp_cal_data->ImgNum = ImageInfo->ImgNum;
    warp_cal_data->ImgFmt = ImageInfo->ImgFmt;
    warp_cal_data->ImgWidth = ImageInfo->Width;
    warp_cal_data->ImgHeight = ImageInfo->Height;

    // image fortmat conversion
    UTL_IMAGE_FORMAT_ENUM ImgFmt = warp_cal_data->ImgFmt;
    if(ImgFmt == UTL_IMAGE_FORMAT_NV21)
    {
        P_UTIL_BASE_IMAGE_STRUCT p_img = &g_image_data;
        MINT32 ImgNum = warp_cal_data->ImgNum;
        MINT32 ImgWidth = warp_cal_data->ImgWidth;
        MINT32 ImgHeight = warp_cal_data->ImgHeight;
        //MINT32 ImgSize = ImgWidth*ImgHeight*3/2;
        for(MINT32 i=0; i<ImgNum; i++)
        {
            p_img->data = warp_cal_data->ImgAddr[i];
            p_img->width = ImgWidth;
            p_img->height = ImgHeight;
            UtlNV21toI420(p_img, (void *)warp_cal_data->ProcBufAddr);
        }
    }

    // matrix init
    memcpy(warp_cal_data->WarpMatrix,
           (MFLOAT *)(ImageInfo->Hmtx),
           (ImageInfo->ImgNum)*9*sizeof(MFLOAT));

    // clipping information
    warp_cal_data->ClipWidth = ImageInfo->ClipWidth;
    warp_cal_data->ClipHeight = ImageInfo->ClipHeight;
    memcpy(warp_cal_data->ClipX, ImageInfo->ClipX, ImageInfo->ImgNum*sizeof(MUINT32));
    memcpy(warp_cal_data->ClipY, ImageInfo->ClipY, ImageInfo->ImgNum*sizeof(MUINT32));
}

void SwWarping(MINT32 ImgIdx, void *pParaOut)
{
    // intermediate data assignment
    P_MAV_WARP_RESULT_INFO ResultInfo = (MAV_WARP_RESULT_INFO *)pParaOut;
    MAV_WARP_SW_CAL_STRUCT *warp_cal_data = &g_wap_sw_cal_data;
    MINT32 ImgWidth = warp_cal_data->ImgWidth;
    MINT32 ImgHeight = warp_cal_data->ImgHeight;
    MINT32 ClipWidth = warp_cal_data->ClipWidth;
    MINT32 ClipHeight = warp_cal_data->ClipHeight;
    //Clang build error fix
    //MINT32 LeftTop[2] = { warp_cal_data->ClipX[ImgIdx], warp_cal_data->ClipY[ImgIdx] };
    MINT32 LeftTop[2] = { static_cast<MINT32>(warp_cal_data->ClipX[ImgIdx]), static_cast<MINT32>(warp_cal_data->ClipY[ImgIdx]) };
    ResultInfo->RetCode = CORE_OK;

    // variables
    //UTIL_ERRCODE_ENUM result = UTIL_OK;

    // warping matrix assignement
    MFLOAT *pWrapMatrix = (warp_cal_data->WarpMatrix + ImgIdx*9);

    // image buffer assignment
    MINT32 ImgSize;
    MINT32 ClipSize;
    UTL_IMAGE_FORMAT_ENUM ImgFmt = warp_cal_data->ImgFmt;
    switch(ImgFmt)
    {
    case UTL_IMAGE_FORMAT_YV12:
    case UTL_IMAGE_FORMAT_NV21:
        ImgSize = ImgWidth*ImgHeight*3/2;
        ClipSize = ClipWidth*ClipHeight*3/2;
        break;
    case UTL_IMAGE_FORMAT_RGB565:
        ImgSize = ImgWidth*ImgHeight*2;
        ClipSize = ClipWidth*ClipHeight*2;
        break;
    default:
        ResultInfo->RetCode = CORE_WARP_ERR_INCORRECT_IMAGE_FORMAT;
        LOGD("[MavWarpSwCore] Error Message: %s\n", CORE_GET_ERRCODE_NAME(ResultInfo->RetCode));
        return;
    }
    MUINT8 *SrcBuffer = (warp_cal_data->ImgAddr[ImgIdx]);
    MUINT8 *DstBuffer = warp_cal_data->ImgBufferAddr;

    // warping for three planes
    if( (ImgFmt == UTL_IMAGE_FORMAT_YV12) || (ImgFmt == UTL_IMAGE_FORMAT_NV21) )
    {
        // matrix settings
        MFLOAT invH[9];
        memcpy(invH, pWrapMatrix, 9*sizeof(MFLOAT));
        MFLOAT row_step[3] = { invH[1], invH[4], invH[7] };
        MFLOAT row_pxl[3]  = { invH[0]*LeftTop[0]+invH[1]*LeftTop[1]+invH[2],
                               invH[3]*LeftTop[0]+invH[4]*LeftTop[1]+invH[5],
                               invH[6]*LeftTop[0]+invH[7]*LeftTop[1]+invH[8] };
        MFLOAT col_step[3] = { invH[0], invH[3], invH[6] };
        MFLOAT col_pxl[3];

        // boundary seetings
        MINT32 flag;
        MUINT32 luma_horz_bound4 = ((warp_cal_data->ImgWidth) - 1)<<2;
        MUINT32 luma_vert_bound4 = ((warp_cal_data->ImgHeight) - 1)<<2;
        MUINT32 chroma_horz_bound4 = ((warp_cal_data->ImgWidth>>1) - 1)<<2;
        MUINT32 chroma_vert_bound4 = ((warp_cal_data->ImgHeight>>1) - 1)<<2;

        // Y settings
        MUINT8 *SrcPlaneY = SrcBuffer;
        MUINT8 *DstPlaneY = DstBuffer;
        MINT32 w_l = warp_cal_data->ClipWidth;
        MINT32 stride_l = warp_cal_data->ImgWidth;

        // UV settings
        MUINT8 *SrcPlaneU = SrcPlaneY + ImgWidth*ImgHeight;
        MUINT8 *DstPlaneU = DstPlaneY + ClipWidth*ClipHeight;
        MUINT8 *SrcPlaneV = SrcPlaneU + (ImgWidth*ImgHeight>>2);
        MUINT8 *DstPlaneV = DstPlaneU + (ClipWidth*ClipHeight>>2);
        MINT32 w_c = (warp_cal_data->ClipWidth)>>1;
        MINT32 h_c = (warp_cal_data->ClipHeight)>>1;
        MINT32 stride_c = (warp_cal_data->ImgWidth)>>1;

        for(MINT32 i=h_c; i>0; i--)
        {
            // column pixel assignment
            col_pxl[0] = row_pxl[0];
            col_pxl[1] = row_pxl[1];
            col_pxl[2] = row_pxl[2];
            for (MINT32 j=w_c; j>0; j--)
            {
                // store information for next row
                MFLOAT col_pxl_next_row[3] = { col_pxl[0]+row_step[0], col_pxl[1]+row_step[1], col_pxl[2]+row_step[2] };
                MUINT8 *DstPlaneY_next_row = DstPlaneY + w_l;

                // compute warped coordinate for Y00 position
                MINT32 x4 = MINT32(4*col_pxl[0]/col_pxl[2]);
                MINT32 y4 = MINT32(4*col_pxl[1]/col_pxl[2]);
                if( unsigned(x4)<luma_horz_bound4 && unsigned(y4)<luma_vert_bound4 )
                {
                    // Y00 pixel
                    *DstPlaneY = SimpleBilinear(SrcPlaneY, x4, y4, stride_l);
                }
                else
                {
                    *DstPlaneY = 0;
                }
                if( unsigned(x4>>1)<chroma_horz_bound4 && unsigned(y4>>1)<chroma_vert_bound4 )
                {
                    // U pixel
                    *DstPlaneU = SimpleBilinear(SrcPlaneU, (x4>>1), (y4>>1), stride_c);

                    // V pixel
                    *DstPlaneV = SimpleBilinear(SrcPlaneV, (x4>>1), (y4>>1), stride_c);
                }
                else
                {
                    *DstPlaneU = 0;
                    *DstPlaneV = 0;
                }
                DstPlaneY++;
                DstPlaneU++;
                DstPlaneV++;
                col_pxl[0] += col_step[0];
                col_pxl[1] += col_step[1];
                col_pxl[2] += col_step[2];

                // compute warped coordinate for Y01 position
                x4 = MINT32(4*col_pxl[0]/col_pxl[2]);
                y4 = MINT32(4*col_pxl[1]/col_pxl[2]);
                flag = unsigned(x4)<luma_horz_bound4 && unsigned(y4)<luma_vert_bound4;
                *DstPlaneY = flag ? SimpleBilinear(SrcPlaneY, x4, y4, stride_l) : 0 ;
                DstPlaneY++;
                col_pxl[0] += col_step[0];
                col_pxl[1] += col_step[1];
                col_pxl[2] += col_step[2];

                // compute warped coordinate for Y10 position
                x4 = MINT32(4*col_pxl_next_row[0]/col_pxl_next_row[2]);
                y4 = MINT32(4*col_pxl_next_row[1]/col_pxl_next_row[2]);
                flag = unsigned(x4)<luma_horz_bound4 && unsigned(y4)<luma_vert_bound4;
                *DstPlaneY_next_row = flag ? SimpleBilinear(SrcPlaneY, x4, y4, stride_l) : 0 ;
                DstPlaneY_next_row++;
                col_pxl_next_row[0] += col_step[0];
                col_pxl_next_row[1] += col_step[1];
                col_pxl_next_row[2] += col_step[2];

                // compute warped coordinate for Y11 position
                x4 = MINT32(4*col_pxl_next_row[0]/col_pxl_next_row[2]);
                y4 = MINT32(4*col_pxl_next_row[1]/col_pxl_next_row[2]);
                flag = unsigned(x4)<luma_horz_bound4 && unsigned(y4)<luma_vert_bound4;
                *DstPlaneY_next_row = flag ? SimpleBilinear(SrcPlaneY, x4, y4, stride_l) : 0 ;
            }
            DstPlaneY += w_l;
            row_pxl[0] += row_step[0]*2;
            row_pxl[1] += row_step[1]*2;
            row_pxl[2] += row_step[2]*2;
        }
    }
    if(ImgFmt == UTL_IMAGE_FORMAT_NV21)
    {
        // convert from YV12 to NV21
        P_UTIL_BASE_IMAGE_STRUCT p_img = &g_image_data;
        p_img->data = DstBuffer;
        p_img->width = ClipWidth;
        p_img->height = ClipHeight;
        UtlI420toNV21(p_img, (void *)SrcBuffer);
        memcpy(SrcBuffer, DstBuffer, ClipSize*sizeof(MUINT8));
    }
    else if(ImgFmt == UTL_IMAGE_FORMAT_YV12)
    {
        memcpy(SrcBuffer, DstBuffer, ClipSize*sizeof(MUINT8));
    }

    ResultInfo->Width = ClipWidth;
    ResultInfo->Height = ClipHeight;
}

static MUINT8 SimpleBilinear(MUINT8 *src, MINT32 x4, MINT32 y4, MINT32 stride)
{
    MUINT8 *p_src;
    MUINT8 *p_src_next_row;
    MINT32 w1_x, w2_x;
    MINT32 w1_y, w2_y;
    MINT32 pixel1, pixel2, value;
    MINT32 bit_mask = 0x3;

    MINT32 x = x4>>2;
    MINT32 y = y4>>2;

    MINT32 idx = y*stride + x;
    w2_x = x4 & bit_mask;    w1_x = 4 - w2_x;
    w2_y = y4 & bit_mask;    w1_y = 4 - w2_y;

    p_src = src + idx;
    p_src_next_row = p_src + stride;
    pixel1 = (*p_src)*w1_x + (*(p_src+1))*w2_x;
    pixel2 = (*p_src_next_row)*w1_x + (*(p_src_next_row+1))*w2_x;
    value = ( pixel1*w1_y + pixel2*w2_y + 8 )>>4;

    return MINT8(value);
}

