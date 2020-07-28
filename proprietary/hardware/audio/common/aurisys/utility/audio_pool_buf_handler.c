#include <audio_pool_buf_handler.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <dlfcn.h> /* dlopen & dlsym */

#include <uthash.h> /* uthash */

#include <wrapped_audio.h>

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_memory_control.h>

#include <arsi_type.h>


#include <audio_fmt_conv.h>

/* pcm dump */
#ifdef AURISYS_DUMP_PCM
#include <sys/prctl.h>
#include <AudioAurisysPcmDump.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "audio_pool_buf_handler"


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#if defined(__LP64__)
#define AUDIO_FMT_CONV_LIB_PATH "/vendor/lib64/libaudiofmtconv.so"
#else
#define AUDIO_FMT_CONV_LIB_PATH "/vendor/lib/libaudiofmtconv.so"
#endif

#define LINK_AUD_FMT_CONV_API_NAME "link_aud_fmt_conv_api"


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef int (*link_aud_fmt_conv_api_fp_t)(struct aud_fmt_conv_api_t *api);


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static void *dlopen_handle; /* for dlopen libaudiofmtconv.so */

static link_aud_fmt_conv_api_fp_t link_aud_fmt_conv_api_fp;
static struct aud_fmt_conv_api_t g_fmt_conv_api;


#ifdef AURISYS_DUMP_PCM
static const char *audio_dump = "/data/vendor/audiohal/audio_dump";
static uint32_t mDumpFileNum = 0;
#endif


/*
 * =============================================================================
 *                     utility
 * =============================================================================
 */

static inline void trans_aud_fmt_cfg(struct aud_fmt_cfg_t *des, const audio_buf_t *src) {
    if (!des || !src) {
        return;
    }

    des->audio_format = src->audio_format;
    des->num_channels = src->num_channels;
    des->sample_rate  = src->sample_rate_buffer;
}


/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

void audio_pool_buf_handler_c_file_init(void) {
    char *dlopen_lib_path = NULL;

    /* get dlopen_lib_path */
    if (access(AUDIO_FMT_CONV_LIB_PATH, R_OK) == 0) {
        dlopen_lib_path = AUDIO_FMT_CONV_LIB_PATH;
    } else {
        AUD_LOG_E("%s(), dlopen_lib_path not found!!", __FUNCTION__);
        AUD_ASSERT(dlopen_lib_path != NULL);
        return;
    }

    /* dlopen for libaudiocomponentenginec.so */
    dlopen_handle = dlopen(dlopen_lib_path, RTLD_NOW);
    if (dlopen_handle == NULL) {
        AUD_LOG_E("dlopen(%s) fail(%s)!!", dlopen_lib_path, dlerror());
        AUD_ASSERT(dlopen_handle != NULL);
        return;
    }

    link_aud_fmt_conv_api_fp = (link_aud_fmt_conv_api_fp_t)dlsym(dlopen_handle,
                                                                 LINK_AUD_FMT_CONV_API_NAME);
    if (link_aud_fmt_conv_api_fp == NULL) {
        AUD_LOG_E("dlsym(%s) for %s fail(%s)!!", dlopen_lib_path, LINK_AUD_FMT_CONV_API_NAME, dlerror());
        AUD_ASSERT(link_aud_fmt_conv_api_fp != NULL);
        return;
    }

    link_aud_fmt_conv_api_fp(&g_fmt_conv_api);
}


void audio_pool_buf_handler_c_file_deinit(void) {
    memset(&g_fmt_conv_api, 0, sizeof(struct aud_fmt_conv_api_t));
    if (dlopen_handle != NULL) {
        dlclose(dlopen_handle);
        dlopen_handle = NULL;
        link_aud_fmt_conv_api_fp = NULL;
    }
}


void config_data_buf_by_ringbuf(
    data_buf_t      *data_buf,
    audio_ringbuf_t *ringbuf) {
    data_buf->memory_size = ringbuf->size;
    data_buf->data_size = 0; //audio_ringbuf_count(ringbuf);
    data_buf->p_buffer = (void *)ringbuf->base;
}


void create_pool_buf(
    audio_pool_buf_t *pool_buf,
    audio_buf_t      *audio_buf_pattern,
    const uint32_t    memory_size) {

    if (pool_buf->buf != NULL) {
        AUD_LOG_W("%s(), data_buf_type %d pool_buf->buf != NULL",
                  __FUNCTION__, audio_buf_pattern->data_buf_type);
        return;
    }

    AUDIO_ALLOC_STRUCT(audio_buf_t, pool_buf->buf);

    /* only get the attributes */
    memcpy(pool_buf->buf, audio_buf_pattern, sizeof(audio_buf_t));

    /* real buffer attributes */
    if (memory_size > 0) {
        AUDIO_ALLOC_CHAR_BUFFER(pool_buf->ringbuf.base, memory_size);
        pool_buf->ringbuf.read  = pool_buf->ringbuf.base;
        pool_buf->ringbuf.write = pool_buf->ringbuf.base;
        pool_buf->ringbuf.size  = memory_size;

        config_data_buf_by_ringbuf(
            &pool_buf->buf->data_buf,
            &pool_buf->ringbuf);
    }
}


void destroy_pool_buf(audio_pool_buf_t *audio_pool_buf) {
    AUDIO_FREE_POINTER(audio_pool_buf->ringbuf.base);
    memset(&audio_pool_buf->ringbuf, 0, sizeof(audio_ringbuf_t));

    AUDIO_FREE_POINTER(audio_pool_buf->buf);
}


void dynamic_change_pool_buf_size(
    audio_pool_buf_t *pool_buf,
    uint32_t write_size) {
    uint32_t old_size = 0;

    if (!pool_buf) {
        AUD_LOG_W("%s(), %p fail!!", __FUNCTION__, pool_buf);
        return;
    }
    if (!write_size) {
        return;
    }

    old_size = pool_buf->ringbuf.size;
    dynamic_change_ring_buf_size(&pool_buf->ringbuf, write_size);
    if (old_size != pool_buf->ringbuf.size) {
        config_data_buf_by_ringbuf(
            &pool_buf->buf->data_buf,
            &pool_buf->ringbuf);
    }
}


void audio_pool_buf_copy_from_linear(
    audio_pool_buf_t *pool_buf,
    const void *linear_buf,
    uint32_t count) {
    uint32_t data_count = 0;
    uint32_t free_space = 0;

    if (!pool_buf || !linear_buf) {
        AUD_LOG_W("%s(), %p %p fail!!", __FUNCTION__,
                  pool_buf, linear_buf);
        return;
    }
    if (!count) {
        return;
    }

    dynamic_change_pool_buf_size(pool_buf, count);

#if 1 /* TODO: debug only */
    data_count = audio_ringbuf_count(&pool_buf->ringbuf);
    free_space = audio_ringbuf_free_space(&pool_buf->ringbuf);
    if (free_space < count) {
        AUD_LOG_W("%s(), data %u, free %u, size %u, count %u", __FUNCTION__,
                  data_count, free_space, pool_buf->ringbuf.size, count);
        AUD_ASSERT(free_space >= count);
        count = free_space;
    }
#endif
    audio_ringbuf_copy_from_linear(
        &pool_buf->ringbuf,
        (char *)linear_buf,
        count);
}


void audio_pool_buf_copy_to_linear(
    void **linear_buf, uint32_t *linear_buf_size,
    audio_pool_buf_t *pool_buf,
    uint32_t count) {

    if (!linear_buf || !linear_buf_size || !pool_buf) {
        AUD_LOG_W("%s(), %p %p %p fail!!", __FUNCTION__,
                  linear_buf, linear_buf_size, pool_buf);
        return;
    }
    if (!count) {
        return;
    }

    int ret = dynamic_change_buf_size(linear_buf, linear_buf_size, count);
    if (ret != 0) {
        AUD_ASSERT(ret == 0);
        audio_ringbuf_drop_data(&pool_buf->ringbuf, count);
    } else {
        audio_ringbuf_copy_to_linear(*linear_buf, &pool_buf->ringbuf, count);
    }
}


void audio_pool_buf_copy_from_ringbuf(
    audio_pool_buf_t *pool_buf,
    audio_ringbuf_t *ringbuf,
    uint32_t count) {
    uint32_t data_count = 0;
    uint32_t free_space = 0;

    if (!pool_buf || !ringbuf) {
        AUD_LOG_W("%s(), %p %pfail!!", __FUNCTION__, pool_buf, ringbuf);
        return;
    }
    if (!count) {
        return;
    }

    dynamic_change_pool_buf_size(pool_buf, count);

#if 1 /* TODO: debug only */
    data_count = audio_ringbuf_count(&pool_buf->ringbuf);
    free_space = audio_ringbuf_free_space(&pool_buf->ringbuf);
    if (free_space < count) {
        AUD_LOG_W("%s(), data %u, free %u, size %u, count %u", __FUNCTION__,
                  data_count, free_space, pool_buf->ringbuf.size, count);
        AUD_ASSERT(free_space >= count);
        count = free_space;
    }
#endif
    AUD_ASSERT(count <= audio_ringbuf_count(ringbuf));
    audio_ringbuf_copy_from_ringbuf(
        &pool_buf->ringbuf,
        ringbuf,
        count);
}


void audio_pool_buf_copy_to_ringbuf(
    audio_ringbuf_t *ringbuf,
    audio_pool_buf_t *pool_buf,
    uint32_t count) {
    uint32_t data_count = 0;
    uint32_t free_space = 0;


    if (!ringbuf || !pool_buf) {
        AUD_LOG_W("%s(), %p %pfail!!", __FUNCTION__, ringbuf, pool_buf);
        return;
    }
    if (!count) {
        return;
    }

    dynamic_change_ring_buf_size(ringbuf, count);

#if 1 /* TODO: debug only */
    data_count = audio_ringbuf_count(&pool_buf->ringbuf);
    free_space = audio_ringbuf_free_space(ringbuf);
    if (free_space < count) {
        AUD_LOG_W("%s(), data %u, free %u, size %u, count %u", __FUNCTION__,
                  data_count, free_space, ringbuf->size, count);
        AUD_ASSERT(free_space >= count);
        count = free_space;
    }
#endif
    AUD_ASSERT(count <= audio_ringbuf_count(&pool_buf->ringbuf));
    audio_ringbuf_copy_from_ringbuf(
        ringbuf,
        &pool_buf->ringbuf,
        count);
}


void audio_pool_buf_formatter_init(audio_pool_buf_formatter_t *formatter) {
    struct aud_fmt_cfg_t source;
    struct aud_fmt_cfg_t target;

    int ret = 0;

#ifdef AURISYS_DUMP_PCM
    char mDumpFileName[128];
#endif

    if (formatter == NULL) {
        AUD_LOG_E("formatter == NULL!! return");
        return;
    }

    formatter->linear_buf = NULL;
    formatter->linear_buf_size = 0;

    trans_aud_fmt_cfg(&source, formatter->pool_source->buf);
    trans_aud_fmt_cfg(&target, formatter->pool_target->buf);
    ret = g_fmt_conv_api.create(
              &source,
              &target,
              &formatter->aud_fmt_conv_hdl);
    if (ret != 0) {
        AUD_ASSERT(ret == 0);
        return;
    }

#ifdef AURISYS_DUMP_PCM
    AUDIO_ALLOC_STRUCT(PcmDump_t, formatter->pcm_dump_source);
    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.source.pcm", audio_dump, LOG_TAG, mDumpFileNum, getpid(), gettid());
    InitPcmDump_t(formatter->pcm_dump_source, 32768);
    formatter->pcm_dump_source->AudioOpendumpPCMFile(
        formatter->pcm_dump_source, mDumpFileName);

    AUDIO_ALLOC_STRUCT(PcmDump_t, formatter->pcm_dump_target);
    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.target.pcm", audio_dump, LOG_TAG, mDumpFileNum, getpid(), gettid());
    InitPcmDump_t(formatter->pcm_dump_target, 32768);
    formatter->pcm_dump_target->AudioOpendumpPCMFile(
        formatter->pcm_dump_target, mDumpFileName);

    mDumpFileNum++;
    mDumpFileNum %= MAX_DUMP_NUM;
#endif
}


void audio_pool_buf_formatter_process(audio_pool_buf_formatter_t *formatter) {
    audio_ringbuf_t *rb_in  = NULL;
    audio_ringbuf_t *rb_out = NULL;

    uint32_t data_count = 0;

    void *buf_out = NULL;
    uint32_t size_out = 0;

    if (formatter == NULL) {
        AUD_ASSERT(formatter != NULL);
        return;
    }

    if (formatter->pool_source == NULL || formatter->pool_target == NULL) {
        AUD_ASSERT(formatter->pool_source != NULL);
        AUD_ASSERT(formatter->pool_target != NULL);
        return;
    }


    rb_in  = &formatter->pool_source->ringbuf;
    rb_out = &formatter->pool_target->ringbuf;

    AUD_LOG_V("%s(+), rb_in  data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(rb_in), audio_ringbuf_free_space(rb_in));
    AUD_LOG_VV("%s(+), rb_out data_count %u, free_count %u", __FUNCTION__,
               audio_ringbuf_count(rb_out), audio_ringbuf_free_space(rb_out));


    data_count = audio_ringbuf_count(rb_in);
    audio_pool_buf_copy_to_linear(
        &formatter->linear_buf,
        &formatter->linear_buf_size,
        formatter->pool_source,
        data_count);
#ifdef AURISYS_DUMP_PCM
    if (formatter->pcm_dump_source != NULL &&
        formatter->pcm_dump_source->mFilep != NULL) {
        formatter->pcm_dump_source->AudioDumpPCMData(
            formatter->pcm_dump_source, (void *)formatter->linear_buf, data_count);
    }
#endif

    g_fmt_conv_api.process(
        formatter->linear_buf, data_count,
        &buf_out, &size_out,
        formatter->aud_fmt_conv_hdl);

    audio_pool_buf_copy_from_linear(
        formatter->pool_target, buf_out, size_out);
#ifdef AURISYS_DUMP_PCM
    if (formatter->pcm_dump_target != NULL &&
        formatter->pcm_dump_target->mFilep != NULL) {
        formatter->pcm_dump_target->AudioDumpPCMData(
            formatter->pcm_dump_target, pBufferAfterFmtConv, bytesAfterFmtConv);
    }
#endif


    AUD_LOG_VV("%s(-), rb_in  data_count %u, free_count %u", __FUNCTION__,
               audio_ringbuf_count(rb_in), audio_ringbuf_free_space(rb_in));
    AUD_LOG_V("%s(-), rb_out data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(rb_out), audio_ringbuf_free_space(rb_out));
}


void audio_pool_buf_formatter_deinit(audio_pool_buf_formatter_t *formatter) {
    if (formatter == NULL) {
        AUD_LOG_E("formatter == NULL!! return");
        return;
    }

    g_fmt_conv_api.destroy(formatter->aud_fmt_conv_hdl);
    formatter->aud_fmt_conv_hdl = NULL;

    AUDIO_FREE_POINTER(formatter->linear_buf);


#ifdef AURISYS_DUMP_PCM
    if (formatter->pcm_dump_source != NULL &&
        formatter->pcm_dump_source->mFilep != NULL) {
        formatter->pcm_dump_source->AudioCloseDumpPCMFile(formatter->pcm_dump_source);
    }
    AUDIO_FREE_POINTER(formatter->pcm_dump_source);

    if (formatter->pcm_dump_target != NULL &&
        formatter->pcm_dump_target->mFilep != NULL) {
        formatter->pcm_dump_target->AudioCloseDumpPCMFile(formatter->pcm_dump_target);
    }
    AUDIO_FREE_POINTER(formatter->pcm_dump_target);
#endif
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

