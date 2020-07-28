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

#include "SkBlitterMTAdaptee.h"
#include "APthread.h"
#include "CoWorker.h"

using namespace std;

//#define __ANDROID_DEBUG__
#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_mt"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif

SkBlitterMTAdaptee::SkBlitterMTAdaptee(): fSegmentedRectNum(0)
                                                , fJobConsumedQ()
                                                , fJobRecycledQ()
                                                , fImageSegmenter(nullptr)
                                                {}

/*depends on job number*/
void SkBlitterMTAdaptee::setSegmentedNum(unsigned int num) {fSegmentedRectNum = num;}

void SkBlitterMTAdaptee::segmenterBlitRectParam(int blitX, int blitY, SkPMColor dstC[],
												int dstRb, int blitW, int blitH,
												const SkPixmap &devPixmap,SkPMColor *span, 
												SkXfermode *xfer, SkBlitRow::Proc32 proc) {
    BlitParam originBP(blitX, blitY, dstC, dstRb, blitW, blitH, devPixmap, span, xfer, proc);
	fImageSegmenter = new ImageSegmenter();
	fImageSegmenter->partition(originBP, fSegmentedRectNum);
}

ImageSegmenter *SkBlitterMTAdaptee::getImageSegmenter() {return fImageSegmenter;}

void SkBlitterMTAdaptee::setJobQueue(Runnable *job[]) {
    unsigned int i = 0;
    if (!fSegmentedRectNum)
        return;

    while (i < fSegmentedRectNum) {
        fJobConsumedQ.push(job[i]);
        fJobRecycledQ.push(job[i]);
        i++;
    }
}

bool SkBlitterMTAdaptee::blitRectWithMT() {
    bool success = false;
    CoWorker *cw =  CoWorker::getInstance();
    if (!cw->requestWorker(fSegmentedRectNum))
        return success;
    if (cw->dispatchJobs(&fJobConsumedQ)) {
        success= true;
    }
    return success;
}

void SkBlitterMTAdaptee::recycle() {
    unsigned int i = 0;
    while (i < fSegmentedRectNum) {
        Runnable *tmp = fJobRecycledQ.front();
        fJobRecycledQ.pop();
        delete tmp;
        i++;
    }
    delete fImageSegmenter;
    fImageSegmenter = nullptr;
}
