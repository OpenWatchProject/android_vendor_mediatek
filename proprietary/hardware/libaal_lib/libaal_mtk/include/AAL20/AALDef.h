#ifndef __AAL_DEF_H__
#define __AAL_DEF_H__

#define DRE_POINT_NUM 29
#define DRE30_BLK_NUM_MAX 16
#define DRE30_BLK_SFLT_SIZE_MAX 7

#define DRE30_17_BIN_MODE
#ifdef DRE30_17_BIN_MODE
#define DRE30_LUMA_POINT_BIT    4 // For ( 17 = 2^4 + 1 ) points
#else
#define DRE30_LUMA_POINT_BIT    5 // For ( 33 = 2^5 + 1 ) points
#endif

#define DRE30_LUMA_HIST_NUM           ( 1 << ( DRE30_LUMA_POINT_BIT ) )
#define DRE30_LUMA_HIST_NUM_WEIGHT    ( DRE30_LUMA_HIST_NUM + 1 )
#define DRE30_LUMA_CURVE_NUM          ( DRE30_LUMA_HIST_NUM + 1 )

enum DebugFlags {
    eDebugDisabled          = 0x0,
    eDebugInput             = 0x1,
    eDebugOutput            = 0x2,
    eDebugTime              = 0x4,
    eDebugContent           = 0x8,
    eDebugLABC              = 0x10,
    eDebugCABC              = 0x20,
    eDebugDRE               = 0x40,
    eDebugFilter            = 0x80,
    eDebugPartial           = 0x100,
    eDebugBasic             = 0x200,
    eDebugAll               = 0x3FF
};

// Event bits
enum AALEvent {
    // Ambilient light changed
    eEvtALI             = 0x1,
    // Screen state changed. See ScreenState
    eEvtScrState        = 0x2,
    // User setting changed. See AALFunction
    eEvtUserSetting     = 0x4,
    eEvtFunction        = eEvtUserSetting,
    // User brightness configuration changed.
    eEvtUserBrightness  = 0x8,
    // Target backlight value changed
    eEvtTargetBacklight = 0x10,
    // Animation target changed
    eEvtLongTermBacklight = 0x20,
    // Configuration changed
    eEvtConfigChanged   = 0x40,
    // Some configuration field has been written
    eEvtFieldWritten    = 0x80,
    // Triggered by algorithm implementation
    eEvtActiveTrigger   = 0x100,
    // Intialize, set when boot or AAL restart
    eEvtInit            = 0x200,
    // FPS changed
    eEvtFPS             = 0x400,
    // Partial update, set when display request partial update
    eEvtPartialUpdate   = 0x800,
    // Some configuration field has been read
    eEvtFieldRead    = 0x1000,
};

enum AALFunction {
    eFuncNone   = 0,
    eFuncLABC   = 0x1,
    eFuncCABC   = 0x2,
    eFuncDRE    = 0x4
};

// The value should be the same as the SCREEN_STATE enum in IAALService
enum ScreenState {
    eScrStateOff = 0,
    eScrStateDoze = 1,
    eScrStateDim = 2,
    eScrStateOn = 3,
};

//enum DebugFlags {
//    eDebugDisabled          = 0x0,
//    eDebugInput             = 0x1,
//    eDebugOutput            = 0x2,
//    eDebugTime              = 0x4,
//    eDebugContent           = 0x8,
//    eDebugLABC              = 0x10,
//    eDebugCABC              = 0x20,
//    eDebugDRE               = 0x40,
//    eDebugFilter            = 0x80,
//    eDebugPartial           = 0x100,
//    eDebugAll               = 0x1FF
//};

enum AALRefreshLatency {
    eRefresh_17ms = 17,
    eRefresh_33ms = 33
};


struct AALInitParam {
    // Screen width & height
    int width;
    int height;
};

struct AALInitReg {
    // DRE
    int dre_map_bypass;
    // CABC
    int cabc_gainlmt[33];

    int dre_s_lower;
    int dre_s_upper;
    int dre_y_lower;
    int dre_y_upper;
    int dre_h_lower;
    int dre_h_upper;
    int dre_x_alpha_base;
    int dre_x_alpha_shift_bit;
    int dre_y_alpha_base;
    int dre_y_alpha_shift_bit;
    int act_win_x_end;
    int dre_blk_x_num;
    int dre_blk_y_num;
    int dre_blk_height;
    int dre_blk_width;
    int dre_blk_area;
    int dre_blk_area_min;
    int hist_bin_type;
    int dre_flat_length_slope;
};

// Fields collected from AAL HW
struct AALInput {
    // Please check the event by
    // if (eventFlags & eEvtALI) { ... }
    unsigned int eventFlags;

    // Calibrated value(LUX) from light sensor
    // < 0 means invalid yet
    int currALI;

    // Test by
    // if (funcFlags & eLABC) { ... }
    // Manual mode = 0x0
    // Auto mode = LABC + CABC + DRE = (eLABC | eCABC | eDRE) = 0x7
    // ECO mode = CABC only = eCABC = 0x2
    //
    // To know which function is toggled, we can use XOR to test
    // ((prevFuncFlags ^ funcFlags) & eLABC)
    unsigned int prevFuncFlags;
    unsigned int funcFlags;

    bool prevIsPartialUpdate;
    bool isPartialUpdate;

    ScreenState prevScrState;
    ScreenState currScrState;

    // Target backlight of Android calculated or user set.
    // This will be used on the moment of user setting changed or mode changed.
    // AAL should smooth the backlight to the target before give up the control.
    // [0, 1023] -> [0, 4095] (12-bit)
    unsigned int targetBacklight;

    // The destination backlight of manual mode, usually the position of the
    // backlight scroll bar position.
    // [0, 1023]
    unsigned int longTermBacklight;

    /*
     * The brightness level of LABC configured by user.
     * [0, 255]
     */
    int userBrightnessLevel;

    // List the registers should be read from HW
    int MaxHisBin[33];        // 25-bit

    // List the Y hist. read from HW
    int YHisBin[33];        // ? -bit
    // Mainly for the statistics of skin-tone pixels
    int ColorHist;

    // Block Histogram
    int DREMaxHisSet[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX][DRE30_LUMA_HIST_NUM_WEIGHT];

    // Block APL
    int DRERGBMaxSum[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX];

    // Block Chroma Confidence
    int DREChromaHist[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX];

    // Block Flat Line Confidence
    int DREFlatLineCountSet[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX];

    // Block Large Diff Confidence
    int DRELargeDiffCountSet[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX];

    // Block Max Diff
    int DREMaxDiffSet[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX];

    // Block Numbers
    int dre_blk_x_num;
    int dre_blk_y_num;
};

// Fields which will be set to HW registers
struct AALOutput {
    // List the register values should be set to HW

    // DRE
    int DREGainFltStatus[ DRE_POINT_NUM ];

    // Block Mapping Curve
    int DRECurveSet[DRE30_BLK_NUM_MAX][DRE30_BLK_NUM_MAX][DRE30_LUMA_CURVE_NUM];

    //CABC
    int cabc_fltgain_force;   // 10-bit; [0,1023]
    int cabc_gainlmt[33];

    int FinalBacklight;         // 10-bit; [0,1023]

    // To set new latency, please always get the minimum. For example,
    // output->refreshLatency = min(output->refreshLatency, newLatency);
    AALRefreshLatency refreshLatency;

    // Trigger main function next time anyway, regardless there is any event.
    // Set to true if output is not stable yet.
    bool activeTrigger;

    // flag for notify display that AAL accept partial update request or not
    bool allowPartialUpdate;
};

struct ImplParameters {
    int brightnessLevel;
    int darkeningSpeedLevel;
    int brighteningSpeedLevel;
    int smartBacklightStrength;
    int smartBacklightRange;
    int readabilityLevel;
    int readabilityBLILevel;
};

#endif
