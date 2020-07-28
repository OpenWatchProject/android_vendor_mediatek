#ifndef _VAL_LOG_H_
#define _VAL_LOG_H_
#ifdef __ANDROID__
#include <log/log.h>
#else
#include <stdio.h>
#endif
#define USE_VIDEO_ION

#define MFV_LOG_ERROR           1	/* /< error */
#define MFV_LOG_WARNING         1	/* /< warning */
#define MFV_LOG_DEBUG           1	/* /<debug information */
#define MFV_LOG_INFO            1	/* /< information */
#define MFV_LOG_DUMP_REG        1	/* /< register */
#define MFV_LOG_SP5CODEC        1	/* /< sw/hybrid lib log */
#define DUMP_SIM_REG            1	/* /< dump register settings in internal log */

extern unsigned int vcodec_public_dbg_level;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ANDROID__
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "VPUD"
#define CODEC_LOG(x...) ALOGE(x)
#else //__ANDROID__
#define CODEC_LOG(x...) fprintf(stderr, x)
#endif

#if MFV_LOG_ERROR
#define MFV_LOGE(...) do { CODEC_LOG(__VA_ARGS__); }while(0);   /* /< show error log */
#define VDO_LOGE(...) do { CODEC_LOG(__VA_ARGS__); }while(0);   /* /< show error log */
#else
#define MFV_LOGE(...)           /* /< NOT show error log */
#define VDO_LOGE(...)           /* /< NOT show error log */
#endif

#if MFV_LOG_WARNING
#define MFV_LOGW(...) do { if((vcodec_public_dbg_level & (1 << 0)) == (1 << 0)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);    /* /< show warning log */
#define VDO_LOGW(...) do { if((vcodec_public_dbg_level & (1 << 0)) == (1 << 0)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);    /* /< show warning log */
#else
#define MFV_LOGW(...)           /* /< NOT show warning log */
#define VDO_LOGW(...)           /* /< NOT show warning log */
#endif

#if MFV_LOG_DEBUG
#define MFV_LOGD(...) do { if((vcodec_public_dbg_level & (1 << 1)) == (1 << 1)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);   /* /< show debug information log */
#define VDO_LOGD(...) do { if((vcodec_public_dbg_level & (1 << 1)) == (1 << 1)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);   /* /< show debug information log */
#else
#define MFV_LOGD(...)           /* /< NOT show debug information log */
#define VDO_LOGD(...)           /* /< NOT show debug information log */
#endif

#if MFV_LOG_INFO
#define MFV_LOGI(...) do { if((vcodec_public_dbg_level & (1 << 2)) == (1 << 2)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);    /* /< show information log */
#define VDO_LOGI(...) do { if((vcodec_public_dbg_level & (1 << 2)) == (1 << 2)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);    /* /< show information log */
#else
#define MFV_LOGI(...)           /* /< NOT show information log */
#define VDO_LOGI(...)           /* /< NOT show information log */
#endif

#if MFV_LOG_DUMP_REG
#define MFV_LOGR(...) do { if((vcodec_public_dbg_level & (1 << 3)) == (1 << 3)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);
#define VDO_LOGR(...) do { if((vcodec_public_dbg_level & (1 << 3)) == (1 << 3)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0);
#else
#define MFV_LOGR(...)           /* /< NOT show debug register log */
#define VDO_LOGR(...)           /* /< NOT show debug register log */
#endif

#if MFV_LOG_SP5CODEC
#define MFV_LOGSP5(...) do { if((vcodec_public_dbg_level & (1 << 4)) == (1 << 4)) \
    { CODEC_LOG(__VA_ARGS__);} }while(0); /* /< show sw/hybrid lib log */
#else
#define MFV_LOGSP5(...)           /* /< NOT show sw/hybrid lib log */
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _VAL_LOG_H_ */
