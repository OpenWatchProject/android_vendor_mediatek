#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_imx386mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "string.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx386mipiraw"

// Define PDAF supporting sensor mode.
typedef enum
{
    MODE_NO_PDAF =0,
    MODE_FULL,
    MODE_BINNING,

} IMX386_PDAF_SUPPORT_MODE_T;


MUINT32 PD_IMX386MIPIRAW::m_u4Reg_AdDaPair_Sz = (sizeof(MUINT16)*82/sizeof(MUINT8));

MUINT16 PD_IMX386MIPIRAW::m_u2Reg_AdDaPair[82] =
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
    //
    0x3100, 0x0000,
    0x3101, 0x0000,
    0x3102, 0x0000,
    0x3103, 0x0000,
    0x3104, 0x0000,
    0x3105, 0x0000,
    0x3106, 0x0000,
    0x3107, 0x0000
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDBufMgrOpen*
PD_IMX386MIPIRAW::
getInstance()
{
    static PD_IMX386MIPIRAW singleton;
    return &singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX386MIPIRAW::
PD_IMX386MIPIRAW()
{
    pLibWrapper    = new SonyIMX386PdafLibraryWrapper;
    m_bDebugEnable = MFALSE;
    m_CurrMode     = MODE_NO_PDAF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PD_IMX386MIPIRAW::
~PD_IMX386MIPIRAW()
{
    if( pLibWrapper)
        delete pLibWrapper;

    pLibWrapper = NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX386MIPIRAW::
IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    // enable/disable debug log
    m_bDebugEnable = property_get_int32("vendor.debug.af_mgr.enable", 0);

    // binning ( V:1/2, H:1/2) mode and all-pixel mode.
    if( iPdProfile.uImgXsz==4032 && iPdProfile.uImgYsz==3016)
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
PD_IMX386MIPIRAW::
GetPDCalSz()
{
    // output number of calibration data in byte
    return (D_IMX386_DCC_IN_EEPROM_BLK_NUM_W*D_IMX386_DCC_IN_EEPROM_BLK_NUM_H*sizeof(signed short));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX386MIPIRAW::
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
            i4OutAry[0] = 4032/D_IMX386_PD_BLK_NUM_W;
            i4OutAry[1] = 3016/D_IMX386_PD_BLK_NUM_H;
            break;
        case MODE_BINNING :
            // bining mode
            i4OutAry[0] = 2016/D_IMX386_PD_BLK_NUM_W;
            i4OutAry[1] = 1508/D_IMX386_PD_BLK_NUM_H;
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
PD_IMX386MIPIRAW::
ExtractPDCL()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->SetPDCLData( m_databuf_size, m_databuf);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX386MIPIRAW::
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
PD_IMX386MIPIRAW::
GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetVersionOfPdafLibrary( tOutSWVer);

        if( tOutSWVer.MajorVersion==0 && tOutSWVer.MinorVersion==0)
        {
            AAA_LOGE("[%s] Please lincess IMX386 pdaf library from Sony FAE!!!", __FUNCTION__);
        }
        else
        {
            AAA_LOGD("[%s] IMX386 Sony pdaf library version %lu.%lu\n", __FUNCTION__, tOutSWVer.MajorVersion, tOutSWVer.MinorVersion);
        }
    }
    else
    {
        AAA_LOGE("[%s] Please do IMX386 pdaf porting!\n", __FUNCTION__);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PD_IMX386MIPIRAW::
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
PD_IMX386MIPIRAW::
sendCommand( MUINT32 i4Cmd, MVOID *arg1, MVOID *arg2, MVOID * /*arg3*/)
{
    MBOOL ret = MFALSE;
    switch( i4Cmd)
    {
    case PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST :
    *((MUINT32 *)arg1)  = m_u4Reg_AdDaPair_Sz;
    *((MUINT16 **)arg2) = &(m_u2Reg_AdDaPair[0]);
        ret = MTRUE;
        break;

    default:
        AAA_LOGD("[%s] Not valid command (%d)", __FUNCTION__, i4Cmd);
        break;
    }
    return ret;
}
