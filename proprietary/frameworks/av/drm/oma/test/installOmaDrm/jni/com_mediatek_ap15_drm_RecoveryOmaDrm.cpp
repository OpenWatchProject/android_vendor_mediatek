/*
 * drm tool
 * it's used to decrypt dcf file to raw data
 */

#define LOG_TAG "mediatek_drm_RecoveryOmaDrm"
#include <utils/Log.h>

#include <jni.h>
#include <JNIHelp.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <drm/drm_framework_common.h>
#include <drm/DrmManagerClient.h>
#include <drm/DrmMetadata.h>
#include <drm/DrmMtkUtil.h>
#include <drm/DrmMtkDef.h>
#include <DrmUtil.h>
#include <ByteBuffer.h>
#include <CipherFileUtil.h>
#include <CryptoHelper.h>
#include <openssl/buffer.h>
#include <openssl/rc4.h>

#define DECRYPT_BUF_LEN 4096

using namespace android;

ByteBuffer getDrmKey(const char * id);
String8 iv("4074BCC61602B5BE");
int decrypt(const char* pathname, int fd_out, const char *id);
int saveToFile(unsigned char* decryptedBuf, int length, int fd);
String8 getStringValue(JNIEnv* env, jstring string);

/**
 * return 1 indicates success
 * return 0 indicates fail
 */
int decrypt(const char* pathname, int fd_out, const char *id)
{
    int fd = open(pathname, O_RDONLY);
    if (0 > fd)
    {
        ALOGE("decrypt: failed to open file.\n");
        return 0;
    }

    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient)
    {
        ALOGE("decrypt: failed to create DrmManagerClient");
        close(fd);
        return 0;
    }
    int totalSize = lseek(fd,0,SEEK_END);
    String8 pathstr(pathname);
    lseek(fd,0,SEEK_SET);
    String8 mime = drmManagerClient->getOriginalMimeType(pathstr, fd);

    DrmMetadata metadata;
    int isDcf = DrmUtil::parseDcf(fd, &metadata);
    ALOGD("isDcf = %d\n",isDcf);
    String8 method = metadata.get(String8(DrmMetaKey::META_KEY_METHOD));
    ALOGD("method = %s\n",method.string());

    String8 dataLen = metadata.get(String8(DrmMetaKey::META_KEY_DATALEN));
    int dataLenI = 0;
    sscanf(dataLen.string(),"%d",&dataLenI);
    ALOGD("dataLen = %s\n", dataLen.string());
    ALOGD("dataLenI = %d\n", dataLenI);

    ALOGD("mime = %s\n", mime.string());
    lseek(fd,0,SEEK_SET);

    // allocate buffer to hold decrypted data
    // just use the length of the encrypted data
    unsigned char *decryptedBuf = (unsigned char*) malloc(totalSize);
    memset(decryptedBuf, 0, totalSize);

    unsigned char buffer[DECRYPT_BUF_LEN];
    memset(buffer, 0, sizeof(buffer));

    int readSize = 0;
    int resultSize = 0; // also the offset of data
    int offset = 0;
    int readLength = 0;
    ByteBuffer keyBuf = getDrmKey(id);
    BYTE key[DrmDef::DRM_MAX_KEY_LENGTH];
    bzero(key, sizeof(key));
    memcpy(key, keyBuf.buffer(), keyBuf.length());
    while (readSize < totalSize)
    {
        offset += readLength;
        memset(buffer, 0, sizeof(buffer));
        int size =
                (totalSize - readSize) > DECRYPT_BUF_LEN ?
                        DECRYPT_BUF_LEN : (totalSize - readSize);
        //int readLength = drmManagerClient->pread(decryptHandle, buffer, size, resultSize);

        CryptoHelper cr_hlp(CryptoHelper::CIPHER_AES128CBC, key, 0);
        readLength = CipherFileUtil::CipherFileRead(fd, buffer, size, offset,
                totalSize - dataLenI, dataLenI, cr_hlp);
        if (readLength < 0)
        {
            ALOGE("decrypt: failed to read valid decrypted data.");
            free(decryptedBuf);
            return 0;
        }else if(readLength == 0) {
            ALOGD("decryption sucess, but not enough data at specified offset.");
            break;
        }
        memcpy(decryptedBuf + resultSize, buffer, readLength);
        readSize += size;
        resultSize += readLength;
    }
    if (resultSize <= 0)
    {
        ALOGE("decrypt: failed to read decrypted data.");
        free(decryptedBuf);
        return 0;
    }

    // finalize
    //drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    int result = saveToFile(decryptedBuf, resultSize, fd_out);
    free(decryptedBuf);
    return result;
}

/**
 * return 1 indicates success
 * return 0 indicates fail
 */
int saveToFile(unsigned char* decryptedBuf, int length, int fd)
{
    /*char fileName[100];
     memset(fileName, 0, sizeof(fileName));
     sprintf(fileName, "test");*/
    //int fd = open(filename, O_WRONLY | O_CREAT, S_IRWXG);
    if (0 > fd)
    {
        ALOGE("saveToFile: failed to open file: %s.\n", strerror(fd));
        return 0;
    }
    if (length != write(fd, decryptedBuf, length))
    {
        ALOGE("saveToFile: failed to write file.\n");
        return 0;
    } else
    {
        ALOGE("saveToFile: success to write file\n");
        return 1;
    }
}

// return the device unique 16 bytes drm key
ByteBuffer getDrmKey(const char * id)
{
    char drmkey[DrmDef::DRM_MAX_ID_LENGTH + 1];
    bzero(drmkey, sizeof(drmkey));

    String8 imei(id);

    char imei_encrypt_key[DrmDef::DRM_MAX_KEY_LENGTH];
    bzero(imei_encrypt_key, sizeof(imei_encrypt_key));

    // get an fixed key to encrypt imei
    long s = 0x3D4FAD6A;
    long v = ~(0xA9832DC6 ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x16F0D768 ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 4] = (char) (0xFF & (v >> (i * 8)));
    }
    v = ~(0x278FB1EA ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 8] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x5F3C54EC ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 12] = (char) (0xFF & (v >> (i * 8)));
    }

    // encrypt imei to get an unique key
    RC4_KEY k;
    RC4_set_key(&k, DrmDef::DRM_MAX_KEY_LENGTH, (unsigned char*)imei_encrypt_key);
    RC4(&k, (unsigned long)imei.length() + 1, (unsigned char*)imei.string(), (unsigned char*)drmkey);

    return ByteBuffer(drmkey, DrmDef::DRM_MAX_KEY_LENGTH);
}

String8 getStringValue(JNIEnv* env, jstring string) {
    String8 dataString("");

    if (NULL != string && string != env->NewStringUTF("")) {
        char* bytes = const_cast< char* > (env->GetStringUTFChars(string, NULL));

        const int length = strlen(bytes) + 1;
        char *data = new char[length];
        strncpy(data, bytes, length);
        dataString = String8(data);

        env->ReleaseStringUTFChars(string, bytes);
        delete [] data; data = NULL;
    }
    return dataString;
}

/**
 * return 1 indicates success
 * return 0 indicates fail
 */
static jint mediatek_drm_RecoveryOmaDrm_recoveryOmaDrmFile(
            JNIEnv* env, jobject thiz, jstring jdevideId, jstring jpath,
            jobject fileDescriptorOutput) {
    ALOGD("recoveryOmaDrmFile Enter fileDescriptorOutput = %p",fileDescriptorOutput);

    int fdOut =
            (fileDescriptorOutput == NULL) ?
                    -1 : jniGetFDFromFileDescriptor(env, fileDescriptorOutput);

    if (fdOut == -1)
    {
        ALOGE("recoveryOmaDrmFile fail fdOut = -1");
        return 0;
    }

    const String8 devideId = getStringValue(env, jdevideId);
    const String8 pathname = getStringValue(env, jpath);
    ALOGD("recoveryOmaDrmFile devideId = %s,",devideId.string());
    ALOGD("recoveryOmaDrmFile pathname = %s,",pathname.string());
    int ret = decrypt(pathname,fdOut, devideId);
    ALOGD("recoveryOmaDrmFile Exit- decrypt result = %d", ret);
    return ret;
}

static JNINativeMethod nativeMethods[] = {
    {"recoveryOmaDrmFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/io/FileDescriptor;)I",
                                    (void*)mediatek_drm_RecoveryOmaDrm_recoveryOmaDrmFile},
};

static int registerNativeMethods(JNIEnv* env) {
    int result = -1;
    /* look up the class */
    return jniRegisterNativeMethods(env, "com/mediatek/ap15/drm/RecoveryOmaDrmFile",
          nativeMethods, NELEM(nativeMethods));

    return result;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        ALOGD("env = %p", env);

        if (NULL != env && registerNativeMethods(env) == 0) {
            result = JNI_VERSION_1_4;
        }
    }
    return result;
}

