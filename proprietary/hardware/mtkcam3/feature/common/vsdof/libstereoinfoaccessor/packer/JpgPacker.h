#ifndef JPG_PACKER_H
#define JPG_PACKER_H

#include "IPacker.h"
#include "StereoInfo.h"
#include "BufferTypeInputStream.h"
#include "BufferTypeOutputStream.h"

namespace stereo {

/**
 * JPG packer.
 */
class JpgPacker : public IPacker {

public:
    /**
     * JpgPacker constructor.
     * @param packInfo
     *            pack information
     */
    explicit JpgPacker(PackInfo *packInfo);
    virtual ~JpgPacker();
    virtual void pack();
    virtual void unpack();

private:
    PackInfo *pPackInfo;
    StereoVector<Section*>* makeJpgSections(int marker, StereoVector<StereoBuffer_t>* buffers);

    StereoVector<Section*>* parseAppInfoFromStream(BufferTypeInputStream &is);

    void checkAppSectionTypeInStream(BufferTypeInputStream &is, Section *section);

    void pack(BufferTypeInputStream &is, BufferTypeOutputStream &os,
            Section *standardSection, StereoVector<Section*> *extendedSections,
            StereoVector<Section*> *customizedSections);

    int findProperLocationForXmp(StereoVector<Section*> *sections);

    StereoString getSectionTag(Section *section);

    void writeSectionToStream(BufferTypeInputStream &is, BufferTypeOutputStream &os,
        Section *sec);

    void writeSectionToStream(BufferTypeOutputStream &os, Section *sec);

    void writeCust(BufferTypeOutputStream &os,
                       StereoVector<Section*> *pCustomizedSections);

    void writeXmp(BufferTypeOutputStream &os,
                      Section *pStandardSection, StereoVector<Section*> *pExtendedSections);

    void copyToStreamWithFixBuffer(BufferTypeInputStream &is,
                                    BufferTypeOutputStream &os);

    void writeImageBuffer(BufferTypeInputStream &blurImageIs,
                                    BufferTypeOutputStream &os);
};

}

#endif