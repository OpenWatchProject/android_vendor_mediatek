#ifndef _LOG_DEFINE_H_
#define _LOG_DEFINE_H_


#undef LOG_TAG
#define LOG_TAG "META"

#include <log/log.h>

#define META_LOG(...) \
        do { \
            ALOGD(__VA_ARGS__); \
        } while (0)

#define TEXT(__a)  __a



#endif /* _LOG_DEFINE_H__ */