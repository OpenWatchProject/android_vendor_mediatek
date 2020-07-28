#ifndef __SK_BITMAP_PROC_SHADER_EXT_H__
#define __SK_BITMAP_PROC_SHADER_EXT_H__
/*
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created on: 2017-11-30
 * Author: Christ Sun
 */

#include "SkColorPriv.h"
#include "SkPixelRef.h"
#include "SkShaderBase.h"
#include "SkBitmapProcShader.h"

struct SkBitmapProcShaderExt {
    /*must be BitmapProcShaderContext*/
    BitmapProcShaderContext *fCtx;
    int fSrcWidthInPixref;// for extension use
    int fSrcHeightInPixref;

    SkBitmapProcShaderExt(SkShaderBase::Context* ctx,
                                    int srcWidthInPixref,
                                    int srcHeightInPixref)
        : fCtx(static_cast<BitmapProcShaderContext *>(ctx))
        , fSrcWidthInPixref(srcWidthInPixref)
        , fSrcHeightInPixref(srcHeightInPixref) {}
    bool meetMtkExtCondition();
    /*
     * x:      dst blit region start coordinate x
     * y:      dst blit region start coordinate y
     * dstC:   dst buffer start address
     * dstRb:  dst original row bytes
     * blitW:  dst blit region width
     * blitH:  dst blit region height
     */
    bool shadeSpanRect(int x, int y, SkPMColor dstC[], 
                int dstRb, int blitW, int blitH, const SkPixmap& device); 
};
#endif