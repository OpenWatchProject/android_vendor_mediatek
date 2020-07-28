#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k2l7mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "pd_s5k2l7setting.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k2l7mipiraw"

PDBufMgr*
PD_S5K2L7MIPIRAW::getInstance()
{
    static PD_S5K2L7MIPIRAW singleton;
    return &singleton;
}

PD_S5K2L7MIPIRAW::PD_S5K2L7MIPIRAW()
{
    AAA_LOGD("S5K2L7");
}

PD_S5K2L7MIPIRAW::~PD_S5K2L7MIPIRAW()
{

}

MBOOL PD_S5K2L7MIPIRAW::IsSupport( SPDProfile_t& /*iPdProfile*/)
{
    MBOOL ret = MFALSE;

    ret = MTRUE;

    return ret;
}


MUINT16* PD_S5K2L7MIPIRAW::ConvertPDBufFormat( MUINT32 /* i4Size */, MUINT32 /* i4Stride */, MUINT8* /* ptrBufAddr */, MUINT32 /* u4BitDepth */, PD_AREA_T* /*ptrPDRegion*/)
{
    return NULL;
}

MBOOL PD_S5K2L7MIPIRAW::GetDualPDVCInfo( MINT32 /*i4CurSensorMode*/, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode)
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

