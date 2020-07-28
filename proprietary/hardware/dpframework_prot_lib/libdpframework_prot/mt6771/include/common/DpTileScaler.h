#ifndef __DP_TILE_SCALER_H__
#define __DP_TILE_SCALER_H__

#include "DpColorFormat.h"

#define TILE_SCALER_SUBPIXEL_SHIFT  (20)

typedef enum DP_TILE_SCALER_ALGO_ENUM
{
    // Cannot modify these enum definition
    DP_TILE_SCALER_ALG0_4_TAPS  = 0,    // 4 tap
    DP_TILE_SCALER_ALG0_6_TAPS  = 0,    // 6 tap
    DP_TILE_SCALER_ALG0_SRC_ACC = 1,    // n tap
    DP_TILE_SCALER_ALGO_CUB_ACC = 2,    // 4n tap
    DP_TILE_SCALER_ALGO_6N_CUB_ACC = 2  // 6n tap
} DP_TILE_SCALER_ALGO_ENUM;


extern void backward_4_taps(int32_t outTileStart,
                            int32_t outTileEnd,
                            int32_t outMaxEnd,
                            int32_t coeffStep,
                            int32_t precision,
                            int32_t cropOffset,
                            int32_t cropFraction,
                            int32_t inMaxEnd,
                            int32_t inAlignment,
                            int32_t &inTileStart,
                            int32_t &inTileEnd);


extern void forward_4_taps(int32_t inTileStart,
                           int32_t inTileEnd,
                           int32_t inMaxEnd,
                           int32_t coeffStep,
                           int32_t precision,
                           int32_t cropOffset,
                           int32_t cropSubpixel,
                           int32_t outMaxEnd,
                           int32_t outAlignment,
                           int32_t backOutStart,
                           int32_t outCalOrder,
                           int32_t &outTileStart,
                           int32_t &outTileEnd,
                           int32_t &lumaOffset,
                           int32_t &lumaSubpixel,
                           int32_t &chromaOffset,
                           int32_t &chromaSubpixel);


extern void backward_6_taps(int32_t outTileStart,
                            int32_t outTileEnd,
                            int32_t outMaxEnd,
                            int32_t coeffStep,
                            int32_t precision,
                            int32_t cropOffset,
                            int32_t cropFraction,
                            int32_t inMaxEnd,
                            int32_t inAlignment,
                            int32_t &inTileStart,
                            int32_t &inTileEnd);


extern void forward_6_taps(int32_t inTileStart,
                           int32_t inTileEnd,
                           int32_t inMaxEnd,
                           int32_t coeffStep,
                           int32_t precision,
                           int32_t cropOffset,
                           int32_t cropSubpixel,
                           int32_t outMaxEnd,
                           int32_t outAlignment,
                           int32_t backOutStart,
                           int32_t outCalOrder,
                           int32_t &outTileStart,
                           int32_t &outTileEnd,
                           int32_t &lumaOffset,
                           int32_t &lumaSubpixel,
                           int32_t &chromaOffset,
                           int32_t &chromaSubpixel);


extern void backward_src_acc(int32_t outTileStart,
                             int32_t outTileEnd,
                             int32_t outMaxEnd,
                             int32_t coeffStep,
                             int32_t precision,
                             int32_t cropOffset,
                             int32_t cropFraction,
                             int32_t inMaxEnd,
                             int32_t inAlignment,
                             int32_t &inTileStart,
                             int32_t &inTileEnd);


extern void forward_src_acc(int32_t inTileStart,
                            int32_t inTileEnd,
                            int32_t inMaxEnd,
                            int32_t coeffStep,
                            int32_t precision,
                            int32_t cropOffset,
                            int32_t cropSubpixel,
                            int32_t outMaxEnd,
                            int32_t outAlignment,
                            int32_t backOutStart,
                            int32_t outCalOrder,
                            int32_t &outTileStart,
                            int32_t &outTileEnd,
                            int32_t &lumaOffset,
                            int32_t &lumaSubpixel,
                            int32_t &chromaOffset,
                            int32_t &chromaSubpixel);


extern void backward_cub_acc(int32_t outTileStart,
                             int32_t outTileEnd,
                             int32_t outMaxEnd,
                             int32_t coeffStep,
                             int32_t precision,
                             int32_t cropOffset,
                             int32_t cropFraction,
                             int32_t inMaxEnd,
                             int32_t inAlignment,
                             int32_t &inTileStart,
                             int32_t &inTileEnd);


extern void forward_cub_acc(int32_t inTileStart,
                            int32_t inTileEnd,
                            int32_t inMaxEnd,
                            int32_t coeffStep,
                            int32_t precision,
                            int32_t cropOffset,
                            int32_t cropSubpixel,
                            int32_t outMaxEnd,
                            int32_t outAlignment,
                            int32_t backOutStart,
                            int32_t outCalOrder,
                            int32_t &outTileStart,
                            int32_t &outTileEnd,
                            int32_t &lumaOffset,
                            int32_t &lumaSubpixel,
                            int32_t &chromaOffset,
                            int32_t &chromaSubpixel);

#endif  // __DP_TILE_SCALER_H__
