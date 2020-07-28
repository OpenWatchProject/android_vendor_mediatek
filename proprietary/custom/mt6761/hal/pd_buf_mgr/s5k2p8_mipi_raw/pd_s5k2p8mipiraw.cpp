#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k2p8mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k2p8mipiraw"

PDBufMgr*
PD_S5K2P8MIPIRAW::getInstance()
{
    static PD_S5K2P8MIPIRAW singleton;
    return &singleton;

}


PD_S5K2P8MIPIRAW::PD_S5K2P8MIPIRAW()
{
    AAA_LOGD("[PD Mgr] S5K2P8\n");
    m_PDBufSz = 0;
    m_PDBuf   = NULL;
    m_PDXSz   = 0;
    m_PDYSz   = 0;
}

PD_S5K2P8MIPIRAW::~PD_S5K2P8MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_S5K2P8MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
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


void PD_S5K2P8MIPIRAW::separate( int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, unsigned short *ptrLROut, unsigned int bitDepth)
{
    unsigned int shift_bits = 0;// converting data to 10 bits

    if( (10<=bitDepth) && (bitDepth<=16))
    {
        shift_bits = bitDepth - 10;
    }
    else
    {
        AAA_LOGE("%s abort : wrong input bitDepth(%d)", __FUNCTION__, bitDepth);
        return;
    }


    int LRoffset = pd_x_num*pd_y_num/2;
    for (int y=0; y<pd_y_num; y+=8)
    {
        unsigned short *pL[4];
        unsigned short *pR[4];
        unsigned short* row[12];

        for (int ri=0; ri<4; ri++)
        {
            pR[ri] = ptrLROut + (y/2 + ri)*pd_x_num;
            pL[ri] = pR[ri] + LRoffset;
        }
        for (int i=0; i<12; i++)
            row[i] = (unsigned short*)ptr + i*stride/2;
        ptr += 12 * stride;

        for (int x=0; x<pd_x_num; x+=4)
        {
            *(pR[0]++) = *(row[0]++)>>shift_bits;
            *(pL[0]++) = *(row[1]++)>>shift_bits;
            *(pR[0]++) = *(row[1]++)>>shift_bits;
            *(pL[0]++) = *(row[2]++)>>shift_bits;
            *(pR[0]++) = *(row[1]++)>>shift_bits;
            *(pL[0]++) = *(row[2]++)>>shift_bits;
            *(pR[0]++) = *(row[0]++)>>shift_bits;
            *(pL[0]++) = *(row[1]++)>>shift_bits;

            *(pL[1]++) = *(row[3]++)>>shift_bits;
            *(pR[1]++) = *(row[4]++)>>shift_bits;
            *(pL[1]++) = *(row[4]++)>>shift_bits;
            *(pR[1]++) = *(row[5]++)>>shift_bits;
            *(pL[1]++) = *(row[4]++)>>shift_bits;
            *(pR[1]++) = *(row[5]++)>>shift_bits;
            *(pL[1]++) = *(row[3]++)>>shift_bits;
            *(pR[1]++) = *(row[4]++)>>shift_bits;

            *(pR[2]++) = *(row[7]++)>>shift_bits;
            *(pL[2]++) = *(row[8]++)>>shift_bits;
            *(pR[2]++) = *(row[6]++)>>shift_bits;
            *(pL[2]++) = *(row[7]++)>>shift_bits;
            *(pR[2]++) = *(row[6]++)>>shift_bits;
            *(pL[2]++) = *(row[7]++)>>shift_bits;
            *(pR[2]++) = *(row[7]++)>>shift_bits;
            *(pL[2]++) = *(row[8]++)>>shift_bits;

            *(pL[3]++) = *(row[10]++)>>shift_bits;
            *(pR[3]++) = *(row[11]++)>>shift_bits;
            *(pL[3]++) = *(row[ 9]++)>>shift_bits;
            *(pR[3]++) = *(row[10]++)>>shift_bits;
            *(pL[3]++) = *(row[ 9]++)>>shift_bits;
            *(pR[3]++) = *(row[10]++)>>shift_bits;
            *(pL[3]++) = *(row[10]++)>>shift_bits;
            *(pR[3]++) = *(row[11]++)>>shift_bits;
        }
    }
}


MUINT16* PD_S5K2P8MIPIRAW::ConvertPDBufFormat( MUINT32 /* i4Size */, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T* /*ptrPDRegion*/)
{
    //first in allocate local PD buffer directly.
    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDXSz   = (m_sPDOHWInfo.u4Pdo_xsz+1)/2;
        m_PDYSz   = (m_sPDOHWInfo.u4Pdo_ysz+1)*2/3;
        m_PDBufSz = m_PDXSz*m_PDYSz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    separate(i4Stride, ptrBufAddr, m_PDXSz, m_PDYSz, m_PDBuf, u4BitDepth);

    return m_PDBuf;
}


MBOOL PD_S5K2P8MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDXSz;
    PDYsz = m_PDYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

