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

#ifndef __MTK_ROTATE_HANDLER_H__
#define __MTK_ROTATE_HANDLER_H__
#include "SkColorPriv.h"
#include "SkMatrix.h"

bool isTurn270CW(const SkMatrix &m, int w);
bool isTurn90CW(const SkMatrix &m, int h);

void rotateSpecialCase_90CW_no_clip(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH);

void rotateSpecialCase_90CW_common(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH);
void rotateSpecialCase_270CW_no_clip(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) ;

void rotateSpecialCase_270CW_common(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) ;

struct MtkRotateHandleParam {
    MtkRotateHandleParam(int blitX, int blitY,
      SkMatrix m, SkPMColor* sa, int srb, int csrb,
      int swIns, int shIns, int sow, int soh,
      SkPMColor* da, int drb, int cdrb, int dwIns, int dhIns,
      int dstBlitW, int dstBlitH)
    : x(blitX), y(blitY),
      invMatrix(m), srcAddr(sa), srcRB(srb), clipSrcRB(csrb),
      srcWInState(swIns), srcHInState(shIns),
      srcOrigW(sow), srcOrigH(soh),
      dstAddr(da), dstRB(drb), clipDstRB(cdrb),
      dstWInState(dwIns), dstHInState(dhIns), 
      blitW(dstBlitW), blitH(dstBlitH){}
    int x,y;
    SkMatrix invMatrix;
    SkPMColor *srcAddr;//src sample start address, may be the subset start of src.
    int srcRB;
    int clipSrcRB;
    int srcWInState;//after clip
    int srcHInState;
    int srcOrigW;
    int srcOrigH;
    SkPMColor *dstAddr;
    int dstRB;
    int clipDstRB;
    int dstWInState;
    int dstHInState;
    int blitW, blitH;
};
bool isSpecialRotation(const SkMatrix &m, int w, int h);
int rotationTransformType(const MtkRotateHandleParam &param);
bool mtkHandleSpecialRotation(const MtkRotateHandleParam &param, int type);

#endif
