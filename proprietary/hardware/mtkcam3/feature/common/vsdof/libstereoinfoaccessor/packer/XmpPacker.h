#ifndef XMP_PACKER_H
#define XMP_PACKER_H

#include "IPacker.h"

namespace stereo {

/**
 * Standard and extended data buffer pack and unpack.
 */
class XmpPacker : public IPacker {

public:
    /**
     * XmpPacker constructor.
     * @param packInfo
     *            PackInfo
     */
    explicit XmpPacker(PackInfo *packInfo);
    virtual ~XmpPacker();
    virtual void pack();
    virtual void unpack();

private:
    PackInfo *pPackInfo;
    StereoVector<StereoBuffer_t>* makeExtXmpData(StereoBuffer_t &extXmpData);
    /**
     * Get extend xmp header.
     * extend xmp header = ext_header + md5 + total length + partition offset
     * @param md5
     *            md5 value
     * @param totalLength
     *            total length
     * @param sectionNumber
     *            section number
     * @param [out]header
     */
    void getXmpCommonHeader(
        StereoString md5, int totalLength, int sectionNumber, StereoBuffer_t &header);
};

}

#endif