#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "types.h"
#include "BufferManager.h"
#include "document.h"

namespace stereo {

class JsonParser {
public:
    explicit JsonParser(const StereoBuffer_t &jsonBuffer);
    virtual ~JsonParser();
    int getValueIntFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName);
    double getValueDoubleFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName);
    StereoString getValueStringFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName);
    bool getValueBoolFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName);
    StereoVector<StereoVector<int>>* getInt2DArrayFromObject(
        StereoString const *objectName, StereoString const *propertyName);
    int getArrayLength(StereoString const *arrayName);
    int getObjectPropertyValueFromArray(StereoString const *arrayName,
        int index, StereoString const *propertyName);

private:
    const int INVALID_VALUE_INT = -1;
    const double INVALID_VALUE_DOUBLE = -1.0;
    rapidjson::Document mDocument;
};

}

#endif
