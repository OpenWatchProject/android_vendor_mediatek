#ifndef XMP_META_H
#define XMP_META_H

#include <string>

#define TXMP_STRING_TYPE std::string

// Ensure XMP templates are instantiated
#include "XMP.incl_cpp"

// Provide access to the API
#include "XMP.hpp"

#include "BufferManager.h"

namespace stereo {

class XmpMeta {

public:

    explicit XmpMeta(const StereoBuffer_t &buffer);

    XmpMeta(XMP_StringPtr buffer, int bufferSize);

    virtual ~XmpMeta();

    bool getPropertyString(const StereoString &nameSpace,
        const StereoString &propName, StereoString* propValue) const;

    void setPropertyString(const StereoString &nameSpace,
        const StereoString &propName, const StereoString &propValue);

    bool getPropertyBase64(const StereoString &nameSpace,
        const StereoString &propName, StereoBuffer_t &propValue) const;

    void setPropertyBase64(const StereoString &nameSpace,
        const StereoString &propName, StereoBuffer_t &propValue);

    bool getStructField(const StereoString &nameSpace, const StereoString &structName,
        const StereoString &fieldNS, const StereoString &fieldName, StereoString* fieldValue) const;

    void setStructField(const StereoString &nameSpace, const StereoString &structName,
        const StereoString &fieldNS, const StereoString &fieldName, const StereoString &fieldValue);

    bool getArrayItem(const StereoString &nameSpace,
        const StereoString &arrayName, int index, StereoString* itemValue) const;

    void setArrayItem(const StereoString &nameSpace,
        const StereoString &arrayName, int index, const StereoString &itemValue);

    bool registerNamespace(const StereoString &nameSpace, const StereoString &prefix) const;

    void serialize(StereoString* rdfString) const;

private:
    SXMPMeta* pMeta;

};

}

#endif