#include <utils/Log.h>
#include <utils/Errors.h>
#include <math.h>
#include "kd_imgsensor.h"
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <aaa_log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_open"

#include <pd_buf_mgr_open.h>
#include <pd_imx230mipiraw.h>
#include <pd_imx386mipiraw.h>
#include <pd_imx338mipiraw.h>
#include <pd_imx519mipiraw.h>

PDBufMgrOpen::PDBufMgrOpen() :
    m_databuf(NULL),
    m_databuf_size(0),
    m_calidatabuf(NULL),
    m_phase_difference(NULL),
    m_confidence_level(NULL),
    m_frm_num(0)
{

}

PDBufMgrOpen::~PDBufMgrOpen()
{
    if( m_databuf)
        delete m_databuf;

    if( m_calidatabuf )
        delete m_calidatabuf;

    m_databuf=NULL;
    m_calidatabuf=NULL;
    m_frm_num=0;
}



PDBufMgrOpen*
PDBufMgrOpen::createInstance(SPDProfile_t &iPdProfile)
{

    PDBufMgrOpen *instance = NULL;
    PDBufMgrOpen *ret      = NULL;

    switch( iPdProfile.i4CurrSensorId)
    {

#if defined(IMX230_MIPI_RAW)
    case IMX230_SENSOR_ID :
        instance = PD_IMX230MIPIRAW::getInstance();
        break;
#endif

#if defined(IMX386_MIPI_RAW)
    case IMX386_SENSOR_ID :
        instance = PD_IMX386MIPIRAW::getInstance();
        break;
#endif
#if defined(IMX338_MIPI_RAW)
    case IMX338_SENSOR_ID :
        instance = PD_IMX338MIPIRAW::getInstance();
        break;
#endif
#if defined(IMX519_MIPI_RAW)
    case IMX519_SENSOR_ID :
        instance = PD_IMX519MIPIRAW::getInstance();
        break;
#endif
    default :
        instance = NULL;
        break;
    }

    if( instance)
        ret = instance->IsSupport(iPdProfile) ? instance : NULL;

    AAA_LOGD("[PD] [SensorId]0x%04x, [%p]", iPdProfile.i4CurrSensorId, instance);
    return ret;
}


MBOOL PDBufMgrOpen::SetDataBuf( MUINT32  i4Size, MUINT8 *ptrBuf, MUINT32 &i4FrmCnt)
{

    if( m_databuf==NULL)
    {
        m_databuf = new MUINT8 [i4Size];
        m_databuf_size = i4Size;
    }

    if( m_databuf_size!=i4Size)
    {
        AAA_LOGD("m_databuf_size!=i4Size\n");
        delete m_databuf;
        m_databuf = new MUINT8 [i4Size];
        m_databuf_size = i4Size;
    }

    m_frm_num = i4FrmCnt;

    memcpy( m_databuf, ptrBuf, i4Size);

    MBOOL bEnable = property_get_int32("vendor.vc.dump.enable", 0);
    if (bEnable)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/vc/%d_databuf.raw", m_frm_num);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            return MFALSE;
        }
        fwrite(reinterpret_cast<void *>(m_databuf), 1, i4Size, fp);
        fclose(fp);
    }

    return ExtractPDCL();
}



MBOOL PDBufMgrOpen::SetCalibrationData( MUINT32  i4Size, MUINT8 *ptrcaldata)
{
    if( m_calidatabuf==NULL)
    {
        m_calidatabuf = new MUINT8 [i4Size];
        m_calidatabuf_size = i4Size;
    }

    if( m_calidatabuf_size!=i4Size)
    {
        AAA_LOGD("m_calidatabuf_size!=i4Size\n");
        delete m_calidatabuf;
        m_calidatabuf = new MUINT8 [i4Size];
        m_calidatabuf_size = i4Size;
    }

    AAA_LOGD("SetCalibrationData");
    memcpy( m_calidatabuf, ptrcaldata, i4Size);

    return ExtractCaliData();
}

MBOOL PDBufMgrOpen::GetPDOHWInfo( MINT32 /*i4CurSensorMode*/, SPDOHWINFO_T &oPDOhwInfo)
{
    oPDOhwInfo.phyAddrBpci_tbl = (MUINTPTR)NULL;
    oPDOhwInfo.virAddrBpci_tbl = (MUINTPTR)NULL;
    oPDOhwInfo.u4Bpci_xsz      = 0;
    oPDOhwInfo.u4Bpci_ysz      = 0;
    oPDOhwInfo.u4Pdo_xsz       = 0;
    oPDOhwInfo.u4Pdo_ysz       = 0;
    return MFALSE;
}

MBOOL PDBufMgrOpen::sendCommand( MUINT32  /*i4Cmd*/, MVOID* /*arg1*/, MVOID* /*arg2*/, MVOID* /*arg3*/)
{
    return MFALSE;
}
