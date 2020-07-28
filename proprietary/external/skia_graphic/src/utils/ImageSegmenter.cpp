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

#include "ImageSegmenter.h"
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


ImageSegmenter::ImageSegmenter() : fBPqueue(), fRecycle(), fPartitionNum(0) {}

void ImageSegmenter::partition(const BlitParam &srcBP, unsigned int num) {
	if (num == 0 || hasPartitioned() || srcBP.height < num)
		return;
	/*initial parameter*/
	fPartitionNum = num;
	int subHeight = (int) (srcBP.height / num);
	int subStartY = srcBP.startY;
	unsigned char *subDstC = (unsigned char *)srcBP.dstC;
	BlitParam *average, *last;
	int remainedHeight = 0;

	/*partition source image into (num - 1) parts on average*/
	while (num > 1) {
		//LOGD("subDstC = 0x%p subStartY = %d subHeight = %d\n", subDstC, subStartY, subHeight);
		average = new BlitParam(srcBP.startX, subStartY, (unsigned int *)subDstC,
								srcBP.rowBytes, srcBP.count, subHeight, srcBP.fDevPixmap,
								srcBP.fSpan, srcBP.fXfer, srcBP.fProc);
		fBPqueue.push(average);
		subStartY += subHeight;
		subDstC = (unsigned char *)subDstC + subHeight * srcBP.rowBytes;
		num--;
	}
	/*the remained image become the last part of source image. only recompute remained height.
	 *other parameters have been got in while loop*/
	remainedHeight = srcBP.height - (fPartitionNum - 1) * subHeight;
	//LOGD("subDstC = 0x%p subStartY = %d remainedHeight = %d\n", subDstC, subStartY, remainedHeight);
	last = new BlitParam(srcBP.startX, subStartY, (unsigned int *)subDstC,
                        srcBP.rowBytes, srcBP.count, remainedHeight, srcBP.fDevPixmap,
                        srcBP.fSpan, srcBP.fXfer, srcBP.fProc);
	fBPqueue.push(last);
}

bool ImageSegmenter::hasPartitioned() {
	return !fBPqueue.empty() || !fRecycle.empty();
}

BlitParam *ImageSegmenter::pop() {
	BlitParam *tmp= fBPqueue.front();
	fBPqueue.pop();
    fRecycle.push(tmp);
	return tmp;
}

unsigned int ImageSegmenter::getUnusedSize() {return fBPqueue.size();}

/*debug*/
void ImageSegmenter::showUnusedParam() {
	if (hasPartitioned()) {
		int i = 1;
		BlitParam *tmp;
		queue<BlitParam *> tmpQ;
		/*print*/
		while (!fBPqueue.empty()) {
			LOGD("the %dth param:\n",i);
			tmp = fBPqueue.front();
			LOGD("startX=%d, startY=%d, dstC=0x%p, rowBytes=%d, count=%d, height=%d\n",
					tmp->startX, tmp->startY, tmp->dstC, tmp->rowBytes, tmp->count, tmp->height);
			fBPqueue.pop();
			tmpQ.push(tmp);
			i++;
		}
		LOGD("queue size = %d:\n", fBPqueue.size());

		/*revert*/
		while (!tmpQ.empty()) {
			tmp = tmpQ.front();
			tmpQ.pop();
			fBPqueue.push(tmp);
		}
	} else
		LOGD("unpartitioned!\n");
}

ImageSegmenter::~ImageSegmenter() {
	BlitParam *tmp;
	while (!fRecycle.empty()) {
		tmp = fRecycle.front();
		fRecycle.pop();
		delete tmp;
	}
	/*should not enter this loop.*/
	while (!fBPqueue.empty()) {
		tmp = fBPqueue.front();
		fBPqueue.pop();
		delete tmp;
	}
	LOGD("queue should be empty!: fBPqueue: %s fRecycle = %s\n",
            fBPqueue.empty()?"true":"false", fRecycle.empty()?"true":"false");
}

