#include <audio_fmt_conv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <errno.h>

#include <audio_log_hal.h>


#include <wrapped_audio.h>

#include <blisrc_exp.h>
#include <Shifter_exp.h>


#include <audio_memory_control.h>

#include <feature_manager.h>
#include <dvfs_config_parser.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AUD][FMT]"

#ifdef conv_dbg
#undef conv_dbg
#endif

#if 0
#define conv_dbg(x...) ALOGD(x)
#else
#define conv_dbg(x...)
#endif



/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

struct aud_src_hdl_t {
    void *lib_hdl;
    Blisrc_Param lib_param;

    void *working_buf;
    uint32_t working_buf_size;

    void *lib_tmp_buf;
    uint32_t lib_tmp_buf_size;

    void *blisrc_out;
    uint32_t blisrc_out_size;

    double buf_times; /* buf size of cfg out : buf size of cfg in */
    uint32_t size_per_frame; /* base frame size of cfg out */
    bool pad_flag;
};


struct aud_bcv_hdl_t { /* only need shifter, no limiter */
    bool need_bcv;

    uint32_t pcm_format;

    void *bcv_out;
    uint32_t bcv_out_size;

    double buf_times; /* buf size of cfg out : buf size of cfg in */
    uint32_t size_per_frame; /* base frame size of cfg out */
};


struct aud_fmt_conv_hdl_t {
    /* source -> src -> bcv -> target */
    struct aud_fmt_cfg_t source;
    struct aud_fmt_cfg_t after_src;
    struct aud_fmt_cfg_t target;

    struct aud_src_hdl_t src_hdl;
    struct aud_bcv_hdl_t bcv_hdl;
};

#define SIZE_OF_AUD_BUF_CONV_HDL (sizeof(struct aud_fmt_conv_hdl_t))


/*
 * =============================================================================
 *                     utility
 * =============================================================================
 */

static inline uint32_t get_size_per_frame(const struct aud_fmt_cfg_t *cfg)
{
    if (!cfg) {
        ALOGW("%s(), %p NULL!!", __FUNCTION__, cfg);
        return 0;
    }

    return cfg->num_channels * AUDIO_BYTES_PER_SAMPLE(cfg->audio_format);
}


static inline uint32_t get_size_per_second(const struct aud_fmt_cfg_t *cfg)
{
    if (!cfg) {
        ALOGW("%s(), %p NULL!!", __FUNCTION__, cfg);
        return 0;
    }

    return cfg->sample_rate * get_size_per_frame(cfg);
}


static inline double get_times_of_buffer(
    const struct aud_fmt_cfg_t *source,
    const struct aud_fmt_cfg_t *target)
{
    uint32_t size_per_second_in = 0;
    uint32_t size_per_second_out = 0;

    if (!source || !target) {
        ALOGW("%s(), %p %p NULL!!", __FUNCTION__, source, target);
        return 1.0;
    }

    size_per_second_in = get_size_per_second(source);
    size_per_second_out = get_size_per_second(target);

    if (size_per_second_in == 0 || size_per_second_out == 0) {
        ALOGW("%s(), audio_format: 0x%x, 0x%x error!!", __FUNCTION__,
              source->audio_format, target->audio_format);
        return 1.0;
    }

    return ((double)size_per_second_out / (double)size_per_second_in);
}


static uint32_t get_cvt_size(
    uint32_t size_in,
    double buf_times,
    uint32_t size_per_frame)
{
    uint32_t size_out = (uint32_t)(size_in * buf_times);

    /* align */
    if ((size_out % size_per_frame) != 0) {
        size_out = ((size_out / size_per_frame) + 1) * size_per_frame;
    }

    return size_out;
}


static void trans_dvfs_cfg_src(
    struct aud_fmt_cfg_t *source,
    struct aud_fmt_cfg_t *target,
    struct dvfs_swip_config_t *dvfs_config) {
    if (!source || !target || !dvfs_config) {
        return;
    }

    dvfs_config->dl_in.format = source->audio_format;
    dvfs_config->dl_out.format = source->audio_format;
    dvfs_config->dl_in.channels = source->num_channels;
    dvfs_config->dl_out.channels = target->num_channels;
    dvfs_config->dl_in.sample_rate = source->sample_rate;
    dvfs_config->dl_out.sample_rate = target->sample_rate;

    memcpy(&dvfs_config->ul_in, &dvfs_config->dl_in, sizeof(struct audio_profile));
    memcpy(&dvfs_config->ul_out, &dvfs_config->dl_out, sizeof(struct audio_profile));
}


static void trans_dvfs_cfg_bcv(
    struct aud_fmt_cfg_t *source,
    struct aud_fmt_cfg_t *target,
    struct dvfs_swip_config_t *dvfs_config) {
    if (!source || !target || !dvfs_config) {
        return;
    }

    dvfs_config->dl_in.format = source->audio_format;
    dvfs_config->dl_out.format = target->audio_format;
    dvfs_config->dl_in.channels = target->num_channels;
    dvfs_config->dl_out.channels = target->num_channels;
    dvfs_config->dl_in.sample_rate = target->sample_rate;
    dvfs_config->dl_out.sample_rate = target->sample_rate;

    memcpy(&dvfs_config->ul_in, &dvfs_config->dl_in, sizeof(struct audio_profile));
    memcpy(&dvfs_config->ul_out, &dvfs_config->dl_out, sizeof(struct audio_profile));
}


/*
 * =============================================================================
 *                     Sampling Rate Converter
 * =============================================================================
 */

static int get_src_pcm_format(struct aud_fmt_cfg_t *source,
                              struct aud_fmt_cfg_t *target,
                              struct aud_fmt_cfg_t *after_src,
                              unsigned int *pcm_format,
                              bool *need_src)
{
    int ret = 0;

    if (!source || !target || !after_src || !pcm_format || !need_src) {
        ALOGW("%s(), %p %p %p %p %p fail!!", __FUNCTION__,
              source, target, after_src, pcm_format, need_src);
        return -EINVAL;
    }

    /* init do nothing */
    memcpy(after_src, source, sizeof(struct aud_fmt_cfg_t));
    *need_src = false;

    /* if need SRC */
    if (source->sample_rate  != target->sample_rate ||
        source->num_channels != target->num_channels) {
        switch (source->audio_format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            *pcm_format = BLISRC_IN_Q1P15_OUT_Q1P15;
            *need_src = true;
            break;
        case AUDIO_FORMAT_PCM_32_BIT:
            *pcm_format = BLISRC_IN_Q1P31_OUT_Q1P31;
            *need_src = true;
            break;
        case AUDIO_FORMAT_PCM_8_24_BIT:
            *pcm_format = BLISRC_IN_Q9P23_OUT_Q9P23;
            *need_src = true;
            break;
        default:
            ALOGE("%s(), SRC format not support (%u->%u)", __FUNCTION__,
                  source->audio_format, target->audio_format);
            ret = -EINVAL;
        }
    }

    if (*need_src == true) {
        after_src->audio_format = source->audio_format;
        after_src->num_channels = target->num_channels;
        after_src->sample_rate  = target->sample_rate;
    }

    return ret;
}


static int aud_src_init(
    struct aud_fmt_cfg_t *source,
    struct aud_fmt_cfg_t *target,
    struct aud_fmt_cfg_t *after_src,
    struct aud_src_hdl_t *src_hdl)
{
    Blisrc_Param *lib_param = NULL;

    bool need_src = false;

    int ret = 0;

    if (!source || !target || !after_src || !src_hdl) {
        ALOGW("%s(), %p %p %p %p NULL!!", __FUNCTION__,
              source, target, after_src, src_hdl);
        return -EINVAL;
    }

    memset(src_hdl, 0, sizeof(struct aud_src_hdl_t));

    lib_param = &src_hdl->lib_param;


    /* get cfg after SRC */
    ret = get_src_pcm_format(source, target, after_src,
                             &lib_param->PCM_Format,
                             &need_src);
    if (ret != 0) {
        return ret;
    }
    if (need_src == false) { /* no need SRC */
        src_hdl->lib_hdl = NULL;
        return 0;
    }


    /* init param */
    lib_param->in_sampling_rate = source->sample_rate;
    lib_param->in_channel       = source->num_channels;
    lib_param->ou_sampling_rate = after_src->sample_rate;
    lib_param->ou_channel       = after_src->num_channels;


    /* alloc blisrc working buffer */
    ret = Blisrc_GetBufferSize((unsigned int *)&src_hdl->working_buf_size,
                               (unsigned int *)&src_hdl->lib_tmp_buf_size,
                               lib_param);
    if (ret < 0) {
        ALOGE("%s(), Blisrc_GetBufferSize() return err %d", __FUNCTION__, ret);
        return ret;
    }

    if (src_hdl->working_buf_size != 0) {
        src_hdl->working_buf = AUDIO_MALLOC(src_hdl->working_buf_size);
        if (src_hdl->working_buf == NULL) {
            ALOGW("%s(), working_buf malloc fail!!", __FUNCTION__);
            return -ENOMEM;
        }
        memset(src_hdl->working_buf, 0, src_hdl->working_buf_size);
    }

    if (src_hdl->lib_tmp_buf_size != 0) {
        src_hdl->lib_tmp_buf = AUDIO_MALLOC(src_hdl->lib_tmp_buf_size);
        if (src_hdl->lib_tmp_buf == NULL) {
            ALOGW("%s(), tmp_buf malloc fail!!", __FUNCTION__);
            return -ENOMEM;
        }
        memset(src_hdl->lib_tmp_buf, 0, src_hdl->lib_tmp_buf_size);
    }


    /* open blisrc */
    ret = Blisrc_Open(&src_hdl->lib_hdl, src_hdl->working_buf, lib_param);
    if (ret < 0) {
        ALOGE("%s(), Blisrc_Open() return err %d", __FUNCTION__, ret);
        return ret;
    }

    /* for alloc blisrc_out dynamically */
    src_hdl->buf_times = get_times_of_buffer(source, after_src);
    src_hdl->size_per_frame = get_size_per_frame(after_src);

    return 0;
}


static int aud_src_process(void *buf_in, uint32_t size_in,
                           void **buf_out, uint32_t *size_out,
                           struct aud_src_hdl_t *src_hdl)
{
    uint32_t expect_size = 0;

    uint32_t raw_size = 0;
    uint32_t src_size = 0;

    uint32_t pad_size = 0;

    int ret = 0;


    if (!buf_in || size_in == 0 ||
        !buf_out || !size_out ||
        !src_hdl) {
        ALOGW("%s(), %p %u %p %p %p fail!!", __FUNCTION__,
              buf_in, size_in, buf_out, size_out, src_hdl);
        return -EINVAL;
    }

    if (src_hdl->lib_hdl == NULL) { /* No need SRC */
        *buf_out = buf_in;
        *size_out = size_in;
        return 0;
    }

    /* get expect src out size */
    expect_size = get_cvt_size(size_in,
                               src_hdl->buf_times,
                               src_hdl->size_per_frame);

    /* make sure src out buf size is enough */
    ret = dynamic_change_buf_size(&src_hdl->blisrc_out,
                                  &src_hdl->blisrc_out_size,
                                  expect_size);
    if (ret != 0) {
        return ret;
    }

    /* do blisrc */
    raw_size = size_in;
    src_size = src_hdl->blisrc_out_size; /* tell lib max avail out buf size */
    ret = Blisrc_Process(src_hdl->lib_hdl,
                         src_hdl->lib_tmp_buf,
                         buf_in, &raw_size,
                         src_hdl->blisrc_out, &src_size);
    if (raw_size > 0) {
        ALOGE("%s(), size_in %u, raw_size %u !! src_size = %u",
              __FUNCTION__, size_in, raw_size, src_size);
#if 0 /* only sound break.. continue!! */
        return -ENOMEM;
#endif
    }

    /* pad zero for frame size alignment */
    if (src_hdl->pad_flag == false && src_size < expect_size) {
        conv_dbg("%s(), compensate %u => %u", __FUNCTION__, src_size, expect_size);

        pad_size = expect_size - src_size;
        if (expect_size <= src_hdl->blisrc_out_size) {
            memmove(((uint8_t *)src_hdl->blisrc_out) + pad_size,
                    src_hdl->blisrc_out,
                    src_size);
            memset(src_hdl->blisrc_out, 0, pad_size);
            src_size = expect_size;
        }
    }
    src_hdl->pad_flag = true; /* only pad 0 at the first process */

    /* update out */
    *buf_out = src_hdl->blisrc_out;
    *size_out = src_size;

    conv_dbg("%s(), %u -> %u", __FUNCTION__, size_in, *size_out);

    return 0;
}


static int aud_src_deinit(struct aud_src_hdl_t *src_hdl)
{
    if (!src_hdl) {
        ALOGW("%s(), %p NULL!!", __FUNCTION__, src_hdl);
        return -EINVAL;
    }

    if (src_hdl->lib_hdl) {
#if 0 /* lib do not implemented close func */
        Blisrc_Close(src_hdl->lib_hdl);
#endif
        src_hdl->lib_hdl = NULL;
    }

    if (src_hdl->working_buf) {
        AUDIO_FREE(src_hdl->working_buf);
        src_hdl->working_buf = NULL;
    }

    if (src_hdl->lib_tmp_buf != NULL) {
        AUDIO_FREE(src_hdl->lib_tmp_buf);
        src_hdl->lib_tmp_buf = NULL;
    }

    if (src_hdl->blisrc_out) {
        AUDIO_FREE(src_hdl->blisrc_out);
        src_hdl->blisrc_out = NULL;
    }

    return 0;
}



/*
 * =============================================================================
 *                     Bit Convert
 * =============================================================================
 */


static int get_bcv_pcm_format(audio_format_t source,
                              audio_format_t target,
                              uint32_t *pcm_format,
                              bool *need_bcv)
{
    int ret = 0;

    if (!pcm_format || !need_bcv) {
        ALOGW("%s(), %p %p fail!!", __FUNCTION__, pcm_format, need_bcv);
        return -EINVAL;
    }

    if (source == target) {
        *need_bcv = false;
        return 0;
    }
    if (source == AUDIO_FORMAT_PCM_24_BIT_PACKED ||
        target == AUDIO_FORMAT_PCM_24_BIT_PACKED) {
        ALOGE("%s(), AUDIO_FORMAT_PCM_24_BIT_PACKED not support!!", __FUNCTION__);
        *need_bcv = false;
        return 0;
    }

    *need_bcv = true;

    if (source == AUDIO_FORMAT_PCM_16_BIT) {
        if (target == AUDIO_FORMAT_PCM_8_24_BIT) {
            *pcm_format = SHFTR_IN_Q1P15_OUT_Q9P23;
        } else if (target == AUDIO_FORMAT_PCM_32_BIT) {
            *pcm_format = SHFTR_IN_Q1P15_OUT_Q1P31;
        }
    } else if (source == AUDIO_FORMAT_PCM_8_24_BIT) {
        if (target == AUDIO_FORMAT_PCM_16_BIT) {
            *pcm_format = SHFTR_IN_Q9P23_OUT_Q1P15;
        } else if (target == AUDIO_FORMAT_PCM_32_BIT) {
            *pcm_format = SHFTR_IN_Q9P23_OUT_Q1P31;
        }
    } else if (source == AUDIO_FORMAT_PCM_32_BIT) {
        if (target == AUDIO_FORMAT_PCM_16_BIT) {
            *pcm_format = SHFTR_IN_Q1P31_OUT_Q1P15;
        } else if (target == AUDIO_FORMAT_PCM_8_24_BIT) {
            *pcm_format = SHFTR_IN_Q1P31_OUT_Q9P23;
        }
    }
    ALOGV("%s(), pcm_format %d", __FUNCTION__, *pcm_format);

    return ret;
}


static int aud_bcv_init(
    struct aud_fmt_cfg_t *source,
    struct aud_fmt_cfg_t *target,
    struct aud_bcv_hdl_t *bcv_hdl)
{
    int ret = 0;

    if (!source || !target || !bcv_hdl) {
        ALOGW("%s(), %p %p %p NULL!!", __FUNCTION__,
              source, target, bcv_hdl);
        return -EINVAL;
    }

    memset(bcv_hdl, 0, sizeof(struct aud_bcv_hdl_t));

    /* only support bit convert here */
    if (source->num_channels != target->num_channels ||
        source->sample_rate  != target->sample_rate) {
        ALOGW("%s(), (%u %u), (%u %u)!!", __FUNCTION__,
              source->num_channels, target->num_channels,
              source->sample_rate,  target->sample_rate);
        return -EINVAL;
    }


    /* get pcm format */
    ret = get_bcv_pcm_format(source->audio_format, target->audio_format,
                             &bcv_hdl->pcm_format, &bcv_hdl->need_bcv);
    if (ret != 0) {
        return ret;
    }
    if (bcv_hdl->need_bcv == false) {
        return 0;
    }

    /* for alloc blisrc_out dynamically */
    bcv_hdl->buf_times = get_times_of_buffer(source, target);
    bcv_hdl->size_per_frame = get_size_per_frame(target);

    return 0;
}


static int aud_bcv_process(void *buf_in, uint32_t size_in,
                           void **buf_out, uint32_t *size_out,
                           struct aud_bcv_hdl_t *bcv_hdl)
{
    uint32_t expect_size = 0;

    uint32_t raw_size = 0;
    uint32_t bcv_size = 0;

    int ret = 0;


    if (!buf_in || size_in == 0 ||
        !buf_out || !size_out ||
        !bcv_hdl) {
        ALOGW("%s(), %p %u %p %p %p fail!!", __FUNCTION__,
              buf_in, size_in, buf_out, size_out, bcv_hdl);
        return -EINVAL;
    }

    if (bcv_hdl->need_bcv == false) { /* No need BCV */
        *buf_out = buf_in;
        *size_out = size_in;
        return 0;
    }

    /* get expect src out size */
    expect_size = get_cvt_size(size_in,
                               bcv_hdl->buf_times,
                               bcv_hdl->size_per_frame);

    /* make sure src out buf size is enough */
    ret = dynamic_change_buf_size(&bcv_hdl->bcv_out,
                                  &bcv_hdl->bcv_out_size,
                                  expect_size);
    if (ret != 0) {
        return ret;
    }

    /* do blisrc */
    raw_size = size_in;
    bcv_size = bcv_hdl->bcv_out_size; /* tell lib max avail out buf size */
    ret = Shifter_Process(buf_in, &raw_size,
                          bcv_hdl->bcv_out, &bcv_size,
                          bcv_hdl->pcm_format);
#if 0 /* lib not clean... */
    if (raw_size > 0) {
        ALOGE("%s(), raw_size %u > 0!! bcv_size = %u",
              __FUNCTION__, raw_size, bcv_size);
    }
#endif

    if (bcv_size != expect_size) {
        ALOGW("%s(), %u => %u", __FUNCTION__, bcv_size, expect_size);
    }

    /* update out */
    *buf_out = bcv_hdl->bcv_out;
    *size_out = bcv_size;

    conv_dbg("%s(), %u -> %u", __FUNCTION__, size_in, *size_out);

    return 0;
}


static int aud_bcv_deinit(struct aud_bcv_hdl_t *bcv_hdl)
{
    if (!bcv_hdl) {
        ALOGW("%s(), %p NULL!!", __FUNCTION__, bcv_hdl);
        return -EINVAL;
    }

    if (bcv_hdl->bcv_out) {
        AUDIO_FREE(bcv_hdl->bcv_out);
        bcv_hdl->bcv_out = NULL;
    }

    return 0;
}



/*
 * =============================================================================
 *                     API
 * =============================================================================
 */

int aud_fmt_conv_create(struct aud_fmt_cfg_t *source,
                        struct aud_fmt_cfg_t *target,
                        void                **handler)
{
    struct aud_fmt_conv_hdl_t *conv_hdl = NULL;

    int ret = 0;


    if (!source || !target || !handler) {
        ALOGW("%s(), %p %p %p NULL!!", __FUNCTION__, source, target, handler);
        return -EINVAL;
    }

    if (source->sample_rate  == target->sample_rate &&
        source->num_channels == target->num_channels &&
        source->audio_format == target->audio_format) {
        *handler = NULL;
        return 0;
    }


    /* alloc buf */
    conv_hdl = (struct aud_fmt_conv_hdl_t *)AUDIO_MALLOC(SIZE_OF_AUD_BUF_CONV_HDL);
    if (!conv_hdl) {
        ALOGW("%s(), conv_hdl malloc fail!!", __FUNCTION__);
        ret = -EINVAL;
        goto FMT_CONV_DONE;
    }
    memset((void *)conv_hdl, 0, SIZE_OF_AUD_BUF_CONV_HDL);

    /* copy config */
    memcpy((void *)&conv_hdl->source, source, sizeof(struct aud_fmt_cfg_t));
    memcpy((void *)&conv_hdl->target, target, sizeof(struct aud_fmt_cfg_t));

    /* init blisrc */
    ret = aud_src_init(&conv_hdl->source,
                       &conv_hdl->target,
                       &conv_hdl->after_src,
                       &conv_hdl->src_hdl);
    if (ret != 0) {
        aud_fmt_conv_destroy(conv_hdl);
        goto FMT_CONV_DONE;
    }

    /* init bit converter */
    ret = aud_bcv_init(&conv_hdl->after_src,
                       &conv_hdl->target,
                       &conv_hdl->bcv_hdl);
    if (ret != 0) {
        aud_fmt_conv_destroy(conv_hdl);
        goto FMT_CONV_DONE;
    }

    struct dvfs_swip_config_t dvfs_config_src;
    struct dvfs_swip_config_t dvfs_config_bcv;
    memset(&dvfs_config_src, 0, sizeof(struct dvfs_swip_config_t));
    memset(&dvfs_config_bcv, 0, sizeof(struct dvfs_swip_config_t));

    trans_dvfs_cfg_src(&conv_hdl->source, &conv_hdl->target, &dvfs_config_src);
    trans_dvfs_cfg_bcv(&conv_hdl->source, &conv_hdl->target, &dvfs_config_bcv);
    dvfs_add_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SRC, &dvfs_config_src);
    dvfs_add_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SHIFTER, &dvfs_config_bcv);

    *handler = (void *)conv_hdl;

    ALOGV("%s(), sample_rate: %u => %u, num_channels: %d => %d, audio_format: 0x%x => 0x%x"
          " SRC: pcm_format %u, work %u, tmp %u, buf_times %lf, frame sz %u"
          " BCV: pcm_format %u, buf_times %lf, frame sz %u, ret 0x%x, hdl %p",
          __FUNCTION__,
          source->sample_rate,  target->sample_rate,
          source->num_channels, target->num_channels,
          source->audio_format, target->audio_format,
          conv_hdl->src_hdl.lib_param.PCM_Format,
          conv_hdl->src_hdl.working_buf_size,
          conv_hdl->src_hdl.lib_tmp_buf_size,
          conv_hdl->src_hdl.buf_times,
          conv_hdl->src_hdl.size_per_frame,
          conv_hdl->bcv_hdl.pcm_format,
          conv_hdl->bcv_hdl.buf_times,
          conv_hdl->bcv_hdl.size_per_frame,
          ret,
          *handler);


FMT_CONV_DONE:
    return ret;
}


int aud_fmt_conv_process(void  *buf_in,  uint32_t  size_in,
                         void **buf_out, uint32_t *size_out,
                         void  *handler)
{
    struct aud_fmt_conv_hdl_t *conv_hdl = (struct aud_fmt_conv_hdl_t *)handler;

    void *buf_after_src = NULL;
    uint32_t size_after_src = 0;

    int ret = 0;

    if (!buf_in || size_in == 0 ||
        !buf_out || !size_out) {
        ALOGW("%s(), %p %u %p %p %p fail!!", __FUNCTION__,
              buf_in, size_in, buf_out, size_out, conv_hdl);
        return -EINVAL;
    }

    if (!conv_hdl) { /* No need convert */
        *buf_out = buf_in;
        *size_out = size_in;
        return 0;
    }

    AUDIO_CHECK(buf_in);

    /* blisrc */
    ret = aud_src_process(buf_in, size_in,
                          &buf_after_src, &size_after_src,
                          &conv_hdl->src_hdl);
    AUDIO_CHECK(buf_in);
    AUDIO_CHECK(buf_after_src);
    if (ret != 0) {
        return ret;
    }

    /* bit convert */
    ret = aud_bcv_process(buf_after_src, size_after_src,
                          buf_out, size_out,
                          &conv_hdl->bcv_hdl);
    AUDIO_CHECK(buf_in);
    AUDIO_CHECK(buf_after_src);
    AUDIO_CHECK(*buf_out);
    if (ret != 0) {
        return ret;
    }

    return 0;
}


int aud_fmt_conv_destroy(void *handler)
{
    struct aud_fmt_conv_hdl_t *conv_hdl = (struct aud_fmt_conv_hdl_t *)handler;

    if (!conv_hdl) {
        return 0;
    }

    struct dvfs_swip_config_t dvfs_config_src;
    struct dvfs_swip_config_t dvfs_config_bcv;
    memset(&dvfs_config_src, 0, sizeof(struct dvfs_swip_config_t));
    memset(&dvfs_config_bcv, 0, sizeof(struct dvfs_swip_config_t));

    trans_dvfs_cfg_src(&conv_hdl->source, &conv_hdl->target, &dvfs_config_src);
    trans_dvfs_cfg_bcv(&conv_hdl->source, &conv_hdl->target, &dvfs_config_bcv);
    dvfs_delete_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SRC, &dvfs_config_src);
    dvfs_delete_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SHIFTER, &dvfs_config_bcv);

    /* deinit bit convert */
    aud_bcv_deinit(&conv_hdl->bcv_hdl);

    /* deinit src */
    aud_src_deinit(&conv_hdl->src_hdl);

    /* deinit handler */
    if (conv_hdl != NULL) {
        AUDIO_FREE(conv_hdl);
        conv_hdl = NULL;
    }

    return 0;
}



/*
 * =============================================================================
 *                     for dlopen & dlsym
 * =============================================================================
 */

int link_aud_fmt_conv_api(struct aud_fmt_conv_api_t *api)
{
    if (api == NULL) {
        ALOGW("%s(), %p NULL!!", __FUNCTION__, api);
        return -1;
    }

    api->create  = aud_fmt_conv_create;
    api->process = aud_fmt_conv_process;
    api->destroy = aud_fmt_conv_destroy;

    return 0;
}


#ifdef __cplusplus
}  /* extern "C" */
#endif

