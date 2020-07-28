#include <utils/Log.h>
#include <utils/Errors.h>
#include <math.h>
#include <string.h>
#include "kd_imgsensor.h"
#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_log.h>

#define LOG_TAG "pd_buf_mgr_open"

#include <pd_buf_mgr_open.h>
#include <pd_imx230mipiraw.h>

PDBufMgrOpen::PDBufMgrOpen() :
    m_databuf(NULL),
    m_databuf_size(0),
    m_phase_difference(NULL),
    m_confidence_level(NULL),
    m_calidatabuf(NULL),
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



    default :
        instance = NULL;
        break;
    }

    if( instance)
        ret = instance->IsSupport(iPdProfile) ? instance : NULL;

    MY_LOG("[PD] [SensorId]0x%04x, [%x]", iPdProfile.i4CurrSensorId, instance);


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
        MY_LOG("m_databuf_size!=i4Size\n");
        delete m_databuf;
        m_databuf = new MUINT8 [i4Size];
        m_databuf_size = i4Size;
    }

    m_frm_num = i4FrmCnt;

    memcpy( m_databuf, ptrBuf, i4Size);


    char value[200] = {'\0'};
    property_get("vendor.vc.dump.enable", value, "0");
    MBOOL bEnable = atoi(value);
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
    static MINT32 frameCnt = 0;


    if( m_calidatabuf==NULL)
    {
        m_calidatabuf = new MUINT8 [i4Size];
        m_calidatabuf_size = i4Size;
    }

    if( m_calidatabuf_size!=i4Size)
    {
        MY_LOG("m_calidatabuf_size!=i4Size\n");
        delete m_calidatabuf;
        m_calidatabuf = new MUINT8 [i4Size];
        m_calidatabuf_size = i4Size;
    }

    MY_LOG("SetCalibrationData\n");
    memcpy( m_calidatabuf, ptrcaldata, i4Size);


    char value[200] = {'\0'};
    property_get("vendor.vc.dump.enable", value, "0");
    MBOOL bEnable = atoi(value);


    if (bEnable)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/vc/%d_calipuf.raw", frameCnt++);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            return MFALSE;
        }
        fwrite(reinterpret_cast<void *>(m_calidatabuf), 1, i4Size, fp);
        fclose(fp);
    }
    else
    {
        frameCnt = 0;
    }

    return ExtractCaliData();


}



