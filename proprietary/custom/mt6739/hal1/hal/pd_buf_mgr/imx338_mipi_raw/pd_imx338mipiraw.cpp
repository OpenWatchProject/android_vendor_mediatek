#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_imx338mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx338mipiraw"

// Define PDAF supporting sensor mode.
typedef enum
{
    MODE_NO_PDAF =0,
    MODE_FULL,
    MODE_BINNING,

} IMX338_PDAF_SUPPORT_MODE_T;


MUINT32 PD_IMX338MIPIRAW::m_u4Reg_AdDaPair_Sz = (sizeof(MUINT16)*82/sizeof(MUINT8));

MUINT16 PD_IMX338MIPIRAW::m_u2Reg_AdDaPair[82] =
{
    0x0101, 0x0000,
    0x0220, 0x0000,
    0x0221, 0x0000,
    0x0344, 0x0000,
    0x0345, 0x0000,
    0x0346, 0x0000,
    0x0347, 0x0000,
    0x0348, 0x0000,
    0x0349, 0x0000,
    0x034A, 0x0000,
    0x034B, 0x0000,
    0x034C, 0x0000,
    0x034D, 0x0000,
    0x034E, 0x0000,
    0x034F, 0x0000,
    0x0381, 0x0000,
    0x0383, 0x0000,
    0x0385, 0x0000,
    0x0387, 0x0000,
    0x0401, 0x0000,
    0x0404, 0x0000,
    0x0405, 0x0000,
    0x0408, 0x0000,
    0x0409, 0x0000,
    0x040A, 0x0000,
    0x040B, 0x0000,
    0x040C, 0x0000,
    0x040D, 0x0000,
    0x040E, 0x0000,
    0x040F, 0x0000,
    0x0900, 0x0000,
    0x0901, 0x0000,
    0x300D, 0x0000,
    0x3150, 0x0000,
    0x3151, 0x0000,
    0x3152, 0x0000,
    0x3153, 0x0000,
    0x3154, 0x0000,
    0x3155, 0x0000,
    0x3156, 0x0000,
    0x3157, 0x0000
};

MUINT32 PD_IMX338MIPIRAW::m_u4Reg_AdDaPair_PDWin_Flexible_Sz = (sizeof(MUINT16)*146/sizeof(MUINT8));

MUINT16 PD_IMX338MIPIRAW::m_u2Reg_AdDaPair_PDWin_Flexible[146] =
{
    /*AREA MODE*/
    0x31B0, 0x02,
    /*Flexible area enable, 0:disable, 1:enable*/
    0x31b4, 0x00,
    0x31b5, 0x00,
    0x31b6, 0x00,
    0x31b7, 0x00,
    0x31b8, 0x00,
    0x31b9, 0x00,
    0x31ba, 0x00,
    0x31bb, 0x00,
    /*Area setting*/
    //Win 0
    0x3158, 0x00,
    0x3159, 0x00,
    0x315a, 0x00,
    0x315b, 0x00,
    0x315c, 0x00,
    0x315d, 0x00,
    0x315e, 0x00,
    0x315f, 0x00,
    //Win 1
    0x3160, 0x00,
    0x3161, 0x00,
    0x3162, 0x00,
    0x3163, 0x00,
    0x3164, 0x00,
    0x3165, 0x00,
    0x3166, 0x00,
    0x3167, 0x00,
    //Win 2
    0x3168, 0x00,
    0x3169, 0x00,
    0x316a, 0x00,
    0x316b, 0x00,
    0x316c, 0x00,
    0x316d, 0x00,
    0x316e, 0x00,
    0x316f, 0x00,
    //Win 3
    0x3170, 0x00,
    0x3171, 0x00,
    0x3172, 0x00,
    0x3173, 0x00,
    0x3174, 0x00,
    0x3175, 0x00,
    0x3176, 0x00,
    0x3177, 0x00,
    //Win 4
    0x3178, 0x00,
    0x3179, 0x00,
    0x317a, 0x00,
    0x317b, 0x00,
    0x317c, 0x00,
    0x317d, 0x00,
    0x317e, 0x00,
    0x317f, 0x00,
    //Win 5
    0x3180, 0x00,
    0x3181, 0x00,
    0x3182, 0x00,
    0x3183, 0x00,
    0x3184, 0x00,
    0x3185, 0x00,
    0x3186, 0x00,
    0x3187, 0x00,
    //Win 6
    0x3188, 0x00,
    0x3189, 0x00,
    0x318a, 0x00,
    0x318b, 0x00,
    0x318c, 0x00,
    0x318d, 0x00,
    0x318e, 0x00,
    0x318f, 0x00,
    //Win 7
    0x3190, 0x00,
    0x3191, 0x00,
    0x3192, 0x00,
    0x3193, 0x00,
    0x3194, 0x00,
    0x3195, 0x00,
    0x3196, 0x00,
    0x3197, 0x00,
};

// Reference in sensor kernel driver
MUINT32 PD_IMX338MIPIRAW::m_u4Reg_AdDaPair_PDWin_Fix_Sz = (sizeof(MUINT16)*34/sizeof(MUINT8));

MUINT16 PD_IMX338MIPIRAW::m_u2Reg_AdDaPair_PDWin_Fix[34] =
{
    /*AREA MODE, 0:Fixed area(8x6), 1:Fixed area(16x12), 2:Flexible area*/
    0x31B0, 0x01,
    /*Fixed area mode*/
    0x3150, 0x00,
    0x3151, 0x70,
    0x3152, 0x00,
    0x3153, 0x58,
    0x3154, 0x02,
    0x3155, 0x80,
    0x3156, 0x02,
    0x3157, 0x80,
    /*Disable flexible area*/
    0x31b4, 0x00,
    0x31b5, 0x00,
    0x31b6, 0x00,
    0x31b7, 0x00,
    0x31b8, 0x00,
    0x31b9, 0x00,
    0x31ba, 0x00,
    0x31bb, 0x00,
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDBufMgrOpen*
PD_IMX338MIPIRAW::
getInstance()
{
    static PD_IMX338MIPIRAW singleton;
    return &singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX338MIPIRAW::
PD_IMX338MIPIRAW()
{
    memset( &m_tPDProfile, 0 , sizeof(m_tPDProfile));
    pLibWrapper    = new SonyIMX338PdafLibraryWrapper;
    m_bDebugEnable = MFALSE;
    m_CurrMode     = MODE_NO_PDAF;
    m_WinMode      = 3;  // 0:Fixed area(16x12), 1:Fixed area(8x6), 2:Fiexible window, 3:Reserved.
    m_WinEn        = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX338MIPIRAW::
~PD_IMX338MIPIRAW()
{
    if( pLibWrapper)
        delete pLibWrapper;

    pLibWrapper = NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
GetPDWinSetting( MVOID* arg1, MVOID* arg2, MVOID* arg3)
{
    MBOOL ret = MFALSE;

    PDBUF_CFG_T *cfgRoi = (PDBUF_CFG_T *)arg1;

    AAA_LOGD_IF( m_bDebugEnable, "%s cfgNum(0x%x)+", __FUNCTION__, cfgRoi->u4cfgNum);

#ifdef USE_FLEXIBLE_WIN_CFG
    MUINT8 bit_idx = cfgRoi->u4cfgNum;
    MUINT8 roi_idx = 0;

    for( MUINT32 i=0; i<8; i++)
    {
        MINT32 i4XStart;
        MINT32 i4YStart;
        MINT32 i4XEnd;
        MINT32 i4YEnd;
        MUINT8 roi_en;


        if( bit_idx&0x1)
        {
            i4XStart = cfgRoi->vecROIs.at(roi_idx).i4XStart;
            i4YStart = cfgRoi->vecROIs.at(roi_idx).i4YStart;
            i4XEnd   = cfgRoi->vecROIs.at(roi_idx).i4XEnd;
            i4YEnd   = cfgRoi->vecROIs.at(roi_idx).i4YEnd;

            cfgRoi->vecROIs.at(roi_idx).i4Info = i; // !! MUST !! Output

            roi_en   = 1;

            roi_idx++;
        }
        else
        {
            i4XStart = 0;
            i4YStart = 0;
            i4XEnd   = 0;
            i4YEnd   = 0;
            roi_en   = 0;
        }

        MUINT8 xStart_H = (i4XStart>>8) & 0x1f;
        MUINT8 xStart_L = (i4XStart   ) & 0xff;

        MUINT8 yStart_H = (i4YStart>>8) & 0xf;
        MUINT8 yStart_L = (i4YStart   ) & 0xff;

        MUINT8 xEnd_H   = (i4XEnd>>8) & 0x1f;
        MUINT8 xEnd_L   = (i4XEnd   ) & 0xff;

        MUINT8 yEnd_H   = (i4YEnd>>8) & 0xf;
        MUINT8 yEnd_L   = (i4YEnd   ) & 0xff;

        MUINT32 offset = 0;

#define NEXT_REG_SETTING_OFFSET 2

#define FIRST_AREA_REG_SETTING_OFFSET 9*2
        offset = FIRST_AREA_REG_SETTING_OFFSET + 8*i*2 + 1; //offset to register setting

        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = xStart_H;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = xStart_L;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = yStart_H;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = yStart_L;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = xEnd_H;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = xEnd_L;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = yEnd_H;
        offset += NEXT_REG_SETTING_OFFSET;
        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = yEnd_L;
        offset += NEXT_REG_SETTING_OFFSET;


#define FIRST_AREAEN_REG_SETTING_OFFSET 1*2
        offset = FIRST_AREAEN_REG_SETTING_OFFSET + i*2 + 1; //offset to register setting

        m_u2Reg_AdDaPair_PDWin_Flexible[offset] = roi_en;
        offset += NEXT_REG_SETTING_OFFSET;

        bit_idx = bit_idx >> 1;
    }

    *((MUINT32 *)arg2)  = m_u4Reg_AdDaPair_PDWin_Flexible_Sz;
    *((MUINT16 **)arg3) = &(m_u2Reg_AdDaPair_PDWin_Flexible[0]);
    ret = MTRUE;
#else
    *((MUINT32 *)arg2)  = m_u4Reg_AdDaPair_PDWin_Fix_Sz;
    *((MUINT16 **)arg3) = &(m_u2Reg_AdDaPair_PDWin_Fix[0]);
#endif

    AAA_LOGD_IF( m_bDebugEnable, "%s cfgNum(0x%x)-", __FUNCTION__, cfgRoi->u4cfgNum);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //
    memcpy( &m_tPDProfile, &iPdProfile, sizeof(SPDProfile_t));

    // enable/disable debug log
    m_bDebugEnable = property_get_int32("vendor.debug.af_mgr.enable", 0);

    // binning ( V:1/2, H:1/2) mode and all-pixel mode.
    if( iPdProfile.uImgXsz==5344 && iPdProfile.uImgYsz==4016)
    {
        ret = MTRUE;
        m_CurrMode = MODE_FULL;
    }
    else
    {
        m_CurrMode = MODE_NO_PDAF;
    }

    AAA_LOGD("%s:%d, CurMode:%d, ImgSZ:(%d, %d)\n", __FUNCTION__, ret, m_CurrMode, iPdProfile.uImgXsz, iPdProfile.uImgYsz);

    if( pLibWrapper && (m_CurrMode!=MODE_NO_PDAF))
    {
        pLibWrapper->Init( iPdProfile);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
PD_IMX338MIPIRAW::
GetPDCalSz()
{
    // output number of calibration data in byte
    return (D_IMX338_DCC_IN_EEPROM_BLK_NUM_W*D_IMX338_DCC_IN_EEPROM_BLK_NUM_H*sizeof(signed short));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
GetPDInfo2HybridAF( MINT32  i4InArySz, MINT32 *i4OutAry )
{
    MBOOL ret = MTRUE;

    // rest
    memset( i4OutAry, 0, i4InArySz*sizeof(MINT32));

    // i4InArySz should be 10 which is defined by core PD algorithm.
    if( i4InArySz==10 && i4OutAry)
    {
        // output block size setting for Hybrid AF to crop PD alayzing area.
        switch( m_CurrMode)
        {
        case MODE_FULL :
            // all pixel mode
            i4OutAry[0] = 5344/D_IMX338_PD_BLK_NUM_W;
            i4OutAry[1] = 4016/D_IMX338_PD_BLK_NUM_H;
            break;
        case MODE_BINNING :
            // bining mode
            i4OutAry[0] = 2672/D_IMX338_PD_BLK_NUM_W;
            i4OutAry[1] = 2008/D_IMX338_PD_BLK_NUM_H;
            break;
        default :
            ret = MFALSE;
            AAA_LOGD("Current Sensor mode(%d) is not support PDAF\n", m_CurrMode);
            break;
        }
    }
    else
    {
        ret = MFALSE;
        AAA_LOGD("[%s] Fail, Sz=%d, Addr=%p\n", __FUNCTION__, i4InArySz, i4OutAry);
    }


    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
ExtractPDCL()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->SetPDCLData( m_databuf_size, m_databuf, m_WinMode, m_WinEn);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
ExtractCaliData()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->SetCaliData( m_calidatabuf_size, m_calidatabuf, m_u4Reg_AdDaPair_Sz, m_u2Reg_AdDaPair);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
PD_IMX338MIPIRAW::
GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetVersionOfPdafLibrary( tOutSWVer);

        if( tOutSWVer.MajorVersion==0 && tOutSWVer.MinorVersion==0)
        {
            AAA_LOGE("[%s] Please lincess IMX338 pdaf library from Sony FAE!!!", __FUNCTION__);
        }
        else
        {
            AAA_LOGD("[%s] IMX338 Sony pdaf library version %lu.%lu\n", __FUNCTION__, tOutSWVer.MajorVersion, tOutSWVer.MinorVersion);
        }
    }
    else
    {
        AAA_LOGE("[%s] Please do IMX338 pdaf porting!\n", __FUNCTION__);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData)
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetDefocus( iPDInputData, oPdOutputData);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX338MIPIRAW::
sendCommand( MUINT32 i4Cmd, MVOID *arg1, MVOID *arg2, MVOID *arg3)
{
    MBOOL ret = MFALSE;
    switch( i4Cmd)
    {
    /******************************************************
     *
     * The input argument's type is define in pd_puf_open.
     *
     *            !!! Please MUST follow it !!!
     *
     ******************************************************/
    case PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST :
    *((MUINT32 *)arg1)  = m_u4Reg_AdDaPair_Sz;
    *((MUINT16 **)arg2) = &(m_u2Reg_AdDaPair[0]);
        ret = MTRUE;
        break;

    case PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING :
        ret = GetPDWinSetting( arg1, arg2, arg3);
        break;

    case PDBUFMGR_OPEN_CMD_GET_PD_WIN_MODE_SETTING :
#ifdef USE_FLEXIBLE_WIN_CFG
        *((MINT32 *)arg1)  = 0x02;
#else
        *((MINT32 *)arg1)  = 0x01;
#endif
        ret = MTRUE;
        break;

    case PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO :
        *((MINT32 *)arg1)  = m_WinMode;
        *((MUINT32 *)arg2) = m_WinEn;
        ret = MTRUE;
        break;



    default:
        AAA_LOGD("[%s] Not valid command (%d)", __FUNCTION__, i4Cmd);
        break;
    }
    return ret;
}
