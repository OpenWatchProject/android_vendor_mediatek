#ifndef VOWENGINE_TYPES
#define VOWENGINE_TYPES

#ifdef __cplusplus
extern "C" {
#endif

/*-- byte data -----------------------------------------------------------------------------------------------*/
typedef struct {
    int length;
    char* beginPtr;
} ByteDataSection;

typedef ByteDataSection FileData;
typedef ByteDataSection ParsedData;


typedef struct {
    ParsedData apData;
    ParsedData dspData;
} ParsedDataCollection;


typedef struct {
    int apSize;
    int dspSize;
} ParsedDataSizes;


typedef struct {
    int status;
    double score;
    int startIdx;
    int endIdx;
} RecognitionResults;



#ifdef __cplusplus
}
#endif

#endif // VOWENGINE_TYPES

