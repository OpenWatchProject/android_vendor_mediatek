/*
 * Copyright (C) 2018-2019 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_FMT_H_
#define GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_FMT_H_

// see external/libdrm/include/drm/drm_fourcc.h for color format

#define fourcc_code(a, b, c, d) ((__u32)(a) | ((__u32)(b) << 8) | \
                                ((__u32)(c) << 16) | ((__u32)(d) << 24))

/* color index */
// [7:0] C
#define MMDUMP_FMT_C8 fourcc_code('C', '8', ' ', ' ')

/* 8 bpp Red */
// [7:0] R
#define MMDUMP_FMT_R8 fourcc_code('R', '8', ' ', ' ')

/* 16 bpp Red */
// [15:0] R little endian
#define MMDUMP_FMT_R16 fourcc_code('R', '1', '6', ' ')

/* 16 bpp RG */
// [15:0] R:G 8:8 little endian
#define MMDUMP_FMT_RG88 fourcc_code('R', 'G', '8', '8')
// [15:0] G:R 8:8 little endian
#define MMDUMP_FMT_GR88 fourcc_code('G', 'R', '8', '8')

/* 32 bpp RG */
// [31:0] R:G 16:16 little endian
#define MMDUMP_FMT_RG1616 fourcc_code('R', 'G', '3', '2')
// [31:0] G:R 16:16 little endian
#define MMDUMP_FMT_GR1616 fourcc_code('G', 'R', '3', '2')

/* 8 bpp RGB */
// [7:0] R:G:B 3:3:2
#define MMDUMP_FMT_RGB332 fourcc_code('R', 'G', 'B', '8')
// [7:0] B:G:R 2:3:3
#define MMDUMP_FMT_BGR233 fourcc_code('B', 'G', 'R', '8')

/* 16 bpp RGB */
// [15:0] x:R:G:B 4:4:4:4 little endian
#define MMDUMP_FMT_XRGB4444 fourcc_code('X', 'R', '1', '2')
// [15:0] x:B:G:R 4:4:4:4 little endian
#define MMDUMP_FMT_XBGR4444 fourcc_code('X', 'B', '1', '2')
// [15:0] R:G:B:x 4:4:4:4 little endian
#define MMDUMP_FMT_RGBX4444 fourcc_code('R', 'X', '1', '2')
// [15:0] B:G:R:x 4:4:4:4 little endian
#define MMDUMP_FMT_BGRX4444 fourcc_code('B', 'X', '1', '2')

// [15:0] A:R:G:B 4:4:4:4 little endian
#define MMDUMP_FMT_ARGB4444 fourcc_code('A', 'R', '1', '2')
// [15:0] A:B:G:R 4:4:4:4 little endian
#define MMDUMP_FMT_ABGR4444 fourcc_code('A', 'B', '1', '2')
// [15:0] R:G:B:A 4:4:4:4 little endian
#define MMDUMP_FMT_RGBA4444 fourcc_code('R', 'A', '1', '2')
// [15:0] B:G:R:A 4:4:4:4 little endian
#define MMDUMP_FMT_BGRA4444 fourcc_code('B', 'A', '1', '2')

// [15:0] x:R:G:B 1:5:5:5 little endian
#define MMDUMP_FMT_XRGB1555 fourcc_code('X', 'R', '1', '5')
// [15:0] x:B:G:R 1:5:5:5 little endian
#define MMDUMP_FMT_XBGR1555 fourcc_code('X', 'B', '1', '5')
// [15:0] R:G:B:x 5:5:5:1 little endian
#define MMDUMP_FMT_RGBX5551 fourcc_code('R', 'X', '1', '5')
// [15:0] B:G:R:x 5:5:5:1 little endian
#define MMDUMP_FMT_BGRX5551 fourcc_code('B', 'X', '1', '5')

// [15:0] A:R:G:B 1:5:5:5 little endian
#define MMDUMP_FMT_ARGB1555 fourcc_code('A', 'R', '1', '5')
// [15:0] A:B:G:R 1:5:5:5 little endian
#define MMDUMP_FMT_ABGR1555 fourcc_code('A', 'B', '1', '5')
// [15:0] R:G:B:A 5:5:5:1 little endian
#define MMDUMP_FMT_RGBA5551 fourcc_code('R', 'A', '1', '5')
// [15:0] B:G:R:A 5:5:5:1 little endian
#define MMDUMP_FMT_BGRA5551 fourcc_code('B', 'A', '1', '5')

// [15:0] R:G:B 5:6:5 little endian
#define MMDUMP_FMT_RGB565 fourcc_code('R', 'G', '1', '6')
// [15:0] B:G:R 5:6:5 little endian
#define MMDUMP_FMT_BGR565 fourcc_code('B', 'G', '1', '6')

/* 24 bpp RGB */
// [23:0] R:G:B little endian
#define MMDUMP_FMT_RGB888 fourcc_code('R', 'G', '2', '4')
// [23:0] B:G:R little endian
#define MMDUMP_FMT_BGR888 fourcc_code('B', 'G', '2', '4')

/* 32 bpp RGB */
// [31:0] x:R:G:B 8:8:8:8 little endian
#define MMDUMP_FMT_XRGB8888 fourcc_code('X', 'R', '2', '4')
// [31:0] x:B:G:R 8:8:8:8 little endian
#define MMDUMP_FMT_XBGR8888 fourcc_code('X', 'B', '2', '4')
// [31:0] R:G:B:x 8:8:8:8 little endian
#define MMDUMP_FMT_RGBX8888 fourcc_code('R', 'X', '2', '4')
// [31:0] B:G:R:x 8:8:8:8 little endian
#define MMDUMP_FMT_BGRX8888 fourcc_code('B', 'X', '2', '4')

// [31:0] A:R:G:B 8:8:8:8 little endian
#define MMDUMP_FMT_ARGB8888 fourcc_code('A', 'R', '2', '4')
// [31:0] A:B:G:R 8:8:8:8 little endian
#define MMDUMP_FMT_ABGR8888 fourcc_code('A', 'B', '2', '4')
// [31:0] R:G:B:A 8:8:8:8 little endian
#define MMDUMP_FMT_RGBA8888 fourcc_code('R', 'A', '2', '4')
// [31:0] B:G:R:A 8:8:8:8 little endian
#define MMDUMP_FMT_BGRA8888 fourcc_code('B', 'A', '2', '4')

// [31:0] pre-multiplied A:R:G:B 8:8:8:8 little endian
#define MMDUMP_FMT_PARGB8888 fourcc_code('P', 'R', '2', '4')
// [31:0] pre-multiplied A:B:G:R 8:8:8:8 little endian
#define MMDUMP_FMT_PABGR8888 fourcc_code('P', 'B', '2', '4')
// [31:0] R:G:B:pre-multiplied A 8:8:8:8 little endian
#define MMDUMP_FMT_PRGBA8888 fourcc_code('R', 'P', '2', '4')
// [31:0] B:G:R:pre-multiplied A 8:8:8:8 little endian
#define MMDUMP_FMT_PBGRA8888 fourcc_code('B', 'P', '2', '4')

// [31:0] x:R:G:B 2:10:10:10 little endian
#define MMDUMP_FMT_XRGB2101010 fourcc_code('X', 'R', '3', '0')
// [31:0] x:B:G:R 2:10:10:10 little endian
#define MMDUMP_FMT_XBGR2101010 fourcc_code('X', 'B', '3', '0')
// [31:0] R:G:B:x 10:10:10:2 little endian
#define MMDUMP_FMT_RGBX1010102 fourcc_code('R', 'X', '3', '0')
// [31:0] B:G:R:x 10:10:10:2 little endian
#define MMDUMP_FMT_BGRX1010102 fourcc_code('B', 'X', '3', '0')

// [31:0] A:R:G:B 2:10:10:10 little endian
#define MMDUMP_FMT_ARGB2101010 fourcc_code('A', 'R', '3', '0')
// [31:0] A:B:G:R 2:10:10:10 little endian
#define MMDUMP_FMT_ABGR2101010 fourcc_code('A', 'B', '3', '0')
// [31:0] R:G:B:A 10:10:10:2 little endian
#define MMDUMP_FMT_RGBA1010102 fourcc_code('R', 'A', '3', '0')
// [31:0] B:G:R:A 10:10:10:2 little endian
#define MMDUMP_FMT_BGRA1010102 fourcc_code('B', 'A', '3', '0')

// [31:0] pre-multiplied A:R:G:B 2:10:10:10 little endian
#define MMDUMP_FMT_PARGB2101010 fourcc_code('P', 'R', '3', '0')
// [31:0] pre-multiplied A:B:G:R 2:10:10:10 little endian
#define MMDUMP_FMT_PABGR2101010 fourcc_code('P', 'B', '3', '0')
// [31:0] R:G:B:pre-multiplied A 10:10:10:2 little endian
#define MMDUMP_FMT_PRGBA1010102 fourcc_code('R', 'P', '3', '0')
// [31:0] B:G:R:pre-multiplied A 10:10:10:2 little endian
#define MMDUMP_FMT_PBGRA1010102 fourcc_code('B', 'P', '3', '0')

/* packed YCbCr */
// [31:0] Cr0:Y1:Cb0:Y0 8:8:8:8 little endian
#define MMDUMP_FMT_YUYV fourcc_code('Y', 'U', 'Y', 'V')
// [31:0] Cb0:Y1:Cr0:Y0 8:8:8:8 little endian
#define MMDUMP_FMT_YVYU fourcc_code('Y', 'V', 'Y', 'U')
// [31:0] Y1:Cr0:Y0:Cb0 8:8:8:8 little endian
#define MMDUMP_FMT_UYVY fourcc_code('U', 'Y', 'V', 'Y')
// [31:0] Y1:Cb0:Y0:Cr0 8:8:8:8 little endian
#define MMDUMP_FMT_VYUY fourcc_code('V', 'Y', 'U', 'Y')

// [31:0] A:Y:Cb:Cr 8:8:8:8 little endian
#define MMDUMP_FMT_AYUV fourcc_code('A', 'Y', 'U', 'V')

/*
 * 2 plane RGB + A
 * index 0 = RGB plane, same format as the corresponding non _A8 format has
 * index 1 = A plane, [7:0] A
 */
#define MMDUMP_FMT_XRGB8888_A8 fourcc_code('X', 'R', 'A', '8')
#define MMDUMP_FMT_XBGR8888_A8 fourcc_code('X', 'B', 'A', '8')
#define MMDUMP_FMT_RGBX8888_A8 fourcc_code('R', 'X', 'A', '8')
#define MMDUMP_FMT_BGRX8888_A8 fourcc_code('B', 'X', 'A', '8')
#define MMDUMP_FMT_RGB888_A8   fourcc_code('R', '8', 'A', '8')
#define MMDUMP_FMT_BGR888_A8   fourcc_code('B', '8', 'A', '8')
#define MMDUMP_FMT_RGB565_A8   fourcc_code('R', '5', 'A', '8')
#define MMDUMP_FMT_BGR565_A8   fourcc_code('B', '5', 'A', '8')

/*
 * 2 plane YCbCr
 * index 0 = Y plane, [7:0] Y
 * index 1 = Cr:Cb plane, [15:0] Cr:Cb little endian
 * or
 * index 1 = Cb:Cr plane, [15:0] Cb:Cr little endian
 */
// 2x2 subsampled Cr:Cb plane
#define MMDUMP_FMT_NV12 fourcc_code('N', 'V', '1', '2')
// 2x2 subsampled Cb:Cr plane
#define MMDUMP_FMT_NV21 fourcc_code('N', 'V', '2', '1')
// 2x1 subsampled Cr:Cb plane
#define MMDUMP_FMT_NV16 fourcc_code('N', 'V', '1', '6')
// 2x1 subsampled Cb:Cr plane
#define MMDUMP_FMT_NV61 fourcc_code('N', 'V', '6', '1')
// non-subsampled Cr:Cb plane
#define MMDUMP_FMT_NV24 fourcc_code('N', 'V', '2', '4')
// non-subsampled Cb:Cr plane
#define MMDUMP_FMT_NV42 fourcc_code('N', 'V', '4', '2')

/*
 * 3 plane YCbCr
 * index 0: Y plane, [7:0] Y
 * index 1: Cb plane, [7:0] Cb
 * index 2: Cr plane, [7:0] Cr
 * or
 * index 1: Cr plane, [7:0] Cr
 * index 2: Cb plane, [7:0] Cb
 */
// 4x4 subsampled Cb (1) and Cr (2) planes
#define MMDUMP_FMT_YUV410 fourcc_code('Y', 'U', 'V', '9')
// 4x4 subsampled Cr (1) and Cb (2) planes
#define MMDUMP_FMT_YVU410 fourcc_code('Y', 'V', 'U', '9')
// 4x1 subsampled Cb (1) and Cr (2) planes
#define MMDUMP_FMT_YUV411 fourcc_code('Y', 'U', '1', '1')
// 4x1 subsampled Cr (1) and Cb (2) planes
#define MMDUMP_FMT_YVU411 fourcc_code('Y', 'V', '1', '1')
// 2x2 subsampled Cb (1) and Cr (2) planes
#define MMDUMP_FMT_YUV420 fourcc_code('Y', 'U', '1', '2')
// 2x2 subsampled Cr (1) and Cb (2) planes
#define MMDUMP_FMT_YVU420 fourcc_code('Y', 'V', '1', '2')
// 2x1 subsampled Cb (1) and Cr (2) planes
#define MMDUMP_FMT_YUV422 fourcc_code('Y', 'U', '1', '6')
// 2x1 subsampled Cr (1) and Cb (2) planes
#define MMDUMP_FMT_YVU422 fourcc_code('Y', 'V', '1', '6')
// non-subsampled Cb (1) and Cr (2) planes
#define MMDUMP_FMT_YUV444 fourcc_code('Y', 'U', '2', '4')
// non-subsampled Cr (1) and Cb (2) planes
#define MMDUMP_FMT_YVU444 fourcc_code('Y', 'V', '2', '4')

/* MTK proprietary color format */
#define MMDUMP_FMT_ABGRFP16  fourcc_code('A', 'B', '4', '8')
#define MMDUMP_FMT_PABGRFP16 fourcc_code('P', 'B', '4', '8')

/* color space enum */
enum mmdump_color_space {
    MM_COLORPROFILE_BT601,
    MM_COLORPROFILE_BT709,
    MM_COLORPROFILE_JPEG,
    MM_COLORPROFILE_FULL_BT601 = MM_COLORPROFILE_JPEG,
    MM_COLORPROFILE_BT2020,       // not support for output
    MM_COLORPROFILE_FULL_BT709,   // not support for output
    MM_COLORPROFILE_FULL_BT2020,  // not support for output
};

#endif  // GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_FMT_H_
