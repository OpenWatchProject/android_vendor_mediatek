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

#ifndef _MTK_Detection_H
#define _MTK_Detection_H

#ifdef SIM_MAIN
#include "..\..\source\pthread\include\pthread.h"
//#include <pthread.h>
#endif

#include "kal_release.h"
#include "MTKDetectionType.h"

#define SmileDetect              (1)

#define FD_MAX_FACE_NUM          (15)
#define FD_SCALE_NUM             (14)
#define CAMERA_FD_MAX_NO         (1024)
#define MAX_FACE_SEL_NUM         (CAMERA_FD_MAX_NO+2)
#define UTL_NUMBER_OF_BINS       (12)                // Confidence table size for one feature
#define FD_POSE_NUM              (12)                // Total pose number of each degree
#define FD_CASCADE_LAYER_MAX_NUM (1000)        // maximum cascade layer number of 24x24 learning data
#define FACE_SIZE_NUM_MAX        (14)                // The max number of face sizes could be detected, for feature scaling
#define UTL_HAAR_PIX_MAX         (2)                // 2 for shrink, 8 for all, pixel position numbers for one feature

#define MAX_CROP_NUM             (16) // maximum number of crop patches
#define MAX_CROP_W               (256)
#define MAX_AIE_FMAP_SZ          (1024)
#define MAX_AIE2_ATT_LEN         (8)    // Male/Female/Asian/White/Black/isIndian/NotIndian/AgeGroup

typedef enum DRVFDObject_s {
    DRV_FD_OBJ_NONE = 0,
    DRV_FD_OBJ_SW,
    DRV_FD_OBJ_HW,
    DRV_FD_OBJ_FDFT_SW,
    DRV_FD_OBJ_UNKNOWN = 0xFF,
} DrvFDObject_e;

typedef enum
{
    FDVT_IDLE_MODE =0,
    FDVT_GFD_MODE = 0x01,
    FDVT_LFD_MODE = 0x02,
    FDVT_OT_MODE = 0x04,
    FDVT_SD_MODE = 0x08,
} FDVT_OPERATION_MODE_ENUM;

typedef enum
{
    FACEDETECT_TRACKING_REALPOS   = 0,
    FACEDETECT_TRACKING_DISPLAY,
    FACEDETECT_TRACKING_UI
}FACEDETECT_TRACKING_RESULT_TYPE_ENUM;

typedef enum
{
    AIE_ATTR_TYPE_GENDER   = 0,
    AIE_ATTR_TYPE_POSE,
    MAX_AIE_ATTR_TYPE
} AIE_ATTR_TYPE_ENUM;

typedef enum
{
    //!!! Related with AppFDFT_SW::gSensorDirToPoseTable[5] mapping, take care !!!
    FACEDETECT_GSENSOR_DIRECTION_0 = 0,
    FACEDETECT_GSENSOR_DIRECTION_90,
    FACEDETECT_GSENSOR_DIRECTION_270,
    FACEDETECT_GSENSOR_DIRECTION_180,
    FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR,
}FACEDETECT_GSENSOR_DIRECTION;

typedef enum
{
    FACEDETECT_IMG_Y_SINGLE = 0,
    FACEDETECT_IMG_YUYV_SINGLE,
    FACEDETECT_IMG_Y_SCALES,
    FACEDETECT_IMG_RGB565,
}FACEDETECT_IMG_TYPE;

typedef enum
{
    HWPIPE_FLOW_P1 = 1,
    HWPIPE_FLOW_P2,
}HWPIPE_FLOW_TYPE;

#if defined(MTEE_TA_MODE) || defined(UREE_CA_MODE)
struct FDFTICSResultPara{
            MUINT32 hA_FD_ICS_Result;
            MUINT32 hMtkCameraFace_faces;   
            MUINT32 hMtkFaceInfo_posInfo;
            MUINT32 hA_FD_Results;
            MUINT32 pWidth;
            MUINT32 pHeight;
            MUINT32 pLCM;
            MUINT32 pSensor;
            MUINT32 pCamera_TYPE;
            MUINT32 pDraw_TYPE;
};
struct FDFTJoinPara{
    MUINT8 pbuf_status[MAX_CROP_NUM];
    MINT16 pfeature_map_in_hal[MAX_CROP_NUM][MAX_AIE_FMAP_SZ];
    MINT32 pIdx_attr;
};
struct FDFTResultPara{
    MUINT32 ResultHandle;
    MUINT32 Result_Num1;
    MUINT32 Result_Num2;
    MUINT32 Width;
    MUINT32 Height;
    float WidthScale;
    float HeightScale;
    MUINT32 Type;
};
#endif
struct result
{
    unsigned int id;
    bool    af_face_indicator;// face detected flag
    int     face_index;       // priority of this face
    int     type;             // means this face is GFD, LFD, OT face
    int     x0;               // up-left x pos
    int     y0;               // up-left y pos
    int     x1;               // down-right x pos
    int     y1;               // down-right y pos
    int     fcv;              // confidence value
    int     rip_dir;          // in plane rotate direction
    int     rop_dir;          // out plane rotate direction(0/1/2/3/4/5 = ROP00/ROP+50/ROP-50/ROP+90/ROP-90)
    int     size_index;       // face size index
    int     face_num;         // total face number
    int     motion[2];        // face motion against prev frame

    //<<<<<<<<<<<<<<<<<<<< for dump fd result
    int     x0real;
    int     y0real;
    int     x1real;
    int     y1real;
    //>>>>>>>>>>>>>>>>>>>> for dump fd result

    // dlfld: only used in FD50
    int dl_leyex0;
    int dl_leyey0;
    int dl_leyex1;
    int dl_leyey1;
    int dl_reyex0;
    int dl_reyey0;
    int dl_reyex1;
    int dl_reyey1;
    int dl_nosex;
    int dl_nosey;
    int dl_mouthx0;
    int dl_mouthy0;
    int dl_mouthx1;
    int dl_mouthy1;
    int dl_bbox_flag;
    int rop_from_dlfld;

    // 20160106: add for FaceAlignment
    int leyex0;
    int leyey0;
    int leyex1;
    int leyey1;
    int reyex0;
    int reyey0;
    int reyex1;
    int reyey1;
    int nosex;
    int nosey;
    int mouthx0;
    int mouthy0;
    int mouthx1;
    int mouthy1;
    int fa_cv;
    int fld_rip;
    int fld_rop;
    int YUVsts[5];

    int leyeux;
    int leyeuy;
    int leyedx;
    int leyedy;
    int reyeux;
    int reyeuy;
    int reyedx;
    int reyedy;
    /* fld gender classifier */
    unsigned char fld_oGenderLabel;
    unsigned char fld_GenderLabel;
    int fld_oGenderScore;
    int fld_GenderInfo;

    unsigned char GenderLabel;
    unsigned char oGenderLabel;
    unsigned char GenderCV;
    unsigned char RaceLabel;
    unsigned char RaceCV[4];
    unsigned char AgeVal; //AIE2.0

    MINT16 poseinfo[10];
};

typedef struct
{
    MINT16   wLeft;
    MINT16   wTop;
    MINT16   wWidth;
    MINT16   wHeight;
} FACEDETECT_RECT;


struct MTKFDFTInitInfo
{
    union{
    MUINT8   *WorkingBufAddr;                  // working buffer
        MTEE_INT64 TEMP64_0;
    };
    MUINT32  WorkingBufSize;                   // working buffer size
    MUINT32  FDThreadNum;                      // default 1, suggest range: 1~2
    MUINT32  FDThreshold;                      // default 256, suggest range: 100~400 bigger is harder
    MUINT32  DisLimit;                         // default 4, suggest range: 1 ~ 4
    MUINT32  DecreaseStep;                     // default 384, suggest range: 0 ~ 384
    MUINT32  ModelVersion;                     // from driver
    MUINT32  FDMINSZ;                          // default 0    
    MUINT32  DelayThreshold;                   // default 20, under this goes to median reliability, above goes high
    MUINT32  DelayCount;                       // default 2, for median reliability face, should have deteced in continous frame
    MUINT32  MajorFaceDecision;                // default 1, 0: Size fist.  1: Center first.   2: Size first perframe. 3: Center first per frame
    MUINT8   OTBndOverlap;                     // default 8, suggest range: 6 ~ 9
    MUINT32  OTRatio;                          // default 960, suggest range: 640~1200
    MUINT32  OTds;                             // default 2, suggest range: 1~2
    MUINT32  OTtype;                           // default 1, 0: RGB.  1: YUV
    MUINT32  SmoothLevel;                      // default 8, suggest range: 0~16
    MUINT32  Momentum;                         // default 2819
    MUINT32  SmoothLevelUI;                    // default 8, suggest range: 0~16
    MUINT32  MomentumUI;                       // default 2818
    MUINT32  MaxTrackCount;                    // default 10, suggest range: 0~120
    MUINT8   SilentModeFDSkipNum;              // default 0, suggest range: 0-2, when there is no any potential face, decrease FD frequency
    MUINT8   HistUpdateSkipNum;                // default 1, suggest range: 0-5, for OT.  means update hist. every n detection
    MUINT32  FDSkipStep;                       // default 2, suggest range: 2~6
    MUINT32  FDRectify;                        // default 10000000 means disable and 0 means disable as well. suggest range: 5~10
    MUINT32  FDRefresh;                        // default 3, suggest range: 1~120
    MUINT8   CameraFOV;	                       // this angle decides the pyramid and size of FD
    MUINT32  FDBufWidth;                       // width of FD coordinate from driver
    MUINT32  FDBufHeight;                      // height of FD coordinate from driver
    MUINT32  FDSrcWidth;                       // source width: 640
    MUINT32  FDSrcHeight;                      // source height: 480
    MUINT32  FDTBufWidth;                      // width for tracking buffer
    MUINT32  FDTBufHeight;                     // height for tracking buffer
    MUINT32  FDMinFaceLevel;                   // max face detected level:suggest range 0~13
    MUINT32  FDMaxFaceLevel;                   // min face detected level:suggest range 0~13
    MBOOL    isSecureFD;                       // is secure world(mtee) or not
    union{
    MUINT32  *FDImageWidthArray;
        MTEE_INT64 TEMP64_1;
    };
    union{
    MUINT32  *FDImageHeightArray;
        MTEE_INT64 TEMP64_2;
    };
    MUINT32  FDImageArrayNum;
    FACEDETECT_IMG_TYPE  FDImgFmtCH1;
    FACEDETECT_IMG_TYPE  FDImgFmtCH2;
    FACEDETECT_IMG_TYPE  SDImgFmtCH1;
    FACEDETECT_IMG_TYPE  SDImgFmtCH2;
    MUINT32  SDThreshold;                      // not used
    MUINT32  SDMainFaceMust;                   // not used
    MUINT32  SDMaxSmileNum;                    // not used
    MUINT32  GSensor;                          // default 1, means g-sensor is on the phone
    MUINT32  GenScaleImageBySw;
    union{
    MUINT32*  PThreadAttr;
        MTEE_INT64 TEMP64_3;
    };
    MUINT8    FDModel;                         // model index selected by driver (only used in FD35/FD40)
    MUINT8    OTFlow;                          // HWPIPE_FLOW_P1 = 1, HWPIPE_FLOW_P2 = 2
    MUINT8   FDCurrent_mode;                   //0:FD, 1:SD, 2:vFB  3:CFB
    MUINT8   FDVersion;
    MUINT8   FLDAttribConfig;                  // 0: Turn off attrib, other: Turn on attrib
    MBOOL    FDManualMode;  //0: HW GFD use hard-coded scale table, 1:use user defined scale table(FDImageWidthArray/FDImageHeightArray)
    bool     ParallelRGB565Conversion;
    MINT32   LandmarkEnableCnt;
    MUINT8   GenderEnableCnt;
    MUINT8   PoseEnableCnt;
#ifdef _SIM_PC_
    int     CoreIdx;
    void    (*LockOtBufferFunc)(int);
    void    (*UnlockOtBufferFunc)(int);
#else
    union{
    void    (*LockOtBufferFunc)(void*);
        MTEE_INT64 TEMP64_4;
    };
    union{
    void    (*UnlockOtBufferFunc)(void*);
        MTEE_INT64 TEMP64_5;
    };
    union{
    void    *lockAgent;
        MTEE_INT64 TEMP64_6;
    };
#endif

    MTKFDFTInitInfo()
    {
        SilentModeFDSkipNum = 0;   //low power mode: default off
        HistUpdateSkipNum = 1;     // update OT histogram every 2 detection
        CameraFOV = 80;			   // default value
    }
};


/*******************************************************************************
*
********************************************************************************/
struct FdOptions
{
    FDVT_OPERATION_MODE_ENUM fd_state;
    FACEDETECT_GSENSOR_DIRECTION direction;
    int fd_scale_count;          //by frame set how many scales should GFD do detection
    int fd_scale_start_position; //by frame set which scale should GFD start detection
    int gfd_fast_mode;
    MBOOL ae_stable;
    MBOOL af_stable;
    MUINT8 LV;
    int curr_gtype;
    int inputPlaneCount;
    bool doPhase2 ; // for HAL to decide whether calling HW & phase2 or not.
    bool doGender;
    bool doPose;
    bool doROIFD;	// FDversion >= 52, HW support detect face in ROI
    int ROIFDinfo[8];	// FDversion >= 52,
    int pyramid_num;	// adaptive pyramid number for UW camera: 2020/03/30, Savan
    int pyramid_size[3];// adaptive pyramid resolution for UW camera: 2020/03/30, Savan
    union{
    MUINT8 *ImageScaleBuffer;
        MTEE_INT64 TEMP64_5;
    };
    union{
    MUINT8 *ImageBufferRGB565;
        MTEE_INT64 TEMP64_6;
    };
    union{
    MUINT8 *ImageBufferSrcVirtual;
        MTEE_INT64 TEMP64_7;
    };
    union{
    MUINT8 *ImageBufferPhyPlane1;
        MTEE_INT64 TEMP64_8;
    };
    union{
    MUINT8 *ImageBufferPhyPlane2;
        MTEE_INT64 TEMP64_9;
    };
    union{
    MUINT8 *ImageBufferPhyPlane3;
        MTEE_INT64 TEMP64_10;
    };
    FDVT_OPERATION_MODE_ENUM ForceFDMode;
    MUINT16 YUVstsHratio;
    MUINT16 YUVstsWratio;
    MUINT8 startW;
    MUINT8 startH;
    MUINT8 model_version;
    MUINT8 DynamicLandmarkCnt;
    int TouchID;	//-1: no touch, >=0: touched face id

    FdOptions()
    {
        TouchID = -1;					// default value
    }
};

// svm related
typedef enum
{
    GFD_RST_TYPE=0,
    LFD_RST_TYPE,
    COLOR_COMP_RST_TYPE,
    OT_RST_TYPE
} face_result_enum;

typedef struct
{
    kal_int16     pix_data_x[UTL_HAAR_PIX_MAX];
    kal_int16     pix_data_y[UTL_HAAR_PIX_MAX];
}  UTL_PIX_POSITION_STRUCT, *P_UTL_PIX_POSITION_STRUCT;

typedef struct
{
    kal_int8     bin_value_table[UTL_NUMBER_OF_BINS];
    kal_int8     threshold;
    kal_int8     threshold2 ;
    kal_uint8     feature_range;
    kal_int8     feature_value_8bit_min;
}  UTL_CASCADED_CLASSIFIERS_STRUCT, *P_UTL_CASCADED_CLASSIFIERS_STRUCT;

typedef struct
{
    UTL_CASCADED_CLASSIFIERS_STRUCT cascaded_classifiers[FD_CASCADE_LAYER_MAX_NUM+FD_CASCADE_LAYER_MAX_NUM];    // 0 degree & 30 degree
    kal_int8     pattern_index[(FD_POSE_NUM/3)*FD_CASCADE_LAYER_MAX_NUM+((FD_POSE_NUM*2)/3)*FD_CASCADE_LAYER_MAX_NUM];
    UTL_PIX_POSITION_STRUCT scaled_posed_pix_position[FACE_SIZE_NUM_MAX*(FD_CASCADE_LAYER_MAX_NUM+FD_CASCADE_LAYER_MAX_NUM)];

} fd_data_struct;// learning_data_24x24_struct_fd

struct fd_ensemble_svm_model_int
{
    const kal_int32 *beta;
    const kal_int32 *omega;
    const kal_uint8 *fpx;
    const kal_uint8 *fpy;
    const kal_int8 *parities;
    const kal_int32 *ths;
    const kal_uint32 *alphas;
    kal_uint8 n;
    kal_uint16 d;
    kal_uint8 hog_cell_size;
};
//


typedef struct
{
    union{
    kal_uint32 *integral_img;                // Pointer to integral Image buffer
        MTEE_INT64 TEMP64_0;
    };
    union{
    kal_uint16 *prz_buffer_ptr;              // Pointer to a cacheable buffer copied from prz output buffer
        MTEE_INT64 TEMP64_1;
    };
    kal_uint32 inputPlaneCount;
    union{
    kal_uint8 *srcbuffer_phyical_addr_plane1;
        MTEE_INT64 TEMP64_2;
    };
    union{
    kal_uint8 *srcbuffer_phyical_addr_plane2;
        MTEE_INT64 TEMP64_3;
    };
    union{
    kal_uint8 *srcbuffer_phyical_addr_plane3;
        MTEE_INT64 TEMP64_4;
    };
    union{
    const kal_uint32 *detect_face_size_lut;  // Pointer to face size table
        MTEE_INT64 TEMP64_5;
    };
    union{
    const fd_data_struct *learned_cascaded_classifiers;    // Pointer to 24x24 learning data
        MTEE_INT64 TEMP64_6;
    };
    union{
    const fd_ensemble_svm_model_int *fd_svm_model_00_; // Pointer to svm 00 data
        MTEE_INT64 TEMP64_7;
    };
    union{
    const fd_ensemble_svm_model_int *fd_svm_model_30_; // Pointer to svm 30 data
        MTEE_INT64 TEMP64_8;
    };

    kal_uint8 feature_select_sequence_index;    // Current feature select seq. index for g_direction_feature_sequence table
    kal_uint8 current_fd_detect_column;         // Current frame detect division index
    kal_uint8 current_direction;                // Current phone direction (1: H(0), 2: CR(-90), 3: CCR(90), 4: INV(-180))
    kal_uint8 current_feature_index;            // Current feature index for learning data
    kal_uint8 current_scale;

    kal_uint8 new_face_number;                // Face number detected by GFD
    kal_uint8 lfd_face_number;                // Face number tracked by LFD

    // FD50     -- FD_MAX_FACE_NUM (merged: 15)
    // other    -- MAX_FACE_SEL_NUM (without merge: 1024)
    kal_uint8  fd_priority[MAX_FACE_SEL_NUM];    // face priority array, 0:highest
    kal_bool  display_flag[MAX_FACE_SEL_NUM];    // Record if need to display for each face bin
    kal_uint32 face_reliabiliy_value[MAX_FACE_SEL_NUM];      // Record the reliability value for each face bin
    face_result_enum result_type[MAX_FACE_SEL_NUM];        // Record the detected result type for each face bin (GFD_RST_TYPE, LFD_RST_TYPE, COLOR_COMP_RST_TYPE)

    kal_uint8 detected_face_size_label[MAX_FACE_SEL_NUM];    // Record face size label for each face bin
    kal_uint8 face_feature_set_index[MAX_FACE_SEL_NUM];      // Record used feature set index for each face bin

    kal_uint8 rip_dir[MAX_FACE_SEL_NUM];            // keep rip_dir
    kal_uint8 rop_dir[MAX_FACE_SEL_NUM];            // keep rop_dir

    kal_int32 face_candi_pos_x0[MAX_FACE_SEL_NUM];         // Position of the faces candidates
    kal_int32 face_candi_pos_y0[MAX_FACE_SEL_NUM];
    kal_int32 face_candi_pos_x1[MAX_FACE_SEL_NUM];
    kal_int32 face_candi_pos_y1[MAX_FACE_SEL_NUM];
    kal_int32 face_candi_cv[MAX_FACE_SEL_NUM];
    kal_int32 face_candi_model[MAX_FACE_SEL_NUM];

    kal_uint16 img_width_array[FD_SCALE_NUM];
    kal_uint16 img_height_array[FD_SCALE_NUM];
    union{
    kal_uint8 *img_array[FD_SCALE_NUM];
        MTEE_INT64 TEMP64_9[FD_SCALE_NUM];
    };
    union{
    kal_uint32 *integral_img_array[FD_SCALE_NUM];
        MTEE_INT64 TEMP64_10[FD_SCALE_NUM];
    };

    kal_uint8   g_scale_frame_division[FD_SCALE_NUM];
    kal_uint8   g_scale_detect_column[FD_SCALE_NUM];

    MBOOL fd_manual_mode;

    int fd_scale_count;
    int fd_scale_start_position;
    kal_uint16 fd_img_src_width;
    kal_uint16 fd_img_src_height;


///////////////////////////////////////////////////////////////
    union{
    kal_uint8 *srcbuffer_phyical_addr;
        MTEE_INT64 TEMP64_11;
    };
    kal_bool    rotation_search;                // Phone rotation request flag
    kal_uint8 skip_pattern;                 // Skip pixel counter used in GFD
    kal_uint8 top_skip_pattern_idx;         // Skip pixel counter used in GFD in top direction
    kal_uint8 left_skip_pattern_idx;            // Skip pixel counter used in GFD in top direction
    kal_uint8 right_skip_pattern_idx;           // Skip pixel counter used in GFD in top direction
    kal_uint8 bottom_skip_pattern_idx;      // Skip pixel counter used in GFD in top direction
    kal_bool  is_first_frame;                   // First frame check
    kal_uint8 dir_cycle_count;              // Detection cycle count
    kal_uint8 color_compensate_face_number; // Face number tracked by color compensate
    //  kal_bool  af_face_indicator[MAX_FACE_SEL_NUM];  // Specify which face has highest priority for focus
    kal_bool  box_display_position_update_flag[MAX_FACE_SEL_NUM];   // Record if need to update face rectnage
    kal_bool execute_skin_color_track[MAX_FACE_SEL_NUM];    // Record if need to execute color compensate
    kal_uint8 non_lfd_tracking_count[MAX_FACE_SEL_NUM];     // Record the non-lfd (color comp.) tracking count
    kal_uint8 avg_r_value[MAX_FACE_SEL_NUM];                /* Record color statistics      */
    kal_uint8 avg_g_value[MAX_FACE_SEL_NUM];                /* for color compensate */
    kal_uint8 avg_b_value[MAX_FACE_SEL_NUM];                /* for each face bin        */
    float avg_div_rg[MAX_FACE_SEL_NUM];
    kal_uint32 continuous_lfd_tracking_count[MAX_FACE_SEL_NUM]; // Record continuous lfd tracking count
    kal_int32 face_display_pos_x0[MAX_FACE_SEL_NUM];        // Position of the faces to disply
    kal_int32 face_display_pos_y0[MAX_FACE_SEL_NUM];
    kal_int32 face_display_pos_x1[MAX_FACE_SEL_NUM];
    kal_int32 face_display_pos_y1[MAX_FACE_SEL_NUM];

    // [fld_***_**]
    //  - FD50: pass dlfld result from VPU
    //  - FD51: not used
    //  - FD52: pass pose cv from AIE
    kal_int32 fld_leye_x0[MAX_FACE_SEL_NUM]; // FD52: rip[0] from AIE2.0;
    kal_int32 fld_leye_y0[MAX_FACE_SEL_NUM]; // FD52: rip[1] from AIE2.0;
    kal_int32 fld_leye_x1[MAX_FACE_SEL_NUM]; // FD52: rip[2] from AIE2.0;
    kal_int32 fld_leye_y1[MAX_FACE_SEL_NUM]; // FD52: rip[3] from AIE2.0;
    kal_int32 fld_reye_x0[MAX_FACE_SEL_NUM]; // FD52: rip[4] from AIE2.0;
    kal_int32 fld_reye_y0[MAX_FACE_SEL_NUM]; // FD52: rip[5] from AIE2.0;
    kal_int32 fld_reye_x1[MAX_FACE_SEL_NUM]; // FD52: rip[6] from AIE2.0;
    kal_int32 fld_reye_y1[MAX_FACE_SEL_NUM]; // FD52: rop[0] from AIE2.0;
    kal_int32 fld_nose_x[MAX_FACE_SEL_NUM];  // FD52: rop[1] from AIE2.0;
    kal_int32 fld_nose_y[MAX_FACE_SEL_NUM];  // FD52: rop[2] from AIE2.0;
    kal_int32 fld_mouth_x0[MAX_FACE_SEL_NUM];
    kal_int32 fld_mouth_y0[MAX_FACE_SEL_NUM];
    kal_int32 fld_mouth_x1[MAX_FACE_SEL_NUM];
    kal_int32 fld_mouth_y1[MAX_FACE_SEL_NUM];

    kal_uint8 face_lum[MAX_FACE_SEL_NUM];   // Face luminance for Face AE

    //HAL Add
    kal_uint8   direction_offset;      /* FACE_DIR_OFST_0, FACE_DIR_OFST_270, FACE_DIR_OFST_180, FACE_DIR_OFST_90*/
    kal_uint8   fd_level;
    kal_uint16  svm_candidate_num;

} fd_cal_struct;

typedef struct
{
    MUINT16 new_face_number;  // Face number detected by GFD
    MUINT8 face_feature_set_index[MAX_FACE_SEL_NUM];    // Record the reliability value for each face bin
    MUINT8 rip_dir[MAX_FACE_SEL_NUM]; // keep rip_dir
    MUINT8 rop_dir[MAX_FACE_SEL_NUM]; // keep rop_dir
    MINT32 face_candi_pos_x0[MAX_FACE_SEL_NUM]; // Position of the faces candidates
    MINT32 face_candi_pos_y0[MAX_FACE_SEL_NUM];
    MINT32 face_candi_pos_x1[MAX_FACE_SEL_NUM];
    MINT32 face_candi_pos_y1[MAX_FACE_SEL_NUM];
    MUINT32 face_reliabiliy_value[MAX_FACE_SEL_NUM];
} fd_drv_output;


/*******************************************************************************
*
********************************************************************************/
class MTKDetection {
public:
    static MTKDetection* createInstance(DrvFDObject_e eobject);
    virtual void      destroyInstance() = 0;

    virtual ~MTKDetection() {}
    virtual void FDVTInit(MUINT32 *fd_tuning_data);
    virtual void FDVTInit(MTKFDFTInitInfo *init_data);
    virtual void FDVTMain(FdOptions *options);
    virtual fd_cal_struct* FDGetCalData(void);
    virtual void FDVTMainPhase2();
    virtual void FDVTMainFastPhase(int* gtype_with_gamma);
    virtual void FDVTMainCropPhase(unsigned char todo_list[][MAX_CROP_NUM], unsigned char buf_status[][MAX_CROP_NUM], unsigned char** workbuf, int* patchszarr, int* pNGender);
    virtual void FDVTMainCropPhaseV2(unsigned char todo_list[][MAX_CROP_NUM], unsigned char buf_status[][MAX_CROP_NUM], unsigned short** workbuf, int* patchszarr, int* pNGender);
    virtual void FDVTMainPostPhase();
    virtual void FDVTMainJoinPhase(unsigned char* gender_buf_status, MINT16 feature_map_in_hal[][MAX_AIE_FMAP_SZ], int idx_attr);
    virtual void FDVTMainJoinPhaseV2(unsigned char* att_buf_status, MINT16 att_result_from_hal[][MAX_AIE2_ATT_LEN], int idx_attr);
    virtual void FDVTReset(void);
    virtual MUINT32 FDVTGetResultSize(void);
    virtual MUINT8 FDVTGetResult(MUINT8 *FD_result_Adr, FACEDETECT_TRACKING_RESULT_TYPE_ENUM result_type);
    virtual void FDVTGetICSResult(MUINT8 *a_FD_ICS_Result, MUINT8 *a_FD_Results, MUINT32 Width,MUINT32 Height, MUINT32 LCM, MUINT32 Sensor, MUINT32 Camera_TYPE, MUINT32 Draw_TYPE);
    virtual void FDVTGetFDInfo(MUINT32  FD_Info_Result);
    //virtual void FDVTDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    #ifdef SmileDetect
    virtual void FDVTSDDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    virtual MUINT8 FDVTGetSDResult(MUINT32 FD_result_Adr);
    virtual void FDVTGetMode(FDVT_OPERATION_MODE_ENUM *mode);
    #endif
    #ifdef SIM_MAIN
    virtual void getFdSimHwDrvValue(void* mObj);
    #endif
    #if defined(UREE_CA_MODE)
    virtual MUINT32 FDVTSetInputMTEE(MUINT32 handle, MUINT32 size,MUINT8*);
    virtual void FDVTParseResult(MUINT32 ResultHandle, MUINT32 Result_Num1, MUINT32 Result_Num2, MUINT32 Width, MUINT32 Height, float WidthScale, float HeightScale, MUINT32 Type);
    #endif
private:

};

class AppFDTmp : public MTKDetection {
public:
    //
    static MTKDetection* getInstance();
    virtual void destroyInstance();
    //
    AppFDTmp() {};
    virtual ~AppFDTmp() {};
};

#endif

