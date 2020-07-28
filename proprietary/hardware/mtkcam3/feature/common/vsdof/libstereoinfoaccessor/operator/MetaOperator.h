#ifndef META_OPERATOR_H
#define META_OPERATOR_H

#include "DataItem.h"
#include "types.h"
#include "XmpMeta.h"
#include "CustomizedMeta.h"

#define XMP_META_OPERATOR 0
#define CUSTOMIZED_META_OPERATOR 1

namespace stereo {

// Meta operator behavior definition.
class IMetaOperator {
public:
    virtual ~IMetaOperator() {}

    virtual void encrypt() = 0;

    virtual void decrypt() = 0;

    virtual void read() = 0;

    virtual void write() = 0;

    virtual BufferMapPtr serialize() = 0;

    virtual void setData(DataCollections *dataCollections) = 0;
};

class XmpMetaOperator : public IMetaOperator {

public:
    explicit XmpMetaOperator(const StereoBuffer_t &xmpBuffer);

    virtual ~XmpMetaOperator();

    virtual void encrypt() {}

    virtual void decrypt() {}

    virtual void read();

    virtual void write();

    virtual BufferMapPtr serialize();

    virtual void setData(DataCollections *dataCollections);

private:
    // Keep same with SerializedInfo.XMP_KEY
    const StereoString xmpKey = "XMP";
    int mDest;
    XmpMeta *pXmpMeta;
    StereoVector<SimpleItem*> *pListOfSimpleValue;
    StereoVector<BufferItem*> *pListOfBufferItem;
    StereoVector<ArrayItem*> *pListOfArrayItem;
    StereoVector<StructItem*> *pListOfStructItem;
};

class CustomizedMetaOperator : public IMetaOperator {

public:
    explicit CustomizedMetaOperator(BufferMapPtr customizedDataBuffer);

    virtual ~CustomizedMetaOperator();

    virtual void encrypt() {}

    virtual void decrypt() {}

    virtual void read();

    virtual void write();

    virtual BufferMapPtr serialize();

    virtual void setData(DataCollections *dataCollections);

private:
    CustomizedMeta *pCustMeta;
    StereoVector<BufferItem*> *pListOfCustDataItem;
};

}
#endif