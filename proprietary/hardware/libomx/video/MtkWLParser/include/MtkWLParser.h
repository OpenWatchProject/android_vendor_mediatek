
#ifndef WHITE_LIST_PARSER_
#define WHITE_LIST_PARSER_

#define WLPARSER_MAGIC  0x20150911

#include "MtkWLService.h"

typedef struct _WHITELIST_PARSER_
{
    int32_t     mMagicCode;

    char        mFeature[256];
    char        mApplication[256];
    char        mCallerApp[256];
    bool        mApplicationExist,mFeatureExist;
    int32_t     mOptionCount;
    P_Option    mOptionList[256];
} WLPARSER_T;

typedef WLPARSER_T * P_WLPARSER_T;

extern "C" void *WLInitWLParser(char *feature);
extern "C" bool WLGetAppValidName(void *hWLHandle, char *application);
extern "C" bool WLDeinitWLParser(void *hWLHandle);


#endif

