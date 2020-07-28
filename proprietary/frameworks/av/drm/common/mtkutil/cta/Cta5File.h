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

#ifndef __DRM_CTA5_FILE_H__
#define __DRM_CTA5_FILE_H__

#include <drm/drm_framework_common.h>
#include <DrmEngineBase.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <DrmCtaUtil.h>
#include <stdio.h>
#include <utils/KeyedVector.h>

//#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

namespace android
{
#define CTA_KEY_LEN 16
#define CTA_MAX_MIEM_LEN 64
//for read buf
#define CTA_MAX_BUF_LEN 4096
#define CTA_KDF_NUM 2000
//In the future magic field maybe updated
#define CTA_MAGIC_LEN 8
const static char CTA_MAGIC[CTA_MAGIC_LEN] = "CTA5";
const static char CTA_MM_MAGIC[CTA_MAGIC_LEN] = "CTA5MM";
const static char CTA_MIME_PREFIX[] = "application/cta-";
const static unsigned char IV[CTA_KEY_LEN] =
{ 0xFE, 0xAE, 0xBC, 0xC6, 0x16, 0x02, 0xB5, 0xBE, 0xFF, 0x3E, 0xB6, 0x16, 0x19, 0xA2, 0xB9, 0x3E };

class Cta5FileUtil
{
public:
    static uint16_t nettohosts(uint16_t networks);
    static uint32_t nettohostl(uint32_t networkl);
    static uint64_t nettohostll(uint64_t networkll);

    static uint16_t hosttonets(uint16_t hosts);
    static uint32_t hosttonetl(uint32_t hostl);
    static uint64_t hosttonetll(uint64_t hostll);

    /*
     * Get the file full path from fd
     */
    static String8 getPathFromFd(int fd);

    /*
     * check whether is dcf file
     */
    static bool isDcfFile(int fd);

    /**
     * Generate a Uintvar format value from a normal value
     */
    static String8 getUintVarFromNormal(uint32_t normal);
    /**
     * Get normal value from a Uintvar format value
     */
    static uint32_t getNormalFromUintVar(String8 uvar);

    /**
     * Do hmac base on md5
     * @param key The key to do hmac
     * @param msg the message to be digested
     * @param len the message length
     * @param out whose length should be 16, it's stored the digest
     */
    static bool hmac128Md5Digest(const uint8_t *key, const uint8_t *msg, size_t len, uint8_t *out);
    /**
     * Do md5 digest
     *
     * @param msg the message to be digested
     * @param len the message length
     * @param out whose length should be 16, it's stored the digest
     */
    static bool md5Digest(const uint8_t *msg, size_t len, uint8_t *out);
    static bool aes128cbcEncrypt(const uint8_t *in, uint8_t *out, size_t len, const uint8_t *key,
            uint8_t *iv);
    static bool aes128cbcDecrypt(const uint8_t *in, uint8_t *out, size_t len, const uint8_t *key,
            uint8_t *iv);
    static uint32_t wvcrc32(const uint8_t* p_begin, int i_count);
    static uint32_t wvcrc32n(const uint8_t* p_begin, int i_count);
private:
    static const uint32_t CRC32Table[256];
    static uint32_t wvrunningcrc32(const uint8_t* p_begin, int i_count, uint32_t i_crc);

};

struct KeyInfo
{
public:
    const static uint32_t KEYINFO_SIZE = CTA_KEY_LEN + CTA_KEY_LEN + CTA_KEY_LEN + sizeof(uint64_t)
            + sizeof(uint32_t);
private:
    //The has value of user's password
    uint8_t mHash[CTA_KEY_LEN];
    //An encrypted salt value
    //password--> A key named B
    //slat is encrypted with B, the result is ESA
    uint8_t mSalt[CTA_KEY_LEN];
    //An encrypted key
    uint8_t mEKey[CTA_KEY_LEN];
    //The number of key modified
    uint64_t mCount;
    //mHash, mEsa and mCount's crc value
    uint32_t mCrc;

    //The bellow filed is not written to file
    uint8_t mDerivedKey[CTA_KEY_LEN];
    uint8_t mContentKey[CTA_KEY_LEN];
    //The key set by user
    String8 mUserKey;

public:
    KeyInfo();
    KeyInfo(const KeyInfo &keyInfo);
    //Parse keyinfo from a string8
    KeyInfo(String8 keyInfo);
    //convert String8 to a class format
    bool setKeyInfo(const char *keyInfo);
    String8 getKeyInfo();
    /**
     * Derive keyinfo from key
     */
    bool deriveKeyInfo(String8 key);

    bool calculateCRC();

    //Change key, the new key is key
    bool changeKeyInfo(String8 key);

    //Restore keyinfo from file with key
    bool restoreKeyInfo(String8 key);
    /**
     * Check if key is valid
     */
    bool isKeyValid(String8 key);
    uint8_t *getContentKey();
    uint64_t getCount();
    void setCount(uint64_t count);
    void dump();

};

//For a big file case, the structure record the key info.
struct PartialEncryptHeader
{
public:
    const static uint32_t PARTIAL_ENCRYPT_HEADER_SIZE = sizeof(uint64_t) * 4;
protected:
    //The offset of content to be encrypted in file
    uint64_t mClearHeaderOffset;
    //The size of content to be encrypted in file
    uint64_t mClearHeaderLen;
    //The offset of encrypted content in file
    uint64_t mCipherHeaderOffset;
    //The size of encrypted content in file
    uint64_t mCipherHeaderLen;

public:
    PartialEncryptHeader();
    String8 getPartialEncryptHeader();
    bool setPartialEncryptHeader(const char *header);

    uint64_t getClearHeaderOffset();
    uint64_t getClearHeaderLen();
    uint64_t getCipherHeaderOffset();
    uint64_t getCipherHeaderLen();

    bool setClearHeaderOffset(uint64_t offset);
    bool setClearHeaderLen(uint64_t len);
    bool setCipherHeaderOffset(uint64_t offset);
    bool setCipherHeaderLen(uint64_t len);

    void dump();
};

class Cta5FileHeader
{
//#define CTA5_FILE_HEADER_LEN 304
private:
    //Do construct
    void _init(String8 mimeType, uint64_t dataLen, String8 key, bool needPartialEncrypt);
    String8 calculateHeaderSig();
    void writeFileSig(int fd);

private:
    const static uint8_t HMAC_KEY[MD5_DIGEST_LENGTH];
public:
    Cta5FileHeader(int fd, String8 key);
    //Useful to parse cta file from a file
    //Cta5FileHeader(String8 cta5FileHeader);
    //datalLen is the length of dcf header + dcf content
    Cta5FileHeader(String8 mimeType, uint64_t dataLen, String8 key);
    Cta5FileHeader(String8 mimeType, uint64_t dataLen, String8 key, bool needPartialEncrypt);
    bool setCta5FileHeader(String8 cta5FileHeader);
    String8 calculateFileSig(int fd);
    const static uint32_t CTA5_FILE_HEADER_LEN =
            128 + CTA_MAX_MIEM_LEN + CTA_MAGIC_LEN + 2 * KeyInfo::KEYINFO_SIZE
            + PartialEncryptHeader::PARTIAL_ENCRYPT_HEADER_SIZE
            + sizeof(uint32_t)/*mIsPartialEncrypted*/
            + sizeof(uint64_t) + sizeof(uint64_t)
            + CTA_KEY_LEN + CTA_KEY_LEN;

protected:
    //Reserved for extension
    uint8_t mReserved[128];
    //The mime type
    //such as
    //application/cta-aduio
    //application/cta-video
    //application/cta-image
    uint8_t mMimeType[CTA_MAX_MIEM_LEN];
    //Now the value should be CTA5
    uint8_t mMagic[CTA_MAGIC_LEN];
    KeyInfo mKeyInfo[2];

    //The total size of CTA file
    uint64_t mTotalLen;
    //The CTA file header size
    uint64_t mHeaderLen;

    //If the file is too big, the just do partial encrypt
    //Only if mIsPartialEncrypted is 1, the mPartialEncryptHeader is used
    //Otherwise it should be ignored
    uint32_t mIsPartialEncrypted;
    PartialEncryptHeader mPartialEncryptHeader;

    //The md5 signature of cta5 file header
    uint8_t mHeaderSig[CTA_KEY_LEN];
    //The md5 signature of file except cta5 file header
    uint8_t mContentSig[CTA_KEY_LEN];

    //The bellow filed will not be write to file
    //The deault value is the address of mKeyInfo[0]
    KeyInfo *mCurrentKeyInfo;
    //The deault value is the address of mKeyInfo[1]
    KeyInfo *mNextKeyInfo;
    int mFd = 0;
    //The key set by user
    String8 mUserKey;
public:
    //get the position of real content, the content is encrypted
    uint64_t getContentPos();
    uint64_t getContentLen();
    //If the CTA5 file is partial encrypted, then return true
    bool isPartialEncryptedFile();
    uint64_t getClearHeaderOffset();
    uint64_t getClearHeaderLen();
    uint64_t getCipherHeaderOffset();
    uint64_t getCipherHeaderLen();

    bool setClearHeaderOffset(uint64_t offset);
    bool setClearHeaderLen(uint64_t len);
    bool setCipherHeaderOffset(uint64_t offset);
    bool setCipherHeaderLen(uint64_t len);

    String8 getCtaHeader();
    uint64_t getCtaHeaderLen();
    //static bool isCta5File(int fd);
    String8 getMimeType();
    String8 getOriginalMimeType();
    bool changeKey(String8 oldKey, String8 newKey);
    uint8_t *getContentKey();
    bool isKeyValid(String8 key);
    //return current keyinfo pointer
    KeyInfo *getCurrentKeyInfo();
    //return keyinfo array's address
    KeyInfo *getKeyInofs();
    void dump();
};

/**
 * This class is used to construct a CTA5 file
 * If you want to parse a CTA5 file, the class is your beset choice
 * If you want to Convert a normal file to a CTA5 file, this class is your best choice
 */
class Cta5File
{
private:
    //The bellow 2 field only be used for thread function to decrypt/encrypt
    int mClearFd;
    int mCipherFd;

protected:
    //DcfFile mDcfFile;
    Cta5FileHeader mCta5FileHeader;
    int mFd = 0;
    uint8_t mIv[CTA_KEY_LEN];
    //String8 mKey;
    bool mNeedCancel;

public:
    /**
     * @param fd[in] A Cta file descriptor
     */
    //This constructor is useful when you want to open a CTA5 file and parse it
    //To decrypt a CTA5 file you may need this version
    Cta5File(int fd,String8 key);

    //This constructor is useful when you want to get a Cta5 file format
    //To convert a normal file to a CTA5 file, you may need this version
    Cta5File(String8 mimeType, String8 cid,String8 dcfFlHeaders, uint64_t datatLen, String8 key);

    Cta5File(String8 mimeType, String8 cid, String8 dcfFlHeaders, uint64_t datatLen, String8 key,
            bool needPartialEncrypt);

    Cta5File(String8 mimeType, uint64_t datatLen, String8 key,
            bool needPartialEncrypt);

    static bool isCta5File(int fd);
    static bool isCta5MultimediaFile(int fd);
    static bool isCta5NormalFile(int fd);
    bool isKeyValid(String8 key);
    //Get the key to decrypt/encrypt file
    uint8_t *getKey();
    String8 getMimeType();
    String8 getOriginalMimeType();
    //Get the iv for aes_128_cbc encrypt/decrypt
    uint8_t *getIv();
    //Get the cta5 file's filedescriptor
    int getFd();

    //If the CTA5 file is partial encrypted, then return true
    bool isPartialEncryptedFile();
    uint64_t getClearHeaderOffset();
    uint64_t getClearHeaderLen();
    uint64_t getCipherHeaderOffset();
    uint64_t getCipherHeaderLen();

    bool setClearHeaderOffset(uint64_t offset);
    bool setClearHeaderLen(uint64_t len);
    bool setCipherHeaderOffset(uint64_t offset);
    bool setCipherHeaderLen(uint64_t len);

    String8 getCta5FileHeader();
    String8 calculateFileSig(int fd);

    //DCF interface
    //A binary format complaint with DCF format
    //String8 getDcfHeader();

    //Get cta5 file header + dcf header
    //Now no dcf header
    String8 getHeader();

    uint64_t getContentLen();

    bool cancel();
    bool isNeedCancel();

    //The bellow 4 function only be used for thread function to decrypt/encrypt
    int getClearFd();
    int getCipherFd();
    void setClearFd(int fd);
    void setCipherFd(int fd);
    uint8_t *getContentKey();

    void dump();

    bool notify(const Vector<DrmCtaUtil::Listener> *infoListener, String8 progress);
public:
    virtual ~Cta5File();
    /**
     * Encrypt fd_in to fd_out, fd_out should include the cta5 file header
     * And notify the encrypt progress by calling infoListener
     */
    virtual bool encrypt(int fd_in, int fd_out,const Vector<DrmCtaUtil::Listener> *infoListener) = 0;
    /**
     * Decrypt current cta5 file to fd_out
     * And notify the encrypt progress by calling infoListener
     */
    virtual bool decrypt(int fd_out,const Vector<DrmCtaUtil::Listener> *infoListener) = 0;

    /*
     * This function is used for playing a multimedia decrypted file with pre-decrypt
     */
    virtual int64_t pread(void* buf, uint64_t size, off64_t offset);

    //get the position of real content, the content is encrypted
    virtual uint64_t getContentPos();

    virtual bool changeKey(String8 oldKey, String8 newKey);
/*protected:
    bool notify(const Vector<DrmCtaUtil::Listener> *infoListener, String8 progress);*/

};
}
#endif //__DRM_CTA5_FILE_H__
