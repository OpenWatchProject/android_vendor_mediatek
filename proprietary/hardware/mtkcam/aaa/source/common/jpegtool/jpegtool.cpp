/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

#define LOG_TAG "JPEGTOOL"

#include <utils/Log.h>
#include <cutils/properties.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>

#include "jpegtool.h"


#if 1
#define JPEGTOOL_LOGD(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGD("[%s] " fmt, __FUNCTION__,##arg); })
#define JPEGTOOL_WARN(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGW("[%s] " fmt, __FUNCTION__,##arg); })
#define JPEGTOOL_LOGE(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGE("[%s](%s:%d) " fmt, __FUNCTION__,__FILE__,__LINE__,##arg); })
#define system_call(cmd)              ({ int ret;   printf("[SystemCall] %s\n", cmd);   ALOGD("[SystemCall] %s", cmd);   ret = system(cmd);   ALOGD("[SystemCall] ret=%d", ret);   ret; })
#else
#define JPEGTOOL_LOGD(fmt,arg...)      ALOGD("[%s] " fmt, __FUNCTION__,##arg)
#define JPEGTOOL_WARN(fmt,arg...)      ALOGW("[%s] " fmt, __FUNCTION__,##arg)
#define JPEGTOOL_LOGE(fmt,arg...)      ALOGE("[%s](%s:%d) " fmt, __FUNCTION__,__FILE__,__LINE__,##arg)
#define system_call(cmd)              ({ int ret;   ALOGD("[SystemCall] %s", cmd);   ret = system(cmd);   ret; })
#endif


#include <setjmp.h>

extern "C" {
    //#include "jpeglib.h"
#include "jpeglib_alpha.h"
}

enum EncFormat {
    eENC_YUY2_Format,           // YUYV
    eENC_UYVY_Format,           // UYVY
    eENC_NV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
    eENC_NV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane
    eENC_YV12_Format,           /// YUV420, 2x2 subsampled, 3 plan

    eEncFormatCount
};
#define ALIGN(x,a) ((x + a - 1) & ~(a - 1))
#define FLEN 256

MINT32 _get_value(char *&ptr, int &vlu)
{
    char *pstart = ptr;
    vlu = 0;
    while(ptr && (*ptr >= '0') && (*ptr <= '9')){

        int v = (int)(*ptr - '0');
        vlu = 10 * vlu + v;
        ptr++;
    }

    return (ptr != pstart)? 0:-1;
}
MINT32 _get_filename_w_h(char *filename, int &width, int &height)
{
    char *p;
    int ret = -1;

    width = 0;
    height = 0;

    p = filename;
    while( 1 )
    {
        p = strstr(p, "__");
        //int w = 0, h = 0, b = 0, o = 0;

        p+=2;
        if(0 != _get_value(p, width))
            continue;

        if('x' != *p++)
            continue;

        if(0 != _get_value(p, height))
            continue;

        ret = 0;
        break;
    }
    return ret;
}
MINT32 _check_file_size(char *filename, EncFormat fileformat, int &width, int &height, int &uv_width, int &uv_height)
{
    int calcul_filesize = -1;
    int actual_filesize = -1;

    // actual_filesize
    FILE *filehandle = fopen(filename, "rb");
    if(filehandle != NULL)
    {
        if(0 != fseek(filehandle, 0L, SEEK_END)) {JPEGTOOL_LOGE("fseek failed");}
        actual_filesize = ftell(filehandle);
        fclose(filehandle);
    }

    // calcula_filesize
    switch(fileformat){
    case eENC_YV12_Format:  /// YUV420 3P
        {
            int align_list[] = {1, 2, 16, 32, 64, 128};
            for(int i = 0; i < 6; i++)
            {
                int alignment = align_list[i];

                width = ALIGN(width, alignment);
                height = ALIGN(height, alignment);
                uv_width  = (width  + 1) / 2;
                uv_height = (height + 1) / 2;

                calcul_filesize = width * height + 2 * uv_width * uv_height;
                JPEGTOOL_LOGD("actual_filesize=%d, calcul_filesize=%d, (a=%d, w=%d, h=%d, uv_w=%d, uv_h=%d)",
                    actual_filesize, calcul_filesize, alignment, width, height, uv_width, uv_height);

                if(actual_filesize == calcul_filesize)
                {
                    break;
                }
            }
            break;
        }
    default:
        break;
    }

    if(actual_filesize == -1 || calcul_filesize != actual_filesize)
    {
        JPEGTOOL_LOGE("wrong size: actual_filesize=%d, calcul_filesize=%d", actual_filesize, calcul_filesize);
        return -1;
    }
    if(width == 0 || height == 0 || uv_width == 0 || uv_height == 0)
    {
        JPEGTOOL_LOGE("wrong width/height: width=%d, height=%d, uv_width=%d, uv_height=%d", width, height, uv_width, uv_height);
        return -1;
    }
    return 0;
}
bool onSwEncode(const char *jpeg_filename, EncFormat src_format,
    unsigned int src_width, unsigned int src_height,
    unsigned char *srcAddr, unsigned char *srcCb, unsigned char *srcCr,
    unsigned int srcBufStride, unsigned int srcCbCrBufStride)
{
    JPEGTOOL_LOGD("+");
    JPEGTOOL_LOGD("jpeg_filename=%s, src_format=%d", jpeg_filename, src_format);
    JPEGTOOL_LOGD("src_width=%d, src_height=%d", src_width, src_height);
    JPEGTOOL_LOGD("srcAddr=%p, srcCb=%p, srcCr=%p", srcAddr, srcCb, srcCr);
    JPEGTOOL_LOGD("srcBufStride=%d, srcCbCrBufStride=%d", srcBufStride, srcCbCrBufStride);

    int quality  = 100;

    struct jpeg_compress_struct_ALPHA cinfo;
    struct jpeg_error_mgr_ALPHA jerr;

    int i,j;
    int  encLines = 0;
    JSAMPROW_ALPHA y[16],cb[16],cr[16];
    JSAMPARRAY_ALPHA data[3];
    data[0] = y;
    data[1] = cb;
    data[2] = cr;


    /// Step 1: allocate and initialize JPEG compression object
    FILE *jpeg_file;
    jpeg_file = fopen(jpeg_filename, "w");
    if (!jpeg_file) {
        JPEGTOOL_LOGE("open jpeg_file failed");
        return false;
    }

    cinfo.err = jpeg_std_error_ALPHA(&jerr);
    jpeg_create_compress_ALPHA(&cinfo);
    jpeg_stdio_dest_ALPHA(&cinfo, jpeg_file);


    /// Step 2: set parameters for compression
    cinfo.image_width      = src_width;    /* image width and height, in pixels */
    cinfo.image_height     = src_height;
    cinfo.input_components = 3;          /* # of color components per pixel */
    cinfo.in_color_space   = JCS_YCbCr_ALPHA;  /* colorspace of input image */
    jpeg_set_defaults_ALPHA(&cinfo);

    cinfo.raw_data_in = true;
    jpeg_set_quality_ALPHA(&cinfo, quality, true /* limit to baseline-JPEG values */);
    cinfo.dct_method = JDCT_IFAST_ALPHA;

    /// Step 3: specify data destination (eg, a memory)
    //jpeg_mem_dest_ALPHA (&cinfo, (unsigned char **)&fDstAddr, (unsigned long *)&fDstSize);

    if ((eENC_NV21_Format == src_format) ||      /// YUV420
        (eENC_NV12_Format == src_format) ||
        (eENC_YV12_Format == src_format))
    {
        /// supply downsampled data
        cinfo.comp_info[0].h_samp_factor = 2;
        cinfo.comp_info[0].v_samp_factor = 2;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }
    else if ((eENC_YUY2_Format == src_format) ||  /// YUV422
        (eENC_UYVY_Format == src_format))
    {
        /// supply downsampled data
        cinfo.comp_info[0].h_samp_factor = 2;
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }
    else
    {
        JPEGTOOL_LOGE("unsupport format!");
        fclose(jpeg_file);
        return false;
    }

    switch(src_format)
    {
    case eENC_YV12_Format:  /// YUV420 3P
        {
            unsigned char *compY  = (unsigned char *)srcAddr;
            unsigned char *compCb = (unsigned char *)srcCb;
            unsigned char *compCr = (unsigned char *)srcCr;
            if (!compY || !compCb || !compCr)
            {
                JPEGTOOL_LOGE("Input NULL YUV buffer");
                fclose(jpeg_file);
                return false;
            }
            unsigned char *mcuTempY = NULL;
            unsigned char *mcuTempU = NULL;
            unsigned char *mcuTempV = NULL;

            /// Step 4: Start compressor
            jpeg_start_compress_ALPHA(&cinfo, true);

            for (j = 0; j < (int)src_height; j += 16)
            {
                encLines = ((src_height - j) >= 16)? 16 : (src_height - j);

                // Non-MCU align case need to allocate extra buffer
                if (encLines < 16)
                {
                    //zero allocation check
                    if (0 == srcBufStride || 0 == srcCbCrBufStride) {
                        JPEGTOOL_LOGE("wrong stride: srcBufStride=%d, srcCbCrBufStride=%d", srcBufStride, srcCbCrBufStride);
                        fclose(jpeg_file);
                        return false;
                    }
                    mcuTempY = (unsigned char *)malloc(srcBufStride * 16);
                    mcuTempU = (unsigned char *)malloc(srcCbCrBufStride * 8);
                    mcuTempV = (unsigned char *)malloc(srcCbCrBufStride * 8);
                    if (!mcuTempY || !mcuTempU || !mcuTempV)
                    {
                        JPEGTOOL_LOGE("memory allocation failed");
                        if (mcuTempY)
                        {
                            free(mcuTempY);
                        }
                        if (mcuTempU)
                        {
                            free(mcuTempU);
                        }
                        if (mcuTempV)
                        {
                            free(mcuTempV);
                        }
                        fclose(jpeg_file);
                        return false;
                    }

                    memcpy(mcuTempY, compY, srcBufStride * encLines);
                    memcpy(mcuTempU, compCb, srcCbCrBufStride * (encLines >> 1));
                    memcpy(mcuTempV, compCr, srcCbCrBufStride * (encLines >> 1));

                    for (i = 0; i < 16; i++)
                    {
                        if (i < encLines)
                        {
                            y[i] = mcuTempY + i * srcBufStride;
                            if (i%2 == 0)
                            {
                                cb[i/2] = mcuTempU + (i/2) * ( srcCbCrBufStride );
                                cr[i/2] = mcuTempV + (i/2) * ( srcCbCrBufStride );
                            }
                        }
                        // dummy pointer to avoid libjpeg access invalid address
                        else
                        {
                            y[i] = mcuTempY;
                            if (i%2 == 0)
                            {
                                cb[i/2] = mcuTempU;
                                cr[i/2] = mcuTempV;
                            }
                        }
                    }
                }
                else
                {
                    for (i = 0; i < 16; i++)
                    {
                        y[i] = compY + i * srcBufStride;
                        if (i%2 == 0)
                        {
                            cb[i/2] = compCb + (i/2) * ( srcCbCrBufStride );
                            cr[i/2] = compCr + (i/2) * ( srcCbCrBufStride );
                        }
                    }
                }

                jpeg_write_raw_data_ALPHA (&cinfo, data, 16);
                compY  = compY  + encLines * srcBufStride;
                compCb = compCb + (encLines >> 1)  * srcCbCrBufStride;
                compCr = compCr + (encLines >> 1)  * srcCbCrBufStride;

                if (mcuTempY)
                {
                    free(mcuTempY);
                }
                if (mcuTempU)
                {
                    free(mcuTempU);
                }
                if (mcuTempV)
                {
                    free(mcuTempV);
                }
            }
        }
        break;
    default:
        break;
    }

    /// Step 6: Finish compression
    jpeg_finish_compress_ALPHA(&cinfo);

    /// Step 7: release JPEG compression object
    jpeg_destroy_compress_ALPHA(&cinfo);
    fclose(jpeg_file);

    //if (!fIsAddSOI)
    //{
    //   unsigned char* pt = (unsigned char*)fDstAddr;
    //   pt[0] = 0x00;
    //   pt[1] = 0x00;
    //}

    //*encSize = fDstSize;

    JPEGTOOL_LOGD("-");

    return true;
}

#define CANONICAL_FAIL_LENGTH 500
void _get_canonical_filepath(const char *path, char *buf, size_t bufsiz)
{
    if( NULL == path || NULL == buf || 0 >= bufsiz )
    {
        if(buf)
        {
            buf[0] = 0;
        }
        JPEGTOOL_LOGE("wrong input: path=%s, buf=%s, bufsiz=%lu", path, buf, (unsigned long)bufsiz);
        return;
    }
    if( (-1) == readlink(path, buf, bufsiz) )
    {
        JPEGTOOL_LOGE("readlink failed: path=%s, buf=%s, bufsiz=%lu", path, buf, (unsigned long)bufsiz);
        if( NULL == strncpy(buf, path, bufsiz) )
        {
            buf[0] = 0;
            JPEGTOOL_LOGE("strncpy failed: path=%s, buf=%s, bufsiz=%lu", path, buf, (unsigned long)bufsiz);
            return;
        }
    }
    buf[bufsiz-1] = 0;
    JPEGTOOL_LOGD("done: path=%s, buf=%s", path, buf);
}


int main(int argc, char *argv[])
{
    int status = -1;
    if(argc == 3)
    {
        char in_filename[CANONICAL_FAIL_LENGTH];
        char out_filename[CANONICAL_FAIL_LENGTH];
        EncFormat in_format = eENC_YV12_Format;

        _get_canonical_filepath(argv[1], in_filename, CANONICAL_FAIL_LENGTH);
        _get_canonical_filepath(argv[2], out_filename, CANONICAL_FAIL_LENGTH);

        JPEGTOOL_LOGD("input filename = %s", in_filename);
        JPEGTOOL_LOGD("output filename = %s", out_filename);

        // check ???
        // get file format ???

        if (in_format == eENC_YV12_Format)
        {
            int width  = -1;
            int height = -1;
            int uv_width  = -1;
            int uv_height = -1;

            if(0 != _get_filename_w_h(in_filename, width, height))
            {
                JPEGTOOL_LOGE("parse failed: input filename = %s", in_filename);
                goto ERROR;
            }
            uv_width  = (width  + 1) / 2;
            uv_height = (height + 1) / 2;

            int width_file  = width;
            int height_file = height;
            int uv_width_file  = -1;
            int uv_height_file = -1;
            if( 0 != _check_file_size(in_filename, in_format, width_file, height_file, uv_width_file, uv_height_file) )
            {
                JPEGTOOL_LOGE("wrong file size: input filename = %s, file format = %d", in_filename, in_format);
                goto ERROR;
            }

            FILE *in_file;
            //zero allocation checked in _check_file_size()
            unsigned char * y_data  = (unsigned char *)malloc(width * height);
            unsigned char * cb_data = (unsigned char *)malloc(uv_width * uv_height);
            unsigned char * cr_data = (unsigned char *)malloc(uv_width * uv_height);

            in_file = fopen(in_filename, "rb");
            if(!in_file)
            {
                JPEGTOOL_LOGE("open file failed: input filename = %s", in_filename);
                free(y_data);
                free(cb_data);
                free(cr_data);
                goto ERROR;
            }
            if( width_file == width &&
                height_file == height &&
                uv_width_file == uv_width &&
                uv_height_file == uv_height  )
            {
                if((width * height)       != fread( y_data,  1, width * height,       in_file )) {JPEGTOOL_LOGE("fread failed");}
                if((uv_width * uv_height) != fread( cb_data, 1, uv_width * uv_height, in_file )) {JPEGTOOL_LOGE("fread failed");}
                if((uv_width * uv_height) != fread( cr_data, 1, uv_width * uv_height, in_file )) {JPEGTOOL_LOGE("fread failed");}
            }
            else
            {
                int offset_file = 0;
                for(int i=0; i<height; i++)
                {
                    if(0 != fseek ( in_file, i * width_file, SEEK_SET )) {JPEGTOOL_LOGE("fseek failed");}
                    if(width != fread( y_data + i * width, 1, width, in_file )) {JPEGTOOL_LOGE("fread failed");}
                }

                offset_file += width_file * height_file;
                for(int i=0; i<uv_height; i++)
                {
                    if(0 != fseek ( in_file, offset_file + i * uv_width_file, SEEK_SET )) {JPEGTOOL_LOGE("fseek failed");}
                    if(uv_width != fread( cb_data + i * uv_width, 1, uv_width, in_file )) {JPEGTOOL_LOGE("fread failed");}
                }

                offset_file += uv_width_file * uv_height_file;
                for(int i=0; i<uv_height; i++)
                {
                    if(0 != fseek ( in_file, offset_file + i * uv_width_file, SEEK_SET )) {JPEGTOOL_LOGE("fseek failed");}
                    if(uv_width != fread( cr_data + i * uv_width, 1, uv_width, in_file )) {JPEGTOOL_LOGE("fread failed");}
                }
            }

            onSwEncode(out_filename, eENC_YV12_Format,
                width, height,
                y_data, cr_data, cb_data,
                width, uv_width);

            free(y_data);
            free(cb_data);
            free(cr_data);
            fclose(in_file);
        }
    }

    status = 0;
ERROR:
    return status;
}
