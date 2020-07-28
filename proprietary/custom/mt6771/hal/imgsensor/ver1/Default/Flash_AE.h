const FLASH_AE_NVRAM_T Flash_AE_%04d = {
    .tuningPara = {
        188,   // yTarget
        10,    // fgWIncreaseLevelbySize
        5,     // fgWIncreaseLevelbyRef
        5,     // ambientRefAccuracyRatio
        0,     // flashRefAccuracyRatio
        18,    // backlightAccuracyRatio
        40,    // backlightUnderY
        32,    // backlightWeakRefRatio
        66644, // safetyExp
        1200,  // maxUsableISO
        0,     // yTargetWeight
        13,    // lowReflectanceThreshold
        0,     // flashReflectanceWeight
        20,    // bgSuppressMaxDecreaseEV
        6,     // bgSuppressMaxOverExpRatio
        50,    // fgEnhanceMaxIncreaseEV
        2,     // fgEnhanceMaxOverExpRatio
        1,     // isFollowCapPline
        300,   // histStretchMaxFgYTarget
        480,   // histStretchBrightestYTarget
        0,     // fgSizeShiftRatio
        90,    // backlitPreflashTriggerLV
        90,    // backlitMinYTarget
        80,    // minstameanpass
       188,    // yDecreEVTarget
       188,    // yFaceTarget
         5,    // cfgFlashPolicy
    },
    .dualTuningPara = {
        30,                // toleranceEV_pos
        30,                // toleranceEV_neg
        64,                // XYWeighting
        1,                 // useAwbPreferenceGain
        // envOffsetIndex[4]
        {
            -200,
            -100,
            50,
            150,
        },
        // envXrOffsetValue[4]
        {
            0,
            0,
            0,
            0,
        },
        // envYrOffsetValue[4]
        {
            0,
            0,
            0,
            0,
        },
        1,                 // VarianceTolerance
        FLASH_CHOOSE_COLD, // ChooseColdOrWarm
    },
    .engLevel = {
        6,    // torchDuty
        {},   // torchDutyEx[20]
        6,    // afDuty
        6,    // pfDuty
        25,   // mfDutyMax
        -1,   // mfDutyMin
        0,    // IChangeByVBatEn
        3600, // vBatL
        6,    // pfDutyL
        6,    // mfDutyMaxL
        -1,   // mfDutyMinL
        1,    // IChangeByBurstEn
        6,    // pfDutyB
        6,    // mfDutyMaxB
        -1,   // mfDutyMinB
        1,    // decSysIAtHighEn
        20,   // dutyH
    },
    .engLevelLT = {
        0,    // torchDuty
        {},   // torchDutyEx[20]
        0,    // afDuty
        0,    // pfDuty
        25,   // mfDutyMax
        -1,   // mfDutyMin
        0,    // pfDutyL
        6,    // mfDutyMaxL
        -1,   // mfDutyMinL
        0,    // pfDutyB
        6,    // mfDutyMaxB
        -1,   // mfDutyMinB
    },
};
