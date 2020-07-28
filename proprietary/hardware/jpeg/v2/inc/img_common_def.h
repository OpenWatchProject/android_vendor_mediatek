#ifndef __IMG_COMMON_DEF_H__
#define __IMG_COMMON_DEF_H__



#define TO_CEIL(x,a) ( ( (unsigned long)(x) + ((a)-1)) & ~((a)-1) )

#define TO_FLOOR(x,a) ( (unsigned long)(x) & ~((a)-1) )

#define TO_CEIL32(x,a) ( ( (int)(x) + ((a)-1)) & ~((a)-1) )

#define TO_FLOOR32(x,a) ( (int)(x) & ~((a)-1) )

#define TO_MASK(x,a) ( (x) & (a) )

#ifdef LOG_TAG
#undef LOG_TAG
#endif
 
#define LOG_TAG "JpgDecHal"

#define JPG_DBG(fmt, arg...)    ALOGD(fmt, ##arg)
#define JPG_WARN(fmt, arg...)   ALOGW(fmt, ##arg)
#define JPG_ERR(fmt, arg...)    ALOGE(fmt, ##arg)

#endif
