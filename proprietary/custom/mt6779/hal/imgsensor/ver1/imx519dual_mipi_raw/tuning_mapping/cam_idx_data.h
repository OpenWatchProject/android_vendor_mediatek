#ifndef _CAM_IDX_DATA_H_
#define _CAM_IDX_DATA_H_

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "tuning_mapping/cam_idx_struct_int.h"

using namespace NSIspTuning;
#include "cam_idx_data_SLK.inc"
#include "cam_idx_data_AE.inc"
#include "cam_idx_data_AF.inc"
#include "cam_idx_data_AWB.inc"
#include "cam_idx_data_Flash_AE.inc"
#include "cam_idx_data_Flash_AWB.inc"
#include "cam_idx_data_Flash_Calibration.inc"
#include "cam_idx_data_LCE.inc"
#include "cam_idx_data_GMA.inc"
#include "cam_idx_data_DCE.inc"
#include "cam_idx_data_DM.inc"
#include "cam_idx_data_YNR.inc"
#include "cam_idx_data_YNR_LCE_LINK.inc"
#include "cam_idx_data_YNR_FD.inc"
#include "cam_idx_data_CNR_CNR.inc"
#include "cam_idx_data_EE.inc"
#include "cam_idx_data_NR3D.inc"
#include "cam_idx_data_HFG.inc"
#include "cam_idx_data_OBC.inc"
#include "cam_idx_data_BPC_BPC.inc"
#include "cam_idx_data_BPC_CT.inc"
#include "cam_idx_data_BPC_PDC.inc"
#include "cam_idx_data_LDNR.inc"
#include "cam_idx_data_CNR_CCR.inc"
#include "cam_idx_data_CNR_ABF.inc"
#include "cam_idx_data_SWNR_THRES.inc"
#include "cam_idx_data_SWNR.inc"
#include "cam_idx_data_LPCNR.inc"
#include "cam_idx_data_FUS.inc"
#include "cam_idx_data_ZFUS.inc"
#include "cam_idx_data_MFNR_THRES.inc"
#include "cam_idx_data_MFNR.inc"
#include "cam_idx_data_BSS.inc"
#include "cam_idx_data_MFB.inc"
#include "cam_idx_data_MIX.inc"
#include "cam_idx_data_CA_LTM.inc"
#include "cam_idx_data_CCM.inc"
#include "cam_idx_data_COLOR_PARAM.inc"
#include "cam_idx_data_COLOR.inc"
#include "cam_idx_data_ClearZoom.inc"
#include "cam_idx_data_AINR.inc"
#include "cam_idx_data_DSDN.inc"
#include "cam_idx_data_YNRS.inc"
#include "cam_idx_data_YNR_TBL.inc"
#include "cam_idx_data_LTM.inc"
#include "cam_idx_data_HLR.inc"
#include "cam_idx_data_AINR_THRES.inc"

#define SCENARIO_NUM 29

#define GEN_TOOL_TIMESTAMP "2019-03-05T17:31:09.693000"

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
        "HDR_Preview",
        "HDR_Video",
        "HDR_Capture",
        "N3D_Preview",
        "N3D_Video",
        "N3D_Capture",
        "Flash_Capture",
        "Scene_Preview_ZSD_Flash",
        "Face_Preview_ZSD_Flash",
        "P1_YUV",
        "Zoom_Capture",
        "Zoom_Preview",
        "Default_Capture",
        "Default_Preview",
        "Default_Video",
        "UserDefine",
        "Mismatch",
};

static EISO_GROUP_T _iso_group[SCENARIO_NUM] =
{
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
    EISO_GROUP_00,
};

#define _EIspProfile_NUM_ 62
#define _ESensorMode_NUM_ 10
#define _EFrontBin_NUM_ 2
#define _ESize_NUM_ 4
#define _EFlash_NUM_ 4
#define _EApp_NUM_ 13
#define _EFaceDetection_NUM_ 2
#define _EDriverIC_NUM_ 1
#define _ECustom_00_NUM_ 3
#define _ECustom_01_NUM_ 3
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
        _ESize_NUM_,
        _EFlash_NUM_,
        _EApp_NUM_,
        _EFaceDetection_NUM_,
        _EDriverIC_NUM_,
        _ECustom_00_NUM_,
        _ECustom_01_NUM_,
        _EZoom_NUM_,
        _ELV_NUM_,
        _ECT_NUM_,
        _EISO_NUM_,
    },
    {
        (IDX_BASE_T*)&cam_data_SLK,
        (IDX_BASE_T*)&cam_data_AE,
        (IDX_BASE_T*)&cam_data_AF,
        (IDX_BASE_T*)&cam_data_AWB,
        (IDX_BASE_T*)&cam_data_Flash_AE,
        (IDX_BASE_T*)&cam_data_Flash_AWB,
        (IDX_BASE_T*)&cam_data_Flash_Calibration,
        (IDX_BASE_T*)&cam_data_LCE,
        (IDX_BASE_T*)&cam_data_GMA,
        (IDX_BASE_T*)&cam_data_DCE,
        (IDX_BASE_T*)&cam_data_DM,
        (IDX_BASE_T*)&cam_data_YNR,
        (IDX_BASE_T*)&cam_data_YNR_LCE_LINK,
        (IDX_BASE_T*)&cam_data_YNR_FD,
        (IDX_BASE_T*)&cam_data_CNR_CNR,
        (IDX_BASE_T*)&cam_data_EE,
        (IDX_BASE_T*)&cam_data_NR3D,
        (IDX_BASE_T*)&cam_data_HFG,
        (IDX_BASE_T*)&cam_data_OBC,
        (IDX_BASE_T*)&cam_data_BPC_BPC,
        (IDX_BASE_T*)&cam_data_BPC_CT,
        (IDX_BASE_T*)&cam_data_BPC_PDC,
        (IDX_BASE_T*)&cam_data_LDNR,
        (IDX_BASE_T*)&cam_data_CNR_CCR,
        (IDX_BASE_T*)&cam_data_CNR_ABF,
        (IDX_BASE_T*)&cam_data_SWNR_THRES,
        (IDX_BASE_T*)&cam_data_SWNR,
        (IDX_BASE_T*)&cam_data_LPCNR,
        (IDX_BASE_T*)&cam_data_FUS,
        (IDX_BASE_T*)&cam_data_ZFUS,
        (IDX_BASE_T*)&cam_data_MFNR_THRES,
        (IDX_BASE_T*)&cam_data_MFNR,
        (IDX_BASE_T*)&cam_data_BSS,
        (IDX_BASE_T*)&cam_data_MFB,
        (IDX_BASE_T*)&cam_data_MIX,
        (IDX_BASE_T*)&cam_data_CA_LTM,
        (IDX_BASE_T*)&cam_data_CCM,
        (IDX_BASE_T*)&cam_data_COLOR_PARAM,
        (IDX_BASE_T*)&cam_data_COLOR,
        (IDX_BASE_T*)&cam_data_ClearZoom,
        (IDX_BASE_T*)&cam_data_AINR,
        (IDX_BASE_T*)&cam_data_DSDN,
        (IDX_BASE_T*)&cam_data_YNRS,
        (IDX_BASE_T*)&cam_data_YNR_TBL,
        (IDX_BASE_T*)&cam_data_LTM,
        (IDX_BASE_T*)&cam_data_HLR,
        (IDX_BASE_T*)&cam_data_AINR_THRES,
    },
    SCENARIO_NUM,
    (char (*)[][64])&_scenarios
};
#endif
#endif