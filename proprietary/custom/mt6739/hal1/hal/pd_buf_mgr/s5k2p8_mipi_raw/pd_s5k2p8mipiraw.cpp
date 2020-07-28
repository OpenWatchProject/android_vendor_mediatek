#include <log/log.h>
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


void PD_S5K2P8MIPIRAW::separate( int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, unsigned short *ptrLROut, int iShift)
{
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
            *(pR[0]++) = *(row[0]++)>>iShift;
            *(pL[0]++) = *(row[1]++)>>iShift;
            *(pR[0]++) = *(row[1]++)>>iShift;
            *(pL[0]++) = *(row[2]++)>>iShift;
            *(pR[0]++) = *(row[1]++)>>iShift;
            *(pL[0]++) = *(row[2]++)>>iShift;
            *(pR[0]++) = *(row[0]++)>>iShift;
            *(pL[0]++) = *(row[1]++)>>iShift;

            *(pL[1]++) = *(row[3]++)>>iShift;
            *(pR[1]++) = *(row[4]++)>>iShift;
            *(pL[1]++) = *(row[4]++)>>iShift;
            *(pR[1]++) = *(row[5]++)>>iShift;
            *(pL[1]++) = *(row[4]++)>>iShift;
            *(pR[1]++) = *(row[5]++)>>iShift;
            *(pL[1]++) = *(row[3]++)>>iShift;
            *(pR[1]++) = *(row[4]++)>>iShift;

            *(pR[2]++) = *(row[7]++)>>iShift;
            *(pL[2]++) = *(row[8]++)>>iShift;
            *(pR[2]++) = *(row[6]++)>>iShift;
            *(pL[2]++) = *(row[7]++)>>iShift;
            *(pR[2]++) = *(row[6]++)>>iShift;
            *(pL[2]++) = *(row[7]++)>>iShift;
            *(pR[2]++) = *(row[7]++)>>iShift;
            *(pL[2]++) = *(row[8]++)>>iShift;

            *(pL[3]++) = *(row[10]++)>>iShift;
            *(pR[3]++) = *(row[11]++)>>iShift;
            *(pL[3]++) = *(row[ 9]++)>>iShift;
            *(pR[3]++) = *(row[10]++)>>iShift;
            *(pL[3]++) = *(row[ 9]++)>>iShift;
            *(pR[3]++) = *(row[10]++)>>iShift;
            *(pL[3]++) = *(row[10]++)>>iShift;
            *(pR[3]++) = *(row[11]++)>>iShift;
        }
    }
}


MUINT16* PD_S5K2P8MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData)
{
    //first in allocate local PD buffer directly.
    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDXSz   = (m_bpciInfo.pdo_xsize+1)/2;
        m_PDYSz   = (m_bpciInfo.pdo_ysize+1)*2/3;
        m_PDBufSz = m_PDXSz*m_PDYSz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    separate(i4Stride, ptrBufAddr, m_PDXSz, m_PDYSz, m_PDBuf, (b14BitData ? 4 : 2));

    return m_PDBuf;
}


MBOOL PD_S5K2P8MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDXSz;
    PDYsz = m_PDYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

