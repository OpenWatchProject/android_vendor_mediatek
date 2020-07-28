const NVRAM_CAMERA_FEATURE_MFLL_STRUCT MFNR_%04d = {
    // Gives N, for capturing N frames
    .capture_frame_number        =  6,
    // Gives N, for N-frame blending. E.g.: N = 6, 6-frame blending, do blending 5 times.
    .blend_frame_number          =  6,
    // Give 0 or 1, 0 for half size MC, 1 for full size MC
    .full_size_mc                =  1,
    .memc_bad_mv_range           =  255,
    .memc_bad_mv_rate_th         =  127,
    // The threshold of doing MFB or not
    .mfll_iso_th                 =  200,
    // The threshold of doing Pline changing or not
    .pline_iso_th                =  200,
    // iso range for adaptive frame number mechanism
    .iso_level1                  =  200,
    .iso_level2                  =  400,
    .iso_level3                  =  800,
    .iso_level4                  =  3200,
    .iso_level5                  =  4000,
    .iso_level6                  =  4000,   // reserved for extension
    .iso_level7                  =  4000,   // reserved for extension
    // at least to set blend frame number >= 2
    .frame_num1                  =  4,
    .frame_num2                  =  4,
    .frame_num3                  =  4,
    .frame_num4                  =  4,
    .frame_num5                  =  5,
    .frame_num6                  =  6,
    .frame_num7                  =  6,      // reserved for extension
    .frame_num8                  =  6,      // reserved for extenion
    // MEMC level mapping
    .memc_iso_th0                =  300,
    .memc_iso_th1                =  600,
    .memc_iso_th2                =  1200,
    .memc_iso_th3                =  2400,
    .memc_iso_th4                =  4800,
    .memc_lv_val0                =  4,
    .memc_lv_val1                =  5,
    .memc_lv_val2                =  6,
    .memc_lv_val3                =  6,
    .memc_lv_val4                =  6,
    .memc_lv_val5                =  6,
    //
    .fRsc                        =  0.02774390,
    .fBiasX                      = -0.10535700,
    .fBiasY                      = -0.00918811,
    .fBiasZ                      =  0.01096210,
    .fFocal                      =  1470.14,
    .fTofst                      =  0.004605,
    .uMotionBase                 =  120,
    .uMVThLow                    =  1,
    .uMVThHigh                   =  3,
    .u4MinExpUs                  =  16000,
    .u4MaxIso                    =  3200,
    .u4ParamWidth                =  100,
    .u4ParamHeight               =  100,
    //
    .reserved                    =  {0}
};