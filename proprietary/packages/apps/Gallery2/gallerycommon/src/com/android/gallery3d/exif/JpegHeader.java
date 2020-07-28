/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.gallery3d.exif;

/// M: [FEATURE.MODIFY] Image refocus :change class be public to avoid build fail.@{
public class JpegHeader {
/// @}

    public static final short SOI =  (short) 0xFFD8;
    public static final short APP1 = (short) 0xFFE1;
    public static final short APP0 = (short) 0xFFE0;
    public static final short EOI = (short) 0xFFD9;

    /// M: [FEATURE.ADD]Image refocus :rfs @{
    public static final short APP2 = (short) 0XFFE2;
    public static final short APP3 = (short) 0XFFE3;
    public static final short APP4 = (short) 0XFFE4;
    public static final short APP5 = (short) 0XFFE5;
    public static final short APP6 = (short) 0XFFE6;
    public static final short APP7 = (short) 0XFFE7;
    public static final short APP8 = (short) 0XFFE8;
    public static final short APP9 = (short) 0XFFE9;
    public static final short APP10 = (short) 0XFFEA;
    public static final short APP11 = (short) 0XFFEB;
    public static final short APP12 = (short) 0XFFEC;
    public static final short APP13 = (short) 0XFFED;
    public static final short APP14 = (short) 0XFFEE;
    public static final short APP15 = (short) 0XFFEF;
    /// @}
    /**
     *  SOF (start of frame). All value between SOF0 and SOF15 is SOF marker except for DHT, JPG,
     *  and DAC marker.
     */
    public static final short SOF0 = (short) 0xFFC0;
    public static final short SOF15 = (short) 0xFFCF;
    public static final short DHT = (short) 0xFFC4;
    public static final short JPG = (short) 0xFFC8;
    public static final short DAC = (short) 0xFFCC;

    public static final boolean isSofMarker(short marker) {
        return marker >= SOF0 && marker <= SOF15 && marker != DHT && marker != JPG
                && marker != DAC;
    }
}
