#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k3m2mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#define LOG_TAG "pd_buf_mgr_s5k3m2mipiraw"

PDBufMgr*
PD_S5K3M2MIPIRAW::getInstance()
{
    static PD_S5K3M2MIPIRAW singleton;
    return &singleton;

}


PD_S5K3M2MIPIRAW::PD_S5K3M2MIPIRAW()
{
    MY_LOG("[PD Mgr] S5K3M2\n");
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
        MY_LOG("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}


MINT32 PD_S5K3M2MIPIRAW::GetPDCalSz()
{
    return 0x57c;
}

MUINT16* PD_S5K3M2MIPIRAW::ConvertPDBufFormat( MUINT32  i4Size, MUINT8 *ptrBufAddr, MUINT32 i4FrmCnt)
{
    //s5k3m2 is EPDBuf_Raw type, no need convert PD buffer format.
    return NULL;
}

