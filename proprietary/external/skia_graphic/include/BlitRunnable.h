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

#ifndef __BLIT_RUNNABLE_H__
#define __BLIT_RUNNABLE_H__
#include "SkCoreBlitters.h"
#include "Runnable.h"
#include "ImageSegmenter.h"
#ifdef __MTK_TRACE_MT_BLITTER__
#include "MtkCommonTools.h"
#endif
#include "SkBitmapProcShaderExt.h"
extern "C" {
#include "stdlib.h"
}


class BlitSpecial2DJob: public Runnable {
public:
	BlitSpecial2DJob(SkBitmapProcShaderExt &ext, BlitParam *bp)
				: fExt(ext), fSubBP(bp) {}
	void run() override {
#ifdef __MTK_TRACE_MT_BLITTER__
        MtkCommonTools::AutoSystrace autoTrace("BlitSpecial2DJob");
#endif
		fExt.shadeSpanRect(fSubBP->startX, fSubBP->startY,
										fSubBP->dstC, fSubBP->rowBytes,
										fSubBP->count, fSubBP->height,
										fSubBP->fDevPixmap);
	}
private:
	SkBitmapProcShaderExt &fExt;
	BlitParam *fSubBP;
};

class BlitDirectlyNoShadeProcJob: public Runnable {
public:
	BlitDirectlyNoShadeProcJob(SkShaderBase::Context* context, BlitParam *bp)
				: fShaderContext(context), fSubBP(bp) {}
	void run() override {
#ifdef __MTK_TRACE_MT_BLITTER__
        MtkCommonTools::AutoSystrace autoTrace("BlitDirectlyNoShadeProcJob");
#endif
        int x = fSubBP->startX;
        int y = fSubBP->startY;
        uint32_t *device = fSubBP->dstC;
        int width = fSubBP->count;
        int height = fSubBP->height;
        int deviceRB = fSubBP->rowBytes;
		do {
            fShaderContext->shadeSpan(x, y, device, width);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
	}
private:
	SkShaderBase::Context* fShaderContext;
	BlitParam *fSubBP;
};

class BlitIndirectlyNoXferJob: public Runnable {
public:
	BlitIndirectlyNoXferJob(SkShaderBase::Context* context, BlitParam *bp)
				: fShaderContext(context), fSubBP(bp) {}
	void run() override {
#ifdef __MTK_TRACE_MT_BLITTER__
        MtkCommonTools::AutoSystrace autoTrace("BlitIndirectlyNoXferJob");
#endif       
        int x = fSubBP->startX;
        int y = fSubBP->startY;
        uint32_t *device = fSubBP->dstC;
        int width = fSubBP->count;
        int height = fSubBP->height;
        int deviceRB = fSubBP->rowBytes;

        /*NOTE: CANNOT use fSubBP->fSpan as tmp buffer to cache the
         *to-do proc data! It can lead to issue that one thread is
         *writing to fSubBP->fSpan, but another one is reading data
         *from it.*/
        SkPMColor* tmpSpan = (SkPMColor*)malloc(
                                fSubBP->fDevPixmap.width() * (sizeof(SkPMColor)));
        do {
            fShaderContext->shadeSpan(x, y, tmpSpan, width);
            fSubBP->fProc(device, tmpSpan, width, 255);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
        free(tmpSpan);
	}
private:
	SkShaderBase::Context* fShaderContext;
	BlitParam *fSubBP;
};


#endif
