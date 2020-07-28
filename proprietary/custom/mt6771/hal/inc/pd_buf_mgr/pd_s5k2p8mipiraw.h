#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "pd_buf_mgr.h"

class PD_S5K2P8MIPIRAW : protected PDBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    MUINT32  m_PDBufSz;
    MUINT16 *m_PDBuf;
    MUINT32  m_PDXSz; //pixels in two byte.
    MUINT32  m_PDYSz; //lines

protected :
    /**
    * @brief checking current sensor is supported or not.
    */
    MBOOL IsSupport( SPDProfile_t &iPdProfile);
    /**
    * @brief seprate LR PD
    */
    void separate( int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, unsigned short *ptrLROut, unsigned int bitDepth);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PD_S5K2P8MIPIRAW();
    ~PD_S5K2P8MIPIRAW();

    static PDBufMgr* getInstance();



    /**
    * @brief convert PD data buffer format.
    */
    MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth=12, PD_AREA_T *ptrPDRegion=NULL);
    /**
    * @brief get m_PDXSz, m_PDYSz, and m_PDBufSz.
    */
    MBOOL GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz);

};

