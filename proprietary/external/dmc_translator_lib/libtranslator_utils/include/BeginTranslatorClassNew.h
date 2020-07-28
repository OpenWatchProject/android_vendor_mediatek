#if TRANSLATE_WITH_ASN_DOT_1_ENCODE
    #define KPI_OBJ_TRANSLATOR(feature, type) \
        m_pTranslatorList[feature##_OBJ_TYPE_##type] = new GET_TRANSLATOR_CLASS_NAME(feature, type)(); \
        m_pTranslatorList[feature##_OBJ_TYPE_##type]->setObjId(getObjId(feature##_OBJ_TYPE_##type)); \
        m_pTranslatorList[feature##_OBJ_TYPE_##type]->setAsnDescriptor(getAsnTypeDescriptor(feature##_OBJ_TYPE_##type)); \
        DMC_LOGI("DMC-TranslatorUtils", "New %s Translator(%s), type=%d, oid=%s", #feature, #type, feature##_OBJ_TYPE_##type, getObjId(feature##_OBJ_TYPE_##type)->getOidString());
#else
    #define KPI_OBJ_TRANSLATOR(feature, type) \
        m_pTranslatorList[feature##_OBJ_TYPE_##type] = new GET_TRANSLATOR_CLASS_NAME(feature, type)(); \
        m_pTranslatorList[feature##_OBJ_TYPE_##type]->setObjId(getObjId(feature##_OBJ_TYPE_##type)); \
        DMC_LOGI("DMC-TranslatorUtils", "New %s Translator(%s), type=%d, oid=%s", #feature, #type, feature##_OBJ_TYPE_##type, getObjId(feature##_OBJ_TYPE_##type)->getOidString());
#endif

#define KPI_OBJ_TRANSLATOR_WITH_QUERY(feature, type) KPI_OBJ_TRANSLATOR(feature, type)
