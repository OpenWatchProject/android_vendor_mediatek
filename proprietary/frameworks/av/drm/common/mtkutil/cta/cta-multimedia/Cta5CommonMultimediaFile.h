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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#ifndef __CTA5_COMMON_MULTIMEDIAFILE_H__
#define __CTA5_COMMON_MULTIMEDIAFILE_H__

#include <Cta5File.h>
#include <DrmCtaUtil.h>
#include <DrmCtaMultiMediaUtil.h>
#include <utils/Vector.h>

namespace android
{

/**
 * This class is used to construct a CTA5 common multimedia file
 * If you want to parse a CTA5 file, the class is your beset choice
 * If you want to Convert a normal file to a CTA5 file, this class is your best choice
 * This class is a super class, and it's used for most multimedia fils which only has one header.
 * If you want to convert other multimedia files which have two or more headers,
 * you need to create a new class and implented from Cta5CommonMultimediaFile
 */
class Cta5CommonMultimediaFile : public Cta5File
{
protected:

    // The cta5 multimedia part file header like below:
    // | raw content | cipher | cta header | cta mm header |
    // | raw content | cipher | cta header | header header header ...  size MagicMM |
    class Header  {
    public:
        off64_t clear_header_offset;
        off64_t clear_header_size;
        off64_t cipher_header_offset;
        off64_t cipher_header_size;
    public:
        Header(): clear_header_offset(-1ll), clear_header_size(-1ll), cipher_header_offset(-1ll),
        cipher_header_size(-1ll){}
    };
    //TODO: the max size need to decide
    Vector<Header*> mHeaders;
    uint32_t mmHeaderCount = 0;
    uint8_t mMagicMM[CTA_MAGIC_LEN];
    off64_t mmCtaHeaderSize = 0LL;
    bool isCancelDone = false;

public:
    Cta5CommonMultimediaFile(int fd,String8 key);

    //This constructor is useful when you want to get a Cta5 file format
    //To convert a normal file to a CTA5 file, you may need this version
    Cta5CommonMultimediaFile(String8 mimeType, String8 cid, String8 dcfFlHeaders, uint64_t datatLen, String8 key);

    //Now dcf header is no need
    Cta5CommonMultimediaFile(String8 mimeType, uint64_t datatLen, String8 key);

    bool encryptHeader(int fd, off64_t header_offset, off64_t header_size,
        off64_t& cipher_header_offset, off64_t& cipher_header_size, const Vector<DrmCtaUtil::Listener> *infoListener,
        off64_t total_header_size, off64_t& progress);

    bool decryptHeader(off64_t header_offset, off64_t header_size,
        off64_t& cipher_header_offset, off64_t& cipher_header_size,
        const Vector<DrmCtaUtil::Listener> *infoListener, off64_t total_header_size, off64_t& progress);

    void notifyProgress(off64_t total, off64_t progress, int fd, String8 result, const Vector<DrmCtaUtil::Listener> *infoListener);

    bool recoverClearData(int fd, off64_t original_end_offset);

    bool recoverCipherData(int fd, off64_t header_offset, off64_t header_size,
    off64_t cipher_header_offset, off64_t cipher_header_size);

    bool overwriteClearHeader(int fd, off64_t header_offset, off64_t header_size,
    off64_t cipher_header_offset, off64_t cipher_header_size);

    off64_t getTotalHeaderSize();

    String8 getCtaMultimediaHeader();

    static int verifyCtaMultimediaHeader(int fd);

    bool setCtaMultimediaHeader(int fd);

    ssize_t cipherPread(int fd, void* buffer, ssize_t numBytes, off64_t offset, off64_t cipher_header_offset, off64_t
    cipher_header_size);

    Header* getCipherHeader(off64_t offset);

    static String8 getOriginalMimetype(int fd, const String8 &key);

public:
    virtual ~Cta5CommonMultimediaFile(){
        ALOGD("~Cta5CommonMultimediaFile() clear headers");
        // delete headers
        Vector<Header*>::iterator iter = mHeaders.begin();
        for (;iter != mHeaders.end();) {
            delete *iter;
            iter = mHeaders.erase(iter);
        }
    }
    /**
     * Encrypt fd_in to fd_out, fd_out should include the cta5 file header
     * And notify the encrypt progress by calling infoListener
     */
    virtual bool encrypt(int fd_in, int fd_out,const Vector<DrmCtaUtil::Listener> *infoListener);
    /**
     * Decrypt current cta5 file to fd_out
     * And notify the encrypt progress by calling infoListener
     */
    virtual bool decrypt(int fd_out,const Vector<DrmCtaUtil::Listener> *infoListener);
    /*
     * This function is used for playing a multimedia decrypted file with pre-decrypt
     * Cta5MultimediaFile must implement it
     * return the bytes read, if read fail then return a minus number
     */
    virtual int64_t pread(void* buf, uint64_t size, off64_t offset);

    /*
        * This function is used to parse all main header of specified multimedia files
        * the result is one or more header offset and size
        */
    virtual bool parseHeaders(int fd) = 0;

    /**
     * Get the real content pos
     */
    virtual uint64_t getContentPos();

    virtual bool changeKey(String8 oldKey, String8 newKey);
};
}
#endif //__CTA5_COMMON_MULTIMEDIAFILE_H__
