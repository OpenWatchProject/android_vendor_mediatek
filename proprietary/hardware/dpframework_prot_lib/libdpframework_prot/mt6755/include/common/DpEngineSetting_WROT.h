#ifndef __DP_ENGINE_SETTING_WROT_H__
#define __DP_ENGINE_SETTING_WROT_H__

#include "DpColorFormat.h"

struct WrotInput {
    uint32_t identifier;
    DpColorFormat colorFormat;
    uint32_t rotationAngle;
    uint32_t WROT_tar_xsize;
    uint32_t FIFOMaxSize;
    uint32_t maxLineCount;
};

struct WrotOutput {
    uint32_t WROT_main_blk_width;
    uint32_t WROT_main_buf_line_num;
};

void calWROTSetting(WrotInput *inParam, WrotOutput *outParam);

uint32_t getWROTFIFOSetting(uint32_t identifier);

#endif