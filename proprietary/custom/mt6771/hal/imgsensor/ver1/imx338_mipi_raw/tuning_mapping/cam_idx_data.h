#ifndef _CAM_IDX_DATA_H_
#define _CAM_IDX_DATA_H_

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "tuning_mapping/cam_idx_struct_int.h"

using namespace NSIspTuning;
#include "cam_idx_data_DBS.inc"
#include "cam_idx_data_ADBS.inc"
#include "cam_idx_data_OBC.inc"
#include "cam_idx_data_BNR_BPC.inc"
#include "cam_idx_data_BNR_NR1.inc"
#include "cam_idx_data_BNR_PDC.inc"
#include "cam_idx_data_RMM.inc"
#include "cam_idx_data_RNR.inc"
#include "cam_idx_data_SL2F.inc"
#include "cam_idx_data_SL2.inc"
#include "cam_idx_data_UDM.inc"
#include "cam_idx_data_NBC_ANR.inc"
#include "cam_idx_data_NBC2_ANR2.inc"
#include "cam_idx_data_NBC2_CCR.inc"
#include "cam_idx_data_NBC2_ABF.inc"
#include "cam_idx_data_HFG.inc"
#include "cam_idx_data_EE.inc"
#include "cam_idx_data_NR3D.inc"
#include "cam_idx_data_MFB.inc"
#include "cam_idx_data_MIXER3.inc"
#include "cam_idx_data_CCM.inc"
#include "cam_idx_data_COLOR.inc"
#include "cam_idx_data_PCA.inc"
#include "cam_idx_data_AE.inc"
#include "cam_idx_data_AF.inc"
#include "cam_idx_data_AWB.inc"
#include "cam_idx_data_Flash_AE.inc"
#include "cam_idx_data_Flash_AWB.inc"
#include "cam_idx_data_Flash_Calibration.inc"
#include "cam_idx_data_LCE.inc"
#include "cam_idx_data_MFNR.inc"
#include "cam_idx_data_SWNR.inc"
#include "cam_idx_data_CA_LTM.inc"
#include "cam_idx_data_ClearZoom.inc"
#include "cam_idx_data_GMA.inc"
#include "cam_idx_data_NBC_LCE_LINK.inc"
#include "cam_idx_data_NBC_TBL.inc"
#include "cam_idx_data_COLOR_PARAM.inc"
#include "cam_idx_data_SWNR_THRES.inc"
#include "cam_idx_data_FD_ANR.inc"
#include "cam_idx_data_DCE.inc"

#define SCENARIO_NUM 27

#define GEN_TOOL_TIMESTAMP "2019-08-05T11:39:53.458000"

static char _scenarios[SCENARIO_NUM][64] =
{
        "Scene_Capture",
        "Scene_Capture_4cell",
        "Face_Capture",
        "Face_Capture_4cell",
        "Scene_Preview",
        "Scene_Preview_ZSD",
        "Scene_Preview_4k",
        "Face_Preview",
        "Face_Preview_ZSD",
        "Face_Preview_4k",
        "Video_1080",
        "Video_4k",
        "AutoHDR_Preview",
        "AutoHDR_Video",
        "AutoHDR_Capture",
        "HDR_Preview",
        "HDR_Video",
        "HDR_Capture",
        "N3D_Preview",
        "N3D_Video",
        "N3D_Capture",
        "Flash_Capture",
        "Flash_HDR_Capture",
        "Scene_Preview_ZSD_Flash",
        "Face_Preview_ZSD_Flash",
        "UserDefine",
        "Mismatch",
};

#define _EIspProfile_NUM_ 87
#define _ESensorMode_NUM_ 10
#define _EFrontBin_NUM_ 2
#define _EP2Size_NUM_ 4
#define _EFlash_NUM_ 3
#define _EApp_NUM_ 13
#define _EFaceDetection_NUM_ 2
#define _ELensID_NUM_ 1
#define _EDriverIC_NUM_ 1
#define _ECustom_NUM_ 1
#define _EZoom_NUM_ 4
#define _ELV_NUM_ 6
#define _ECT_NUM_ 10
#define _EISO_NUM_ 20

#include "cam_idx_data_Check.inc"

const IDX_MODULE_ARRAY idx_mod_array = {
    {
        _EIspProfile_NUM_,
        _ESensorMode_NUM_,
        _EFrontBin_NUM_,
        _EP2Size_NUM_,
        _EFlash_NUM_,
        _EApp_NUM_,
        _EFaceDetection_NUM_,
        _ELensID_NUM_,
        _EDriverIC_NUM_,
        _ECustom_NUM_,
        _EZoom_NUM_,
        _ELV_NUM_,
        _ECT_NUM_,
        _EISO_NUM_,
    },
    {
        (IDX_BASE_T*)&cam_mask_DBS,
        (IDX_BASE_T*)&cam_mask_ADBS,
        (IDX_BASE_T*)&cam_mask_OBC,
        (IDX_BASE_T*)&cam_mask_BNR_BPC,
        (IDX_BASE_T*)&cam_mask_BNR_NR1,
        (IDX_BASE_T*)&cam_mask_BNR_PDC,
        (IDX_BASE_T*)&cam_mask_RMM,
        (IDX_BASE_T*)&cam_mask_RNR,
        (IDX_BASE_T*)&cam_mask_SL2F,
        (IDX_BASE_T*)&cam_mask_SL2,
        (IDX_BASE_T*)&cam_mask_UDM,
        (IDX_BASE_T*)&cam_mask_NBC_ANR,
        (IDX_BASE_T*)&cam_mask_NBC2_ANR2,
        (IDX_BASE_T*)&cam_mask_NBC2_CCR,
        (IDX_BASE_T*)&cam_mask_NBC2_ABF,
        (IDX_BASE_T*)&cam_mask_HFG,
        (IDX_BASE_T*)&cam_mask_EE,
        (IDX_BASE_T*)&cam_mask_NR3D,
        (IDX_BASE_T*)&cam_mask_MFB,
        (IDX_BASE_T*)&cam_mask_MIXER3,
        (IDX_BASE_T*)&cam_mask_CCM,
        (IDX_BASE_T*)&cam_mask_COLOR,
        (IDX_BASE_T*)&cam_mask_PCA,
        (IDX_BASE_T*)&cam_mask_AE,
        (IDX_BASE_T*)&cam_mask_AF,
        (IDX_BASE_T*)&cam_mask_AWB,
        (IDX_BASE_T*)&cam_mask_Flash_AE,
        (IDX_BASE_T*)&cam_mask_Flash_AWB,
        (IDX_BASE_T*)&cam_mask_Flash_Calibration,
        (IDX_BASE_T*)&cam_mask_LCE,
        (IDX_BASE_T*)&cam_mask_MFNR,
        (IDX_BASE_T*)&cam_mask_SWNR,
        (IDX_BASE_T*)&cam_mask_CA_LTM,
        (IDX_BASE_T*)&cam_mask_ClearZoom,
        (IDX_BASE_T*)&cam_mask_GMA,
        (IDX_BASE_T*)&cam_mask_NBC_LCE_LINK,
        (IDX_BASE_T*)&cam_mask_NBC_TBL,
        (IDX_BASE_T*)&cam_mask_COLOR_PARAM,
        (IDX_BASE_T*)&cam_mask_SWNR_THRES,
        (IDX_BASE_T*)&cam_mask_FD_ANR,
        (IDX_BASE_T*)&cam_mask_DCE,
    },
    SCENARIO_NUM,
    (char (*)[][64])&_scenarios
};
#endif
#endif