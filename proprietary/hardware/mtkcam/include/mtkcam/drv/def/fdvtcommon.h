/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_FDVT_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_FDVT_COMMON_H_
//
#include <mtkcam/def/common.h>
//#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#include <utils/Trace.h>
#define DRV_TRACE_NAME_LENGTH                   32
#define DRV_TRACE_CALL()                        ATRACE_CALL()
#define DRV_TRACE_NAME(name)                    ATRACE_NAME(name)
#define DRV_TRACE_INT(name, value)              ATRACE_INT(name, value)
#define DRV_TRACE_BEGIN(name)                   ATRACE_BEGIN(name)
#define DRV_TRACE_END()                         ATRACE_END()
#define DRV_TRACE_ASYNC_BEGIN(name, cookie)     ATRACE_ASYNC_BEGIN(name, cookie)
#define DRV_TRACE_ASYNC_END(name, cookie)       ATRACE_ASYNC_END(name, cookie)
#define DRV_TRACE_FMT_BEGIN(fmt, arg...)                    \
do{                                                         \
    if( ATRACE_ENABLED() )                                  \
    {                                                       \
        char buf[DRV_TRACE_NAME_LENGTH];                    \
        snprintf(buf, DRV_TRACE_NAME_LENGTH, fmt, ##arg);   \
        DRV_TRACE_BEGIN(buf);                               \
    }                                                       \
}while(0)
#define DRV_TRACE_FMT_END()                     DRV_TRACE_END()
#define MAX_FACE_NUM                            1024


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

enum SUB_ENGINE_ID{
   eFDVT = 0,
   eENGINE_MAX
};

/*FDVT_CTRL*/
enum FDVTMODE {
    FDMODE = 0,
    ATTRIBUTEMODE = 1,
    POSEMODE = 2
};

enum FDVTFORMAT {
    FMT_NA = 0,
    FMT_YUV_2P = 1,
    FMT_YVU_2P = 2,
    FMT_YUYV = 3, //1 plane
    FMT_YVYU = 4, //1 plane
    FMT_UYVY = 5, //1 plane
    FMT_VYUY = 6, //1 plane
    FMT_MONO = 7         // AIE2.0
};

enum FDVTINPUTDEGREE {
    DEGREE_0 = 0,
    DEGREE_90 = 1,
    DEGREE_270 = 2,
    DEGREE_180 = 3
};

struct EGNBufInfo
{
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;    //  Vir Address of pool
    MUINTPTR    u4BufPA;    //  Phy Address of pool
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4Stride;   //  Buffer Stride
    MUINT32     width;
    MUINT32     height;
        EGNBufInfo()
        : memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        , width(0)
        , height(0)
        {
        }
};

struct EGNInitInfo
{
    MUINT16 MAX_SRC_IMG_WIDTH;
    MUINT16 MAX_SRC_IMG_HEIGHT;
    signed short feature_threshold;
    MUINT16 pyramid_width; // AIE2.0 MAX: 640
    MBOOL IS_FDVT_SECURE;
    MUINT32 SEC_MEM_TYPE;
        EGNInitInfo()
        : MAX_SRC_IMG_WIDTH(0)
        , MAX_SRC_IMG_HEIGHT(0)
        , feature_threshold(5)
        , pyramid_width(400)
        , IS_FDVT_SECURE(0)
        , SEC_MEM_TYPE(1){}
};

struct FDRESULT
{
    MUINT16 anchor_x0[MAX_FACE_NUM];
    MUINT16 anchor_x1[MAX_FACE_NUM];
    MUINT16 anchor_y0[MAX_FACE_NUM];
    MUINT16 anchor_y1[MAX_FACE_NUM];
    MUINT16 landmark_x0[MAX_FACE_NUM];
    MUINT16 landmark_x1[MAX_FACE_NUM];
    MUINT16 landmark_x2[MAX_FACE_NUM];
    MUINT16 landmark_x3[MAX_FACE_NUM];
    MUINT16 landmark_x4[MAX_FACE_NUM];
    MUINT16 landmark_x5[MAX_FACE_NUM];
    MUINT16 landmark_x6[MAX_FACE_NUM];
    MUINT16 landmark_y0[MAX_FACE_NUM];
    MUINT16 landmark_y1[MAX_FACE_NUM];
    MUINT16 landmark_y2[MAX_FACE_NUM];
    MUINT16 landmark_y3[MAX_FACE_NUM];
    MUINT16 landmark_y4[MAX_FACE_NUM];
    MUINT16 landmark_y5[MAX_FACE_NUM];
    MUINT16 landmark_y6[MAX_FACE_NUM];
    signed short anchor_score[MAX_FACE_NUM];
    signed short landmark_score0[MAX_FACE_NUM];
    signed short landmark_score1[MAX_FACE_NUM];
    signed short landmark_score2[MAX_FACE_NUM];
    signed short landmark_score3[MAX_FACE_NUM];
    signed short landmark_score4[MAX_FACE_NUM];
    signed short landmark_score5[MAX_FACE_NUM];
    signed short landmark_score6[MAX_FACE_NUM];
    MUINT16 face_result_index[MAX_FACE_NUM];
    MUINT16 anchor_index [MAX_FACE_NUM];
    MUINT32 fd_partial_result;

    FDRESULT(): anchor_x0{0},
                anchor_x1{0},
                anchor_y0{0},
                anchor_y1{0},
                landmark_x0{0},
                landmark_x1{0},
                landmark_x2{0},
                landmark_x3{0},
                landmark_x4{0},
                landmark_x5{0},
                landmark_x6{0},
                landmark_y0{0},
                landmark_y1{0},
                landmark_y2{0},
                landmark_y3{0},
                landmark_y4{0},
                landmark_y5{0},
                landmark_y6{0},
                anchor_score{0},
                landmark_score0{0},
                landmark_score1{0},
                landmark_score2{0},
                landmark_score3{0},
                landmark_score4{0},
                landmark_score5{0},
                landmark_score6{0},
                face_result_index{0},
                anchor_index {0},
                fd_partial_result(0){}
};

struct FD_RESULT
{
    struct FDRESULT PYRAMID0_RESULT;
    struct FDRESULT PYRAMID1_RESULT;
    struct FDRESULT PYRAMID2_RESULT;
    MUINT16 FD_TOTAL_NUM;
    MUINT64 *FD_raw_result_0_va; // AIE2.0 for bit-ture test only
    MUINT64 *FD_raw_result_1_va; // AIE2.0 for bit-ture test only
    MUINT64 *FD_raw_result_2_va; // AIE2.0 for bit-ture test only

    FD_RESULT(): FD_TOTAL_NUM(0),
                 FD_raw_result_0_va(NULL),
                 FD_raw_result_1_va(NULL),
                 FD_raw_result_2_va(NULL){}
};

struct RACERESULT
{
    signed short RESULT[4][64]; // RESULT[Channel][Feature]
    RACERESULT(): RESULT{0}{}
};

struct GENDERRESULT
{
    signed short RESULT[2][64]; // RESULT[Channel][Feature]
    GENDERRESULT(): RESULT{0}{}
};

struct RIPRESULT
{
    signed short RESULT[7][64]; // RESULT[Channel][Feature]
    RIPRESULT(): RESULT{0}{}
};

struct ROPRESULT
{
    signed short RESULT[3][64]; // RESULT[Channel][Feature]
    ROPRESULT(): RESULT{0}{}
};

struct MERGED_RACERESULT // AIE2.0
{
    signed short RESULT[4]; // RESULT[Feature]
    MERGED_RACERESULT(): RESULT{0}{}
};

struct MERGED_GENDERRESULT // AIE2.0
{
    signed short RESULT[2]; // RESULT[Feature]
    MERGED_GENDERRESULT(): RESULT{0}{}
};

struct MERGED_RIPRESULT // AIE2.0
{
    signed short RESULT[7]; // RESULT[Feature]
    MERGED_RIPRESULT(): RESULT{0}{}
};

struct MERGED_ROPRESULT // AIE2.0
{
    signed short RESULT[3]; // RESULT[Feature]
    MERGED_ROPRESULT(): RESULT{0}{}
};


struct ATTRIBUTE_RESULT
{
    struct GENDERRESULT GENDER_RESULT;
    struct RACERESULT RACE_RESULT;
    struct MERGED_GENDERRESULT MERGED_GENDER_RESULT;
    struct MERGED_RACERESULT MERGED_RACE_RESULT;
    MUINT64 *ATTR_raw_result_0_va; // AIE2.0 for bit-ture test only
    MUINT64 *ATTR_raw_result_1_va; // AIE2.0 for bit-ture test only
    ATTRIBUTE_RESULT(): ATTR_raw_result_0_va(NULL),
                        ATTR_raw_result_1_va(NULL){}
};

struct POSE_RESULT
{
    struct RIPRESULT RIP_RESULT;
    struct ROPRESULT ROP_RESULT;
    struct MERGED_RIPRESULT MERGED_RIP_RESULT;
    struct MERGED_ROPRESULT MERGED_ROP_RESULT;
    MUINT64 *POSE_raw_result_0_va; // AIE2.0 for bit-ture test only
    MUINT64 *POSE_raw_result_1_va; // AIE2.0 for bit-ture test only
    POSE_RESULT(): POSE_raw_result_0_va(NULL),
                   POSE_raw_result_1_va(NULL){}
};

struct feedback {
    MUINT32 reg1;
    MUINT32 reg2;
    feedback(): reg1(0)
              , reg2(0)
              {}
};

typedef struct FDVT_ROI // AIE2.0
{
    MUINT x1;
    MUINT y1;
    MUINT x2;
    MUINT y2;
    FDVT_ROI(): x1(0),
                y1(0),
                x2(0),
                y2(0){}
}FDVT_ROI;

typedef struct FDVT_PADDING // AIE2.0
{
    MUINT left;
    MUINT right;
    MUINT down;
    MUINT up;
    FDVT_PADDING(): left(0),
                    right(0),
                    down(0),
                    up(0){}
}FDVT_PADDING;

struct  FDVTConfig
{
    FDVTMODE FD_MODE;
    FDVTFORMAT SRC_IMG_FMT;
    MUINT16 SRC_IMG_WIDTH;
    MUINT16 SRC_IMG_HEIGHT;
    MUINT16 SRC_IMG_STRIDE;   // AIE2.0
    FDVTINPUTDEGREE INPUT_ROTATE_DEGREE;
    bool enROI;               // AIE2.0
    FDVT_ROI src_roi;         // AIE2.0
    bool enPadding;           // AIE2.0
    FDVT_PADDING src_padding; // AIE2.0
    MUINT64 *source_img_address;
    MUINT64 *source_img_address_UV;
    unsigned int Fd_version;
    unsigned int Attr_version;
    unsigned int Pose_version;
    struct FD_RESULT FDOUTPUT;
    struct ATTRIBUTE_RESULT ATTRIBUTEOUTPUT;
    struct POSE_RESULT POSEOUTPUT;
    feedback feedback;
    MUINT64 *pY2R_config;
    MUINT64 *pRS_config;
    MUINT64 *pFd_config;

    FDVTConfig(): FD_MODE(FDMODE),
                  SRC_IMG_FMT(FMT_NA),
                  SRC_IMG_WIDTH(0),
                  SRC_IMG_HEIGHT(0),
                  SRC_IMG_STRIDE(SRC_IMG_WIDTH),
                  INPUT_ROTATE_DEGREE(DEGREE_0),
                  enROI(false),
                  enPadding(false),
                  source_img_address(NULL),
                  source_img_address_UV(NULL),
                  Fd_version(0),
                  Attr_version(0),
                  Pose_version(0),
                  pY2R_config(NULL),
                  pRS_config(NULL),
                  pFd_config(NULL){}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_FDVT_COMMON_H_

