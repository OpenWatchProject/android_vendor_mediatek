/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _FDVT_DRIVER_H_
#define _FDVT_DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "camera_fdvt.h"
#include "fdvtreg.h"

#include "fd_config.h"
#include "rs_config.h"

#define FRAME_NUM_WITHOUT_FACE_TO_DO_ROTATION_SEARCH 60
#define FRAME_DETECT_DIVISION 1
#define GFD_BOUNDARY_OFF_RATIO 0

#define SUCCEEDED(Status)   ((MRESULT)(Status) >= 0)
#define FAILED(Status)      ((MRESULT)(Status) < 0)

#define MODULE_MTK_Detection (0) // Temp value

#define CAMERA_FD_MAX_NO 				(512)
#define CAMERA_FD_IMAGE_BYTES 			(2)
#define MAX_TRACKING_FACE_NUM 			(15)
#define MAX_FACE_SEL_NUM				(CAMERA_FD_MAX_NO+2)

#define FACE_SIZE_NUM_MAX               14   // The max number of face sizes could be detected, for feature scaling
#define FD_SCALE_NUM                    15   // FACE_SIZE_NUM_MAX + 1, first scale for input image W/H
#define LEARNDATA_NUM 					8
#define FDVT_PARA_NUM					256
#define FDVT_BUFF_NUM					1024

#define FD_RESULT_MAX_SIZE (512 * 16 + 16)	// 1024 faces, 16 bytes/face
/*#define RS_BUFFER_MAX_SIZE (640 * 480 * 15 * 2)  //Resizer Buffer*/
#define RS_BUFFER_MAX_SIZE (1144394 * 2)
/*
MUINT16 source_img_width[FD_SCALE_NUM] = {640, 640, 510, 400, 320, 260, 210, 170, 140, 100, 80, 70, 50, 44, 36};
MUINT16 source_img_height[FD_SCALE_NUM] = {480, 480, 380, 300, 240, 200, 150, 125, 100, 80, 60, 50, 40, 32, 26};
307200+307200+193800+120000+76800+52000+31500+21250+14000+8000+4800+3500+2000+1408+936 = 1144394
*/
//#define REG_RMAP 0x01230101    //FD3.1
#define REG_RMAP 0x05230401    //FD3.5+
//#define REG_RMAP_LFD 0x01230100    //FD3.1 LFD
#define REG_RMAP_LFD 0x05230400    //FD3.5+ LFD

//#define MTKDetection_OKCODE(errid)         OKCODE(MODULE_MTK_Detection, errid)
//#define MTKDetection_ERRCODE(errid)        ERRCODE(MODULE_MTK_Detection, errid)

// Detection error code
#define S_Detection_OK                  0x0000
#define E_Detection_NEED_OVER_WRITE     0x0001
#define E_Detection_NULL_OBJECT         0x0002
#define E_Detection_WRONG_STATE         0x0003
#define E_Detection_WRONG_CMD_ID        0x0004
#define E_Detection_WRONG_CMD_PARAM     0x0005
#define E_Detection_Driver_Fail         0x0010

typedef uint64_t            MUINT64;
typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef signed int          MINT32;
typedef signed short        MINT16;
typedef signed char         MINT8;

typedef char                kal_char;
typedef unsigned short      kal_wchar;

typedef intptr_t            MINTPTR;
typedef uintptr_t           MUINTPTR;

typedef enum
{
  MFALSE,
  MTRUE
} mtk_bool;

typedef enum
{
    FDVT_MODE_IDLE =0,
    FDVT_MODE_GFD = 0x01,
    FDVT_MODE_LFD = 0x02,
    FDVT_MODE_OT = 0x04,
    FDVT_MODE_SD = 0x08,
} FDVT_OPERATION_MODE;
/*
typedef enum
{
    GFD_RST_TYPE=0,
    LFD_RST_TYPE,
    COLOR_COMP_RST_TYPE,
    OT_RST_TYPE
} face_result_enum;
*/

/* In FD HW, uses Little Endian of 32 bits, but in SW, uses byte address, so the order of byte should be opposite in HW: SW[YUYV] = HW [VYUY]  */
typedef enum
{
	FMT_YUYV = 4, // SW YUYV = HW VYUY
	FMT_YVYU = 5, // SW YVYU = HW UYVY
	FMT_UYVY = 3, // SW UYVU = HW YVYU
	FMT_VYUY = 2, // SW VYUY = HW YUYV
	FMT_YUV_2P = 1, // SW Y UV = HW Y VU
	FMT_YVU_2P = 1, // SW Y VU = HW Y UV
	FMT_RGB = 0,
} INPUT_FORMAT;

struct FdDrv_input_struct
{
    MUINT8  fd_mode;
    MUINT64 *source_img_address;
    MUINT64 *source_img_address_UV;
    MUINT16 source_img_width[FD_SCALE_NUM];
    MUINT16 source_img_height[FD_SCALE_NUM];
    MUINT8  RIP_feature;
    MUINT8  GFD_skip;
    MUINT8  feature_threshold;
    MUINT8  source_img_fmt;
    bool    scale_from_original;
    bool    scale_manual_mode = 0;
    MUINT8  scale_num_from_user = 0; // Only work when scale_manual_mode = 1
};

typedef struct
{
    bool       af_face_indicator;// face detected flag
    MINT32     face_index;       // priority of this face
    MINT32     type;             // means this face is GFD, LFD, OT face
    MUINT32     x0;               // up-left x pos
    MUINT32     y0;               // up-left y pos
    MUINT32     x1;               // down-right x pos
    MUINT32     y1;               // down-right y pos
    MINT32     fcv;              // confidence value
    MINT32     rip_dir;          // in plane rotate direction
    MINT32     rop_dir;          // out plane rotate direction(0/1/2/3/4/5 = ROP00/ROP+50/ROP-50/ROP+90/ROP-90)
    MINT32     size_index;       // face size index
    MINT32     face_num;         // total face number
} FD_Result_Struct;

typedef struct
{
    // Search range
    MINT32     x0          ;   //   9 bit
    MINT32     y0          ;   //   8 bit
    MINT32     x1          ;   //   9 bit
    MINT32     y1          ;   //   8 bit

    // Direction information
    MUINT64	   pose        ;   //  60 bit (0-11: ROP00, 12-23: ROP+50, 24-35: ROP-50, 36-47: ROP+90, 48-59: ROP-90)

} GFD_Info_Struct;

typedef struct
{
	MUINT32 *integral_img;				// Pointer to integral Image buffer
	MUINT16 *prz_buffer_ptr;				// Pointer to a cacheable buffer copied from prz output buffer
	MUINT8 *srcbuffer_phyical_addr;

	const MUINT32 *detect_face_size_lut;	// Pointer to face size table
	//const fd_data_struct *learned_cascaded_classifiers;	// Pointer to 24x24 learning data
    //const fd_ensemble_svm_model_int *fd_svm_model_00_; // Pointer to svm 00 data
    //const fd_ensemble_svm_model_int *fd_svm_model_30_; // Pointer to svm 30 data


	MUINT8 feature_select_sequence_index;	// Current feature select seq. index for g_direction_feature_sequence table
	MUINT8 current_fd_detect_column;		// Current frame detect division index
	MUINT8 current_direction;				// Current phone direction (1: H(0), 2: CR(-90), 3: CCR(90), 4: INV(-180))
	MUINT8 current_feature_index;			// Current feature index for learning data
    MUINT8 current_scale;

	MUINT16 new_face_number;				// Face number detected by GFD
	MUINT16 lfd_face_number;				// Face number tracked by LFD

	MUINT8  fd_priority[MAX_FACE_SEL_NUM];	// face priority array, 0:highest
	mtk_bool display_flag[MAX_FACE_SEL_NUM];		// Record if need to display for each face bin
	MUINT32 face_reliabiliy_value[MAX_FACE_SEL_NUM];		// Record the reliability value for each face bin
	//face_result_enum result_type[MAX_FACE_SEL_NUM];		// Record the detected result type for each face bin (GFD_RST_TYPE, LFD_RST_TYPE, COLOR_COMP_RST_TYPE)

	MUINT8 detected_face_size_label[MAX_FACE_SEL_NUM];		// Record face size label for each face bin
	MUINT8 face_feature_set_index[MAX_FACE_SEL_NUM];			// Record used feature set index for each face bin

    // FD 4.0
    MUINT8 rip_dir[MAX_FACE_SEL_NUM];			// keep rip_dir
    MUINT8 rop_dir[MAX_FACE_SEL_NUM];			// keep rop_dir

	MINT32 face_candi_pos_x0[MAX_FACE_SEL_NUM]; 		// Position of the faces candidates
	MINT32 face_candi_pos_y0[MAX_FACE_SEL_NUM];
	MINT32 face_candi_pos_x1[MAX_FACE_SEL_NUM];
	MINT32 face_candi_pos_y1[MAX_FACE_SEL_NUM];
	MINT32 face_candi_cv[MAX_FACE_SEL_NUM];

    MUINT16 img_width_array[FD_SCALE_NUM];
    MUINT16 img_height_array[FD_SCALE_NUM];
    MUINT8 *img_array[FD_SCALE_NUM];
    MUINT32 *integral_img_array[FD_SCALE_NUM];

	MUINT8   g_scale_frame_division[FD_SCALE_NUM];
	MUINT8   g_scale_detect_column[FD_SCALE_NUM];

} FdDrv_output_struct;


typedef struct
{
	MUINT32 GFD_Total_Num = 0;
	MUINT16 RS_Num = 14;
	MUINT16 SW_Input_Width[FD_SCALE_NUM] = {0};
	MUINT16 SW_Input_Height[FD_SCALE_NUM] = {0};
	MUINT16 HW_Used_Width = 640;
	MUINT16 HW_Used_Height = 480;
	MUINT32 Learning_Type = 0;
	MINT32  HW_RIP[15] = {0};
	MUINT8  GFD_Skip_Num = 0;
	GFD_Info_Struct GFD_Info;
	FD_Result_Struct FD_Result[FDVT_BUFF_NUM];

	MUINT32 *Learning_Data0_PA=NULL;
	MUINT32 *Learning_Data1_PA=NULL;
	MUINT32 *Learning_Data2_PA=NULL;
	MUINT32 *Learning_Data3_PA=NULL;
	MUINT32 *Learning_Data4_PA=NULL;
	MUINT32 *Learning_Data5_PA=NULL;
	MUINT32 *Learning_Data6_PA=NULL;
	MUINT32 *Learning_Data7_PA=NULL;
	MUINT32 *Learning_Data8_PA=NULL;
	MUINT32 *Learning_Data9_PA=NULL;
	MUINT32 *Learning_Data10_PA=NULL;
	MUINT32 *Learning_Data11_PA=NULL;
	MUINT32 *Learning_Data12_PA=NULL;
	MUINT32 *Learning_Data13_PA=NULL;
	MUINT32 *Learning_Data14_PA=NULL;
	MUINT32 *Learning_Data15_PA=NULL;
	MUINT32 *Learning_Data16_PA=NULL;
	MUINT32 *Learning_Data17_PA=NULL;

	MUINT64 *Learning_Data0_VA=NULL;
	MUINT64 *Learning_Data1_VA=NULL;
	MUINT64 *Learning_Data2_VA=NULL;
	MUINT64 *Learning_Data3_VA=NULL;
	MUINT64 *Learning_Data4_VA=NULL;
	MUINT64 *Learning_Data5_VA=NULL;
	MUINT64 *Learning_Data6_VA=NULL;
	MUINT64 *Learning_Data7_VA=NULL;
	MUINT64 *Learning_Data8_VA=NULL;
	MUINT64 *Learning_Data9_VA=NULL;
	MUINT64 *Learning_Data10_VA=NULL;
	MUINT64 *Learning_Data11_VA=NULL;
	MUINT64 *Learning_Data12_VA=NULL;
	MUINT64 *Learning_Data13_VA=NULL;
	MUINT64 *Learning_Data14_VA=NULL;
	MUINT64 *Learning_Data15_VA=NULL;
	MUINT64 *Learning_Data16_VA=NULL;
	MUINT64 *Learning_Data17_VA=NULL;

	MUINT64 *FD_Config_VA = NULL;
	MUINT64 *FD_Result_VA = NULL;
	MUINT64 *RS_Config_VA = NULL;
	MUINT64 *RS_Result_VA = NULL;

	MUINT32 *FD_Config_PA = NULL;
	MUINT32 *FD_Result_PA = NULL;
	MUINT32 *RS_Config_PA = NULL;
	MUINT32 *RS_Result_PA = NULL;
} FdDrv_Para;


MINT32 FDVT_Imem_flush();
MINT32 FDVT_Imem_invalid();
MINT32 FDVT_Imem_alloc(MUINT32 size, MINT32 *memId, MUINT8 **vAddr, MUINTPTR *pAddr);
MINT32 FDVT_Imem_free(MUINT8 *vAddr, MUINTPTR phyAddr, MUINT32 size, MINT32 memId);
MINT32 FDVT_IOCTL_OpenDriver();
MINT32 FDVT_IOCTL_ParaSetting(MUINT32* Adr, MUINT32* value, MINT32 num, FDVT_OPERATION_MODE fd_state);
MINT32 FDVT_IOCTL_StartHW();
MINT32 FDVT_IOCTL_WaitIRQ();
MINT32 FDVT_IOCTL_GetHWResult(MUINT32* Adr, MUINT32* value, MINT32 num, MUINT32 &result);
MINT32 FDVT_IOCTL_CloseDriver();
MINT32 FDVT_IOCTL_DumpReg();
void FDVT_AllocMem_LearningData();
void FDVT_AllocMem_RSFDConfigData();
void FDVT_AllocMem_RSBuffer();
void FDVT_AllocMem_ResultData();
void FDVT_FreeMem_LearningData(MUINT8 learnDataNum);
void FDVT_FreeMem_ConfigData();
void FDVT_FreeMem_RsBufferData();
void FDVT_FreeMem_FdResultData();
void FDVT_InitialDRAM(MUINT32 learning_type);
void FDVT_UninitDRAM();
void FDVT_ResetGFDInfo();
void FDVT_ResetLFDInfo();
void FDVT_SetHWRIPPose(FdDrv_input_struct *FdDrv_input);
void FDVT_RIPindexFromHWtoFW(FD_Result_Struct *FD_Result);
void FDVT_RIPindexFromFWtoHW(FdDrv_output_struct *FdDrv_output);
void FDVT_RSRegisterConfig(FdDrv_input_struct *FdDrv_input);
void FDVT_FDRegisterConfig(FdDrv_input_struct *FdDrv_input);
void FDVT_FDRegisterConfig_LFD(void* src_image_data, MUINT8 GFD_skip, FdDrv_output_struct *FdDrv_output);
MINT32 FDVT_FDGetHWResult(/*FDVT_OPERATION_MODE_ENUM fd_state*/);
void FDVT_SetFDResultToSW(FdDrv_output_struct *FdDrv_output);
MINT32 FDVT_OpenDriverWithUserCount(MUINT32 learning_type);
MINT32 FDVT_CloseDriverWithUserCount();
void FDVT_Enque(FdDrv_input_struct *FdDrv_input);
void FDVT_Deque(FdDrv_output_struct *FdDrv_output);

#endif
