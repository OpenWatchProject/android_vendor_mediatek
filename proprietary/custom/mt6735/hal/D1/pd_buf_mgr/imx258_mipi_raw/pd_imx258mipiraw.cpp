#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_imx258mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#define LOG_TAG "pd_buf_mgr_imx258mipiraw"

#define _IMX258_HDR_TYPE_ 2
#define _IMX258_BINNING_TYPE_ 3

/* trace */
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define PDBUF_TRACE_NAME_LENGTH                 64

#define PDBUF_TRACE_CALL()                      ATRACE_CALL()
#define PDBUF_TRACE_NAME(name)                  ATRACE_NAME(name)
#define PDBUF_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define PDBUF_TRACE_END()                       ATRACE_END()
#define PDBUF_TRACE_FMT_BEGIN(fmt, arg...)                  \
do{                                                       \
    if( ATRACE_ENABLED() )                                \
    {                                                     \
        char buf[AF_TRACE_NAME_LENGTH];                  \
        snprintf(buf, AF_TRACE_NAME_LENGTH, fmt, ##arg); \
        PDBUF_TRACE_BEGIN(buf);                             \
    }                                                     \
}while(0)
#define PDBUF_TRACE_FMT_END()                   PDBUF_TRACE_END()
#else
#define PD_TRACE_CALL()
#define PD_TRACE_NAME(name)
#define PD_TRACE_BEGIN(name)
#define PD_TRACE_END()
#define PD_TRACE_FMT_BEGIN(fmt, arg)
#define PD_TRACE_FMT_END()
#endif



/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX258.
*/

PDBufMgr*
PD_IMX258MIPIRAW::getInstance()
{
    static PD_IMX258MIPIRAW singleton;
    return &singleton;

}


PD_IMX258MIPIRAW::PD_IMX258MIPIRAW()
{
    MY_LOG("[PD Mgr] IMX258\n");
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}

PD_IMX258MIPIRAW::~PD_IMX258MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_IMX258MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);


    // getting sensor type.
    m_SensorType = iPdProfile.uSensorType;


    //all-pixel mode is supported.
    if( iPdProfile.uSensorType==_IMX258_HDR_TYPE_) //HDR type.
    {
        if( iPdProfile.u4IsZSD!=0)
        {
            ret = MTRUE;
        }
    }
    if( iPdProfile.uSensorType==_IMX258_BINNING_TYPE_) //Binning type.
    {
        if( iPdProfile.u4IsZSD!=0  || (iPdProfile.uImgXsz==2100 && iPdProfile.uImgYsz==1560))
        {
            ret = MTRUE;
        }
    }

    if( ret==MFALSE)
    {
        MY_LOG("PDAF Mode is not Supported (%d, %d), type %d\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz, m_SensorType);
    }


    return ret;

}

MINT32 PD_IMX258MIPIRAW::GetPDCalSz()
{
    return 0x54c;
}


MUINT16* PD_IMX258MIPIRAW::ConvertPDBufFormat( MUINT32  i4Size, MUINT8 *ptrBufAddr, MUINT32 i4FrmCnt)
{
    //Input :
    // i4Size        - DMA buffer size
    // ptrBufAddr - data buffer from dma
    // i4FrmCnt   - current frame number


    MY_LOG("[%s]\n", __FUNCTION__);


    MUINT16 *ptr2Bbuf = (MUINT16 *)ptrBufAddr;
    MINT32   sz2Bbuf  = i4Size/2; //because using 2byte pointer.


    //----------------------------------------------------------------------------------

    //decide width and height of pd datam
    MINT32   pdW  = 0;
    MINT32   pdH  = 0;
    MINT32   pdSz = 0;
    MINT32   cntW = 0;
    MINT32   cntH = 0;
    MBOOL    iswaitLS  = MTRUE; // waiting to meet the first element of one line

    PDBUF_TRACE_BEGIN("GetSZ");
    for( MINT32 i=0; i<sz2Bbuf; i++)
    {
        if( ptr2Bbuf[i]!=0)
        {
            cntW++;
            cntH += iswaitLS ? 1 : 0; //count how many lines by counting how many first element is got
            iswaitLS = false;
        }
        else
        {
            //latch how many elements are in one line
            if( iswaitLS==false)
            {
                if( pdW!=cntW && pdW!=0)
                    MY_LOG("w size is not the same %d %d!!\n", pdW, cntW);
                pdW = cntW;
            }

            //rest counter
            cntW = 0;
            iswaitLS = true;
        }
    }
    PDBUF_TRACE_END();

    pdH  = cntH;
    pdSz = pdW*pdH;

    MY_LOG_IF( m_bDebugEnable, "PD data size, W=%d H=%d, SZ=%d, bufsz %df\n", pdW, pdH, pdSz, sz2Bbuf);


    //extract pd data from dam buffer
    PDBUF_TRACE_BEGIN("ExtFromDMA");
    int k=0;
    MUINT16 *tmpbuf = new MUINT16 [pdSz];
    for( int i=0; i<sz2Bbuf; i++)
    {
        if(ptr2Bbuf[i]!=0)
        {
            tmpbuf[k] = ptr2Bbuf[i];
            k++;
        }
    }
    PDBUF_TRACE_END();

    if( k!=pdSz)
    {
        MY_LOG("Auto detect PD data size fail : PD data size %d %d\n", k, pdSz);
    }

    //----------------------------------------------------------------------------------
    //first in allocate local PD buffer directly.
    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDBufSz = pdSz;
        if( pdH==768) //Only binning type sensor outputs 260x768 PD data
        {
            m_PDBufSz = m_PDBufSz/2;
        }
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }
    else
    {
        //check PD buffer size.
        if(m_PDBufSz != (pdSz/((pdH==786)?2:1))) //Only binning type sensor outputs 260x768 PD data
        {
            m_PDBufSz = pdSz;
            if( pdH==768) //Only binning type sensor outputs 260x768 PD data
            {
                m_PDBufSz = m_PDBufSz/2;
            }

            if( m_PDBuf)
                delete m_PDBuf;

            m_PDBuf = NULL;
            m_PDBuf = new MUINT16 [m_PDBufSz];
        }


    }

    //separate L and R pd data
    PDBUF_TRACE_BEGIN("Separate");
    MUINT16 **ptr=NULL;
    MUINT16 *ptrL =  m_PDBuf;
    MUINT16 *ptrR = &m_PDBuf[m_PDBufSz/2];

    if( pdH == 768)
    {
        //For Binning type sensor do SW PD data binning
        for ( int i=0; i < pdH; i+=2 )
        {
            //LLRRRRLL
            if(i%8==0 || i%8==6)
                ptr = &ptrL;
            else
                ptr = &ptrR;


            for ( int j=0; j < pdW; j++ )
            {
                (*ptr)[j] = (tmpbuf[i*pdW+j]+tmpbuf[(i+1)*pdW+j])/2;
            }
            (*ptr) += pdW;
        }
    }
    else if(pdH == 384)
    {

        MY_LOG("pdBuf=0x%x, SZ=%d", m_PDBuf, m_PDBufSz);
        for ( int i=0; i < pdH; i++ )
        {
            //LRRL
            if(i%4==0 || i%4==3)
                ptr = &ptrL;
            else
                ptr = &ptrR;


            for ( int j=0; j < pdW; j++ )
            {
                (*ptr)[j] = tmpbuf[i*pdW+j];
            }
            (*ptr) += pdW;
        }
    }
    PDBUF_TRACE_END();


    delete[] tmpbuf;
    tmpbuf=NULL;



    //store debug information.
    char value[200] = {'\0'};
    property_get("vendor.vc.dump.enable", value, "0");
    MBOOL bEnable = atoi(value);
    if (bEnable)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/vc/%d_pd.raw", i4FrmCnt);
        FILE *fp = fopen(fileName, "w");
        if ( fp!=NULL)
        {
            fwrite(reinterpret_cast<void *>(m_PDBuf), 1, m_PDBufSz*2, fp);
            fclose(fp);
        }

        sprintf(fileName, "/sdcard/vc/%d_in.raw", i4FrmCnt);
        fp = fopen(fileName, "w");
        if (fp!=NULL)
        {
            fwrite(reinterpret_cast<void *>(ptrBufAddr), 1, i4Size, fp);
            fclose(fp);
        }
    }

    return m_PDBuf;

}

