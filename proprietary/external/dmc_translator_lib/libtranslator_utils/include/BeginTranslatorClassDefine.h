#ifndef TAG
#define TAG "DMC-TranslatorUtils"
#endif

#define KPI_OBJ_TRANSLATOR(feature, type) \
class GET_TRANSLATOR_CLASS_NAME(feature, type): public BaseTranslator \
{ \
public: \
    GET_TRANSLATOR_CLASS_NAME(feature, type)():BaseTranslator(false) \
    { \
        m_bRegistingTrapTable = true; \
        m_type_string = #type; \
        m_type = feature##_OBJ_TYPE_##type; \
        \
        KpiObj tmpObj; \
        TranslateEx(tmpObj); \
        CreateDumpFile();\
    } \
private: \
    void TranslateEx(KpiObj& obj); \
};

#define KPI_OBJ_TRANSLATOR_WITH_QUERY(feature, type) \
class GET_TRANSLATOR_CLASS_NAME(feature, type): public BaseTranslator \
{ \
public: \
    GET_TRANSLATOR_CLASS_NAME(feature, type)():BaseTranslator(true) \
    { \
        m_bRegistingTrapTable = true; \
        m_type_string = #type; \
        m_type = feature##_OBJ_TYPE_##type; \
        \
        KpiObj tmpObj; \
        TranslateEx(tmpObj); \
        CreateDumpFile();\
    } \
private: \
    void TranslateEx(KpiObj& obj); \
    bool Query(KpiObj &obj) override; \
};

