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
#define LOG_TAG "DipUtility"

#include <DipUtility.h>

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h> 
#include <stdio.h>
using namespace std;

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(DipUtility);
// Clear previous define, use our own define.
#undef DIP_UTILITY_VRB
#undef DIP_UTILITY_DBG
#undef DIP_UTILITY_INF
#undef DIP_UTILITY_WRN
#undef DIP_UTILITY_ERR
#undef DIP_UTILITY_AST
#define DIP_UTILITY_VRB(fmt, arg...)        do { if (DipUtility_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DIP_UTILITY_DBG(fmt, arg...)        do { if (DipUtility_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DIP_UTILITY_INF(fmt, arg...)        do { if (DipUtility_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DIP_UTILITY_WRN(fmt, arg...)        do { if (DipUtility_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DIP_UTILITY_ERR(fmt, arg...)        do { if (DipUtility_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DIP_UTILITY_AST(cond, fmt, arg...)  do { if (DipUtility_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


/******************************************************************************
 *
 ******************************************************************************/
bool DrvMkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
        {
            DIP_UTILITY_INF("fail to mkdir [%s]: %d[%s]", path, errno, ::strerror(errno));
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        DIP_UTILITY_ERR("!S_ISDIR");
        return  false;
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool DrvMakePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    char*copypath = strdup(path);
    if  ( copypath == NULL )
    {
        DIP_UTILITY_ERR("fail to copypath [%s]", path);
        return  false;
    }
    char*pp = copypath;
    char*sp;
    while ( ret && 0 != (sp = strchr(pp, '/')) )
    {
        if (sp != pp)
        {
            *sp = '\0';
            ret = DrvMkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (ret) {
        ret = DrvMkdir(path, mode);
    }
    free(copypath);
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool saveToFile(char const* filepath, unsigned char* pBuf, size_t  size)
{
    bool ret = false;
    int fd = -1;

    //
    DIP_UTILITY_DBG("save to %s", filepath);
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if  ( fd < 0 )
    {
        DIP_UTILITY_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        size_t  written = 0;
        int nw = 0, cnt = 0;
        while ( written < size )
        {
            nw = ::write(fd, pBuf+written, size-written);
            if  (nw < 0)
            {
                DIP_UTILITY_ERR(
                    "fail to write %s, write-count:%d, written-bytes:%zu : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        DIP_UTILITY_DBG("write %zu bytes to %s", size, filepath);
    }
    //
    ret = true;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    return  ret;

}

/******************************************************************************
 *
 ******************************************************************************/
bool saveToRegFmtFile(char const* filepath, unsigned char* pBuf, size_t  size)
{
    bool ret = false;
    //int fd = -1;
    FILE* outFile = NULL;

    //
    DIP_UTILITY_DBG("save to %s", filepath);
    //fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    outFile = ::fopen(filepath, "w+");
    if  ( outFile == NULL )
    {
        DIP_UTILITY_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        int* pTmpBuf = (int*)pBuf;
        char* pCharBuf = NULL;
        int divsize = size / 4;
        int dipbase = DIP_A_BASE_HW;
        int remainingsie = (size - (divsize <<2));
        int i = 0;
        for (i=0;i<divsize;i++)
        {
            ::fprintf(outFile, "(0x%08X,0x%08X)\n", dipbase, (*pTmpBuf));
            pTmpBuf++;
            dipbase = dipbase+4;
        }
        if (remainingsie > 0)
        {
            pCharBuf = (char*)pTmpBuf;        
            switch (remainingsie){
                case 1:
                    {
                        ::fprintf(outFile, "(0x%08X,0x%02X)\n", dipbase, (*pCharBuf));                
                    }
                    break;
                case 2:
                    {
                        ::fprintf(outFile, "(0x%08X,0x%02X%02X)\n", dipbase, (*pCharBuf), (*(pCharBuf+1)));                
                    }
                    break;
                case 3:
                    {
                        ::fprintf(outFile, "(0x%08X,0x%02X%02X%02X)\n", dipbase, (*pCharBuf), (*(pCharBuf+1)), (*(pCharBuf+2)));                
                    }
                    break;
                default:
                    {
                        DIP_UTILITY_ERR("error behavior, remainingsie:%d", remainingsie);                
                    }
                    break;
            }
        }
        DIP_UTILITY_DBG("write %zu bytes to %s", size, filepath);
    }
    //
    ret = true;
lbExit:
    //
    if  ( outFile != NULL )
    {
        ::fclose(outFile);
    }
    //
    return  ret;

}


/******************************************************************************
 *
 ******************************************************************************/
bool saveToTextFile(char const* filepath, unsigned char* pBuf, size_t  size)
{
    bool ret = false;
    //int fd = -1;
    FILE* outFile = NULL;

    //
    DIP_UTILITY_DBG("save to %s", filepath);
    //fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    outFile = ::fopen(filepath, "w+");
    if  ( outFile == NULL )
    {
        DIP_UTILITY_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        int* pTmpBuf = (int*)pBuf;
        char* pCharBuf = NULL;
        int divsize = size / 4;
        int remainingsie = (size - (divsize <<2));
        int i = 0;
        for (i=0;i<divsize;i++)
        {
            ::fprintf(outFile, "0x%08X\n", (*pTmpBuf));
            pTmpBuf++;
        }
        if (remainingsie > 0)
        {
            pCharBuf = (char*)pTmpBuf;        
            switch (remainingsie){
                case 1:
                    {
                        ::fprintf(outFile, "0x%02X\n", (*pCharBuf));                
                    }
                    break;
                case 2:
                    {
                        ::fprintf(outFile, "0x%02X%02X\n", (*pCharBuf), (*(pCharBuf+1)));                
                    }
                    break;
                case 3:
                    {
                        ::fprintf(outFile, "0x%02X%02X%02X\n", (*pCharBuf), (*(pCharBuf+1)), (*(pCharBuf+2)));                
                    }
                    break;
                default:
                    {
                        DIP_UTILITY_ERR("error behavior, remainingsie:%d", remainingsie);                
                    }
                    break;
            }
        }
        DIP_UTILITY_DBG("write %zu bytes to %s", size, filepath);
    }
    //
    ret = true;
lbExit:
    //
    if  ( outFile != NULL )
    {
        ::fclose(outFile);
    }
    //
    return  ret;

}
/******************************************************************************
 *
 ******************************************************************************/

