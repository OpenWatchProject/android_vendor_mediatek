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

const IDX_MODULE_ARRAY idx_mod_array = {
    {
        EIspProfile_NUM,
        ESensorMode_NUM,
        EFrontBin_NUM,
        EP2Size_NUM,
        EFlash_NUM,
        EApp_NUM,
        EFaceDetection_NUM,
        ELensID_NUM,
        EDriverIC_NUM,
        ECustom_NUM,
        EZoom_NUM,
        ELV_NUM,
        ECT_NUM,
        EISO_NUM,
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
    }
};
#endif
#endif