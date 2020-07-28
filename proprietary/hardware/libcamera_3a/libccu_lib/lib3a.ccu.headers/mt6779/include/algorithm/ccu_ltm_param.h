#ifndef _CCU_LTM_PARAM_H__
#define _CCU_LTM_PARAM_H__
#ifndef WIN32
#include "ccu_ae_param.h"   // For AE info
#endif

#define LTM_ALGO_CHIP_VERSION 6779
#define LTM_ALGO_MAIN_VERSION 1
#define LTM_ALGO_SUB_VERSION 1
#define LTM_ALGO_SYSTEM_VERSION 1

#define LTM_WIDTH 12
#define LTM_HEIGHT 9
#define LTM_BIN 33

// LTM setting define
#define LTM_BLK_NUM_MAX 16
#define LTM_MAX_BIN_NUM_BIT 5 // 33 bins
#define LTM_MAX_BIN_NUM ( ( 1 << ( LTM_MAX_BIN_NUM_BIT ) ) + 1 )
#define LTM_CURVE_POINT_NUM_BIT LTM_MAX_BIN_NUM_BIT
#define LTM_CURVE_POINT_NUM ( ( 1 << ( LTM_CURVE_POINT_NUM_BIT ) ) + 1 )

#define LTM_BIN_SHIFT_BIT 6
#define LTM_FIR_TAPNUM 5

#define LTM_LUMA_BIT 14 //u8.6
#define LTM_LUMA_MAX (255<<(LTM_LUMA_BIT-8))

#define LTM_MAX_BIN_RANGE_BIT ( ( LTM_LUMA_BIT ) - ( LTM_MAX_BIN_NUM_BIT ) )
#define LTM_MAX_BIN_RANGE ( 1 << ( LTM_MAX_BIN_RANGE_BIT ) )

#define LTM_MAX_V 16383
#define LTM_12_BIT_DEPTH 4095
#define LTM_16_BIT_DEPTH 65535

#define MAX_BLK_X_NUM 12
#define MAX_BLK_Y_NUM 9

// Histogram
#define LTMS_MAX_BIN_NUM_BIT 5 // 33 bins
#define LTM_CURVE_POINT_NUM ( ( 1 << ( LTMS_MAX_BIN_NUM_BIT ) ) + 1 )

//AE Histogram Y
#define AE_HISTOGRAM_BIN_SW 256
#define AE_HISTOGRAM_BIN_HW 256

// Face LTM define
#define CCU_LTM_MAX_FACE_AREAS 9
#define FDAREA_ICS_OFFSET 1000
#define LV_MAX 13
// utility
#define NUM_HIGHBOUND_THD 5

// FD Area define
struct ccu_ltm_ae_meter_area_t {
	int32_t i4_left;
	int32_t i4_top;
	int32_t i4_right;
	int32_t i4_bottom;
	int32_t i4_weight;
	int32_t i4_id;
	int32_t i4_type; // 0:GFD, 1:LFD, 2:OT
	int32_t i4_motion[2];
	int32_t i4_landmark_cv;
	int32_t i4_rop;
	int32_t i4_landmark_rip;
	int32_t i4_landmark_rop;
};

// Face State enum
typedef enum {
	LTM_FACE_STATE_FDIN,
	LTM_FACE_STATE_FDOUT,
	LTM_FACE_STATE_FDCONTINUE,
	LTM_FACE_STATE_FDDROP,
	LTM_FACE_STATE_END
} LTM_FACE_STATE_T;

typedef enum {
	LTM_FACE_SMOOTH_FROM_NORMAL_TO_FACE_WAITING_ROBUST,
	LTM_FACE_SMOOTH_NON_ROBUST_FACE_BEFORE_TIMEOUT,
	LTM_FACE_SMOOTH_NON_ROBUST_FACE_AFTER_TIMEOUT,
	LTM_FACE_SMOOTH_ROBUST_FACE_CONFIRMED,
	LTM_FACE_SMOOTH_LOST_FACE_BEFORE_TIMEOUT,
	LTM_FACE_SMOOTH_LOST_FACE_AFTER_TIMEOUT,
	LTM_FACE_SMOOTH_ERROR
} LTM_FACE_SMOOTH_TYPE_T;


// AE infor
struct ccu_ltm_ae_info_t {
	int32_t  i4RealLightValue_x10;
	int32_t  i4deltaIndex;
	int32_t u4AECurrentTarget;
	uint32_t u4MaxIndexStableCount;
	bool   bAETouchEnable;
	uint32_t u4FaceAEStable;
	uint32_t u4FaceNum;
	struct ccu_ltm_ae_meter_area_t FDArea[CCU_LTM_MAX_FACE_AREAS];
	int32_t  i4FaceBrightToneProtectValue_12bit[CCU_LTM_MAX_FACE_AREAS];
	bool   bEnableFaceAE;
	uint8_t  m_u4FaceState;
	uint32_t m_u4FaceRobustCnt;
	uint32_t m_u4FaceRobustTrustCnt;
	uint32_t m_u4FD_Lock_MaxCnt;
	uint32_t m_u4FDDropTempSmooth;
	uint32_t m_u4OTFaceCnt;
	bool   bOTFaceTimeOutLockAE;
};
#endif