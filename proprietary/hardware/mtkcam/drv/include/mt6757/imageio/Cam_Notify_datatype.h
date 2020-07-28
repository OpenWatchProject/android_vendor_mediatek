#ifndef _CAM_NOTIFY_DATATYPE_H_
#define _CAM_NOTIFY_DATATYPE_H_

////////////////////////////
//EIS structrue
///////////////////////////
typedef struct _EIS_REG_CFG{
    MBOOL   bEIS_Bypass;
    struct{
        MUINT32 CTRL_1;
        MUINT32 CTRL_2;
        MUINT32 LMV_TH;
        MUINT32 FL_ofs;
        MUINT32 MB_ofs;
        MUINT32 MB_int;
        MUINT32 GMV;
        MUINT32 ERR_CTRL;
        MUINT32 IMG_CTRL;
    }_EIS_REG;              //oreder within this sturcutre can't change, and this structure can't add any new element, because of cmd passing
    MBOOL   bEIS_EN;
}EIS_REG_CFG;

typedef struct _SGG2_REG_CFG{
    struct{
        MUINT32 PGN;
        MUINT32 GMRC_1;
        MUINT32 GMRC_2;
    }_SGG2_REG;
    MBOOL   bSGG2_Bypass;
    MBOOL   bSGG2_EN;
}SGG2_REG_CFG;

typedef struct _RSS_REG_CFG{
    struct{
        MUINT32 CTRL_1;
        MUINT32 IN_IMG;
        MUINT32 OUT_IMG;
        MUINT32 HORI_STEP;
        MUINT32 VERT_STEP;
        MUINT32 HORI_INT_OFST;
        MUINT32 HORI_SUB_OFST;
        MUINT32 VERT_INT_OFST;
        MUINT32 VERT_SUB_OFST;
    }_RSS_REG;              //oreder within this sturcutre can't change, and this structure can't add any new element, because of cmd passing
    MBOOL   bRSS_Bypass;
    MBOOL   bRSS_EN;
}RSS_REG_CFG;

typedef struct _EIS_SIZE{
    MUINT32 w;
    MUINT32 h;
}EIS_SIZE;
typedef struct _EIS_INPUT_INFO{
    EIS_SIZE    sRMXOut;
    EIS_SIZE    sHBINOut;   //HDS
    EIS_SIZE    sTGOut;
    MBOOL       bYUVFmt;

    MUINT32     pixMode;

    struct{
        MUINT32 start_x;
        MUINT32 start_y;
        MUINT32 crop_size_w;
        MUINT32 crop_size_h;

        MUINT32 in_size_w;
        MUINT32 in_size_h;
    }RRZ_IN_CROP;
}EIS_INPUT_INFO;

typedef struct _LCS_INPUT_INFO{
    EIS_SIZE    sRRZOut;
    EIS_SIZE    sHBINOut;   //QBIN2
    EIS_SIZE    sTGOut;

    MBOOL       bIsHbin;
    MUINT32     magic;

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
    CAM_RELEASE_IDLE
} CAM_STATE_OP;

typedef MUINT32 (*FP_STATE_NOTIFY)(CAM_STATE_OP state, MVOID *Obj);

typedef struct _CAM_STATE_NOTIFY {
    FP_STATE_NOTIFY fpNotifyState;
    MVOID           *Obj;
} CAM_STATE_NOTIFY;

#endif
