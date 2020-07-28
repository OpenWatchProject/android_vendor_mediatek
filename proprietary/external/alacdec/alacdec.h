#ifndef _ALACDEC_H_
#define _ALACDEC_H_

#ifdef __cplusplus
extern "C"{
#endif

// Version x.0x
#define CODEC_ALACDEC_MINOR_VER      (0x01)
#define CODEC_ALACDEC_RELEAE_VER     (0x00)

typedef struct
{
    uint8_t *input_buffer;
    int32_t input_buffer_bitaccumulator; /* used so we can do arbitary bit reads */

    int32_t samplesize;
    int32_t numchannels;
    int32_t bytespersample;


    /* buffers */
    int32_t *predicterror_buffer_a;
    int32_t *predicterror_buffer_b;

    int32_t *outputsamples_buffer_a;
    int32_t *outputsamples_buffer_b;

    int32_t *uncompressed_bytes_buffer_a;
    int32_t *uncompressed_bytes_buffer_b;

    /* stuff from setinfo */
    uint32_t setinfo_max_samples_per_frame; /* 0x1000 = 4096 */    /* max samples per frame? */
    uint8_t setinfo_7a; /* 0x00 */
    uint8_t setinfo_sample_size; /* 0x10 */
    uint8_t setinfo_rice_historymult; /* 0x28 */
    uint8_t setinfo_rice_initialhistory; /* 0x0a */
    uint8_t setinfo_rice_kmodifier; /* 0x0e */
    uint8_t setinfo_7f; /* 0x02 */
    uint16_t setinfo_80; /* 0x00ff */
    uint32_t setinfo_82; /* 0x000020e7 */ /* max sample size?? */
    uint32_t setinfo_86; /* 0x00069fe4 */ /* bit rate (avarge)?? */
    uint32_t setinfo_8a_rate; /* 0x0000ac44 */
    /* end setinfo stuff */
}alac_file;

void alac_init(alac_file *alac, uint8_t *inputbuffer);
void alac_deinit(alac_file *alac);
void alac_reset(alac_file *alac);
void alac_decode_frame(alac_file *alac, uint8_t *inbuffer, void *outbuffer, int32_t *outputsize);
uint32_t alac_get_version(void);

#ifdef __cplusplus
}
#endif

#endif

