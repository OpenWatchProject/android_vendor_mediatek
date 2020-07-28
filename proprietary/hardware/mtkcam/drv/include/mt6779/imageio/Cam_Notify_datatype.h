#ifndef _CAM_NOTIFY_DATATYPE_H_
#define _CAM_NOTIFY_DATATYPE_H_

////////////////////////////
//Structure
///////////////////////////
#include <vector>
using namespace std;


typedef struct _LMV_SIZE{
    MUINT32 w;
    MUINT32 h;
}LMV_SIZE;

typedef struct _LCES_INPUT_INFO{
    LMV_SIZE    sRRZOut;
    LMV_SIZE    sHBINOut;   //QBIN2
    LMV_SIZE    sTGOut;

    MBOOL       bIsHbin;
    MUINT32     magic;
    MBOOL       bIsDbin;
    MBOOL       bIsbin;
    MBOOL       bIsVbn;  //LafiteEP_TODO: need remove
    MBOOL       bIsfbnd;
    MUINT32     bQbinRatio;

    struct{
        MUINT32 start_x;
        MUINT32 start_y;
        MUINT32 crop_size_w;
        MUINT32 crop_size_h;

        MUINT32 in_size_w;
        MUINT32 in_size_h;
    }RRZ_IN_CROP;
}LCES_INPUT_INFO;

typedef struct _LCES_REG_CFG{
    MBOOL   bLCES_EN;
    MBOOL   bLCES_Bypass;    // 1 if no need to update LCS or LCS_D reg
    MUINT32 u4LCESO_Stride;
    struct{
        MUINT32 LCES_IN_SIZE;
        MUINT32 LCES_OUT_SIZE;
        MUINT32 LCES_LRZRX;
        MUINT32 LCES_LRZRY;
    }_LCES_REG;
}LCES_REG_CFG;

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
    CAM_EXIT_ENQ,
    CAM_ENTER_SWITCH,
    CAM_EXIT_SWITCH,
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
    MUINT32 SEP_W;
    MUINT32 SEP_H;
    MBOOL   bBin;
}T_CAM_THRU_PUT;
typedef vector<T_CAM_THRU_PUT>  V_CAM_THRU_PUT;

#endif
