#define LOG_TAG "Cta5NormalFile"
#include <utils/Log.h>

#include <Cta5NormalFile.h>
#include <sys/types.h>
#include <unistd.h>
#include <DrmCtaUtil.h>
#include <drm/DrmInfoEvent.h>
#include <CipherFileUtil.h>
#include <CryptoHelper.h>
#include <DrmDef.h>
#include <DrmCtaUtil.h>

using namespace android;
#define BLK_LEN 4096
#define DECRYPT_BUF_LEN 4096
#define ENCRYPT_DECRYPT_COUNT 100
#define CALLBACK_LEN (1024*1024)

#if 0
static uint8_t DEBUG_KEY[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF };
static uint8_t DEBUG_IV[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00 };
#endif

Cta5NormalFile::Cta5NormalFile(int fd, String8 key) :
        Cta5File(fd, key)
{

    lseek64(fd, 0L, SEEK_SET);
    lseek64(fd, 0L, SEEK_END);
    lseek64(fd, 0L, SEEK_SET);
    //read cta header, its length should be 304
    //The first filed is MAGIC
    uint8_t cta5FileHaderBuf[Cta5FileHeader::CTA5_FILE_HEADER_LEN] = { 0 };
    read(fd, cta5FileHaderBuf, Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    String8 cta5FileHaderStr((const char *) cta5FileHaderBuf, Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    mCta5FileHeader.setCta5FileHeader(cta5FileHaderStr);
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5NormalFile::Cta5NormalFile(String8 mimeType, String8 cid, String8 dcfFlHeaders, uint64_t datatLen, String8 key) :
        Cta5File(mimeType, cid, dcfFlHeaders, datatLen, key, false)
{

}

Cta5NormalFile::Cta5NormalFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5File(mimeType, datatLen, key, false)
{

}

bool Cta5NormalFile::encrypt(int fd_in, int fd_out, const Vector<DrmCtaUtil::Listener> *infoListener)
{
    ALOGD("encrypt.");

    CryptoHelper hlp(CryptoHelper::CIPHER_AES128CBC, getContentKey(), 1);
    if (fd_in < 0 || fd_out < 0)
    {
        ALOGE("[ERROR]encrypt: error fd_in=[%d], fd_out=[%d]", fd_in, fd_out);
        return false;
    }
    if (-1 == lseek(fd_in, 0, SEEK_SET))
    {
        ALOGE("[ERROR]encrypt: seek fd_in set error,reason=[%s]", strerror(errno));
        return false;
    }
    int data_size = lseek(fd_in, 0, SEEK_END);
    if (-1 == data_size)
    {
        ALOGE("[ERROR]encrypt: seek fd_in end error,reason=[%s]", strerror(errno));
        return false;
    }
    ALOGD("encrypt: fd_clear=[%d], fd_cipher[%d], data_size=[%d]", fd_in, fd_out, data_size);
    if (0 == data_size)
    {
        ALOGE("[ERROR]encrypt: file length is error: [%d]", data_size);
        return false;
    }
    if (-1 == lseek(fd_in, 0, SEEK_SET))
    {
        ALOGE("[ERROR]encrypt: seek fd_in set error.reason=[%s]", strerror(errno));
        return false;
    }

    int data_offset = 0;
    size_t result = 0;
    // size_t cipher_offset = 0;
    size_t cipher_size = 0;

    //write cta header
    String8 header = getCta5FileHeader();
    uint32_t headerLen = getCta5FileHeader().length();
    result = write(fd_out, header.string(), headerLen);

    // copy the first iv
    unsigned char iv_arr[16];
    bzero(iv_arr, sizeof(iv_arr));
    memcpy(iv_arr, mIv, sizeof(iv_arr));

    //***************
    // write the first iv to cipher header
    result = write(fd_out, iv_arr, sizeof(iv_arr));
    if (result != 16)
    {
        ALOGE("[ERROR]encrypt: failed to write first iv to cipher header.reason=[%s]", strerror(errno));
        return false;
    }
    cipher_size += result;

    // seek to the first byte of clear file
    if (-1 == lseek(fd_in, data_offset, SEEK_SET))
    {
        ALOGE("[ERROR]encrypt: failed to seek to the start of content!reason=[%s]", strerror(errno));
        return false;
    }
    // call back parameters
    String8 path = Cta5FileUtil::getPathFromFd(fd_in);
    String8 callBackStr("");
    char data_size_str[32] = { 0 };
    String8 data_str8("");
    char cnt_size_str[32] = { 0 };
    String8 cnt_str8("");

    // input buffer
    unsigned char buf_in[BLK_LEN];
    bzero(buf_in, sizeof(buf_in));

    // output buffer for encryption
    int desired_len = hlp.desiredOutBufSize(BLK_LEN);
    unsigned char* buf_out = new unsigned char[desired_len];
    bzero(buf_out, (size_t) desired_len);

    // read, encrypt, and write to file
    size_t cnt = 0;
    size_t cnt_total = 0;
    int len_out = 0;
    int encrypt_count = 0;
    int block_size = data_size / ENCRYPT_DECRYPT_COUNT;
    block_size = (block_size > CALLBACK_LEN) ? block_size : CALLBACK_LEN;
    while ((int) cnt_total < data_size)
    {
        bzero(buf_in, sizeof(buf_in));
        bzero(buf_out, (size_t) desired_len);

        cnt = read(fd_in, buf_in, ((data_size - cnt_total) > BLK_LEN) ? BLK_LEN : (data_size - cnt_total));
        cnt_total += cnt;

        if (1 != hlp.doCryption(buf_in, (int) cnt, buf_out, len_out, iv_arr, ((int) cnt_total >= data_size)))
        {
            ALOGE("[ERROR]encrypt: encryption operation failed.");
            goto ERROR;
        }

        // copy next iv
        memcpy(iv_arr, buf_out + len_out - DrmDef::DRM_DCF_IV_LENGTH, sizeof(iv_arr));

        // save to output file
        result = write(fd_out, buf_out, (size_t) len_out);
        if (result != (size_t) len_out)
        {
            ALOGE("[ERROR]encrypt: failed to write data to output file.");
            goto ERROR;
        }
        cipher_size += result;

        // call back to app
        bzero(data_size_str, sizeof(data_size_str));
        snprintf(data_size_str, sizeof(data_size_str), "%d", data_size);
        data_str8 = data_size_str;

        bzero(cnt_size_str, sizeof(cnt_size_str));
        snprintf(cnt_size_str, sizeof(cnt_size_str), "%ld", (long) cnt_total);
        cnt_str8 = cnt_size_str;

        callBackStr.clear();
        callBackStr.append("data_s:");
        callBackStr.append(data_str8);
        callBackStr.append("::cnt_s:");
        callBackStr.append(cnt_str8);
        callBackStr.append("::path:");
        callBackStr.append(path);
        callBackStr.append("::result:");
        if (this->mNeedCancel)
        {
            ALOGW("hongen encrypt canceled by user");
            callBackStr.append(DrmDef::CTA5_CANCEL_DONE);
            //Notify listener
            notify(infoListener, callBackStr);
            this->mNeedCancel = false;
            delete[] buf_out;
            return true;
        } else
        {
            if ((int) cnt_total < data_size && ((int) cnt_total > block_size * encrypt_count))
            {
                callBackStr.append(DrmDef::CTA5_UPDATING);
                notify(infoListener, callBackStr);
                encrypt_count++;
            }
        }

    }

    delete[] buf_out;
    callBackStr.append(DrmDef::CTA5_DONE);
    notify(infoListener, callBackStr);
    ALOGD("encrypt done:data_size=[%d], cipher_size=[%d],cnt_total=[%d],",data_size, (int) cipher_size,(int) cnt_total);
    // Calculate file content's md5 sig
    this->mCta5FileHeader.calculateFileSig(fd_out);
    return true;

    ERROR:
    delete[] buf_out;
    return false;

}

/**
 * Decrypt current cta5 file to fd_out
 * And notify the decrypt progress by calling infoListener
 */
bool Cta5NormalFile::decrypt(int fd_out, const Vector<DrmCtaUtil::Listener> *infoListener)
{
    ALOGD("decrypt.");
    if (fd_out < 0)
    {
        ALOGE("[ERROR]decrypt: error fd_out=[%d]", fd_out);
        return false;
    }
    int cipher_fd = this->getFd();

    uint32_t headerLen = getCta5FileHeader().length();

    if (-1 == lseek(cipher_fd, 0, SEEK_SET))
    {
        ALOGE("[ERROR]decrypt. lseek cipher_fd set failed. reason=[%s]", strerror(errno));
        return false;
    }
    int fileLength = lseek(cipher_fd, 0, SEEK_END);
    if (-1 == fileLength)
    {
        ALOGE("[ERROR]decrypt. lseek cipher_fd end failed. reason=[%s]", strerror(errno));
        return false;
    }
    uint32_t contentLength = fileLength - headerLen;
    if (-1 == lseek(cipher_fd, headerLen, SEEK_SET))
    {
        ALOGE("[ERROR]decrypt:lseek cipher_fd header failed. reason=[%s]", strerror(errno));
        return false;
    }
    // call back parameters
    String8 path = Cta5FileUtil::getPathFromFd(cipher_fd);
    String8 callBackStr("");
    char data_size_str[32] = { 0 };
    String8 data_str8("");
    char cnt_size_str[32] = { 0 };
    String8 cnt_str8("");

    uint32_t readSize = 0; //  read cipher size
    int resultSize = 0; // clear data offset
    unsigned char buffer[DECRYPT_BUF_LEN];
    memset(buffer, 0, sizeof(buffer));

    int decrypt_count = 0;
    int block_size = contentLength / ENCRYPT_DECRYPT_COUNT;
    block_size = (block_size > CALLBACK_LEN) ? block_size : CALLBACK_LEN;
    while (readSize < contentLength)
    {
        memset(buffer, 0, sizeof(buffer));
        int size = (contentLength - readSize) > DECRYPT_BUF_LEN ? DECRYPT_BUF_LEN : (contentLength - readSize);
        int readLength = Cta5NormalFile::readCipherFile(cipher_fd, buffer, size, resultSize, headerLen, contentLength);
        if (readLength <= 0)
        {
            ALOGE("[ERROR]decrypt:failed to read valid decrypted data.");
            // call back to app
            bzero(data_size_str, sizeof(data_size_str));
            snprintf(data_size_str, sizeof(data_size_str), "%d", contentLength);
            data_str8 = data_size_str;

            bzero(cnt_size_str, sizeof(cnt_size_str));
            snprintf(cnt_size_str, sizeof(cnt_size_str), "%d", contentLength);
            cnt_str8 = cnt_size_str;

            callBackStr.clear();
            callBackStr.append("data_s:");
            callBackStr.append(data_str8);
            callBackStr.append("::cnt_s:");
            callBackStr.append(cnt_str8);
            callBackStr.append("::path:");
            callBackStr.append(path);
            callBackStr.append("::result:");
            break;
        }
        readSize += size;
        resultSize += readLength;

        if (!write(fd_out, buffer, readLength))
        {
            ALOGE("[ERROR]decrypt:failed to write data to output file. readLength[%d]", readLength);
            return false;
        }
        // call back to app
        bzero(data_size_str, sizeof(data_size_str));
        snprintf(data_size_str, sizeof(data_size_str), "%d", contentLength);
        data_str8 = data_size_str;

        bzero(cnt_size_str, sizeof(cnt_size_str));
        snprintf(cnt_size_str, sizeof(cnt_size_str), "%d", readSize);
        cnt_str8 = cnt_size_str;

        callBackStr.clear();
        callBackStr.append("data_s:");
        callBackStr.append(data_str8);
        callBackStr.append("::cnt_s:");
        callBackStr.append(cnt_str8);
        callBackStr.append("::path:");
        callBackStr.append(path);
        callBackStr.append("::result:");
        if (this->mNeedCancel)
        {
            ALOGW("decrypt cancled by user");
            callBackStr.append(DrmDef::CTA5_CANCEL_DONE);
            //Notify listener
            notify(infoListener, callBackStr);
            this->mNeedCancel = false;
            return true;
        } else
        {
            if (readSize < contentLength && (readSize > block_size * decrypt_count))
            {
                callBackStr.append(DrmDef::CTA5_UPDATING);
                notify(infoListener, callBackStr);
                decrypt_count++;
            }
        }
    }
    callBackStr.append(DrmDef::CTA5_DONE);
    notify(infoListener, callBackStr);
    ALOGD("decrypt done: read size=[%d], resultSize=[%d],contentL=[%d]", readSize, resultSize,contentLength);
    return true;
}

ssize_t Cta5NormalFile::readCipherFile(int fd, void* buffer, int numBytes, int offset, int cipher_content_offset, int cipher_content_size)
{
    ssize_t bytesRead = -1; // returns the actual bytes read.
    if (NULL != buffer && numBytes > -1 && offset > -1)
    {
        CryptoHelper cr_hlp(CryptoHelper::CIPHER_AES128CBC, getContentKey(), 0);
        bytesRead = CipherFileUtil::CipherFileRead(fd, buffer, numBytes, offset, cipher_content_offset,
                cipher_content_size, cr_hlp);
        ALOGV("pread(): read %d bytes", (int) bytesRead);
    } else
    {
        ALOGE("pread(): error, parameter invalid");
    }

    return bytesRead;
}

int64_t Cta5NormalFile::pread(void* buf, uint64_t size, off64_t offset)
{
    ssize_t bytesRead = -1;
    int cipher_fd = this->getFd();
    uint32_t headerLen = Cta5FileHeader::CTA5_FILE_HEADER_LEN;
    uint32_t contentLength = this->getContentLen();
    // ALOGD("content length [%d],", contentLength);

    bytesRead = Cta5NormalFile::readCipherFile(cipher_fd, buf, size, offset, (int) headerLen, (int) contentLength);
    ALOGV("pread, size=[%ld], offset=[%ld],byteRead=[%d]", (long) size, (long) offset, (int) bytesRead);
    return (int) bytesRead;
}

