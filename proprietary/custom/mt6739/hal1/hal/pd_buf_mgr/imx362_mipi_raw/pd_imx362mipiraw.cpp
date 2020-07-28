#define LOG_TAG "pd_buf_mgr_imx362mipiraw"
#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_imx362mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "pd_imx362setting.h"


PDBufMgr*
PD_IMX362MIPIRAW::getInstance()
{
    static PD_IMX362MIPIRAW singleton;
    return &singleton;
}

PD_IMX362MIPIRAW::PD_IMX362MIPIRAW()
{
    AAA_LOGD("IMX362");
    m_PDBufSz = 0;
    m_PDBuf   = NULL;
    m_PDXSz   = 0;
    m_PDYSz   = 0;}

PD_IMX362MIPIRAW::~PD_IMX362MIPIRAW()
{

}

MBOOL PD_IMX362MIPIRAW::IsSupport( SPDProfile_t& /*iPdProfile*/)
{
    MBOOL ret = MFALSE;

    ret = MTRUE;

    return ret;
}

MUINT16* PD_IMX362MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 /*i4Stride*/, MUINT8* /*ptrBufAddr*/, MBOOL /*b14BitData*/)
{
    return NULL;
}


MBOOL PD_IMX362MIPIRAW::GetLRBufferInfo( MINT32 /*i4CurSensorMode*/, SPDOHWINFO_T& /*oPDOhwInfo*/)
{
    return MTRUE;
}

MBOOL PD_IMX362MIPIRAW::GetDualPDVCInfo( MINT32 /*i4CurSensorMode*/, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode)
{
    if (i4AETargetMode == 3) //AE_MODE_ZVHDR_TARGET
    {
        oDualPDVChwInfo.u4VCBinningX = dualPDVCSetting[DualPD_VCBuf_zHDR].i4BinningX;
        oDualPDVChwInfo.u4VCBinningY = dualPDVCSetting[DualPD_VCBuf_zHDR].i4BinningY;
        oDualPDVChwInfo.u4VCBufFmt   = dualPDVCSetting[DualPD_VCBuf_zHDR].i4BufFmt;
        return MTRUE;
    }
    else if (i4AETargetMode == 0) //AE_MODE_NORMAL
    {
        oDualPDVChwInfo.u4VCBinningX = dualPDVCSetting[DualPD_VCBuf_Normal].i4BinningX;
        oDualPDVChwInfo.u4VCBinningY = dualPDVCSetting[DualPD_VCBuf_Normal].i4BinningY;
        oDualPDVChwInfo.u4VCBufFmt   = dualPDVCSetting[DualPD_VCBuf_Normal].i4BufFmt;
        return MTRUE;
    }
    return MFALSE;
}

