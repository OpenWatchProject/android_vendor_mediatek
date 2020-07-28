#ifndef AUDIO_FMT_CONV_H
#define AUDIO_FMT_CONV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

/* buffer config */
struct aud_fmt_cfg_t {
    uint32_t audio_format;      /* audio_format_t */
    uint32_t num_channels : 4;  /* 1, 2, 3, 4, ..., 15 */
    uint32_t sample_rate  : 19; /* 8000, 16000, 32000, ..., 352800, 384000 */
#if 0
    uint32_t b_interleave : 1;  /* 0: non-interleave, 1: interleave */
    uint32_t frame_size_ms: 8;  /* ex, 20ms; 0: not frame base. [0 ~ 255] */
#endif
};


/*
 * =============================================================================
 *                     API
 * =============================================================================
 */

int aud_fmt_conv_create(struct aud_fmt_cfg_t *source,
                        struct aud_fmt_cfg_t *target,
                        void                **handler);

int aud_fmt_conv_process(void  *buf_in,  uint32_t  size_in,
                         void **buf_out, uint32_t *size_out,
                         void  *handler);

int aud_fmt_conv_destroy(void *handler);


/*
 * =============================================================================
 *                     for dlopen & dlsym
 * =============================================================================
 */

struct aud_fmt_conv_api_t {
    /* see aud_fmt_conv_create() */
    int (*create)(struct aud_fmt_cfg_t *source,
                  struct aud_fmt_cfg_t *target,
                  void                **handler);

    /* see aud_fmt_conv_process() */
    int (*process)(void  *buf_in,  uint32_t  size_in,
                   void **buf_out, uint32_t *size_out,
                   void  *handler);

    /* see aud_fmt_conv_destroy() */
    int (*destroy)(void *handler);
};


/* dlsym this func to link all api */
int link_aud_fmt_conv_api(struct aud_fmt_conv_api_t *api);



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_FMT_CONV_H */

