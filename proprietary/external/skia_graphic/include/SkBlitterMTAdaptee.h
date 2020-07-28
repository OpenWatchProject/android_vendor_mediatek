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

#ifndef __SKBLITTERMTADPTEE_H__
#define __SKBLITTERMTADPTEE_H__
#include <queue>
#include "Runnable.h"
#include "ImageSegmenter.h"

class SkBlitterMTAdaptee {
public:
    SkBlitterMTAdaptee();
    //void blitRectWithMT(int x, int y, SkPMColor dstC[], int rb, int count, int height);
    /*set segmented rect number*/
    void setSegmentedNum(unsigned int num);
    /*segmenter image into several parts. the number is same to thread number*/
    void segmenterBlitRectParam(int blitX, int blitY, SkPMColor dstC[],
    									int dstRb, int blitW, int blitH,
    									const SkPixmap &devPixmap, SkPMColor *span, 
									    SkXfermode *xfer, SkBlitRow::Proc32 proc);
    /*ImageSegmenter has unblitted sub-image parameters*/
    ImageSegmenter *getImageSegmenter();
    /*push job array into job queue.*/
    void setJobQueue(Runnable *job[]);
    /*if we can use CoWorker, we will finish job with multi-threads. Or we only can use
     *main thread to run job.*/
    bool blitRectWithMT();
    /*delete ImageSegmenter and jobs which have been completed.*/
    void recycle();

private:
    unsigned int fSegmentedRectNum;
    std::queue<Runnable *> fJobConsumedQ;
    std::queue<Runnable *> fJobRecycledQ;
    ImageSegmenter *fImageSegmenter;
};

#endif
