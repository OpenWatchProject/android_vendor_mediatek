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

#include "SkBlitterMTAdapterHandler.h"
#include "BlitRunnable.h"
#include "SkBitmapProcShaderExt.h"
#include "MtkCommonTools.h"

#define __ANDROID_DEBUG__
#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_mt"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif

void BlitDirectlyNoSPHandlerMT::handleRequest(BlitRectHandlerParam *handlerParam,
                                                    const int &x, int &y,
                                                    const int &width, int &height) {
#ifdef __MTK_TRACE_MT_BLITTER__
    MtkCommonTools::AutoSystrace autoTrace("BlitDirectlyNoSPHandlerMT");
#endif

    if (!handlerParam) {
        LOGD("---%s fatal error: blit handler param is nullptr\n", __PRETTY_FUNCTION__);
        return;
    }

    void* ctx;
    //auto shadeProc = handlerParam->fShaderContext->asAShadeProc(&ctx);
    bool isLinearGradient = (handlerParam->fShaderContext->fShader.asAGradient(nullptr) == SkShader::kLinear_GradientType);
    SkBitmapProcShaderExt ext(handlerParam->fShaderContext,
                                handlerParam->fSrcWidthInPixref,
                                handlerParam->fSrcHeightInPixref);
    /*the request of more than 800*600 pixels can be handled.*/
    const int limitX = width - x;
    const int limitY = height - y;
    if (!handlerParam->fConstInY && handlerParam->fShadeDirectlyIntoDevice
        && !ext.meetMtkExtCondition()
        && isBootCompleted()
        && limitX >= 800 && limitY >= 600
        && !isLinearGradient) {
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
        LOGD("---%s\n", __PRETTY_FUNCTION__);
#endif
        bool canRunInCoworker = false;
	    const int threadNum = 4;
        int i = 0;
        Runnable *jobArray[threadNum];

		handlerParam->fMTAdaptee->setSegmentedNum(threadNum);
        handlerParam->fMTAdaptee->segmenterBlitRectParam(x, y, handlerParam->fDevice,
                                                 handlerParam->fDeviceRB, width, height,
                                                 handlerParam->fDevPixmap,
                                                 handlerParam->fSpan,
                                                 handlerParam->fXfer,
                                                 handlerParam->fProc);
        while (i < threadNum) {
            jobArray[i] = new BlitDirectlyNoShadeProcJob(handlerParam->fShaderContext,
                                        handlerParam->fMTAdaptee->getImageSegmenter()->pop());
            i++;
        }

        handlerParam->fMTAdaptee->setJobQueue(jobArray);
        /*can we use CoWorker now? if not, we do aosp flow.*/
        canRunInCoworker = handlerParam->fMTAdaptee->blitRectWithMT();
        LOGD("canRunInCoworker = %s\n", canRunInCoworker?"true":"false");
        handlerParam->fMTAdaptee->recycle();
        if (!canRunInCoworker) {
            do {
                handlerParam->fShaderContext->shadeSpan(x, y, handlerParam->fDevice, width);
                y += 1;
                handlerParam->fDevice = (uint32_t*)((char*)handlerParam->fDevice + handlerParam->fDeviceRB);
            } while (--height > 0);
        }
    } else {
#ifdef __MTK_TRACE_MT_BLITTER__
        autoTrace.end();
#endif
        BlitHandler *next = getNextHandler();
        if (next)
            next->handleRequest(handlerParam, x, y, width, height);
    }
}

void BlitSpecial2DHandlerMT::handleRequest(BlitRectHandlerParam *handlerParam,
                                                    const int &x, int &y,
                                                    const int &width, int &height) {
#ifdef __MTK_TRACE_MT_BLITTER__
    MtkCommonTools::AutoSystrace autoTrace("BlitSpecial2DHandlerMT");
#endif

    if (!handlerParam) {
        LOGD("---%s fatal error: blit handler param is nullptr\n", __PRETTY_FUNCTION__);
        return;
    }

    void* ctx;
    //auto shadeProc = handlerParam->fShaderContext->asAShadeProc(&ctx);
    SkBitmapProcShaderExt ext(handlerParam->fShaderContext,
                                handlerParam->fSrcWidthInPixref,
                                handlerParam->fSrcHeightInPixref);
    /*the request of more than 800*600 pixels can be handled.*/
    const int limitX = width - x;
    const int limitY = height - y;
    if (!handlerParam->fConstInY && handlerParam->fShadeDirectlyIntoDevice
   		&& ext.meetMtkExtCondition()
        && isBootCompleted()
        && limitX >= 800 && limitY >= 600) {
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
				LOGD("---%s\n", __PRETTY_FUNCTION__);
#endif

        bool canRunInCoworker = false;
	    const int threadNum = 4;
        int i = 0;
        Runnable *jobArray[threadNum];

		handlerParam->fMTAdaptee->setSegmentedNum(threadNum);
        handlerParam->fMTAdaptee->segmenterBlitRectParam(x, y, handlerParam->fDevice,
                                                 handlerParam->fDeviceRB, width, height,
                                                 handlerParam->fDevPixmap,
                                                 handlerParam->fSpan,
                                                 handlerParam->fXfer,
                                                 handlerParam->fProc);
        while (i < threadNum) {
            jobArray[i] = new BlitSpecial2DJob(ext,
                                                handlerParam->fMTAdaptee->getImageSegmenter()->pop());
            i++;
        }

        handlerParam->fMTAdaptee->setJobQueue(jobArray);
        /*can we use CoWorker now? if not, we do aosp flow.*/
        canRunInCoworker = handlerParam->fMTAdaptee->blitRectWithMT();
        LOGD("canRunInCoworker = %d\n", canRunInCoworker);
        handlerParam->fMTAdaptee->recycle();
        if (!canRunInCoworker) {
            ext.shadeSpanRect(x, y, handlerParam->fDevice,
                                handlerParam->fDeviceRB, width, height,
                                handlerParam->fDevPixmap);
        }
    } else {
#ifdef __MTK_TRACE_MT_BLITTER__
        autoTrace.end();
#endif
        BlitHandler *next = getNextHandler();
        if (next)
            next->handleRequest(handlerParam, x, y, width, height);
    }
}

void BlitIndirectlyNoXferHandlerMT::handleRequest(BlitRectHandlerParam *handlerParam,
                                            const int &x, int &y,
                                            const int &width, int &height) {
#ifdef __MTK_TRACE_MT_BLITTER__
    MtkCommonTools::AutoSystrace autoTrace("BlitIndirectlyNoXferHandlerMT");
#endif

    if (!handlerParam) {
        LOGD("---%s fatal error: blit handler param is nullptr\n", __PRETTY_FUNCTION__);
        return;
    }
    /*the request of more than 800*600 pixels can be handled.*/
    const int limitX = width - x;
    const int limitY = height - y;
    bool isLinearGradient = (handlerParam->fShaderContext->fShader.asAGradient(nullptr) == SkShader::kLinear_GradientType);
    if (!handlerParam->fConstInY && !handlerParam->fShadeDirectlyIntoDevice
        && !handlerParam->fXfer
        && isBootCompleted()
        && limitX >= 800 && limitY >= 600
        && !isLinearGradient) {
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
				LOGD("---%s\n", __PRETTY_FUNCTION__);
#endif

        bool canRunInCoworker = false;
        const int threadNum = 4;
        int i = 0;
        Runnable *jobArray[threadNum];

        handlerParam->fMTAdaptee->setSegmentedNum(threadNum);
        handlerParam->fMTAdaptee->segmenterBlitRectParam(x, y, handlerParam->fDevice,
                                                 handlerParam->fDeviceRB, width, height,
                                                 handlerParam->fDevPixmap,
                                                 handlerParam->fSpan,
                                                 handlerParam->fXfer,
                                                 handlerParam->fProc);
        while (i < threadNum) {
            jobArray[i] = new BlitIndirectlyNoXferJob(handlerParam->fShaderContext,
                                                handlerParam->fMTAdaptee->getImageSegmenter()->pop());
            i++;
        }

        handlerParam->fMTAdaptee->setJobQueue(jobArray);
        /*can we use CoWorker now? if not, we do aosp flow.*/
        canRunInCoworker = handlerParam->fMTAdaptee->blitRectWithMT();
        LOGD("canRunInCoworker = %d\n", canRunInCoworker);
        handlerParam->fMTAdaptee->recycle();
        if (!canRunInCoworker) {
            BlitHandler *next = getNextHandler();
            if (next)
                next->handleRequest(handlerParam, x, y, width, height);
        }
    } else {
#ifdef __MTK_TRACE_MT_BLITTER__
        autoTrace.end();
#endif

        BlitHandler *next = getNextHandler();
        if (next)
            next->handleRequest(handlerParam, x, y, width, height);
    }
}

