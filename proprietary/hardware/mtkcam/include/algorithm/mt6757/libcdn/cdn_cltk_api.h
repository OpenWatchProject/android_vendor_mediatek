#ifndef _CLTK_API_H_
#define _CLTK_API_H_

/* cl_channel_type */
#define CLTK_SNORM_INT8                               0x10D0
#define CLTK_SNORM_INT16                              0x10D1
#define CLTK_UNORM_INT8                               0x10D2
#define CLTK_UNORM_INT16                              0x10D3
#define CLTK_UNORM_SHORT_565                          0x10D4
#define CLTK_UNORM_SHORT_555                          0x10D5
#define CLTK_UNORM_INT_101010                         0x10D6
#define CLTK_SIGNED_INT8                              0x10D7
#define CLTK_SIGNED_INT16                             0x10D8
#define CLTK_SIGNED_INT32                             0x10D9
#define CLTK_UNSIGNED_INT8                            0x10DA
#define CLTK_UNSIGNED_INT16                           0x10DB
#define CLTK_UNSIGNED_INT32                           0x10DC
#define CLTK_HALF_FLOAT                               0x10DD
#define CLTK_FLOAT                                    0x10DE
#define CLTK_UNORM_INT24                              0x10DF

typedef enum{
    CLTK_IMAGE_HOST,
    CLTK_IMAGE_DEVICE,
    CLTK_IMAGE_EGL,
    CLTK_IMAGE_GL_BUF,
    CLTK_IMAGE_GL_IMG
} cltk_image_source;

typedef struct __cltk_context * cltk_context;
typedef struct __cltk_kernel * cltk_kernel;
typedef struct __cltk_image * cltk_image;


cltk_context cltk_cdn_init();
cltk_context cltk_cdn_fast_init();
cltk_context cltk_cdn_str_init(const char *cl_src, const char *build_opt);
cltk_context cltk_cdn_src_init(const char* cl_file, const char *build_opt);
cltk_context cltk_cdn_bin_init(const char* bin_file);
int cltk_cdn_end(cltk_context ctx);

cltk_image cltk_cdn_image_alloc(cltk_context ctx, unsigned int width, unsigned height, int unit_size, void *host_buf);
void cltk_cdn_image_release(cltk_image img);

#endif
