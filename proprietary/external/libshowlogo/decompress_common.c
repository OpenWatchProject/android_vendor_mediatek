/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#define MTK_LOG_ENABLE 0
#include <stdio.h>
#include <string.h>
#include <cutils/klog.h>
#include "decompress_common.h"

#ifdef SLOGD
#undef SLOGD
//#define SLOGD(x,...) do { KLOG_ERROR("libshowlogo", x); } while (0)
#define SLOGD(...) do { KLOG_ERROR("libshowlogo", __VA_ARGS__); } while (0)
#endif

#ifdef SLOGE
#undef SLOGE
//#define SLOGE(x,...) do { KLOG_ERROR("libshowlogo", x); } while (0)
#define SLOGE(...) do { KLOG_ERROR("libshowlogo", __VA_ARGS__); } while (0)
#endif

/* Decompress zlib compressed file */

int decompress_logo(void *in, void *out, int inlen, int outlen)
{
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[decompress_logo %s %d]in=0x%08x, out=0x%08x, inlen=%d, logolen=%d\n",__FUNCTION__,__LINE__,
                    (unsigned int)in, (unsigned int)out, inlen, outlen);
    }

    int ret;
    int have = 0;
    z_stream strm;

    memset(&strm, 0, sizeof(z_stream));
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[decompress_logo %s %d]inflateInit fail\n",__FUNCTION__,__LINE__);
            (void)inflateEnd(&strm);
        }
        return ret;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = inlen;

        if (strm.avail_in <= 0)
        {
            if (MTK_LOG_ENABLE == 1) {
                SLOGE("[decompress_logo %s %d]strm.avail_in <= 0\n",__FUNCTION__,__LINE__);
            }
            break;
        }
        strm.next_in = (Bytef*)in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = outlen;
            strm.next_out = (Bytef*)out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
             	if (MTK_LOG_ENABLE == 1) {
                    SLOGD("[decompress_logo %s %d]Z_NEED_DICT\n",__FUNCTION__,__LINE__);
                }
            case Z_DATA_ERROR:
            	if (MTK_LOG_ENABLE == 1) {
                    SLOGD("[decompress_logo %s %d]Z_DATA_ERROR\n",__FUNCTION__,__LINE__);
                }
            case Z_MEM_ERROR:
            	if (MTK_LOG_ENABLE == 1) {
                    SLOGD("[decompress_logo %s %d]Z_MEM_ERROR\n",__FUNCTION__,__LINE__);
                }
                (void)inflateEnd(&strm);
                return ret;
            }
            have += outlen - strm.avail_out;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    if (ret == Z_STREAM_END)
    /* clean up and return */
    (void)inflateEnd(&strm);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[decompress_logo %s %d]have=%d\n",__FUNCTION__,__LINE__,have);
    }

    //return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    return have;
}
