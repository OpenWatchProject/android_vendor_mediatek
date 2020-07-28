#include <audio_pool_buf_handler.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <uthash.h> /* uthash */

#include <wrapped_audio.h>

#include <audio_log_hal.h>
#include <audio_assert.h>
#include <audio_memory_control.h>

#include <arsi_type.h>


#include <audio_fmt_conv.h>



#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AURI][POOL]"


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

#ifdef AURISYS_DUMP_PCM
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

}


void audio_pool_buf_handler_c_file_deinit(void) {

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
    ret = aud_fmt_conv_create(
              &source,
              &target,
              &formatter->aud_fmt_conv_hdl);
    if (ret != 0) {
        AUD_ASSERT(ret == 0);
        return;
    }

#ifdef AURISYS_DUMP_PCM
    sprintf(mDumpFileName, "%s/fmt_conv.%d.%d.%d.source.pcm", audio_dump, mDumpFileNum, getpid(), gettid());
    sprintf(mDumpFileName, "%s/fmt_conv.%d.%d.%d.target.pcm", audio_dump, mDumpFileNum, getpid(), gettid());

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


    AUDIO_CHECK(rb_in->base);
    AUDIO_CHECK(rb_out->base);
    AUDIO_CHECK(formatter->linear_buf);


    data_count = audio_ringbuf_count(rb_in);
    audio_pool_buf_copy_to_linear(
        &formatter->linear_buf,
        &formatter->linear_buf_size,
        formatter->pool_source,
        data_count);

    aud_fmt_conv_process(
        formatter->linear_buf, data_count,
        &buf_out, &size_out,
        formatter->aud_fmt_conv_hdl);

    audio_pool_buf_copy_from_linear(
        formatter->pool_target, buf_out, size_out);


    AUDIO_CHECK(rb_in->base);
    AUDIO_CHECK(rb_out->base);
    AUDIO_CHECK(formatter->linear_buf);


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

    aud_fmt_conv_destroy(formatter->aud_fmt_conv_hdl);
    formatter->aud_fmt_conv_hdl = NULL;

    AUDIO_FREE_POINTER(formatter->linear_buf);
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

