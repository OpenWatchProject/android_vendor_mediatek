// copy to isp_tuning_cam_info.h
typedef union
{
    struct {
        EIspProfile_T eIspProfile;
        ESensorMode_T eSensorMode;
        EFrontBin_T eFrontBin;
        EP2Size_T eP2Size;
        EFlash_T eFlash;
        EApp_T eApp;
        EFaceDetection_T eFaceDetection;
        ELensID_T eLensID;
        EDriverIC_T eDriverIC;
        ECustom_T eCustom;
        EZoom_T eZoom;
        ELV_T eLV;
        ECT_T eCT;
        EISO_T eISO;
    };
    unsigned int query[NSIspTuning::EDim_NUM];
} CAM_IDX_QRY_COMB;

// copy to camera_custom_nvram.h
typedef struct NVRAM_CAMERA_IDX_STRUCT_T
{
    unsigned short idx_factor_ns[NSIspTuning::EDim_NUM];
    void * modules[NSIspTuning::EModule_NUM];
} NVRAM_CAMERA_IDX_STRUCT, *PNVRAM_CAMERA_IDX_STRUCT;
