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

#include "SkBitmapProcShaderExt.h"
#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkShader.h"

#ifdef __MTK_AFFINE_TRANSFORM_EXT__
#include "MtkTransformExt.h"
#endif
#include "MtkCommonTools.h"

#define __ANDROID_DEBUG__
#if defined(__LOCAL_DEBUG__)
#define LOGD(fmt, ...) printf(fmt, __VA_ARGS__)
#elif defined(__ANDROID_DEBUG__)
#define TAG "skia_shader_ext"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif
extern "C" {
    #include <stdio.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <stdlib.h>
}


extern void ClampX_ClampY_nofilter_scale_neon(const SkBitmapProcState& s,
                                                uint32_t xy[], int count, int x, int y);
extern void S32_opaque_D32_nofilter_DX_neon(const SkBitmapProcState& s,
                                                const uint32_t* SK_RESTRICT xy,
                                                int count, SkPMColor* SK_RESTRICT colors);
extern void ClampX_ClampY_nofilter_scale(const SkBitmapProcState& s,
                                                uint32_t xy[], int count, int x, int y);
extern void S32_opaque_D32_nofilter_DX(const SkBitmapProcState& s,
                                                const uint32_t* SK_RESTRICT xy,
                                                int count, SkPMColor* SK_RESTRICT colors);

bool SkBitmapProcShaderExt::meetMtkExtCondition() {
#ifdef __MTK_TRACE_MT_BLITTER__
    MtkCommonTools::AutoSystrace autoTrace("meetMtkExtCondition");
#endif
    const SkBitmapProcState& state = *fCtx->fState;
    SkBitmapProcState::MatrixProc    mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
    bool ret = false;
#ifdef __MTK_AFFINE_TRANSFORM_EXT__
    if (fSrcWidthInPixref != 0 || fSrcHeightInPixref != 0) {
        ret = needMtkTransformExt(state.fInvMatrix, fSrcWidthInPixref, fSrcHeightInPixref);
        if (ret)
            return ret;
    }
#endif

#if defined(__ARM_HAVE_NEON_COMMON)
    if ((ClampX_ClampY_nofilter_scale_neon == mproc) && (S32_opaque_D32_nofilter_DX_neon == sproc)) {
        return true;
    } 
#endif
    return false;

}


bool SkBitmapProcShaderExt::shadeSpanRect(int x, int y, SkPMColor dstC [ ], int dstRb,
                                            int blitW, int blitH, const SkPixmap & device) {
#ifdef __MTK_TRACE_MT_BLITTER__
    MtkCommonTools::AutoSystrace autoTrace("shadeSpanRect");
#endif
    SkBitmapProcState& state = *fCtx->fState;
    SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
    bool ret = false;

#ifdef __MTK_AFFINE_TRANSFORM_EXT__
    if (state.fInvType & SkMatrix::kAffine_Mask) {
        MtkRotateHandleParam rotationParam(x, y, state.fInvMatrix, (SkPMColor*)state.fPixmap.addr(),
                                            state.fPixmap.rowBytes(), state.fPixmap.width() * 4,
                                            state.fPixmap.width(), state.fPixmap.height(),
                                            fSrcWidthInPixref,
                                            fSrcHeightInPixref,
                                            dstC, dstRb, blitW * 4,
                                            device.width(), device.height(), blitW, blitH);
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
        char invMatrixCstr[100];
        rotationParam.invMatrix.toCStr(invMatrixCstr, 100);
        LOGD("---mtkHandleSpecialRotation: \n\
        x(%d) y(%d) dstBlitW(%d) dstBlitH(%d)\n\
        src pixmap width(%d) height(%d) rb(%d)\n\
        dst pixmap width(%d) height(%d) rb(%d)\n\
        srcBitmap pixel ref w(%d) y(%d)\n\
        inverse matrix: \n\
        %s\n",
            rotationParam.x, rotationParam.y, rotationParam.blitW, rotationParam.blitH,
            rotationParam.srcWInState, rotationParam.srcHInState, rotationParam.srcRB,
            rotationParam.dstWInState, rotationParam.dstHInState, rotationParam.dstRB,
            rotationParam.srcOrigW, rotationParam.srcOrigH, invMatrixCstr);
#endif

        ret = mtkTransformExt(rotationParam);
        return ret;

    }
#endif


    /*now here is handle quadrant flow*/
#if defined(__ARM_HAVE_NEON_COMMON)
    if ((ClampX_ClampY_nofilter_scale_neon != mproc) || (S32_opaque_D32_nofilter_DX_neon != sproc)) {
        return false;
    }

    uint32_t *buffer = (uint32_t*)malloc((blitW/2+2) * sizeof(uint32_t));
    if (buffer == NULL) {
        LOGD("insufficient memory in %s", __func__);
        return false;
    }

    SkASSERT(state.fPixmap.addr());

    SkFixed fy;
    const unsigned maxY = state.fPixmap.height() - 1;
    SkPoint pt;

    mproc(state, buffer, blitW, x, y);
    while (blitH > 0) {
        sproc(state, buffer, blitW, dstC);

        y++;
        state.fInvProc(state.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                        SkIntToScalar(y) + SK_ScalarHalf, &pt);
        fy = SkScalarToFixed(pt.fY);
        buffer[0] = SkClampMax((fy) >> 16, maxY);

        dstC = (SkPMColor*)((char*)dstC + dstRb);
        blitH--;
    }

    free(buffer);

    return true;
#else
    return false;
#endif
}