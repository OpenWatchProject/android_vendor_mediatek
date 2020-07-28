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

#include "MtkRotateHandler.h"
#include <cmath>
#include "MtkCommonTools.h"
#include "SkString.h"

#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "skia", __VA_ARGS__)
#endif

/*      [scale-x    skew-x      trans-x]   [X]   [X']
        [skew-y     scale-y     trans-y] * [Y] = [Y']
        [persp-0    persp-1     persp-2]   [1]   [1 ]
*/

/*      rotate 90CW maps to rotate 270CW in screen coordinate
        [ 0    1     0]   [X]   [X']
        [-1    0     w] * [Y] = [Y']
        [ 0    0     1]   [1]   [1 ]
        
        w must be the src original width without clip

        skia use inverse matrix when blit:
        [0    -1     w]   [X]   [X']
        [1     0     0] * [Y] = [Y']
        [0     0     1]   [1]   [1 ]
*/
bool isTurn270CW(const SkMatrix &m, int w) {
    return std::abs(m.getScaleX() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getSkewX() - (-1.0000f)) < FLT_EPSILON
            && std::abs(m.getTranslateX() - w) < FLT_EPSILON
            && std::abs(m.getSkewY() - 1.0000f) < FLT_EPSILON
            && std::abs(m.getScaleY() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getTranslateY() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getPerspX() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getPerspY() - 0.0000f) < FLT_EPSILON;
}

/*      rotate 270CW maps to rotate 90CW in screen coordinate
        [ 0    -1     h]   [X]   [X']
        [ 1     0     0] * [Y] = [Y']
        [ 0     0     1]   [1]   [1 ]

        h must be the src original height without clip
        but skia use inverse matrix when blit:
        [ 0     1     0]   [X]   [X']
        [-1     0     h] * [Y] = [Y']
        [ 0     0     1]   [1]   [1 ]
*/

bool isTurn90CW(const SkMatrix &m, int h) {
    return std::abs(m.getScaleX() - 0.0000f) < FLT_EPSILON 
            && std::abs(m.getSkewX() - 1.0000f) < FLT_EPSILON
            && std::abs(m.getTranslateX() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getSkewY() - (-1.0000f)) < FLT_EPSILON
            && std::abs(m.getScaleY() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getTranslateY() - h) < FLT_EPSILON
            && std::abs(m.getPerspX() - 0.0000f) < FLT_EPSILON
            && std::abs(m.getPerspY() - 0.0000f) < FLT_EPSILON;
}


/*we must use screen coordinate to handle*/
void rotateSpecialCase_90CW_no_clip(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) {
    /*rotate 90, dstw = srch, dsth = srcw*/
    int srcH = dstW;
    int srcW = dstH;
    int dstSkipRowPixels = 0;
    int srcSkipRowPixels = 0;

    for (int srcy = 0; srcy < srcH; srcy++) {
        /*fill pixels from right to left dues to rotate clockwise.*/
        dstSkipRowPixels = srcH - 1 - srcy;
        for (int srcx = 0; srcx < srcW; srcx++) {
            //dst write along y and src read along x
            dstC[dstSkipRowPixels] = srcAddr[srcx];
            dstSkipRowPixels += dstW;
        }
        srcAddr += srcW;
    }
}

/**************************************************************************
 *  clip orig  srcw                                                       *
 *        read ->                                                         *
 *  ----|------------                      ---------                      *
 * |----| rowbytes   | srch               |         |                     *
 * |    |            |                 -- |---------|-- dsth              *
 * |    |            |                    |<- write |   |                 *
 *  ----|------------                     | dstw    |   \/                *
 *                                        |         |                     *
 *                                        |         |                     *
 *                                        |         |                     *
 *                                        ----------                      *
 **************************************************************************/
void rotateSpecialCase_90CW_common(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) {
    size_t realSrcWidth = srcRb / 4;
    size_t realDstWidth = dstRb / 4;
    /*rotate 90, dstw = srch, dsth = srcw*/
    int srcH = dstW;
    int srcW = dstH;
    int dstSkipRowPixels = 0;

    for (int srcy = 0; srcy < srcH; srcy++) {
        /*fill pixels from right to left dues to rotate clockwise.*/
        dstSkipRowPixels = srcH - 1 - srcy;
        for (int srcx = 0; srcx < srcW; srcx++) {
            //dst write along y and src read along x
            dstC[dstSkipRowPixels] = srcAddr[srcx];
            /*partial update*/
            dstSkipRowPixels += realDstWidth;
        }
        srcAddr += realSrcWidth;
    }
}

void rotateSpecialCase_270CW_no_clip(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) {
    /*rotate 90, dstw = srch, dsth = srcw*/
    int srcH = dstW;
    int srcW = dstH;
    int dstSkipRowPixels = 0;
    const int allPixelNum = dstH * dstW;

    for (int srcy = 0; srcy < srcH; srcy++) {
        dstSkipRowPixels = allPixelNum + srcy;
        for (int srcx = 0; srcx < srcW; srcx++) {
            dstSkipRowPixels -= dstW;//control dst y
            //dst write along -y and src read along +x
            dstC[dstSkipRowPixels] = srcAddr[srcx];
        }
        srcAddr += srcW;
    }
}

void rotateSpecialCase_270CW_common(SkPMColor *srcAddr, int srcRb, SkPMColor dstC[], 
                                    int dstRb, int dstW, int dstH) {
    size_t realSrcWidth = srcRb / 4;
    size_t realDstWidth = dstRb / 4;
    /*rotate 90, dstw = srch, dsth = srcw*/
    int srcH = dstW;
    int srcW = dstH;
    int dstSkipRowPixels = 0;
    const int allPixelNumInDstW = dstH * realDstWidth;

    for (int srcy = 0; srcy < srcH; srcy++) {
        dstSkipRowPixels = allPixelNumInDstW + srcy;
        for (int srcx = 0; srcx < srcW; srcx++) {
            dstSkipRowPixels -= realDstWidth;//control dst y
            //dst write along -y and src read along +x
            dstC[dstSkipRowPixels] = srcAddr[srcx];
        }
        srcAddr += realSrcWidth;
    }
}

bool isSpecialRotation(const SkMatrix &m, int w, int h) {
    return isTurn270CW(m, w) || isTurn90CW(m, h);
}

int rotationTransformType(const MtkRotateHandleParam &param) {    
    int type = 0;
    if (isTurn90CW(param.invMatrix, param.srcOrigH)) {
        type += 1;
        //if (param.srcRB == param.clipSrcRB && param.dstRB == param.clipDstRB)
            //type += 2;
    } else if (isTurn270CW(param.invMatrix, param.srcOrigW)) {
        type += 2;
        //if (param.srcRB == param.clipSrcRB && param.dstRB == param.clipDstRB)
            //type += 2;
    }
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
    switch (type) {
        case 0:
            LOGD("---mtkHandleSpecialRotation: non-rotateSpecialCase\n");
            break;
        case 1:
            LOGD("---mtkHandleSpecialRotation: rotate 90CW common\n");
            break;
        case 2:
            LOGD("---mtkHandleSpecialRotation: rotate 270CW common\n");
            break;
        //case 3:
            //LOGD("---mtkHandleSpecialRotation: rotate 90CW without clip\n");
            //break;
        //case 4:
            //LOGD("---mtkHandleSpecialRotation: rotate 270CW without clip\n");
            //break;
    }
#endif

    return type;
}

bool mtkHandleSpecialRotation(const MtkRotateHandleParam &param, int type) {
    typedef void (*mtkRotateProc)(SkPMColor *, int, SkPMColor *, int, int, int);
    static const mtkRotateProc gMtkRotateProc[5] = {nullptr,
                                                    rotateSpecialCase_90CW_common,
                                                    rotateSpecialCase_270CW_common,
                                                    rotateSpecialCase_90CW_no_clip,
                                                    rotateSpecialCase_270CW_no_clip};
    if (!gMtkRotateProc[type])
        return false;
    gMtkRotateProc[type](param.srcAddr, param.srcRB, param.dstAddr,
                            param.dstRB, param.blitW, param.blitH);
    return true;
}

