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

#ifndef __BLIT_PARAM_H__
#define __BLIT_PARAM_H__
#include "SkPixmap.h"
#include "SkXfermodePriv.h"
#include "SkColor.h"
#include "SkBlitRow.h"

struct BlitParam {
	BlitParam(int x, int y, unsigned int *dst,
			int rb, int cnt, int h, const SkPixmap &devPixmap,
			SkPMColor *span, SkXfermode *xfer, SkBlitRow::Proc32 proc)
			: startX(x), startY(y), dstC(dst), rowBytes(rb)
			, count(cnt), height(h), fDevPixmap(devPixmap)
			, fSpan(span), fXfer(xfer), fProc(proc){}
	int startX;
	int startY;
	unsigned int *dstC;
	unsigned int rowBytes;
	unsigned int count;
	unsigned int height;
    const SkPixmap &fDevPixmap;
    SkPMColor  *fSpan;//do not use!!!
    SkXfermode *fXfer;
    SkBlitRow::Proc32 fProc;
};

#endif
