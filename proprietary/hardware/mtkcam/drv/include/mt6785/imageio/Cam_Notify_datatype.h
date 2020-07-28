#ifndef _CAM_NOTIFY_DATATYPE_H_
#define _CAM_NOTIFY_DATATYPE_H_

////////////////////////////
//Structure
///////////////////////////
#include <vector>
using namespace std;


typedef struct _EIS_SIZE{
    MUINT32 w;
    MUINT32 h;
}EIS_SIZE;

typedef struct _LCS_INPUT_INFO{
    EIS_SIZE    sRRZOut;
    EIS_SIZE    sHBINOut;   //QBIN2
    EIS_SIZE    sTGOut;

    MBOOL       bIsHbin;
    MUINT32     magic;
    MBOOL       bIsDbin;
    MBOOL       bIsbin;
    MBOOL       bIsVbn;
    MUINT32     bQbinRatio;

    struct{
        MUINT32 start_x;
        MUINT32 start_y;
        MUINT32 crop_size_w;
        MUINT32 crop_size_h;

        MUINT32 in_size_w;
        MUINT32 in_size_h;
    }RRZ_IN_CROP;
}LCS_INPUT_INFO;

typedef struct _LCS_REG_CFG{
    MBOOL   bLCS_EN;
    MBOOL   bLCS_Bypass;    // 1 if no need to update LCS or LCS_D reg
    MUINT32 u4LCSO_Stride;
    struct{
        MUINT32 LCS_ST;
        MUINT32 LCS_AWS;
        MUINT32 LCS_LRZR_1;
        MUINT32 LCS_LRZR_2;
        MUINT32 LCS_CON;
        MUINT32 LCS_FLR;
        MUINT32 LCS_SATU1;
        MUINT32 LCS_SATU2;
        MUINT32 LCS_GAIN_1;
        MUINT32 LCS_GAIN_2;
        MUINT32 LCS_OFST_1;
        MUINT32 LCS_OFST_2;
        MUINT32 LCS_G2G_CNV_1;
        MUINT32 LCS_G2G_CNV_2;
        MUINT32 LCS_G2G_CNV_3;
        MUINT32 LCS_G2G_CNV_4;
        MUINT32 LCS_G2G_CNV_5;
        MUINT32 LCS_LPF;
    }_LCS_REG;
}LCS_REG_CFG;

typedef struct _CQ_DUMP_INFO{
    MUINT32 size;
    MUINT32 magic;
    void    *pReg;
}CQ_DUMP_INFO;

typedef MVOID (*FP_SEN)(MUINT32, MUINT32);


class SENINF_DBG
{
    public:
        static FP_SEN m_fp_Sen;
        static FP_SEN m_fp_Sen_Camsv;
};

typedef enum {
    CAM_ENTER_WAIT = 0,
    CAM_EXIT_WAIT,
    CAM_HOLD_IDLE,
    CAM_RELEASE_IDLE,
    CAM_ENTER_SUSPEND,
    CAM_EXIT_SUSPEND,
    CAM_ENTER_ENQ,
    CAM_EXIT_ENQ
} CAM_STATE_OP;

typedef enum {
    NOTIFY_PASS = 0,
    NOTIFY_FAIL
} E_NOTIFY_STATUS;

typedef MUINT32 (*FP_STATE_NOTIFY)(CAM_STATE_OP state, MVOID *Obj);

typedef struct _CAM_STATE_NOTIFY {
    FP_STATE_NOTIFY fpNotifyState;
    MVOID           *Obj;
} CAM_STATE_NOTIFY;


typedef struct _CAM_THRU_PUT{
    MUINT32 DMX_W;
    MUINT32 DMX_H;
    MBOOL   bBin;
}T_CAM_THRU_PUT;
typedef vector<T_CAM_THRU_PUT>  V_CAM_THRU_PUT;

#endif
