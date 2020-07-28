/*!
    \file versionGit.h
    \brief Flash algorithm git version header file
    \version 0.1
    \date 2019-12-18
*/

#define LOCAL_MODULE_VERSION_STRING "$Rev: 814 $"

static inline const char* getModuleVersionString()
{
    return LOCAL_MODULE_VERSION_STRING;
}

static int getModuleVersion()
{
    static const char kVersionStr[] = LOCAL_MODULE_VERSION_STRING;
    int version = 0;

    {
        enum
        {
            kSizeOfPrefix   = 6,
            kSizeOfPostfix  = 3,
        };

        const char* str = kVersionStr+kSizeOfPrefix;
        int size = sizeof(kVersionStr)/sizeof(*kVersionStr)-kSizeOfPrefix-kSizeOfPostfix;
        int num = 0;
        int i = 0;

        for (i = 0; i < size; ++i)
        {
            num *= 10;
            num += str[i]-'0';
        }

        version = num;
    }

#ifdef _DEBUG
    version *= -1;
#endif

    return version;
}

#undef LOCAL_MODULE_VERSION_STRING
