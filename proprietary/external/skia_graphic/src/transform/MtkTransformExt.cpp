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

#include "MtkTransformExt.h"

bool needMtkTransformExt(const SkMatrix &m, int w, int h) {
    return isSpecialRotation(m, w, h);
}

bool mtkTransformExt(MtkRotateHandleParam &rotationParam) {
    bool ret = false;
    /*in multi-thread case, dst buffer is segmented by y-direction of dst.
     *so, we only know that if y != 0, there are only 2 cases:
     *(1) src buffer is clip by x-dirction of src.
     *  ---------                 
     * |  |      | clip by y of src               
     * |  |      |                
     * |  |      |                
     *  --------- 
     *(2) dst buffer is segmented by y-direction of dst.
     *  --------                 
     * |--------| segmented by y of dst               
     * |--------|                
     * |--------|                
     *  -------- 
     */
     /*must handle src sample start address in multi-thread case.
      *case 1 don't occur.*/
    if (rotationParam.clipSrcRB == rotationParam.srcRB) {
        if (isTurn90CW(rotationParam.invMatrix, rotationParam.srcOrigH)) {
            rotationParam.srcAddr += rotationParam.y;
        } else if (isTurn270CW(rotationParam.invMatrix, rotationParam.srcOrigW)) {
            rotationParam.srcAddr += rotationParam.dstHInState - rotationParam.y - rotationParam.blitH;
        }            
    } else if (rotationParam.clipSrcRB != rotationParam.srcRB) { //case 1 occur
        if (isTurn90CW(rotationParam.invMatrix, rotationParam.srcOrigH)) {
            rotationParam.srcAddr += rotationParam.y;// 90CW case don't need special handler.
        } else if (isTurn270CW(rotationParam.invMatrix, rotationParam.srcOrigW)) {
            /*same solution as above.*/
            rotationParam.srcAddr += rotationParam.dstHInState - rotationParam.y - rotationParam.blitH;
        } 
    }
    int type = rotationTransformType(rotationParam);
    ret = mtkHandleSpecialRotation(rotationParam, type);
    return ret;
}


