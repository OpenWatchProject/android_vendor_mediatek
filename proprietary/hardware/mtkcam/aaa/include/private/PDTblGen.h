#ifndef __PD_TBL_MGR_H__

#define __PD_TBL_MGR_H__

#include <string>
#include <vector>

namespace NSCam
{
class IImageBuffer;
}

using namespace std;
using namespace NSCam;

struct TblInfo
{
    unsigned int  memID;
    unsigned int  tbl_xsz;
    unsigned int  tbl_ysz;
    void*         tbl_pa;
    void*         tbl_va;
    unsigned int  pdo_xsz;
    unsigned int  pdo_ysz;
    unsigned int  img_xsz;
    unsigned int  img_ysz;

    TblInfo() :
        memID(0),
        tbl_xsz(0),
        tbl_ysz(0),
        tbl_pa(nullptr),
        tbl_va(nullptr),
        pdo_xsz(0),
        pdo_ysz(0),
        img_xsz(0),
        img_ysz(0) {}

    TblInfo( unsigned int  _memID,
             unsigned int  _tbl_xsz,
             unsigned int  _tbl_ysz,
             void*         _tbl_pa,
             void*         _tbl_va,
             unsigned int  _pdo_xsz,
             unsigned int  _pdo_ysz,
             unsigned int  _img_xsz,
             unsigned int  _img_ysz) :
        memID(_memID),
        tbl_xsz(_tbl_xsz),
        tbl_ysz(_tbl_ysz),
        tbl_pa(_tbl_pa),
        tbl_va(_tbl_va),
        pdo_xsz(_pdo_xsz),
        pdo_ysz(_pdo_ysz),
        img_xsz(_img_xsz),
        img_ysz(_img_ysz){}

    TblInfo& operator=( TblInfo &_in)
    {
        memID   = _in.memID;
        tbl_xsz = _in.tbl_xsz;
        tbl_ysz = _in.tbl_ysz;
        tbl_pa  = _in.tbl_pa;
        tbl_va  = _in.tbl_va;
        pdo_xsz = _in.pdo_xsz;
        pdo_ysz = _in.pdo_ysz;
        img_xsz = _in.img_xsz;
        img_ysz = _in.img_ysz;
        return (*this);
    }
};

struct Tbl
{
    //
    TblInfo tbl;
    TblInfo tbl_bin;
    //
    IImageBuffer *pbuf;
    IImageBuffer *pbuf_bin;

    Tbl() :
        tbl(TblInfo()),
        tbl_bin(TblInfo()),
        pbuf(nullptr),
        pbuf_bin(nullptr) {}
};

class IPDTblGen
{
public:
    static IPDTblGen* getInstance();

    IPDTblGen() {};

    virtual ~IPDTblGen() {};

    virtual bool start( int const sensorDev, int const sensorIdx) = 0;

    virtual bool stop( int const sensorDev) = 0;

    virtual bool getTbl( int const sensorDev, int const scenario, Tbl &outTbl) = 0;
};

#endif //__PD_TBL_GEN_H__
