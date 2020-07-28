#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k3m2mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k3m2mipiraw"

PDBufMgr*
PD_S5K3M2MIPIRAW::getInstance()
{
    static PD_S5K3M2MIPIRAW singleton;
    return &singleton;

}


PD_S5K3M2MIPIRAW::PD_S5K3M2MIPIRAW()
{
    AAA_LOGD("[PD Mgr] S5K3M2\n");
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}

PD_S5K3M2MIPIRAW::~PD_S5K3M2MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_S5K3M2MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0)
    {
        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}

MUINT16* PD_S5K3M2MIPIRAW::ConvertPDBufFormat( MUINT32 /* i4Size */, MUINT32 /* i4Stride */, MUINT8* /* ptrBufAddr */, MUINT32 /* u4BitDepth */, PD_AREA_T* /*ptrPDRegion*/)
{
    //s5k3m2 is EPDBuf_Raw type, no need convert PD buffer format.
    return NULL;
}

