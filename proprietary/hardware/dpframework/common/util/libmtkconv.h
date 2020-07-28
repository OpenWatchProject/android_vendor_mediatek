#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern
int do_b_to_nv12(uint8_t *pO0, uint8_t *pO1, uint8_t *pY, uint8_t *pC,
          size_t bufWidth, size_t bufHeight);

extern
int do_b_to_i420(uint8_t *pO0, uint8_t *pO1, uint8_t *pO2, uint8_t *pY, uint8_t *pC,
          size_t bufWidth, size_t bufHeight);

extern
int do_b_to_yuy2(uint8_t *pO, uint8_t *pY, uint8_t *pC,
          size_t bufWidth, size_t bufHeight);

extern
int do_b_to_xrgb(uint8_t *pO, uint8_t *pY, uint8_t *pC,
          size_t bufWidth, size_t bufHeight);

#if 0
extern
int do_b_to_rgb16(uint8_t *pO, uint8_t *pY, uint8_t *pC,
          size_t bufWidth, size_t bufHeight);
#endif

extern
int do_rgbx_to_nv12(uint8_t *pY, uint8_t *pC, uint8_t *pSrc,
            size_t width, size_t height);

#ifdef __cplusplus
}
#endif
