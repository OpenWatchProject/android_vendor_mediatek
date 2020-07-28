//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/JsonParser"

#include "JsonParser.h"
#include "StereoLog.h"
#include "Utils.h"

using namespace stereo;
using namespace rapidjson;


Value* getSearchNode(Document &document, StereoString const *objectName,
        StereoString const *subObjectName) {
    StereoLogD("<getSearchNode>");
    Value *searchNode = &document;
    Value::ConstMemberIterator iter;
    if (objectName != nullptr) {
        // if has object, get object firstly
        iter = document.FindMember(objectName->c_str());
        if (iter == document.MemberEnd()) {
            StereoLogE("<getSearchNode> The member not found, object name = %s",
                objectName->c_str());
            return nullptr;
        }
        if (!iter->value.IsObject()) {
            StereoLogE("<getSearchNode> the value not object, object name = %s",
                objectName->c_str());
            return nullptr;
        }
        searchNode = const_cast<Value *>(&iter->value);

        // if has sub object, then get sub object
        if (subObjectName != nullptr) {
            bool foundSubObj = false;
            for(auto &subObj : searchNode->GetObject()) {
                if (subObj.name == subObjectName->c_str()) {
                    foundSubObj = true;
                    searchNode = const_cast<Value *> (&subObj.value);
                    break;
                }
            }
            if (!foundSubObj) {
                StereoLogE("<getSearchNode> The member not found, sub object name = %s",
                    subObjectName->c_str());
                return nullptr;
            }
        }
    }
    return searchNode;
}

JsonParser::JsonParser(const StereoBuffer_t &jsonBuffer) {
    StereoLogD("<JsonParser>");
    if (!jsonBuffer.isValid()) {
        StereoLogE("json buffer is null");
        return;
    }
    // parse json buffer
    mDocument.Parse((char *)jsonBuffer.data, jsonBuffer.size);
    StereoLogD("<JsonParser>, mDocument: %p, mDocument.IsObject(): %d", &mDocument, mDocument.IsObject());
    StereoLogD("<JsonParser>, buffer = %s", Utils::buffer2Str(jsonBuffer).c_str());
}

JsonParser::~JsonParser() {
    StereoLogD("<~JsonParser>");
}

int JsonParser::getValueIntFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName) {
    if (!mDocument.IsObject() || propertyName == nullptr || propertyName->empty()) {
        StereoLogE("<getValueIntFromObject> error!!");
        StereoLogE("mDocument.IsObject(): %d, propertyName: %s",
            mDocument.IsObject(), propertyName->c_str());
        return INVALID_VALUE_INT;
    }

    Value *searchNode = getSearchNode(mDocument, objectName, subObjectName);
    if (searchNode == nullptr) {
        StereoLogE("<getValueIntFromObject> can not find search node");
        return INVALID_VALUE_INT;
    }

    // read property from search node
    Value::ConstMemberIterator iter;
    iter = searchNode->FindMember(propertyName->c_str());
    if (iter == searchNode->MemberEnd()) {
        StereoLogE("<getValueIntFromObject> The member not found, property name = %s",
            propertyName->c_str());
        return INVALID_VALUE_INT;
    }
    if (!iter->value.IsInt()) {
        StereoLogE("<getValueIntFromObject> the value not int, property = %s",
            propertyName->c_str());
        return INVALID_VALUE_INT;
    }
    return iter->value.GetInt();
}

double JsonParser::getValueDoubleFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName) {
    if (!mDocument.IsObject() || propertyName == nullptr || propertyName->empty()) {
        StereoLogE("<getValueDoubleFromObject> error!!");
        return INVALID_VALUE_DOUBLE;
    }

     Value *searchNode = getSearchNode(mDocument, objectName, subObjectName);
    if (searchNode == nullptr) {
        StereoLogE("<getValueDoubleFromObject> can not find search node");
        return INVALID_VALUE_DOUBLE;
    }

    // read property from search node
    Value::ConstMemberIterator iter;
    iter = searchNode->FindMember(propertyName->c_str());
    if (iter == searchNode->MemberEnd()) {
        StereoLogE("<getValueDoubleFromObject> The member not found, property name = %s",
            propertyName->c_str());
        return INVALID_VALUE_DOUBLE;
    }
    if (!iter->value.IsDouble()) {
        StereoLogE("<getValueDoubleFromObject> the value not double, property = %s",
            propertyName->c_str());
        return INVALID_VALUE_DOUBLE;
    }
    return iter->value.GetDouble();
}

StereoString JsonParser::getValueStringFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName) {
    StereoString retValue = "";
    if (!mDocument.IsObject() || propertyName == nullptr || propertyName->empty()) {
        StereoLogE("<getValueStringFromObject> error!!");
        return retValue;
    }

    Value *searchNode = getSearchNode(mDocument, objectName, subObjectName);
    if (searchNode == nullptr) {
        StereoLogE("<getValueStringFromObject> can not find search node");
        return retValue;
    }

    // read property from search node
    Value::ConstMemberIterator iter;
    iter = searchNode->FindMember(propertyName->c_str());
    if (iter == searchNode->MemberEnd()) {
        StereoLogE("<getValueStringFromObject> The member not found, property name = %s",
            propertyName->c_str());
        return retValue;
    }
    if (!iter->value.IsString()) {
        StereoLogE("<getValueStringFromObject> the value not string, property = %s",
            propertyName->c_str());
        return retValue;
    }
    retValue = StereoString(iter->value.GetString(), iter->value.GetStringLength());
    return retValue;
}

bool JsonParser::getValueBoolFromObject(StereoString const *objectName,
        StereoString const *subObjectName, StereoString const *propertyName) {
    if (!mDocument.IsObject() || propertyName == nullptr || propertyName->empty()) {
        StereoLogE("<getValueBoolFromObject> error!!");
        return false;
    }

    Value *searchNode = getSearchNode(mDocument, objectName, subObjectName);
    if (searchNode == nullptr) {
        StereoLogE("<getValueBoolFromObject> can not find search node");
        return false;
    }

    // read property from search node
    Value::ConstMemberIterator iter;
    iter = searchNode->FindMember(propertyName->c_str());
    if (iter == searchNode->MemberEnd()) {
        StereoLogE("<getValueBoolFromObject> The member not found, property name = %s",
            propertyName->c_str());
        return false;
    }
    if (!iter->value.IsBool()) {
        StereoLogE("<getValueBoolFromObject> the value not string, property = %s",
            propertyName->c_str());
        return false;
    }
    return iter->value.GetBool();
}

StereoVector<StereoVector<int>>* JsonParser::getInt2DArrayFromObject(
    StereoString const *objectName, StereoString const *propertyName) {
    if (!mDocument.IsObject() || propertyName == nullptr || propertyName->empty()) {
        StereoLogE("<getInt2DArrayFromObject> error!!");
        return nullptr;
    }

    Value *searchNode = getSearchNode(mDocument, objectName, nullptr);
    if (searchNode == nullptr) {
        StereoLogE("<getInt2DArrayFromObject> can not find search node");
        return nullptr;
    }

    // read property from search node
    Value::ConstMemberIterator iter;
    iter = searchNode->FindMember(propertyName->c_str());
    if (iter == searchNode->MemberEnd()) {
        StereoLogE("<getInt2DArrayFromObject> The member not found, property name = %s",
            propertyName->c_str());
        return nullptr;
    }
    if (!iter->value.IsArray()) {
        StereoLogE("<getValueBoolFromObject> the value not array, property = %s",
            propertyName->c_str());
        return nullptr;
    }

    int dimen1Size, dimen2Size;
    const Value& dimen1Array = iter->value;
    dimen1Size = dimen1Array.GetArray().Size();
    if (dimen1Size <= 0) {
        StereoLogE("<getValueBoolFromObject> the array is empty");
        return nullptr;
    }

    StereoVector<StereoVector<int>> *arr = new StereoVector<StereoVector<int>>(dimen1Size);
    for (int i = 0; i < dimen1Size; i++) {
        dimen2Size = dimen1Array[i].GetArray().Size();
        if (dimen2Size <= 0) {
            continue;
        }
        (*arr)[i].resize(dimen2Size);
        const Value& dimen2Array = dimen1Array[i];
        for (int j = 0; j < dimen2Size; j++) {
            if (dimen2Array[j].IsInt()) {
                (*arr)[i][j] = dimen2Array[j].GetInt();
            }
        }
    }
    return arr;
}

int JsonParser::getArrayLength(StereoString const *arrayName) {
    if (!mDocument.IsObject() || arrayName == nullptr || arrayName->empty()) {
        StereoLogE("<getArrayLength> error!!");
        return INVALID_VALUE_INT;
    }
    // read array
    Value::ConstMemberIterator iter;
    iter = mDocument.FindMember(arrayName->c_str());
    if (iter == mDocument.MemberEnd()) {
        StereoLogE("<getArrayLength> The member not found, array name = %s",
            arrayName->c_str());
        return INVALID_VALUE_INT;
    }
    if (!iter->value.IsArray()) {
        StereoLogE("<getArrayLength> the value not array, array name = %s",
            arrayName->c_str());
        return INVALID_VALUE_INT;
    }
    return iter->value.GetArray().Size();
}

int JsonParser::getObjectPropertyValueFromArray(StereoString const *arrayName,
        int index, StereoString const *propertyName) {
    if (!mDocument.IsObject() || arrayName == nullptr || arrayName->empty()) {
        StereoLogE("<getObjectPropertyValueFromArray> error!!");
        return INVALID_VALUE_INT;
    }
    // read array
    Value::ConstMemberIterator iter;
    iter = mDocument.FindMember(arrayName->c_str());
    if (iter == mDocument.MemberEnd()) {
        StereoLogE("<getObjectPropertyValueFromArray> The member not found, array name = %s",
            arrayName->c_str());
        return INVALID_VALUE_INT;
    }
    if (!iter->value.IsArray()) {
        StereoLogE("<getObjectPropertyValueFromArray> the value not array, array name = %s",
            arrayName->c_str());
        return INVALID_VALUE_INT;
    }

    // read property from array item
    const Value& arrayItem = iter->value[index];
    iter = arrayItem.FindMember(propertyName->c_str());
    if (iter == mDocument.MemberEnd()) {
        StereoLogE("<getObjectPropertyValueFromArray> The member not found, property name = %s",
            propertyName->c_str());
        return INVALID_VALUE_INT;
    }
    if (!iter->value.IsInt()) {
        StereoLogE("<getObjectPropertyValueFromArray> the value not int, property name = %s",
            propertyName->c_str());
        return INVALID_VALUE_INT;
    }
    return iter->value.GetInt();
}