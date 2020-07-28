//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/XmpMeta"

#include "XmpMeta.h"
#include "StereoLog.h"
#include "BufferManager.h"
#include <utils/Trace.h>
#include <cutils/trace.h>

using namespace stereo;

#define ATRACE_TAG ATRACE_TAG_ALWAYS

XmpMeta::XmpMeta(const StereoBuffer_t &buffer) {
    StereoLogD("consutruct XmpMeta by StereoBuffer");
    try {
        if(!SXMPMeta::Initialize()) {
            StereoLogE("<XmpMeta>Could not initialize toolkit!");
            return;
        }

        StereoLogD("<XmpMeta> new XMPMeta");
        pMeta = new SXMPMeta();
        if (buffer.isValid()) {
            StereoLogD("<XmpMeta> ParseFromBuffer");
            ATRACE_BEGIN("XmpMeta-ParseFromBuffer1");
            pMeta->ParseFromBuffer((char*)buffer.data, buffer.size);
            ATRACE_END();
        }
    } catch (XMP_Error & excep) {
        StereoLogE("<XmpMeta1> Caught XMP_Error %d : %s\n",
            excep.GetID(), excep.GetErrMsg() );
    }
}


XmpMeta::XmpMeta(XMP_StringPtr buffer, int bufferSize) {
    StereoLogD("consutruct XmpMeta by XMP_StringPtr");
    try {
        if(!SXMPMeta::Initialize()) {
            StereoLogE("<XmpMeta>Could not initialize toolkit!");
            return;
        }

        StereoLogD("<XmpMeta> new XMPMeta");
        pMeta = new SXMPMeta();
        if (buffer != nullptr) {
            StereoLogD("<XmpMeta> ParseFromBuffer");
            ATRACE_BEGIN("XmpMeta-ParseFromBuffer2");
            pMeta->ParseFromBuffer(buffer, bufferSize);
            ATRACE_END();
        }
    } catch (XMP_Error & excep) {
        StereoLogE("<XmpMeta2> Caught XMP_Error %d : %s\n",
            excep.GetID(), excep.GetErrMsg() );
    }
}

XmpMeta::~XmpMeta() {
    StereoLogD("~XmpMeta");
    if (pMeta != nullptr) {
        delete pMeta;
    }
}

bool XmpMeta::getPropertyString(const StereoString &nameSpace,
        const StereoString &propName, StereoString* propValue) const {
    StereoLogD("getPropertyString, nameSpace = %s, propName = %s",
        nameSpace.c_str(), propName.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not get property string");
        return false;
    }

    try {
        return pMeta->GetProperty(nameSpace.c_str(), propName.c_str(), propValue, 0);
    } catch (XMP_Error & excep) {
        char *msg = "<getPropertyString> Caught XMP_Error %d : %s\n, \
nameSpace = %s, propName = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), propName.c_str());
        return false;
    }
}

void XmpMeta::setPropertyString(const StereoString &nameSpace,
        const StereoString &propName, const StereoString &propValue) {
    StereoLogD("setPropertyString, nameSpace = %s, propName = %s, propValue = %s",
        nameSpace.c_str(), propName.c_str(), propValue.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not set property string");
        return;
    }

    try {
        pMeta->SetProperty(nameSpace.c_str(), propName.c_str(), propValue);
    } catch (XMP_Error & excep) {
        char *msg = "<setPropertyString> Caught XMP_Error %d : %s\n, \
nameSpace = %s, propName = %s, propValue = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), propName.c_str(), propValue.c_str());
    }
}

bool XmpMeta::getPropertyBase64(const StereoString &nameSpace,
        const StereoString &propName, StereoBuffer_t &propValue) const {
    StereoLogD("getPropertyBase64, nameSpace = %s, propName = %s",
        nameSpace.c_str(), propName.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not get property base64");
        return false;
    }

    ATRACE_NAME("XmpMeta-getPropertyBase64");
    // get string property
    try {
        StereoString encodedValue;
        if (!pMeta->GetProperty(nameSpace.c_str(), propName.c_str(), &encodedValue, 0)) {
            StereoLogE("can not get property");
            return false;
        }

        // base64 decode
        StereoString decodedValue;
        SXMPUtils::DecodeFromBase64(encodedValue, &decodedValue);
        int decodedValueSize = decodedValue.length();

        // output
        BufferManager::createBuffer(decodedValueSize, propValue);
        memcpy(propValue.data, &decodedValue[0], decodedValueSize);
    } catch (XMP_Error & excep) {
        char *msg = "<getPropertyBase64> Caught XMP_Error %d : %s\n, \
nameSpace = %s, propName = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), propName.c_str());
        return false;
    }
    return true;
}

void XmpMeta::setPropertyBase64(const StereoString &nameSpace,
        const StereoString &propName, StereoBuffer_t &propValue) {
    StereoLogD("setPropertyBase64, nameSpace = %s, propName = %s",
        nameSpace.c_str(), propName.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not set property base64");
        return;
    }

    ATRACE_NAME("XmpMeta-setPropertyBase64");
    try {
        // base64 encode
        StereoString encodedValue;
        SXMPUtils::EncodeToBase64((char*)propValue.data, propValue.size, &encodedValue);

        // set string property
        pMeta->SetProperty(nameSpace.c_str(), propName.c_str(), encodedValue);
    } catch (XMP_Error & excep) {
        char *msg = "<setPropertyBase64> Caught XMP_Error %d : %s\n, \
nameSpace = %s, propName = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), propName.c_str());
    }
}

bool XmpMeta::getStructField(const StereoString &nameSpace, const StereoString &structName,
        const StereoString &fieldNS, const StereoString &fieldName, StereoString* fieldValue) const {
    StereoLogD("getStructField, nameSpace = %s, structName = %s, fieldNS = %s, fieldName = %s",
        nameSpace.c_str(), structName.c_str(), fieldNS.c_str(), fieldName.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not get struct field");
        return false;
    }
    try {
        return pMeta->GetStructField(nameSpace.c_str(), structName.c_str(),
                            fieldNS.c_str(), fieldName.c_str(), fieldValue, 0);
    } catch (XMP_Error & excep) {
        char *msg = "<getStructField> Caught XMP_Error %d : %s\n, \
nameSpace = %s, structName = %s, fieldNS = %s, fieldName = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), structName.c_str(), fieldNS.c_str(), fieldName.c_str());
        return false;
    }
}

void XmpMeta::setStructField(const StereoString &nameSpace, const StereoString &structName,
        const StereoString &fieldNS, const StereoString &fieldName, const StereoString &fieldValue) {
    StereoLogD("setStructField, nameSpace = %s, structName = %s, fieldNS = %s,\
fieldName = %s, fieldValue = %s", nameSpace.c_str(), structName.c_str(),
            fieldNS.c_str(),fieldName.c_str(), fieldValue.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not set struct field");
        return;
    }

    try {
        pMeta->SetStructField(nameSpace.c_str(), structName.c_str(),
            fieldNS.c_str(), fieldName.c_str(), fieldValue);
    } catch (XMP_Error & excep) {
        char *msg = "<setStructField> Caught XMP_Error %d : %s\n, \
nameSpace = %s, structName = %s, fieldNS = %s, fieldName = %s, fieldValue = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), structName.c_str(),
            fieldNS.c_str(),fieldName.c_str(), fieldValue.c_str());
    }
}

bool XmpMeta::getArrayItem(const StereoString &nameSpace,
                const StereoString &arrayName, int index, StereoString* itemValue) const {
    StereoLogD("getArrayItem, nameSpace = %s, arrayName = %s, index = %d",
        nameSpace.c_str(), arrayName.c_str(), index);
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not get array item");
        return false;
    }

    try {
        return pMeta->GetArrayItem(nameSpace.c_str(), arrayName.c_str(), index, itemValue, 0);
    } catch (XMP_Error & excep) {
        char *msg = "<getArrayItem> Caught XMP_Error %d : %s\n, \
nameSpace = %s, arrayName = %s, index = %d";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), arrayName.c_str(), index);
        return false;
    }
}

void XmpMeta::setArrayItem(const StereoString &nameSpace,
                const StereoString &arrayName, int index, const StereoString &itemValue) {
    StereoLogD("setArrayItem, nameSpace = %s, arrayName = %s, index = %d, itemValue = %s",
        nameSpace.c_str(), arrayName.c_str(), index, itemValue.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not set array item");
        return;
    }

    try {
        pMeta->SetArrayItem(nameSpace.c_str(), arrayName.c_str(), index, itemValue);
    } catch (XMP_Error & excep) {
    char *msg = "<setArrayItem> Caught XMP_Error %d : %s\n, \
nameSpace = %s, arrayName = %s, index = %d, itemValue = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
            nameSpace.c_str(), arrayName.c_str(), index, itemValue.c_str());
    }
}

bool XmpMeta::registerNamespace(const StereoString &nameSpace, const StereoString &prefix) const {
    StereoLogD("registerNamespace, nameSpace = %s, prefix = %s",
        nameSpace.c_str(), prefix.c_str());
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not register namespace");
        return false;
    }
    try {
        StereoString registeredPrefix;
        bool ret = pMeta->RegisterNamespace(nameSpace.c_str(),
            prefix.c_str(), &registeredPrefix);
        StereoLogD("registered prefix = %s", registeredPrefix.c_str());
        return ret;
    } catch (XMP_Error & excep) {
        char *msg = "<registerNamespace> Caught XMP_Error %d : %s\n, \
nameSpace = %s, prefix = %s";
        StereoLogE(msg, excep.GetID(), excep.GetErrMsg(),
        nameSpace.c_str(), prefix.c_str() );
        return false;
    }
}

void XmpMeta::serialize(StereoString* rdfString) const {
    StereoLogD("serialize");
    if (pMeta == nullptr) {
        StereoLogE("pMeta is nullptr, can not serialize");
        return;
    }
    ATRACE_NAME("XmpMeta-serialize");
    try {
        pMeta->SerializeToBuffer(rdfString, kXMP_UseCompactFormat|kXMP_OmitPacketWrapper, 0);
    } catch (XMP_Error & excep) {
        StereoLogE("<serialize> Caught XMP_Error %d : %s\n",
            excep.GetID(), excep.GetErrMsg() );
    }
}


