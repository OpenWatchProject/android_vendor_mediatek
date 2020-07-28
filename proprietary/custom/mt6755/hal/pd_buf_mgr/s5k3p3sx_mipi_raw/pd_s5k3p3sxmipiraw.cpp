#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k3p3sxmipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#define LOG_TAG "pd_buf_mgr_s5k3p3sxmipiraw"
 

PDBufMgr* 
PD_S5K3P3SXMIPIRAW::getInstance()
{
    static PD_S5K3P3SXMIPIRAW singleton;
    return &singleton;

}


PD_S5K3P3SXMIPIRAW::PD_S5K3P3SXMIPIRAW()
{ 	
	MY_LOG("[PD Mgr] S5K3P3\n");
	m_PDBufSz = 0;
	m_PDBuf = NULL;
}

PD_S5K3P3SXMIPIRAW::~PD_S5K3P3SXMIPIRAW()
{
	if( m_PDBuf)
		delete m_PDBuf;

	m_PDBufSz = 0;
	m_PDBuf = NULL;
}


MBOOL PD_S5K3P3SXMIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
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


MINT32 PD_S5K3P3SXMIPIRAW::GetPDCalSz()
{
	return 0x57c;
}

MUINT16* PD_S5K3P3SXMIPIRAW::ConvertPDBufFormat( MUINT32  i4Size, MUINT8 *ptrBufAddr, MUINT32 i4FrmCnt)
{
	//s5k3p3 is EPDBuf_Raw type, no need convert PD buffer format.
	return NULL;
}
