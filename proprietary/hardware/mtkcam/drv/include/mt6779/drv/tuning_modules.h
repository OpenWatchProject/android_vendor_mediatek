#ifndef TUNING_MODULES_H_
#define TUNING_MODULES_H_
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
enum ETuningMgrFunc  // need mapping to EIspTuningMgrFunc
{
    eTuningMgrFunc_Null = 0,
    eTuningMgrFunc_OBC_R1,
    eTuningMgrFunc_BPC_R1,     // BNR rename to BPC
    eTuningMgrFunc_LSC_R1,
    eTuningMgrFunc_AA_R1,
    eTuningMgrFunc_AF_R1,
    eTuningMgrFunc_FLK_R1,
    eTuningMgrFunc_PBN_R1,
    eTuningMgrFunc_PDE_R1,

    // new Module
    eTuningMgrFunc_MOBC_R2, // CAM_C only
    eTuningMgrFunc_WB_R1,
    eTuningMgrFunc_DGN_R1,
    eTuningMgrFunc_TSFS_R1,    // PS rename to TSFS

    // new Module - YUV DM path
    eTuningMgrFunc_DM_R1,
    eTuningMgrFunc_FLC_R1,
    eTuningMgrFunc_CCM_R1,
    eTuningMgrFunc_GGM_R1,
    eTuningMgrFunc_GGM_R2,
    eTuningMgrFunc_G2C_R1,
    eTuningMgrFunc_G2C_R2,
    eTuningMgrFunc_YNRS_R1,
    eTuningMgrFunc_RSS_R2,
    eTuningMgrFunc_SLK_R1,
    eTuningMgrFunc_SLK_R2,
    eTuningMgrFunc_BPCI_R1,
    eTuningMgrFunc_LCES_R1,
    eTuningMgrFunc_LTMS_R1,
    eTuningMgrFunc_LTM_R1,
    eTuningMgrFunc_LTMTC_R1_CURVE,
    eTuningMgrFunc_HLR_R1,
    eTuningMgrFunc_Num,
};
#endif // TUNING_MODULES_H_
