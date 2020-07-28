#define LOG_NDEBUG 0
#define LOG_TAG "Cta5CommonMultimediaFile"
#include <utils/Log.h>

#include <CryptoHelper.h>
#include <CipherFileUtil.h>

#include <Cta5CommonMultimediaFile.h>
#include <DrmCtaMultiMediaUtil.h>
#include <sys/types.h>
#include <unistd.h>
#include <DrmCtaUtil.h>
#include <drm/DrmInfoEvent.h>
#include <DrmDef.h>


using namespace android;

#define BLK_LEN 1024
#define DECRYPT_BUF_LEN 4096
#define ENCRYPT_DECRYPT_COUNT 100
#define CALLBACK_LEN (1024*100)

Cta5CommonMultimediaFile::Cta5CommonMultimediaFile(int fd, String8 key) : Cta5File(fd, key)
{
    //TODO do erorr handle
    ALOGD("Cta5MultimediaFile(fd, key)");

    // parse multimedia header
    setCtaMultimediaHeader(fd);

    lseek64(fd, 0L, SEEK_SET);
    uint64_t len = lseek64(fd, 0LL, SEEK_END);

    lseek64(fd, len - mmCtaHeaderSize - Cta5FileHeader::CTA5_FILE_HEADER_LEN, SEEK_SET);
    //read cta header, its length should be 308
    uint8_t cta5FileHaderBuf[Cta5FileHeader::CTA5_FILE_HEADER_LEN] =  { 0 };
    read(fd, cta5FileHaderBuf, Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    String8 cta5FileHaderStr((const char *) cta5FileHaderBuf, Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    mCta5FileHeader.setCta5FileHeader(cta5FileHaderStr);
    //TODO need modify?
    lseek64(fd, 0LL, SEEK_SET);

    // clear Vector<Header>
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5CommonMultimediaFile::Cta5CommonMultimediaFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5File(mimeType, cid, dcfFlHeaders, datatLen, key, true)
{

}

Cta5CommonMultimediaFile::Cta5CommonMultimediaFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5File(mimeType, datatLen, key, true)
{

}

bool Cta5CommonMultimediaFile::encrypt(int fd_in, int fd_out __attribute__((unused)),
        const Vector<DrmCtaUtil::Listener> *infoListener)
{
    ALOGD("encrypt...");
    // 1. parse multimedia headers
    if(!parseHeaders(fd_in)) {
        ALOGE("[ERROR][CTA5]encrypt - parseHeaders() fail.");
        return false;
    }

    // TODO
    // sort mHeaders

    // get total header size
    off64_t total_header_size = getTotalHeaderSize();
    off64_t progress = 0LL;

    // 2. while(...) encrypt header
    ALOGD("encrypt - encrypt multimedia header");
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(header->clear_header_offset != -1
            && header->clear_header_size != -1) {
            // do ecnrypt header
            ALOGD("encrypt - before encrypt header, clear_header_offset[%ld]\
                clear_header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                (long) header->clear_header_offset, (long) header->clear_header_size,
                (long) header->cipher_header_offset, (long) header->cipher_header_size);

            if(!encryptHeader(fd_in,
                header->clear_header_offset, header->clear_header_size,
                header->cipher_header_offset, header->cipher_header_size,
                infoListener, total_header_size, progress)) {
                if(isCancelDone) {
                    return true;
                }
                ALOGE("[ERROR][CTA5]encrypt - encrypt header fail.");
                return false;
            }

            ALOGD("encrypt - after encrypt header, clear_header_offset[%ld]\
                clear_header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                (long) header->clear_header_offset, (long) header->clear_header_size,
                (long) header->cipher_header_offset, (long) header->cipher_header_size);
        }
        continue;
    }

    // 3. add cta header
    ALOGD("encrypt - add cta header");
    String8 ctaHeader = getHeader();
    int ctaHeaderLen = ctaHeader.length();

    //write header to the end of file
    if(-1 == lseek64(fd_in, 0, SEEK_END)) {
        ALOGE("[ERROR][CTA5]encrypt - failed to seek to the end of file, reason[%s]", strerror(errno));
        return false;
    }

    if (ctaHeaderLen != write(fd_in, ctaHeader.string(), ctaHeaderLen)) {
        ALOGE("[ERROR][CTA5]encrypt - failed to write cta header file end.");
        return false;
    }

    // 4. add multimedia header
    ALOGD("encrypt - add multimudia header");
    String8 ctaMmHeader = getCtaMultimediaHeader();
    int ctaMmHeaderLen = ctaMmHeader.length();

    //write header to the end of file
    if(-1 == lseek64(fd_in, 0, SEEK_END)) {
        ALOGE("[ERROR][CTA5]encrypt - failed to seek to the end of file, reason[%s]", strerror(errno));
        return false;
    }

    if (ctaMmHeaderLen != write(fd_in, ctaMmHeader.string(), ctaMmHeaderLen)) {
        ALOGE("[ERROR][CTA5]encrypt - failed to write cta header file end.");
        return false;
    }

    // 5. notify encrypt finish
    notifyProgress(total_header_size, progress, fd_in, DrmDef::CTA5_MULTI_MEDIA_ENCRYPT_DONE, infoListener);
    ALOGD("encrypt - finish");

    return true;
}

/**
 * Decrypt current cta5 file to fd_out
 * And notify the encrypt progress by calling infoListener
 */
bool Cta5CommonMultimediaFile::decrypt(int fd_out __attribute__((unused)),
                                       const Vector<DrmCtaUtil::Listener> *infoListener)
{
    ALOGD("decrypt...");

    // 1.  check headers whether is empty
    if(mHeaders.empty()) {
        ALOGE("[ERROR][CTA5]decrypt - mHeaders is empty, return false.");
        return false;
    }

    // get total header size
    off64_t total_header_size = getTotalHeaderSize();
    off64_t progress = 0LL;

    // 2. while(...) decrypt header
    ALOGD("decrypt - decrypt multimedia header");
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(header->clear_header_offset != -1
            && header->clear_header_size != -1) {
            // do ecnrypt header
            ALOGD("decrypt - before encrypt header, clear_header_offset[%ld]\
                clear_header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                (long) header->clear_header_offset, (long) header->clear_header_size,
                (long) header->cipher_header_offset, (long) header->cipher_header_size);

            if(!decryptHeader(header->clear_header_offset, header->clear_header_size,
                header->cipher_header_offset, header->cipher_header_size,
                infoListener, total_header_size, progress)) {
                if(isCancelDone) {
                    return true;
                }
                ALOGE("[ERROR][CTA5]decrypt - encrypt header fail.");
                return false;
            }
        }
        continue;
    }

    // 3. trunc chiper text && cta multimedia header
    ALOGD("decrypt - truncate file");
    // get file size
    off64_t filesize = lseek64(getFd(), 0, SEEK_END);
    // TODO
    // use the first cipher header offset as bound
    off64_t bounds = mHeaders.top()->cipher_header_offset;
    ALOGD("total size before ftruncate: %ld, des: %ld", (long) filesize, (long) bounds);
    ftruncate64(getFd(), bounds);
    filesize = lseek64(getFd(), 0, SEEK_END);
    ALOGD("total size after ftruncate: %ld", (long) filesize);

    // 5. notify decrypt finish
    notifyProgress(total_header_size, progress, getFd(), DrmDef::CTA5_MULTI_MEDIA_DECRYPT_DONE, infoListener);
    ALOGD("decrypt - finish");

    return true;
}

// read raw data from cta file
// if data is in clear text, directly return
// if data with cipher text, merge and return
int64_t Cta5CommonMultimediaFile::pread(void* buf, uint64_t size, off64_t offset)
{
    //ALOGV("pread() offset: %lld, size: %lld", offset, size);

    int fd = getFd();
    // construct a tmp vector to store cipher bounds
    Vector<off64_t> cipherBounds;
    //don't use element 0
    cipherBounds.push_back(-1LL);

    // find next position
    bool found_position = false;
    off64_t position = 0LL;
    int count = 0;
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(header->clear_header_offset != -1
            && header->clear_header_size != -1) {
            cipherBounds.push_back(header->clear_header_offset);
            ++ count;
            // beacuse the mHeaders is sorted by offset, we only need to check offset whether is less than clear header bounds
            if(!found_position && offset < header->clear_header_offset) {
                position = count;
                found_position = true;
            }
            // ALOGD("bounds: %lld", header->clear_header_offset);
            cipherBounds.push_back(header->clear_header_offset + header->clear_header_size);
            ++ count;
            // beacuse the mHeaders is sorted by offset, we only need to check offset whether is less than clear header bounds
            if(!found_position && offset < header->clear_header_offset + header->clear_header_size) {
                position = count;
                found_position = true;
            }
            //ALOGD("bounds: %lld", header->clear_header_offset  + header->clear_header_size);
        }
    }
    //ALOGD("the position is %lld", position);

    off64_t readLength = 0LL;
    // not found position, directly returnn clear text
    if(position == 0LL || found_position == false) {
        // ALOGD("not found position, directly read clear buffer, offset[%lld], size[%lld]", offset, size);
        if(-1ll == lseek64(fd, offset, SEEK_SET)) {
            ALOGE("[ERROR][CTA5]pread - lseek fail, reason[%s]", strerror(errno));
            return -1;
        }
        readLength = read(fd, buf, size);
        if(-1 == readLength) {
            ALOGE("[ERROR][CTA5]pread - read fail, reason[%s]", strerror(errno));
            return -1;
        }
        return readLength;
    }

    off64_t curr_offset = offset;
    off64_t buffer_offset = 0;
    off64_t off_size = size;
    off64_t total_read_len = 0LL;
    while(curr_offset < offset + off_size && position <= cipherBounds.size()) {
        // get end offset
        off64_t end;
        if(position == cipherBounds.size()) {
            end = offset + off_size;
        } else {
            end = cipherBounds[position];
        }

        // check end
        if(end > offset + off_size) {
            end = offset + off_size;
        }
        //ALOGD("curr_offset: %lld, end: %lld", curr_offset, end);

        readLength = end - curr_offset;
        //ALOGD("position: %lld", position);
        if(position % 2 == 1) {// if  odd, first block is clear buffer
            //ALOGD("read clear buffer, offset[%lld], size[%lld]", curr_offset, readLength);
            if(-1ll == lseek64(fd, curr_offset, SEEK_SET)) {
                ALOGE("[ERROR][CTA5]pread - lseek fail, reason[%s]", strerror(errno));
                return -1;
            }
            readLength = read(fd, (uint8_t *)buf + buffer_offset, readLength);
            if(-1 == readLength) {
                ALOGE("[ERROR][CTA5]pread - read fail, reason[%s]", strerror(errno));
                return -1;
            }
            total_read_len += readLength;
            buffer_offset += readLength;
        } else if(position % 2 == 0) {// if  even, first block is cipher buffer
            //ALOGD("read cipher buffer, offset[%lld], size[%lld]", curr_offset, readLength);
            // find corresponding cipher header offset
            Header* header = getCipherHeader(curr_offset);
            if(header == NULL) {
                ALOGE("header is NULL");
                return -1;
            }

            // decrypt and write to buffer
            ssize_t cipherLength = cipherPread(fd, (uint8_t *)buf + buffer_offset, readLength, curr_offset - header->clear_header_offset,
                header->cipher_header_offset, header->cipher_header_size);
            if(cipherLength == -1) {
                ALOGE("[ERROR][CTA5]pread - read cipher text fail");
                return -1;
            }
            total_read_len += cipherLength;
            buffer_offset += cipherLength;
        }

        position++;
        curr_offset = end;
    }
    return total_read_len;
}

uint64_t Cta5CommonMultimediaFile::getContentPos()
{
    return 0LL;
}

bool Cta5CommonMultimediaFile::encryptHeader(int fd, off64_t header_offset, off64_t header_size,
    off64_t& cipher_header_offset, off64_t& cipher_header_size, const Vector<DrmCtaUtil::Listener> *infoListener,
    off64_t total_header_size, off64_t& progress) {

    // check parameters
    if(fd < 0 || header_offset < 0 || header_size < 0) {
        ALOGE("encryptHeader: error parameter: fd[%d], header_offset[%ld], header_size[%ld]",
                fd, (long) header_offset, (long) header_size);
        return false;
    }
    ALOGD("encryptHeader: fd[%d], header_offset[%ld], header_size[%ld]", fd, (long) header_offset, (long) header_size);

    CryptoHelper cr_hlp(CryptoHelper::CIPHER_AES128CBC, (BYTE*)getKey(), 1);

    size_t result = 0;
    cipher_header_offset = 0ll;
    cipher_header_size = 0ll;

    // get original file end offset
    // it also be cipher header offset
    off64_t originalEndOffset = lseek64(fd, 0, SEEK_END);
    if(-1 == originalEndOffset) {
        ALOGE("[ERROR][CTA5]encryptHeader: failed to seek to the end of content, reason[%s]", strerror(errno));
        return false;
    }
    cipher_header_offset = originalEndOffset;
    ALOGD("encryptHeader: originalEndOffset[%ld], cipher_header_offset[%ld]", (long) originalEndOffset, (long) cipher_header_offset);

    // copy the first iv
    unsigned char iv_arr[16];
    bzero(iv_arr, sizeof(iv_arr));
    memcpy(iv_arr, getIv(), sizeof(iv_arr));

    //***************
    // write the first iv to cipher header
    result = write(fd, iv_arr, sizeof(iv_arr));
    if (result != 16)
    {
        ALOGE("[ERROR][CTA5]encryptHeader: failed to write first iv to cipher header.");
        return false;
    }
    cipher_header_size += result;
    // ***********

    // seek to header offset
    if (-1 == lseek64(fd, header_offset, SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]encryptHeader: failed to seek to the start of content, reason[%s]", strerror(errno));
        return false;
    }

    // input buffer
    unsigned char buf_in[BLK_LEN];
    bzero(buf_in, sizeof(buf_in));

    // output buffer for encryption
    int desired_len = cr_hlp.desiredOutBufSize(BLK_LEN);
    unsigned char* buf_out = new unsigned char[desired_len];
    bzero(buf_out, (size_t)desired_len);

    // read, encrypt, and write to file
    size_t cnt = 0;
    off64_t cnt_total = 0ll;
    int len_out = 0;
    off64_t temp_offset = 0ll;
    int encrypt_count = 0;
    int block_size = header_size / ENCRYPT_DECRYPT_COUNT;
    block_size = (block_size > CALLBACK_LEN) ? block_size : CALLBACK_LEN;
    // encrypt and write to the end of file
    while (cnt_total < header_size && !mNeedCancel)
    {
        bzero(buf_in, sizeof(buf_in));
        bzero(buf_out, (size_t)desired_len);

        cnt = read(fd, buf_in, ((header_size - cnt_total) > BLK_LEN) ? BLK_LEN : (header_size - cnt_total));
        cnt_total += cnt;
        // update progress
        progress += cnt;
        ALOGD("encryptHeader: read[%ld], cnt_total[%ld]", (long) cnt, (long) cnt_total);

        // temporary save current offset
        temp_offset = lseek64(fd, 0, SEEK_CUR);
        if (-1 == temp_offset)
        {
            ALOGE("[ERROR][CTA5]encryptHeader: failed to get current offset, reason[%s]", strerror(errno));
            goto ERROR;
        }

        if (1 != cr_hlp.doCryption(buf_in, (int)cnt, buf_out, len_out, iv_arr, ((int)cnt_total >= header_size)))
        {
            ALOGE("[ERROR][CTA5]encryptHeader: encryption operation failed.");
            goto ERROR;
        }
        ALOGD("encryptHeader: len_out[%d]", len_out);

        // copy next iv
        memcpy(iv_arr, buf_out + len_out - 16, sizeof(iv_arr));

        // write cipher to the end of file
        // seek to end offset
        if (-1 == lseek64(fd, 0, SEEK_END))
        {
            ALOGE("[ERROR][CTA5]encryptHeader: failed to seek to the start of content, reason[%s]", strerror(errno));
            goto ERROR;
        }
        // save to file
        result = write(fd, buf_out, (size_t)len_out);
        if (result != (size_t)len_out)
        {
            ALOGE("[ERROR][CTA5]encryptHeader: failed to write data to output file.");
            goto ERROR;
        }
        cipher_header_size += result;

        // reset offset to read
        if (-1 == lseek64(fd, temp_offset, SEEK_SET))
        {
            ALOGE("[ERROR][CTA5]encryptHeader: failed to seek to read offset, reason[%s]", strerror(errno));
            goto ERROR;
        }
        if ((int) cnt_total < header_size && ((int) cnt_total > block_size * encrypt_count))
        {
            // notify progress
            notifyProgress(total_header_size, progress, fd, DrmDef::CTA5_UPDATING, infoListener);
            encrypt_count++;
        }
    }

    if(mNeedCancel) {
        ALOGD("encryptHeader: cancel encrypt");
        // recover
        recoverClearData(fd, originalEndOffset);
        //  notify cancel
        notifyProgress(total_header_size, progress, fd, DrmDef::CTA5_CANCEL_DONE, infoListener);
        isCancelDone = true;
        mNeedCancel = false;
        goto ERROR;
    }

    // to protect original file, use cipher to overwrite clear header
    overwriteClearHeader(fd, header_offset, header_size, cipher_header_offset, cipher_header_size);

    // output
    ALOGD("encryptHeader: header offset[%ld], header size[%ld], cipher header offset[%ld], cipher header size[%ld]",
        (long) header_offset, (long) header_size, (long) cipher_header_offset, (long) cipher_header_size);

    delete[] buf_out;
    return true;
ERROR:
    delete[] buf_out;
    return false;
}

// use CipherFileUtil#CipherFileRead to decrypt, like pread
bool Cta5CommonMultimediaFile::decryptHeader(off64_t header_offset, off64_t header_size,
    off64_t& cipher_header_offset, off64_t& cipher_header_size,
    const Vector<DrmCtaUtil::Listener> *infoListener, off64_t total_header_size, off64_t& progress) {

    int fd = getFd();

    // check parameters
    if(fd < 0 || header_offset < 0 || header_size < 0
        || cipher_header_offset < 0 || cipher_header_size < 0) {
        ALOGE("[ERROR][CTA5]decryptHeader: error parameters: fd[%d], header_offset[%ld], header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                fd, (long) header_offset, (long) header_size, (long) cipher_header_offset, (long) cipher_header_size);
        return false;
    }
    ALOGD("decryptHeader: parameters: fd[%d], header_offset[%ld], header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                fd, (long) header_offset, (long) header_size, (long) cipher_header_offset, (long) cipher_header_size);

    off64_t readSize = 0;
    off64_t resultSize = 0;
    //int fileLength = cipher_header_size;
    unsigned char buffer[DECRYPT_BUF_LEN];

    int decrypt_count = 0;
    int block_size = cipher_header_size / ENCRYPT_DECRYPT_COUNT;
    block_size = (block_size > CALLBACK_LEN) ? block_size : CALLBACK_LEN;
    while(readSize <= cipher_header_size && !mNeedCancel) {
        bzero(buffer, sizeof(buffer));
        ssize_t size = (cipher_header_size - readSize) > DECRYPT_BUF_LEN ? DECRYPT_BUF_LEN : (cipher_header_size - readSize);
        ALOGD("decryptHeader: preare to decrypt[%d]", (int) size);
        ssize_t readLength = cipherPread(fd, buffer, size, resultSize, cipher_header_offset, cipher_header_size);
        if(readLength <=0) {
            ALOGE("[ERROR][CTA5]decryptHeader: failed to read valid decrypted data.");
            break;
        }
        ALOGD("decryptHeader: read length[%d]", (int) readLength);

        // recover clear text to header
        // seek to header offset
        if (-1 == lseek64(fd, header_offset + resultSize, SEEK_SET))
        {
            ALOGE("[ERROR][CTA5]decryptHeader: failed to seek to header offset, reason[%s]", strerror(errno));
            return false;
        }

        // recover clear text to header
        ssize_t result = write(fd, buffer, readLength);
        if (result != readLength)
        {
            ALOGE("[ERROR][CTA5]decryptHeader: failed to write data to output file. readLength[%d], result[%d]", (int) readLength, (int) result);
            return false;
        }

        readSize += size;
        resultSize += readLength;
        progress += readLength;
        if (readSize < cipher_header_size && (readSize > (off64_t) block_size * decrypt_count))
        {
            // notify progress
            notifyProgress(total_header_size, progress, fd, DrmDef::CTA5_UPDATING, infoListener);
            decrypt_count++;
        }
    }

    if(mNeedCancel) {
        ALOGD("decryptHeader: cancel decrypt");
        // recover
        recoverCipherData(fd, header_offset, header_size, cipher_header_offset, cipher_header_size);
        //  notify cancel
        notifyProgress(total_header_size, progress, fd, DrmDef::CTA5_CANCEL_DONE, infoListener);
        isCancelDone = true;
        mNeedCancel = false;
        return false;
    }

    if(resultSize <= 0) {
        ALOGE("[ERROR][CTA5]decryptHeader: failed to read decrypted data. resultSize[%ld]", (long) resultSize);
        return false;
    }

    ALOGD("decryptHeader: finish.");
    return true;
}

void Cta5CommonMultimediaFile::notifyProgress(off64_t total, off64_t progress, int fd, String8 result, const Vector<DrmCtaUtil::Listener> *infoListener) {
    ALOGD("notifyProgress:data_size=[%ld], cnt_total=[%ld],",(long) total, (long) progress);
    String8 r("");
    char data_size_str[32] = { 0 };
    snprintf(data_size_str, sizeof(data_size_str), "%ld", (long) total);
    String8 data_str8(data_size_str);

    char cnt_size_str[32] = { 0 };
    snprintf(cnt_size_str, sizeof(cnt_size_str), "%ld", (long) progress);
    String8 cnt_str8(cnt_size_str);
    String8 path = Cta5FileUtil::getPathFromFd(fd);

    r.append("data_s:");
    r.append(data_str8);
    r.append("::cnt_s:");
    r.append(cnt_str8);
    r.append("::path:");
    r.append(path);
    r.append("::result:");
    r.append(result);

    ALOGD("notifyProgress - message[%s]", r.string());

    notify(infoListener, r);
}

// remove unfinished cipher header at the end of file
bool Cta5CommonMultimediaFile::recoverClearData(int fd, off64_t original_end_offset) {
    ALOGD("recoverClearData: original end offset[%ld]", (long) original_end_offset);
    if(-1 == ftruncate64(fd, original_end_offset)) {
        ALOGE("[ERROR][CTA5]recover: failed to recover, offset[%ld], reason[%s]", (long) original_end_offset, strerror(errno));
        return false;
    }
    return true;
}

// use cipher to overwrite already decrypted header
bool Cta5CommonMultimediaFile::recoverCipherData(int fd, off64_t header_offset, off64_t header_size,
    off64_t cipher_header_offset, off64_t cipher_header_size) {
    ALOGD("recoverCipherData");
    return overwriteClearHeader(fd, header_offset, header_size, cipher_header_offset, cipher_header_size);
}

// for data protection, use cipher to overwrite clear header
bool Cta5CommonMultimediaFile::overwriteClearHeader(int fd, off64_t header_offset, off64_t header_size,
    off64_t cipher_header_offset, off64_t cipher_header_size __attribute__((unused))) {
    ALOGD("overwriteClearHeader...");
    off64_t readSize = 0;
    off64_t resultSize = 0;
    //int fileLength = cipher_header_size;
    unsigned char buffer[DECRYPT_BUF_LEN];

    off64_t cipher_offset = cipher_header_offset;

    while(readSize <= header_size) {
        bzero(buffer, sizeof(buffer));
        ssize_t size = (header_size - readSize) > DECRYPT_BUF_LEN ? DECRYPT_BUF_LEN : (header_size - readSize);

        ssize_t readLength = DrmCtaMultiMediaUtil::readAt(fd, cipher_offset, buffer, size);
        if(readLength <=0) {
            ALOGE("[ERROR][CTA5]overwriteClearHeader: failed to read data.");
            return false;
        }
        ALOGD("overwriteClearHeader: read length[%ld]", (long) readLength);
        // record current cipher read offset
        cipher_offset = lseek64(fd, 0, SEEK_CUR);
        if(-1 == cipher_offset) {
            ALOGE("[ERROR][CTA5]overwriteClearHeader: get currnt cipher read offset fail, reason[%s]",strerror(errno));
            return false;
        }

        // seek to clear header write offset
        if (-1 == lseek64(fd, header_offset + resultSize, SEEK_SET))
        {
            ALOGE("[ERROR][CTA5]overwriteClearHeader: failed to seek to header offset, reason[%s]", strerror(errno));
            return false;
        }

        // recover clear text to moov
        ssize_t result = write(fd, buffer, readLength);
        if (result != readLength)
        {
            ALOGE("[ERROR][CTA5]overwriteClearHeader: failed to write data to output file. readLength[%d], result[%d]", (int) readLength, (int) result);
            return false;
        }

        readSize += size;
        resultSize += readLength;
    }

    return true;
}

off64_t Cta5CommonMultimediaFile::getTotalHeaderSize() {
    off64_t totalHeaderSize = 0LL;
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(header->clear_header_offset != -1
            && header->clear_header_size != -1) {
            totalHeaderSize += header->clear_header_size;
        }
        continue;
    }
    ALOGD("getTotalHeaderSize[%ld]", (long) totalHeaderSize);
    return totalHeaderSize;
}

String8 Cta5CommonMultimediaFile::getCtaMultimediaHeader() {
    ALOGD("getCtaMultimediaHeader()...");
    String8 str8;
    mmHeaderCount = 0;
    // add header
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(header->clear_header_offset != -1
            && header->clear_header_size != -1) {
            uint64_t clearHeaderOffset = Cta5FileUtil::hosttonetll(header->clear_header_offset);
            str8.append((const char*) &clearHeaderOffset, sizeof(clearHeaderOffset));

            uint64_t clearHeaderLen = Cta5FileUtil::hosttonetll(header->clear_header_size);
            str8.append((const char*) &clearHeaderLen, sizeof(clearHeaderLen));

            uint64_t cipherHeaderOffset = Cta5FileUtil::hosttonetll(header->cipher_header_offset);
            str8.append((const char*) &cipherHeaderOffset, sizeof(cipherHeaderOffset));

            uint64_t cipherHeaderLen = Cta5FileUtil::hosttonetll(header->cipher_header_size);
            str8.append((const char*) &cipherHeaderLen, sizeof(cipherHeaderLen));

            mmHeaderCount++;
        }
        continue;
    }

    // add size
    uint32_t count = Cta5FileUtil::hosttonetl(mmHeaderCount);
    ALOGD("getCtaMultimediaHeader - count: 0x%02x, 0x%02x, 0x%02x, 0x%02x",
        ((uint8_t*)&count)[0], ((uint8_t*)&count)[1], ((uint8_t*)&count)[2], ((uint8_t*)&count)[3]);
    ALOGD("count: %d, mmHeaderCount; %d", count, mmHeaderCount);
    str8.append((const char*) &count, sizeof(count));

    // add magic
    str8.append((const char *)CTA_MM_MAGIC, sizeof(CTA_MM_MAGIC));

    ALOGD("getCtaMultimediaHeader - len = %d\n", (int) str8.length());

    return str8;
}

bool Cta5CommonMultimediaFile::setCtaMultimediaHeader(int fd) {
    ALOGD("setCtaMultimediaHeader()...");
    // read cta multimedia header
    uint64_t len = lseek64(fd, 0LL, SEEK_END);
    // 1. read mm magic
    lseek64(fd, len - CTA_MAGIC_LEN, SEEK_SET);
    read(fd, mMagicMM, sizeof(mMagicMM));
    if(0 != memcmp(mMagicMM, CTA_MM_MAGIC, CTA_MAGIC_LEN)) {
        ALOGE("[ERROR][CTA5]setCtaMultimediaHeader - verfiy cta mm magic fail.[%s]", mMagicMM);
    }
    ALOGD("setCtaMultimediaHeader - mm magic[%s]", (char*)mMagicMM);
    // 2. read header count
    lseek64(fd, len - CTA_MAGIC_LEN - sizeof(mmHeaderCount), SEEK_SET);
    uint32_t count = 0L;
    read(fd, (uint8_t *)&count, sizeof(count));
    mmHeaderCount = Cta5FileUtil::nettohostl(count);
    ALOGD("setCtaMultimediaHeader - header count[%d]", mmHeaderCount);
    // 3. read headers
    int header_size = mmHeaderCount * 32;
    uint8_t *headers = (uint8_t*)malloc(header_size);
    lseek64(fd, len - CTA_MAGIC_LEN - sizeof(mmHeaderCount) - header_size, SEEK_SET);
    read(fd, headers, header_size);
    int pos = 0;
    mHeaders.clear();
    for(uint32_t i = 0; i < mmHeaderCount; i++) {
        pos += i * 32;
        Header* header = new Header();
        header->clear_header_offset = Cta5FileUtil::nettohostll(*(uint64_t*) (headers + pos));
        header->clear_header_size = Cta5FileUtil::nettohostll(*(uint64_t*) (headers + pos + 8));
        header->cipher_header_offset = Cta5FileUtil::nettohostll(*(uint64_t*) (headers + pos + 16));
        header->cipher_header_size = Cta5FileUtil::nettohostll(*(uint64_t*) (headers + pos + 24));

        ALOGD("setCtaMultimediaHeader - clear_header_offset[%ld]\
                clear_header_size[%ld], cipher_header_offset[%ld], cipher_header_size[%ld]",
                (long) header->clear_header_offset, (long) header->clear_header_size,
                (long) header->cipher_header_offset, (long) header->cipher_header_size);
        mHeaders.push_back(header);
    }
    free(headers);

    // set cta mm header size
    mmCtaHeaderSize = CTA_MAGIC_LEN + sizeof(count) + header_size;
    ALOGD("setCtaMultimediaHeader - mmCtaHeaderSize[%ld]", (long) mmCtaHeaderSize);
    return true;
}

int Cta5CommonMultimediaFile::verifyCtaMultimediaHeader(int fd) {
    ALOGD("verifyCtaMultimediaHeader()...");
    // read cta multimedia header
    uint64_t len = lseek64(fd, 0LL, SEEK_END);
    uint8_t magic[CTA_MAGIC_LEN] = {0};
    // 1. read mm magic
    lseek64(fd, len - CTA_MAGIC_LEN, SEEK_SET);
    read(fd, magic, sizeof(magic));
    if(0 != memcmp(magic, CTA_MM_MAGIC, CTA_MAGIC_LEN)) {
        ALOGE("[ERROR][CTA5]verifyCtaMultimediaHeader - verfiy cta mm magic fail.[%s]", magic);
        return -1;
    }
    ALOGD("verifyCtaMultimediaHeader - magic[%s]", (char*)magic);

    // 2. read header count
    uint32_t count = 0L;
    uint32_t header_count;
    lseek64(fd, len - CTA_MAGIC_LEN - sizeof(count), SEEK_SET);
    read(fd, (uint8_t *)&count, sizeof(count));
    ALOGD("getCtaMultimediaHeader - count: 0x%02x, 0x%02x, 0x%02x, 0x%02x",
        ((uint8_t*)&count)[0], ((uint8_t*)&count)[1], ((uint8_t*)&count)[2], ((uint8_t*)&count)[3]);
    ALOGD("verifyCtaMultimediaHeader - count: 0x%08x", count);
    header_count = Cta5FileUtil::nettohostl(count);
    ALOGD("verifyCtaMultimediaHeader - header count[%d]", header_count);
    // 3. read headers
    int header_size = header_count * 32;

    // get cta mm header size
    int totalMmCtaHeaderSize = CTA_MAGIC_LEN + sizeof(count) + header_size;
    ALOGD("verifyCtaMultimediaHeader - totalMmCtaHeaderSize[%d]", totalMmCtaHeaderSize);

    return totalMmCtaHeaderSize;
}


// read some data from protected content.
// the {offset} is the offset from the real beginning of the raw content
ssize_t Cta5CommonMultimediaFile::cipherPread(int fd, void* buffer, ssize_t numBytes, off64_t offset, off64_t cipher_header_offset, off64_t cipher_header_size) {
    ssize_t bytesRead = -1; // returns the actual bytes read.
    if (NULL != buffer && numBytes > -1 && offset > -1) {
        CryptoHelper cr_hlp(CryptoHelper::CIPHER_AES128CBC, (BYTE*)getKey(), 0);
        bytesRead =
            CipherFileUtil::CipherFileRead(
                fd, buffer, numBytes, offset,
                cipher_header_offset,
                cipher_header_size, cr_hlp);
//        ALOGD("cipherPread() : read %d bytes", (int)bytesRead);
    } else {
        ALOGE("[ERROR][CTA5]cipherPread() : error, parameter invalid");
    }

    return bytesRead;
}

Cta5CommonMultimediaFile::Header* Cta5CommonMultimediaFile::getCipherHeader(off64_t offset) {
    for (Vector<Header*>::iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter) {
        Header *header = (*iter);
        if(offset >= header->clear_header_offset
            && offset <= header->clear_header_offset + header->clear_header_size) {
            return header;
        }
        continue;
    }
    ALOGE("[ERROR][CTA5]getCipherHeader() - can't find the specified header");
    return NULL;
}

String8 Cta5CommonMultimediaFile::getOriginalMimetype(int fd, const String8 &key) {
    //chec if it's a multimedia file
    int totalMmCtaHeaderSize = verifyCtaMultimediaHeader(fd);
    if(-1 ==  totalMmCtaHeaderSize){
        ALOGD("getOriginalMimetype - it is a normal file");
        return String8("");
    }

    uint64_t len = lseek64(fd, 0L, SEEK_END);
    //printf("len = %lld\n",len);
    int64_t offset = len - totalMmCtaHeaderSize - Cta5FileHeader::CTA5_FILE_HEADER_LEN;
    //printf("offset = %lld\n",offset);
    lseek64(fd, offset, SEEK_SET);
    uint8_t cta5FileHaderBuf[Cta5FileHeader::CTA5_FILE_HEADER_LEN] = { 0 };
    memset(cta5FileHaderBuf,0,sizeof(cta5FileHaderBuf));
    read(fd, cta5FileHaderBuf, Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    String8 cta5FileHaderStrMul((const char *) cta5FileHaderBuf,
            Cta5FileHeader::CTA5_FILE_HEADER_LEN);
    //printf("cta5FileHaderBuf %c %c\n",cta5FileHaderBuf[0],cta5FileHaderBuf[1]);
    //Cta5FileHeader cta5FileHeaderMul(cta5FileHaderStrMul);
    Cta5FileHeader cta5FileHeaderMul(fd, key);
    cta5FileHeaderMul.setCta5FileHeader(cta5FileHaderStrMul);
    if (!Cta5File::isCta5File(fd))
    {
        ALOGD("it is a normal file");
        return String8("");
    }
    String8 mime = cta5FileHeaderMul.getOriginalMimeType();
    ALOGD("getOriginalMimetype - mime[%s]", mime.string());
    return mime;
}

bool Cta5CommonMultimediaFile::changeKey(String8 oldKey, String8 newKey)
{
    ALOGD("Cta5CommonMultimediaFile changeKey oldkey = %s, newKey = %s", oldKey.string(), newKey.string());
    bool ret = mCta5FileHeader.changeKey(oldKey, newKey);
    //Write the result to file
    //find out the pos to be writen

    lseek64(mFd, 0L, SEEK_SET);
    uint64_t len = lseek64(mFd, 0LL, SEEK_END);
    int pos = CTA_MAGIC_LEN + CTA_MAX_MIEM_LEN + 128;
    int offset = len - mmCtaHeaderSize - Cta5FileHeader::CTA5_FILE_HEADER_LEN;
    if (mCta5FileHeader.getCurrentKeyInfo() == &(mCta5FileHeader.getKeyInofs()[0]))
    {
        offset += 0;
    } else if (mCta5FileHeader.getCurrentKeyInfo() == &(mCta5FileHeader.getKeyInofs()[1]))
    {
        offset += KeyInfo::KEYINFO_SIZE;
    } else
    {
        ALOGE("[ERROR][CTA5]Bad case mCurrentKeyInfo point to an invalid address");
        return false;
    }
    if (-1 == lseek(mFd, pos + offset, SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]ChangeKey fail- lseek failed:%s", strerror(errno));
        return false;
    }
    if (-1 == write(mFd, mCta5FileHeader.getCurrentKeyInfo()->getKeyInfo().string(),
                    KeyInfo::KEYINFO_SIZE))
    {
        ALOGE("[ERROR][CTA5]ChangeKey fail- write failed:%s", strerror(errno));
        return false;
    }
    mCta5FileHeader.getKeyInofs()[0].dump();
    mCta5FileHeader.getKeyInofs()[1].dump();
    return ret;
}
