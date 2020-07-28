//MTK_SWIP_PROJECT_START
#ifndef __MTK_DCE_H__
#define __MTK_DCE_H__

#define DCE_ALGO_CHIP_VERSION       (6779)
#define DCE_ALGO_MAIN_VERSION       (20)
#define DCE_ALGO_SUB_VERSION        (1)
#define DCE_ALGO_SYSTEM_VERSION     (2)

#include "MTKDceType.h"
#include "MTKDceErrCode.h"


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
    MTKDCE_FEATURE_BEGIN,               // minimum of feature id
    MTKDCE_FEATURE_SET_ENV_INFO,        // feature id to setup environment information
    MTKDCE_FEATURE_SET_PROC_INFO,       // feature id to setup processing information
    MTKDCE_FEATURE_GET_RESULT,          // feature id to get result
    MTKDCE_FEATURE_GET_EXIF,            // feature id to get EXIF
    MTKDCE_FEATURE_CONFIG_SMOOTH,
    MTKDCE_FEATURE_CONFIG_FLARE,
    MTKDCE_FEATURE_CONFIG_ENHANCE_FACTOR,
    MTKDCE_FEATURE_CONFIG_MODE,
    MTKDCE_FEATURE_MAX                  // maximum of feature id
} MTKDCE_FEATURE_ENUM;

typedef enum
{
    MTKDCE_TUNING_SET_Preview = 0,
    MTKDCE_TUNING_SET_Video,
    MTKDCE_TUNING_SET_Capture,
    MTKDCE_TUNING_SET_HDR_Preview,
    MTKDCE_TUNING_SET_HDR_Video,
    MTKDCE_TUNING_SET_HDR_Capture,
    MTKDCE_TUNING_SET_AutoHDR_Preview,
    MTKDCE_TUNING_SET_AutoHDR_Video,
    MTKDCE_TUNING_SET_AutoHDR_Capture,
    MTKDCE_TUNING_SET_RSV0,
    MTKDCE_TUNING_SET_RSV1,
    MTKDCE_TUNING_SET_RSV2,
    MTKDCE_TUNING_SET_MAX         // maximum of set enum
} eMTKDCE_TUNING_SET_T;

typedef enum
{
    eDCESensorDev_None         = 0x00,
    eDCESensorDev_Main         = 0x01,
    eDCESensorDev_Sub          = 0x02,
    eDCESensorDev_MainSecond   = 0x04,
    eDCESensorDev_Main3D       = 0x05,
    eDCESensorDev_Sub2         = 0x08,
    eDCESensorDev_MainThird    = 0x10
}   eDCESensorDev_T;

#define DCE_AE_HISTOGRAM_BIN                            (256)
#define V_BIN_NUM                                       (64)
#define TILE_NUM                                        (20)
#define DCE_LV_TBL_NUM                                  (19)
#define DCE_CONTRAST_TBL_NUM                            (11)
#define DCE_CONTRAST_TBL_NUM 						    (11)
#define DCE_CONTRAST_INDEX_UNIT						    (10)



struct mtk_dce_tuning_lut
{
	MINT32 dce_tbl1[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
	MINT32 dce_tbl2[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
	MINT32 dce_tbl3[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
};

struct mtk_dce_stats
{
	MINT32 dce_stats0;
	MINT32 dce_stats1;
	MINT32 dce_stats2;
	MINT32 dce_stats3;
	MINT32 dce_stats4;
};


struct mtk_dce_reserve_param {
	MUINT32 dce_rsv0;
	MUINT32 dce_rsv1;
	MUINT32 dce_rsv2;
	MUINT32 dce_rsv3;
	MUINT32 dce_rsv4;
	MUINT32 dce_rsv5;
	MUINT32 dce_rsv6;
	MUINT32 dce_rsv7;
	MUINT32 dce_rsv8;
	MUINT32 dce_rsv9;
	MUINT32 dce_rsv_tbl[2][DCE_LV_TBL_NUM];
};



typedef struct {
	MUINT32 smooth_speed;
	MUINT32 flat_ratio_high;
	MUINT32 flat_ratio_low;
	MBOOL flt_prot;
	MUINT32 max_slope;
	struct mtk_dce_stats dce_stats_p;
	struct mtk_dce_tuning_lut dce_tuning_param_p;
	struct mtk_dce_reserve_param dce_reserve_param_p;
} MTK_DCE_TUNING_T;

typedef struct {
    MUINT32 u4AETarget;
    MUINT32 u4AECurrentTarget;


    MINT32  i4LightValue_x10;


    MINT32  i4GammaIdx;   // next gamma idx    MINT32  LESE_Ratio;    // LE/SE ratio

    MUINT32 u4MaxISO;
    MUINT32 u4AEStableCnt;

    MUINT32 u4Histogrm[DCE_AE_HISTOGRAM_BIN];
    MBOOL bGammaEnable;

    MBOOL bAEStable;
    MUINT32 u4EVRatio;
    MINT32 i4deltaIndex;
    MINT32 u4AEFinerEVIdxBase;

    //For touch dce
    MBOOL bAETouchEnable;

    //For face dce
    MUINT32 u4FaceAEStable;
    MUINT32 u4MeterFDTarget;
    MUINT32 u4FDProb;
    MUINT32 u4FaceNum;

    MINT32 i4Crnt_FDY;


    MUINT32 u4MaxGain;
    MUINT32 u4RequestNum;

}DCE_AE_INFO_T;

typedef struct {
    MINT32 dr_idx_l;
    MINT32 dr_idx_h;
} DCE_LCE_INFO_T;


typedef struct {
	MBOOL  dces_en;
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32* u4Dces;
} DCES_INFO_T;

typedef struct
{
    MTK_DCE_TUNING_T rAutoDCEParam;
} MTK_DCE_ENV_INFO_STRUCT, *P_MTK_DCE_ENV_INFO_STRUCT;

typedef struct
{
    MINT32 chip_version;
    MINT32 main_version;
    MINT32 sub_version;
    MINT32 system_version;
    
    MINT32 lv_idx_l;
    MINT32 lv_idx_h;
    MINT32 dr_idx_l;
    MINT32 dr_idx_h;
    MINT32 dce_flat_ratio_thd_l;
    MINT32 dce_flat_ratio_thd_h;
    MINT32 dce_smooth_speed;

    MINT32 dce_p1;
    MINT32 dce_p2;
    MINT32 dce_p3;
    MINT32 dce_p4;
    MINT32 dce_p5;
    MINT32 dce_p6;
    MINT32 dce_p7;
    MINT32 dce_p8;
    MINT32 dce_p9;
    MINT32 dce_p10;
    MINT32 dce_p11;
    MINT32 dce_p12;
    MINT32 dce_p13;
    MINT32 dce_p14;
    MINT32 dce_p15;
    MINT32 dce_p16;
    MINT32 dce_p17;
    MINT32 dce_p18;
    MINT32 dce_p19;
    MINT32 dce_p20;
    MINT32 dce_p21;
    MINT32 dce_p22;
    MINT32 dce_p23;
    MINT32 dce_p24;
    MINT32 dce_p25;
    MINT32 dce_p26;
    MINT32 dce_p27;
    MINT32 dce_p28;
    MINT32 dce_p29;
    MINT32 dce_p30;
    MINT32 dce_p31;
    MINT32 dce_p32;
    MINT32 dce_p33;
    MINT32 dce_p34;
    MINT32 dce_p35;
    MINT32 dce_p36;
    MINT32 dce_p37;
    MINT32 dce_p38;
    MINT32 dce_p39;
    MINT32 dce_p40;
    MINT32 dce_p41;
    MINT32 dce_p42;
    MINT32 dce_p43;
    MINT32 dce_p44;
    MINT32 dce_p45;
    MINT32 dce_p46;
    MINT32 dce_p47;
    MINT32 dce_p48;
    MINT32 dce_p49;
    MINT32 dce_p50;
    MINT32 dce_p51;
    MINT32 dce_p52;
    MINT32 dce_p53;
    MINT32 dce_p54;
    MINT32 dce_p55;
    MINT32 dce_p56;
    MINT32 dce_p57;
    MINT32 dce_p58;
    MINT32 dce_p59;
    MINT32 dce_p60;
    MINT32 dce_p61;
    MINT32 dce_p62;
    MINT32 dce_p63;
    MINT32 dce_p64;

    MINT32 flare_offset;
	MINT32 flare_gain;

	MINT32 dce_pt0;
	MINT32 dce_pt1;
	MINT32 dce_pt2;
	MINT32 dce_pt3;
	
	MINT32 dce_sub_hist1_height_max;
	MINT32 dce_sub_hist1_height_min;
	MINT32 dce_plat1;

	MINT32 dce_sub_hist2_height_max;
	MINT32 dce_sub_hist2_height_min;
	MINT32 dce_plat2;

	MINT32 dce_sub_hist3_height_max;
	MINT32 dce_sub_hist3_height_min;
	MINT32 dce_plat3;



	MINT32 dce_str1;
	MINT32 dce_str2;
	MINT32 dce_str3;

    MINT32 dce_flat_prot_flag1;
    MINT32 dce_flat_prot_flag2;
    MINT32 dce_flat_prot_flag3;


	MINT32 dces_en;
	MINT32 get_default;

	MINT32 dce_slope_max;
	MINT32 smooth_speed;
	MINT32 dce_flat_prot_flag;

} MTK_DCE_EXIF_INFO_STRUCT, *P_MTK_DCE_EXIF_INFO_STRUCT;

typedef struct
{
    DCE_AE_INFO_T dce_ae_info;   // Get current AE related information
    DCE_LCE_INFO_T dce_lce_info;
    DCES_INFO_T dces_info; // Get current LCS information
   	MBOOL  get_default;
} MTK_DCE_PROC_INFO_STRUCT, *P_MTK_DCE_PROC_INFO_STRUCT;

typedef struct
{
    MINT32 dce_p1;
    MINT32 dce_p2;
    MINT32 dce_p3;
    MINT32 dce_p4;
    MINT32 dce_p5;
    MINT32 dce_p6;
    MINT32 dce_p7;
    MINT32 dce_p8;
    MINT32 dce_p9;
    MINT32 dce_p10;
    MINT32 dce_p11;
    MINT32 dce_p12;
    MINT32 dce_p13;
    MINT32 dce_p14;
    MINT32 dce_p15;
    MINT32 dce_p16;
    MINT32 dce_p17;
    MINT32 dce_p18;
    MINT32 dce_p19;
    MINT32 dce_p20;
    MINT32 dce_p21;
    MINT32 dce_p22;
    MINT32 dce_p23;
    MINT32 dce_p24;
    MINT32 dce_p25;
    MINT32 dce_p26;
    MINT32 dce_p27;
    MINT32 dce_p28;
    MINT32 dce_p29;
    MINT32 dce_p30;
    MINT32 dce_p31;
    MINT32 dce_p32;
    MINT32 dce_p33;
    MINT32 dce_p34;
    MINT32 dce_p35;
    MINT32 dce_p36;
    MINT32 dce_p37;
    MINT32 dce_p38;
    MINT32 dce_p39;
    MINT32 dce_p40;
    MINT32 dce_p41;
    MINT32 dce_p42;
    MINT32 dce_p43;
    MINT32 dce_p44;
    MINT32 dce_p45;
    MINT32 dce_p46;
    MINT32 dce_p47;
    MINT32 dce_p48;
    MINT32 dce_p49;
    MINT32 dce_p50;
    MINT32 dce_p51;
    MINT32 dce_p52;
    MINT32 dce_p53;
    MINT32 dce_p54;
    MINT32 dce_p55;
    MINT32 dce_p56;
    MINT32 dce_p57;
    MINT32 dce_p58;
    MINT32 dce_p59;
    MINT32 dce_p60;
    MINT32 dce_p61;
    MINT32 dce_p62;
    MINT32 dce_p63;
    MINT32 dce_p64;

	MINT32 flare_offset;
	MINT32 flare_gain;

	MINT32 yv_bld;


} MTK_DCE_RESULT_INFO_STRUCT, *P_MTK_DCE_RESULT_INFO_STRUCT;

class MTKDce
{
public:
    static MTKDce* createInstance(eDCESensorDev_T const eSensorDev);
    virtual MVOID   destroyInstance(MTKDce* obj) = 0;

    virtual ~MTKDce(){}

    // Process Control
    virtual MRESULT DceInit(MVOID *InitInData, MVOID *InitOutData) = 0;
    virtual MRESULT DceMain(void) = 0;
    virtual MRESULT DceExit(void) = 0;
    virtual MRESULT DceReset(void) = 0;                 // RESET for each image

    // Feature Control
    virtual MRESULT DceFeatureCtrl(MUINT32 FeatureID, MVOID* pParaIn, MVOID* pParaOut) = 0;
private:

};



#endif  //__MTK_Dce_H__
//MTK_SWIP_PROJECT_END
