#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "SonyPdafTransCoord.h"
#include <pd_imx230mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "string.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx230mipiraw"

PDBufMgrOpen*
PD_IMX230MIPIRAW::getInstance()
{
    static PD_IMX230MIPIRAW singleton;
    return &singleton;
}


PD_IMX230MIPIRAW::PD_IMX230MIPIRAW()
{
    pLibWrapper = new SonyIMX230PdafLibraryWrapper;
}

PD_IMX230MIPIRAW::~PD_IMX230MIPIRAW()
{
    if( pLibWrapper)
        delete pLibWrapper;

    pLibWrapper = NULL;
}


MBOOL PD_IMX230MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->IsSupport( iPdProfile);
    }
    return ret;
}

MINT32 PD_IMX230MIPIRAW::PD_IMX230MIPIRAW::GetPDCalSz()
{
    return 96;
}


MBOOL PD_IMX230MIPIRAW::GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry)
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetPDInfo2HybridAF( i4InArySz, i4OutAry);
    }
    return ret;
}


MBOOL PD_IMX230MIPIRAW::ExtractPDCL()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->ExtractPDCL( m_databuf_size, m_databuf);
    }
    return ret;
}


MBOOL PD_IMX230MIPIRAW::ExtractCaliData()
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->ExtractCaliData( m_calidatabuf_size, m_calidatabuf);
    }
    return ret;
}



MRESULT PD_IMX230MIPIRAW::GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetVersionOfPdafLibrary( tOutSWVer);
    }
    return ret;
}

MBOOL PD_IMX230MIPIRAW::GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData)
{
    MBOOL ret = MFALSE;

    if( pLibWrapper)
    {
        ret = pLibWrapper->GetDefocus( iPDInputData, oPdOutputData);
    }
    return ret;
}
