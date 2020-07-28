
#define LOG_TAG "Cta5File"
#include <utils/Log.h>

#include <Cta5File.h>
#include <sys/types.h>
#include <unistd.h>
#include <DrmCtaUtil.h>
#include <DrmDef.h>
#include <drm/DrmInfoEvent.h>

#include <openssl/hmac.h>

using namespace android;
KeyedVector<String8, String8> sCta5MimeTypeMap;
//sCta5MimeTypeMap.
String8 type("video");
String8 ctaType("application/cta-video");

const uint32_t Cta5FileUtil::CRC32Table[256] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
        0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
        0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
        0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
        0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
        0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
        0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
        0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
        0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
        0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
        0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
        0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
        0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
        0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
        0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
        0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
        0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
        0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
        0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
        0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
        0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
        0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};


uint16_t Cta5FileUtil::nettohosts(uint16_t networks)
{
    uint8_t input[2];
    memcpy(input, &networks, 2);
    return ((((uint32_t) input[0]) << 8) | ((uint32_t) input[1]));
}

uint32_t Cta5FileUtil::nettohostl(uint32_t networkl)
{
    uint8_t input[4];
    memcpy(input, &networkl, 4);

    return ((((uint32_t) input[0]) << 24) | (((uint32_t) input[1]) << 16)
            | (((uint32_t) input[2]) << 8) | ((uint32_t) input[3]));
}

uint64_t Cta5FileUtil::nettohostll(uint64_t networkll)
{
    uint8_t input[8];
    memcpy(input, &networkll, sizeof(input));

    return ((((uint64_t) input[0]) << 56) | (((uint64_t) input[1]) << 48)
            | (((uint64_t) input[2]) << 40) | (((uint64_t) input[3]) << 32)|
            (((uint64_t) input[4]) << 24) | (((uint64_t) input[5]) << 16)
                        | (((uint64_t) input[6]) << 8) | ((uint64_t) input[7]));
}

uint16_t Cta5FileUtil::hosttonets(uint16_t hosts)
{
    uint8_t result_bytes[2];
    result_bytes[0] = (uint8_t) ((hosts >> 8) & 0xFF);
    result_bytes[1] = (uint8_t) (hosts & 0xFF);

    uint16_t result;
    memcpy(&result, result_bytes, 2);
    return result;
}

uint32_t Cta5FileUtil::hosttonetl(uint32_t hostl)
{
    uint8_t result_bytes[4];
    result_bytes[0] = (uint8_t) ((hostl >> 24) & 0xFF);
    result_bytes[1] = (uint8_t) ((hostl >> 16) & 0xFF);
    result_bytes[2] = (uint8_t) ((hostl >> 8) & 0xFF);
    result_bytes[3] = (uint8_t) (hostl & 0xFF);

    uint32_t result;
    memcpy(&result, result_bytes, 4);
    return result;
}

uint64_t Cta5FileUtil::hosttonetll(uint64_t hostll)
{
    uint8_t result_bytes[8];
    result_bytes[0] = (uint8_t) ((hostll >> 56) & 0xFF);
    result_bytes[1] = (uint8_t) ((hostll >> 48) & 0xFF);
    result_bytes[2] = (uint8_t) ((hostll >> 40) & 0xFF);
    result_bytes[3] = (uint8_t) ((hostll >> 32) & 0xFF);
    result_bytes[4] = (uint8_t) ((hostll >> 24) & 0xFF);
    result_bytes[5] = (uint8_t) ((hostll >> 16) & 0xFF);
    result_bytes[6] = (uint8_t) ((hostll >> 8) & 0xFF);
    result_bytes[7] = (uint8_t) (hostll & 0xFF);

    uint64_t result;
    memcpy(&result, result_bytes, sizeof(result_bytes));
    return result;
}


String8 Cta5FileUtil::getPathFromFd(int fd)
{
    char buffer[256];
    char linkto[4096];
    memset(buffer, 0, 256);
    memset(linkto, 0, 256);
    snprintf(buffer,sizeof(buffer), "/proc/%d/fd/%d", gettid(), fd);
    int len = 0;
    len = readlink(buffer, linkto, sizeof(linkto));
    if(len > 4096)
    {
        ALOGE("The file path is too long : %d", len);
        String8 path;
        return path;
    }
    return String8::format("%s",linkto);

}

bool Cta5FileUtil::isDcfFile(int fd)
{
    String8 path = getPathFromFd(fd);
    String8 ext = path.getPathExtension();
    ALOGD("path extension: %s", ext.string());
    if(0 == ext.compare(String8(".dcf"))) {
        return true;
    }
    return false;
}


uint32_t Cta5FileUtil::getNormalFromUintVar(String8 uvar)
{
    uint32_t value = 0;
    uint32_t len = uvar.length();
    uint32_t i = 0;
    uint8_t *str = (uint8_t *) uvar.string();
    while (i < len)
    {
        uint8_t c = str[i];
        printf("i = %u,c = 0x%x\n",i, c);
        value <<= 7;
        value |= (c & 0x7f);
        printf("value = 0x%x\n",value);
        if ((c & 0x80) == 0)
        {
            break;
        }
        ++i;
    }
    return value;
}

String8 Cta5FileUtil::getUintVarFromNormal(uint32_t normal)
{
    String8 str8;
    //printf("getUintVarFromNormal:normal-%x\n",normal);
    if (normal <= 0x0000007F)
    {
        //printf("One bytes\n");
        //Only one byte
        char c = (char) normal;
        str8.append(&c, 1);
    } else if (normal <= 0x00003FFF)
    {
        //printf("Two bytes\n");
        //Two bytes
        //Get the last 7 bits, and set continue bit to 0
        char lastByte = normal & 0x0000007F;

        char firstByte = (normal & 0x00003F80) >> 7;
        //set continue bit to 1
        firstByte = firstByte | 0x80;

        str8.append(&firstByte, 1);
        str8.append(&lastByte, 1);
    } else if (normal <= 0x001FFFFF)
    {
        //printf("Three bytes\n");
        //Three bytes
        //Get the last 7 bits, and set continue bit to 0
        char lastByte = normal & 0x0000007F;
        //printf("lastByte bytes:0x%x\n",lastByte);

        char middleByte = (normal & 0x00003F80) >> 7;
        //set continue bit to 1
        middleByte = middleByte | 0x80;
        //printf("middleByte bytes:0x%x\n",middleByte);

        char firstByte = normal >> 14;
        //set continue bit to 1
        firstByte = firstByte | 0x80;
        //printf("firstByte bytes:0x%x\n",firstByte);

        str8.append(&firstByte, 1);
        str8.append(&middleByte, 1);
        str8.append(&lastByte, 1);

    } else if (normal <= 0x0FFFFFFF)
    {
        //printf("Four bytes\n");
        //Four bytes
        //Get the last 7 bits, and set continue bit to 0
        char lastByte = normal & 0x0000007F;

        //Get the last second 7 bits, and set continue bit to 1
        char lastSecondByte = (normal & 0x00003F80) >> 7;
        //set continue bit to 1
        lastSecondByte = lastSecondByte | 0x80;

        //Get the last third 7 bits, and set continue bit to 1
        char lastThirdByte = (normal & 0x00003FC000) >> 14;
        //set continue bit to 1
        lastThirdByte = lastThirdByte | 0x80;

        char firstByte = normal >> 21;
        //set continue bit to 1
        firstByte = firstByte | 0x80;

        str8.append(&firstByte, 1);
        str8.append(&lastThirdByte, 1);
        str8.append(&lastSecondByte, 1);
        str8.append(&lastByte, 1);
    } else
    {
        //printf("Five bytes\n");
        //Five bytes
        //Get the last 7 bits, and set continue bit to 0
        char lastByte = normal & 0x0000007F;
        //printf("lastByte bytes:0x%x\n",lastByte);

        //Get the last second 7 bits, and set continue bit to 1
        char lastSecondByte = (normal & 0x00003F80) >> 7;
        //set continue bit to 1
        lastSecondByte = lastSecondByte | 0x80;
        //printf("lastSecondByte bytes:0x%x\n",lastSecondByte);

        //Get the last third 7 bits, and set continue bit to 1
        char lastThirdByte = (normal & 0x00003FC000) >> 14;
        //set continue bit to 1
        lastThirdByte = lastThirdByte | 0x80;
        //printf("lastThirdByte bytes:0x%x\n",lastThirdByte);

        //Get the last third 7 bits, and set continue bit to 1
        char lastFourthByte = (normal & 0x00003FE00000) >> 21;
        //set continue bit to 1
        lastFourthByte = lastThirdByte | 0x80;
        //printf("lastFourthByte bytes:0x%x\n",lastFourthByte);

        char firstByte = normal >> 28;
        //set continue bit to 1
        firstByte = firstByte | 0x80;
        //printf("firstByte bytes:0x%x\n",firstByte);

        str8.append(&firstByte, 1);
        str8.append(&lastFourthByte, 1);
        str8.append(&lastThirdByte, 1);
        str8.append(&lastSecondByte, 1);
        str8.append(&lastByte, 1);
    }
    return str8;
}

/**
 *
 * @param key the key used to generate hmac msg
 * @param msg the message to be signed with md5
 * @param out should be a valid pointer whose length should be 16
 */
bool Cta5FileUtil::hmac128Md5Digest(const uint8_t *key, const uint8_t *msg, size_t len, uint8_t *out)
{
    HMAC_CTX ctx;
    unsigned int outLen = MD5_DIGEST_LENGTH;

    HMAC_CTX_init(&ctx);
    int res = HMAC_Init_ex(&ctx, key, sizeof(key), EVP_md5(), NULL);
    if (!res) {
        ALOGE("HMAC_Init_ex EVP_md5 failed");
        goto error;
    }

    if (!HMAC_Update(&ctx, msg, len)) {
        ALOGE("HMAC_Update failed");
        goto error;
    }

    if (!HMAC_Final(&ctx, out, &outLen)) {
        ALOGE("HMAC_Final failed");
        goto error;
    }

    HMAC_CTX_cleanup(&ctx);
    return true;

    error:
    HMAC_CTX_cleanup(&ctx);
    return false;
}

bool Cta5FileUtil::md5Digest(const uint8_t *msg, size_t len, uint8_t *out)
{
    MD5_CTX c;
    int ret = MD5_Init(&c);
    ret = MD5_Update(&c, (const unsigned char *) msg, (size_t) len);
    ret = MD5_Final((unsigned char *) out, &c);
    OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
    return true;
}

bool Cta5FileUtil::aes128cbcEncrypt(const uint8_t *in, uint8_t *out, size_t len,
        const uint8_t *key, uint8_t *iv)
{
    if (in == NULL || out == NULL || key == NULL)
    {
        ALOGE("bad parameters\n");
        return false;
    }
    AES_KEY aesKey;
    if (AES_set_encrypt_key(key, 128, &aesKey) != 0)
    {
        ALOGE("AES_set_decrypt_key fail\n");
        return false;
    }
    AES_cbc_encrypt(in, out, len, &aesKey, iv, true);
    return true;
}

bool Cta5FileUtil::aes128cbcDecrypt(const uint8_t *in, uint8_t *out, size_t len,
        const uint8_t *key, uint8_t *iv)
{
    if (in == NULL || out == NULL || key == NULL)
    {
        ALOGE("bad parameters\n");
        return false;
    }
    AES_KEY aesKey;
    if (AES_set_decrypt_key(key, 128, &aesKey) != 0)
    {
        printf("AES_set_decrypt_key fail\n");
        return false;
    }
    AES_cbc_encrypt(in, out, len, &aesKey, iv, false);
    return true;
}

#define INIT_CRC32 0xffffffff

uint32_t Cta5FileUtil::wvrunningcrc32(const uint8_t* p_begin, int i_count, uint32_t i_crc)
{

    /* Calculate the CRC */
    while (i_count > 0)
    {
        i_crc = (i_crc << 8) ^ CRC32Table[(i_crc >> 24) ^ ((uint32_t) *p_begin)];
        p_begin++;
        i_count--;
    }
    return (i_crc);
}

uint32_t Cta5FileUtil::wvcrc32(const uint8_t* p_begin, int i_count)
{
    return (wvrunningcrc32(p_begin, i_count, INIT_CRC32));
}

//Convert the result to network order
uint32_t Cta5FileUtil::wvcrc32n(const uint8_t* p_begin, int i_count)
{
    return hosttonetl(wvrunningcrc32(p_begin, i_count, INIT_CRC32));
}


//////////////////////////
//keyInfo

KeyInfo::KeyInfo()
{
    memset(mHash, 0, sizeof(mHash));
    memset(mSalt, 0, sizeof(mSalt));
    memset(mEKey, 0, sizeof(mEKey));
    memset(mDerivedKey, 0, sizeof(mDerivedKey));
    memset(mContentKey, 0, sizeof(mContentKey));
    mCount = 0;
    mCrc = 0;
}

KeyInfo::KeyInfo(const KeyInfo &keyInfo)
{
    memcpy(mHash, keyInfo.mHash, sizeof(mHash));
    memcpy(mSalt, keyInfo.mSalt, sizeof(mSalt));
    memcpy(mEKey, keyInfo.mEKey, sizeof(mEKey));
    memcpy(mDerivedKey, keyInfo.mDerivedKey, sizeof(mDerivedKey));
    memcpy(mContentKey, keyInfo.mContentKey, sizeof(mContentKey));
    //Do not modify count and CRC
    //mCount = keyInfo.mCount;
    //mCrc = keyInfo.mCrc;
}

KeyInfo::KeyInfo(String8 keyInfo)
{
    setKeyInfo(keyInfo.string());
}

bool KeyInfo::setKeyInfo(const char *keyInfo)
{
    int pos = 0;
    memcpy(mHash, keyInfo, sizeof(mHash));
    pos += sizeof(mHash);
    memcpy(mSalt, keyInfo + pos, sizeof(mSalt));
    pos += sizeof(mSalt);
    memcpy(mEKey, keyInfo + pos, sizeof(mEKey));
    pos += sizeof(mEKey);
    mCount = Cta5FileUtil::nettohostll(*(uint64_t*) (keyInfo + pos));
    //printf("mCount = %lld\n",mCount);
    pos = pos + sizeof(mCount);
    mCrc = Cta5FileUtil::nettohostl(*(uint32_t*) (keyInfo + pos));
    return true;
}

String8 KeyInfo::getKeyInfo()
{
    String8 str8;
    //str8.append((const char*) mHash, sizeof(mHash));
    //str8.append((const char*) mSalt, sizeof(mSalt));
    //str8.append((const char*) mEKey, sizeof(mEKey));
    uint64_t count = Cta5FileUtil::hosttonetll(mCount);
    //str8.append((const char*) &count, sizeof(count));
    uint32_t crc = Cta5FileUtil::hosttonetl(mCrc);
    //str8.append((const char*) &crc, sizeof(crc));
    uint8_t keyInfo[KEYINFO_SIZE];
    memset(keyInfo, 0, sizeof(keyInfo));
    int pos = 0;
    memcpy(keyInfo + pos, mHash, sizeof(mHash));
    pos += sizeof(mHash);
    memcpy(keyInfo + pos, mSalt, sizeof(mSalt));
    pos += sizeof(mSalt);
    memcpy(keyInfo + pos, mEKey, sizeof(mEKey));
    pos += sizeof(mEKey);
    memcpy(keyInfo + pos, &count, sizeof(count));
    pos += sizeof(count);
    memcpy(keyInfo + pos, &crc, sizeof(crc));
    pos += sizeof(crc);
    str8.append((const char *)keyInfo, sizeof(keyInfo));
    return str8;
}

//Derive key and calculate CRC
bool KeyInfo::deriveKeyInfo(String8 key)
{
    ALOGD("KeyInfo::deriveKeyInfo : key-%s, length = %d",key.string(), (int) key.length());
    //Generate salt
    // initialize seed
    srand (time(NULL));int32_t randNum = rand();
    bool ret = false;
    ret = Cta5FileUtil::md5Digest((const uint8_t *) &randNum, (size_t) sizeof(randNum), mSalt);
    if(ret == false)
    {
        ALOGE("deriveKeyInfo fail because of md5Digest failed(salt)\n");
        return false;
    }
    /*ALOGD("mSalt:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            mSalt[0], mSalt[1],mSalt[2], mSalt[3], mSalt[4], mSalt[5], mSalt[6], mSalt[7], mSalt[8], mSalt[9],
            mSalt[10], mSalt[11], mSalt[12], mSalt[13], mSalt[14], mSalt[15]);*/
    //generate derivedKey form salt and key
    ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *)mSalt, (const uint8_t *)key.string(),
            (size_t)key.length(), mDerivedKey);
    //Iterate 2000 times to make the key hard to hack
    for(int i = 0; i < 2000; ++i)
    {
        ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *)mSalt, (const uint8_t *)mDerivedKey,
                    sizeof(mDerivedKey), mDerivedKey);
    }
    if(ret == false)
    {
        ALOGE("deriveKeyInfo fail because of hmac128Md5Digest failed\n");
        return false;
    }
    /*ALOGD("mDerivedKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               mDerivedKey[0], mDerivedKey[1], mDerivedKey[2],mDerivedKey[3], mDerivedKey[4], mDerivedKey[5], mDerivedKey[6], mDerivedKey[7], mDerivedKey[8], mDerivedKey[9],
               mDerivedKey[10], mDerivedKey[11], mDerivedKey[12], mDerivedKey[13], mDerivedKey[14], mDerivedKey[15]);*/
    //Generate hash
    ret = Cta5FileUtil::md5Digest((const uint8_t *) mDerivedKey, (size_t) sizeof(mDerivedKey),
            mHash);
    if(ret == false)
    {
        ALOGE("deriveKeyInfo fail because of md5Digest failed(hash)\n");
        return false;
    }
    /*ALOGD("mHash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            mHash[0], mHash[1],mHash[2], mHash[3], mHash[4], mHash[5], mHash[6], mHash[7], mHash[8], mHash[9],
            mHash[10], mHash[11], mHash[12], mHash[13], mHash[14], mHash[15]);*/
    //Generate content key
    randNum = rand();
    ret = Cta5FileUtil::md5Digest((const uint8_t *) &randNum, (size_t) sizeof(randNum), mContentKey);
    if(ret == false)
    {
        ALOGE("deriveKeyInfo fail because of md5Digest failed(encrypted key)\n");
        return false;
    }
    /*ALOGD("mContentKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            mContentKey[0], mContentKey[1],mContentKey[2], mContentKey[3], mContentKey[4], mContentKey[5], mContentKey[6], mContentKey[7], mContentKey[8], mContentKey[9],
            mContentKey[10], mContentKey[11], mContentKey[12], mContentKey[13], mContentKey[14], mContentKey[15]);*/
    //Encrypt content to mEKey
    uint8_t iv[CTA_KEY_LEN];
    memcpy(iv , IV, sizeof(iv));
    ret = Cta5FileUtil::aes128cbcEncrypt(mContentKey,mEKey,sizeof(mContentKey),mDerivedKey,iv);
    if(ret == false)
    {
        ALOGE("deriveKeyInfo fail because of encrypt content key failed\n");
        return false;
    }
    calculateCRC();
   // dump();
    return true;

}

bool KeyInfo::calculateCRC()
{
    int length = sizeof(mHash) + sizeof(mSalt) + sizeof(mEKey) + sizeof(mCount);
    uint8_t data[sizeof(mHash) + sizeof(mSalt) + sizeof(mEKey) + sizeof(mCount)] = {0};
    int pos = 0;
    memcpy(data + pos, mHash, sizeof(mHash));
    pos += sizeof(mHash);
    memcpy(data + pos, mSalt, sizeof(mSalt));
    pos += sizeof(mSalt);
    memcpy(data + pos, mEKey, sizeof(mEKey));
    pos += sizeof(mEKey);
    memcpy(data + pos, &mCount, sizeof(mCount));
    mCrc = Cta5FileUtil::wvcrc32n(data, length);
    return true;
}

//The mHash, mDerivedKey and mEKey will be updated
//The mCrc and mCount will also be updated
bool KeyInfo::changeKeyInfo(String8 key)
{
    //Update derivedKey
    ALOGD("changeKeyInfo");
    bool ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt, (const uint8_t *) key.string(),
            (size_t) key.length(), mDerivedKey);
    //Iterate 2000 times to make the key hard to hack
    for (int i = 0; i < 2000; ++i)
    {
        ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt, (const uint8_t *) mDerivedKey,
                sizeof(mDerivedKey), mDerivedKey);
    }
    if (ret == false)
    {
        ALOGE("changeKeyInfo fail because of hmac128Md5Digest failed\n");
        return false;
    }

    //Update hash
    ret = Cta5FileUtil::md5Digest((const uint8_t *) mDerivedKey, (size_t) sizeof(mDerivedKey),
            mHash);
    if (ret == false)
    {
        ALOGE("changeKeyInfo fail because of md5Digest failed(hash)\n");
        return false;
    }

    //Update mEKey
    uint8_t iv[CTA_KEY_LEN];
    memcpy(iv, IV, sizeof(iv));
    ret = Cta5FileUtil::aes128cbcEncrypt(mContentKey, mEKey, sizeof(mContentKey), mDerivedKey, iv);
    if (ret == false)
    {
        ALOGE("changeKeyInfo fail because of encrypt content key failed\n");
        return false;
    }
    //Update mCount
    ++mCount;
    //Update mCrc
    calculateCRC();
    return true;
}

//Before call this function, the mSalt, mHash and mEkey must be restored from file
bool KeyInfo::restoreKeyInfo(String8 key)
{
    ALOGD("restoreKeyInfo: key:%s, length:%d, this:%p", key.string(), (int) key.length(), this);
    //generate derivedKey form salt and key
   /* ALOGD("mSalt:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                mSalt[0], mSalt[1], mSalt[2], mSalt[3], mSalt[4], mSalt[5], mSalt[6], mSalt[7], mSalt[8],
                mSalt[9], mSalt[10], mSalt[11], mSalt[12], mSalt[13], mSalt[14], mSalt[15]);*/
    bool ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt,
            (const uint8_t *) key.string(), (size_t) key.length(), mDerivedKey);
    //Iterate 2000 times to make the key hard to hack
    for (int i = 0; i < CTA_KDF_NUM; ++i)
    {
        ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt, (const uint8_t *) mDerivedKey,
                sizeof(mDerivedKey), mDerivedKey);
    }
    if (ret == false)
    {
        ALOGE("restoreKeyInfo fail because of hmac128Md5Digest failed\n");
        return false;
    }
    /*ALOGD("mDerivedKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            mDerivedKey[0], mDerivedKey[1],mDerivedKey[2], mDerivedKey[3], mDerivedKey[4], mDerivedKey[5], mDerivedKey[6], mDerivedKey[7], mDerivedKey[8], mDerivedKey[9],
            mDerivedKey[10], mDerivedKey[11], mDerivedKey[12], mDerivedKey[13], mDerivedKey[14], mDerivedKey[15]);*/
    //check if key is valid
    //Generate hash
    uint8_t hash[CTA_KEY_LEN] =
    { 0 };
    ret = Cta5FileUtil::md5Digest((const uint8_t *) mDerivedKey, (size_t) sizeof(mDerivedKey),
            hash);
    /*ALOGD("hash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", hash[0], hash[1],
            hash[2],hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12],
                    hash[13], hash[14], hash[15]);*/
    if (ret == false)
    {
        ALOGE("restoreKeyInfo fail because of md5Digest failed(hash)\n");
        return false;
    }
    if (memcmp(hash, mHash, sizeof(mHash)) != 0)
    {
        ALOGE("restoreKeyInfo  fail - invalid key");
        ALOGE("mHash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", mHash[0], mHash[1],
                mHash[2],mHash[3], mHash[4], mHash[5], mHash[6], mHash[7], mHash[8], mHash[9], mHash[10], mHash[11], mHash[12],
                        mHash[13], mHash[14], mHash[15]);
        ALOGE("hash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", hash[0], hash[1],
                hash[2],hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12],
                hash[13], hash[14], hash[15]);
        return false;
    }
    //restore content key
    //decrypt mEKey to mContentKey
    uint8_t iv[CTA_KEY_LEN];
    memcpy(iv, IV, sizeof(iv));
    ret = Cta5FileUtil::aes128cbcDecrypt(mEKey, mContentKey, sizeof(mContentKey), mDerivedKey, iv);
    /*ALOGE("mContentKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", mContentKey[0], mContentKey[1],
            mContentKey[2],mContentKey[3], mContentKey[4], mContentKey[5], mContentKey[6], mContentKey[7], mContentKey[8],
            mContentKey[9], mContentKey[10], mContentKey[11], mContentKey[12],
            mContentKey[13], mContentKey[14], mContentKey[15]);*/
    if (ret == false)
    {
        ALOGE("restoreKeyInfo fail because of decrypt content key failed\n");
        return false;
    }
    //dump();
    ALOGD("restore keyinfo success");
    return true;
}


bool KeyInfo::isKeyValid(String8 key)
{
    ALOGD("isKeyValid: key:%s, length:%d, this=%p", key.string(), (int) key.length(), this);
    //generate derivedKey form salt and key
    uint8_t tmp_derivedKey[CTA_KEY_LEN] = {0};
    bool ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt,
            (const uint8_t *) key.string(), (size_t) key.length(), tmp_derivedKey);
    //Iterate 2000 times to make the key hard to hack
    for (int i = 0; i < CTA_KDF_NUM; ++i)
    {
        ret = Cta5FileUtil::hmac128Md5Digest((const uint8_t *) mSalt, (const uint8_t *) tmp_derivedKey,
                sizeof(tmp_derivedKey), tmp_derivedKey);
    }
    if (ret == false)
    {
        ALOGE("isKeyValid fail because of hmac128Md5Digest failed\n");
        return false;
    }
    //check if key is valid
    //Generate hash
    uint8_t hash[CTA_KEY_LEN] = { 0 };
    ret = Cta5FileUtil::md5Digest((const uint8_t *) tmp_derivedKey, sizeof(tmp_derivedKey),
            hash);
    /*ALOGD("tmp_derivedKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            tmp_derivedKey[0], tmp_derivedKey[1], tmp_derivedKey[2], tmp_derivedKey[3], tmp_derivedKey[4], tmp_derivedKey[5], tmp_derivedKey[6], tmp_derivedKey[7], tmp_derivedKey[8], tmp_derivedKey[9],
            tmp_derivedKey[10], tmp_derivedKey[11], tmp_derivedKey[12], tmp_derivedKey[13], tmp_derivedKey[14], tmp_derivedKey[15]);
    ALOGD("mDerivedKey:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                mDerivedKey[0], mDerivedKey[1], mDerivedKey[2], mDerivedKey[3], mDerivedKey[4], mDerivedKey[5], mDerivedKey[6], mDerivedKey[7], mDerivedKey[8], mDerivedKey[9],
                mDerivedKey[10], mDerivedKey[11], mDerivedKey[12], mDerivedKey[13], mDerivedKey[14], mDerivedKey[15]);
    dump();*/
    if (ret == false)
    {
        ALOGE("isKeyValid fail because of md5Digest failed(hash)\n");
        return false;
    }
    if (memcmp(hash, mHash, sizeof(mHash)) != 0)
    {
        ALOGE("isKeyValid fail - invalid key");
        ALOGE("mHash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", mHash[0], mHash[1],
                mHash[2],mHash[3], mHash[4], mHash[5], mHash[6], mHash[7], mHash[8], mHash[9], mHash[10], mHash[11], mHash[12],
                mHash[13], mHash[14], mHash[15]);
        ALOGE("hash:0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", hash[0], hash[1],
                hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12],
                hash[13], hash[14], hash[15]);
        return false;
    }
    ALOGD("isKeyValid return valid");
    return true;
}

uint8_t *KeyInfo::getContentKey()
{
    return (uint8_t *)mContentKey;
}

uint64_t KeyInfo::getCount()
{
    return mCount;
}
/*void KeyInfo::setCount(uint64_t count)
{
    mCount = count;
}*/

void KeyInfo::dump()
{
    // char buf[4096] = {0};
    String8 keyinfo("KeyInfo[mHash:");
    for (size_t i = 0; i < sizeof(mHash); ++i)
    {
        keyinfo.appendFormat("%02x", mHash[i]);
    }
    keyinfo.append(",mSalt:");
    for (size_t i = 0; i < sizeof(mSalt); ++i)
    {
        keyinfo.appendFormat("%02x", mSalt[i]);
    }
    keyinfo.append(",mEKey:");
    for (size_t i = 0; i < sizeof(mEKey); ++i)
    {
        keyinfo.appendFormat("%02x", mEKey[i]);
    }
    keyinfo.append(",mDerivedKey:");
    for (size_t i = 0; i < sizeof(mDerivedKey); ++i)
    {
        keyinfo.appendFormat("%02x", mDerivedKey[i]);
    }
    keyinfo.append(",mContentKey:");
    for (size_t i = 0; i < sizeof(mContentKey); ++i)
    {
        keyinfo.appendFormat("%02x", mContentKey[i]);
    }
    keyinfo.appendFormat(",mCount:%ld", (long) mCount);
    keyinfo.appendFormat(",mCrc:%u]", mCrc);
    ALOGD("%s", keyinfo.string());
}

/////////////////////////////
//PartialEncryptHeader
PartialEncryptHeader::PartialEncryptHeader()
{
    mClearHeaderOffset = 0;
    mClearHeaderLen = 0;
    mCipherHeaderOffset = 0;
    mCipherHeaderLen = 0;
}

String8 PartialEncryptHeader::getPartialEncryptHeader()
{
    String8 str8;
    uint64_t clearHeaderOffset = Cta5FileUtil::hosttonetll(mClearHeaderOffset);
    //str8.append((const char*) &clearHeaderOffset, sizeof(clearHeaderOffset));

    uint64_t clearHeaderLen = Cta5FileUtil::hosttonetll(mClearHeaderLen);
    //str8.append((const char*) &clearHeaderLen, sizeof(clearHeaderLen));

    uint64_t cipherHeaderOffset = Cta5FileUtil::hosttonetll(mCipherHeaderOffset);
    //str8.append((const char*) &cipherHeaderOffset, sizeof(cipherHeaderOffset));

    uint64_t cipherHeaderLen = Cta5FileUtil::hosttonetll(mCipherHeaderLen);
    //str8.append((const char*) &cipherHeaderLen, sizeof(cipherHeaderLen));
    printf("PartialEncryptHeader len = %d\n", (int) str8.length());
    uint8_t header[PARTIAL_ENCRYPT_HEADER_SIZE];
    memset(header, 0, sizeof(header));
    int pos = 0;
    memcpy(header + pos, &clearHeaderOffset, sizeof(clearHeaderOffset));
    pos += sizeof(clearHeaderOffset);
    memcpy(header + pos, &clearHeaderLen, sizeof(clearHeaderLen));
    pos += sizeof(clearHeaderLen);
    memcpy(header + pos, &cipherHeaderOffset, sizeof(cipherHeaderOffset));
    pos += sizeof(cipherHeaderOffset);
    memcpy(header + pos, &cipherHeaderLen, sizeof(cipherHeaderLen));
    pos += sizeof(cipherHeaderLen);
    str8.append((const char *)header, sizeof(header));
    return str8;
}

bool PartialEncryptHeader::setPartialEncryptHeader(const char *header)
{
    int pos = 0;
    mClearHeaderOffset = Cta5FileUtil::nettohostll(*(uint64_t*) (header + pos));
    pos = pos + sizeof(mClearHeaderOffset);
    mClearHeaderLen = Cta5FileUtil::nettohostll(*(uint64_t*) (header + pos));
    pos = pos + sizeof(mClearHeaderLen);
    mCipherHeaderOffset = Cta5FileUtil::nettohostll(*(uint64_t*) (header + pos));
    pos = pos + sizeof(mCipherHeaderOffset);
    mCipherHeaderLen = Cta5FileUtil::nettohostll(*(uint64_t*) (header + pos));
    return true;
}

uint64_t PartialEncryptHeader::getClearHeaderOffset()
{
    return mClearHeaderOffset;
}

uint64_t PartialEncryptHeader::getClearHeaderLen()
{
    return mClearHeaderLen;
}

uint64_t PartialEncryptHeader::getCipherHeaderOffset()
{
    return mCipherHeaderOffset;
}

uint64_t PartialEncryptHeader::getCipherHeaderLen()
{
    return mCipherHeaderLen;
}

bool PartialEncryptHeader::setClearHeaderOffset(uint64_t offset)
{
    mClearHeaderOffset = offset;
    return true;
}

bool PartialEncryptHeader::setClearHeaderLen(uint64_t len)
{
    mClearHeaderLen = len;
    return true;
}

bool PartialEncryptHeader::setCipherHeaderOffset(uint64_t offset)
{
    mCipherHeaderOffset = offset;
    return true;
}

bool PartialEncryptHeader::setCipherHeaderLen(uint64_t len)
{
    mCipherHeaderLen = len;
    return true;
}

void PartialEncryptHeader::dump()
{
    String8 str;
    str.append("PartialEncryptHeader[");
    str.appendFormat(",mClearHeaderOffset = %ld", (long) mClearHeaderOffset);
    str.appendFormat(",mClearHeaderLen = %ld", (long) mClearHeaderLen);
    str.appendFormat("mCipherHeaderOffset = %ld", (long) mCipherHeaderOffset);
    str.appendFormat("mCipherHeaderLen = %ld]", (long) mCipherHeaderLen);
}
/////////////////////////////
//Cta5FileHeader

const uint8_t Cta5FileHeader:: HMAC_KEY[MD5_DIGEST_LENGTH] =
{ 0x01, 0xFE, 0x9A, 0x50, 0x1C, 0x63, 0xA0, 0x2A, 0x21, 0x5B, 0x49, 0x31, 0xAA, 0xB5, 0xB6, 0x80 };


Cta5FileHeader::Cta5FileHeader(int fd, String8 key)
{
    memset(mReserved, 0, sizeof(mReserved));
    //memset(mKeyInfo, 0, sizeof(mKeyInfo));
    mIsPartialEncrypted = 0;
    mHeaderLen = 0;
    memset(mMagic,0,sizeof(mMagic));
    printf("Cta5FileHeader constructor \n");
    mFd = fd;
    //Parse info from fd the sub class will call Cta5FileHeader
    lseek(mFd, 0, SEEK_SET);
    mTotalLen = lseek(mFd, 0, SEEK_END);
    lseek(mFd, 0, SEEK_SET);
    mCurrentKeyInfo = &mKeyInfo[0];
    mNextKeyInfo = &mKeyInfo[1];
    //All the field will be updated in setCta5FileHeader()
    mUserKey = key;
}

void Cta5FileHeader::_init(String8 mimeType, uint64_t dataLen, String8 key, bool needPartialEncrypt)
{
    memset(mReserved, 0, sizeof(mReserved));
    memset(mMimeType, 0, sizeof(mMimeType));
    memset(mHeaderSig, 0, sizeof(mHeaderSig));
    memset(mContentSig, 0, sizeof(mContentSig));
    if (mimeType.length() > sizeof(mMimeType) - strlen(CTA_MIME_PREFIX))
    {
        ALOGE("[ERROR][CTA5]Bad mimetype:%s", mimeType.string());
    }
    memcpy(mMimeType, CTA_MIME_PREFIX, strlen(CTA_MIME_PREFIX));
    memcpy(mMimeType + strlen(CTA_MIME_PREFIX), mimeType.string(), mimeType.length());
    mIsPartialEncrypted = needPartialEncrypt;
    mTotalLen = dataLen + CTA5_FILE_HEADER_LEN;
    mHeaderLen = CTA5_FILE_HEADER_LEN;
    memcpy(mMagic, CTA_MAGIC, sizeof(mMagic));
    mCurrentKeyInfo = &mKeyInfo[0];
    mNextKeyInfo = &mKeyInfo[1];
    mCurrentKeyInfo->deriveKeyInfo(key);
    //Calculate header signature
    //The multi media file should re-calculate it
    String8 sigStr = calculateHeaderSig();
    memcpy(mHeaderSig,sigStr.string(), sizeof(mHeaderSig));
    //The file content's md5 signature should be set by the client
    mUserKey = key;
}

/**
 * Calculate the header's md5 signature
 * The header do not contain mHeaderSig and mContentSig
 */
String8 Cta5FileHeader::calculateHeaderSig()
{
    String8 sig;
    String8 header = this->getCtaHeader();
    uint8_t out[CTA_KEY_LEN];
    memset(out, 0, sizeof(out));
    Cta5FileUtil::md5Digest((const uint8_t *)header.string()
            , header.length() - sizeof(mHeaderSig) - sizeof(mContentSig), out);
    sig.append((const char *)out, sizeof(out));
    return sig;
}


Cta5FileHeader::Cta5FileHeader(String8 mimeType, uint64_t dataLen, String8 key)
{
    _init(mimeType, dataLen, key, false);
}

Cta5FileHeader::Cta5FileHeader(String8 mimeType, uint64_t dataLen, String8 key,
        bool needPartialEncrypt)
{
    _init(mimeType, dataLen,key, needPartialEncrypt);
}

bool Cta5FileHeader::setCta5FileHeader(String8 cta5FileHeader)
{
    ALOGD("Cta5FileHeader::setCta5FileHeader this:%p",this);
    int pos = 0;
    const char *buf = cta5FileHeader.string();
    memcpy(mMagic, buf + pos, sizeof(CTA_MAGIC));
    ALOGD("mMagic-%c%c%c%c\n", mMagic[0], mMagic[1], mMagic[2], mMagic[3]);
    pos += sizeof(mMagic);
    memcpy(mMimeType, buf + pos, sizeof(mMimeType));
    pos += sizeof(mMimeType);
    memcpy(mReserved, buf + pos, sizeof(mReserved));
    pos += sizeof(mReserved);
    mKeyInfo[0].setKeyInfo(buf + pos);
    pos += mKeyInfo[0].getKeyInfo().length();
    mKeyInfo[1].setKeyInfo(buf + pos);
    pos += mKeyInfo[1].getKeyInfo().length();

    //mTotalLen = Cta5FileUtil::nettohostll(*(uint64_t*) (buf + pos));
    //Now mTotalLen is calculated from fd in constructor
    pos = pos + sizeof(mTotalLen);
    mHeaderLen = Cta5FileUtil::nettohostll(*(uint64_t*) (buf + pos));
    pos = pos + sizeof(mHeaderLen);

    mIsPartialEncrypted = Cta5FileUtil::nettohostl(*(uint32_t*) (buf + pos));
    ALOGD("mIsPartialEncrypted = %u\n",mIsPartialEncrypted);
    pos = pos + sizeof(mIsPartialEncrypted);
    mPartialEncryptHeader.setPartialEncryptHeader(buf + pos);
    pos = pos + PartialEncryptHeader::PARTIAL_ENCRYPT_HEADER_SIZE;

    memcpy(mHeaderSig, buf + pos, sizeof(mHeaderSig));
    pos += sizeof(mHeaderSig);
    memcpy(mContentSig, buf + pos, sizeof(mContentSig));
    pos += sizeof(mContentSig);

    if (mKeyInfo[0].getCount() < mKeyInfo[1].getCount())
    {
        mCurrentKeyInfo = &mKeyInfo[1];
        mNextKeyInfo = &mKeyInfo[0];
    }else
    {
        mCurrentKeyInfo = &mKeyInfo[0];
        mNextKeyInfo = &mKeyInfo[1];
    }
    ALOGD("Then restore keyinfo");
    mCurrentKeyInfo->restoreKeyInfo(this->mUserKey);
    return true;
}

/**
 * Calculate md5 signature for the whole file
 */
String8 Cta5FileHeader::calculateFileSig(int fd)
{
    printf("#############################\n");
    printf("calculateFileSig\n");
    String8 sig;
    if(-1 ==lseek(fd, 0 , SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]calculateFileSig fail, lseek begin fail:%s",strerror(errno));
        return sig;
    }
    int totalLen = lseek(fd, 0 , SEEK_END);
    if(totalLen == -1)
    {
        ALOGE("[ERROR][CTA5]calculateFileSig fail, lseek end fail:%s", strerror(errno));
        return sig;
    }
    int contentLen = totalLen - Cta5FileHeader::CTA5_FILE_HEADER_LEN;
    if(contentLen <=0 )
    {
        ALOGE("[ERROR][CTA5]calculateFileSig fail, bad file lenegth");
        return sig;
    }

    if (-1 == lseek(fd, Cta5FileHeader::CTA5_FILE_HEADER_LEN, SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]calculateFileSig fail, lseek to content fail:%s", strerror(errno));
        return sig;
    }

    uint8_t content[CTA_MAX_BUF_LEN];
    uint8_t out[CTA_KEY_LEN];
    memset(out, 0, sizeof(out));
    int readSize = 0;
    if (contentLen <= CTA_MAX_BUF_LEN)
    {
        readSize = read(fd, (void *) content, contentLen);
        printf("readSize = %d, contentLen =%d",readSize, contentLen);
        if (readSize == -1)
        {
            ALOGE("[ERROR][CTA5]calculateFileSig fail, read fail:%s", strerror(errno));
            return sig;
        }
        Cta5FileUtil::md5Digest((const uint8_t *)content, readSize, out);
        sig.append((const char *)out, sizeof(out));
        memcpy(mContentSig, out, sizeof(out));
        printf("mContentSig out");
        for(size_t i = 0; i < sizeof(out); ++i)
        {
            printf("%02x", out[i]);
        }
        printf("\n");
        return sig;
    }
    //Come here, it means it's a big file
    MD5_CTX c;
    int ret = MD5_Init(&c);

    int num = contentLen / CTA_MAX_BUF_LEN;
    int remainder = contentLen % CTA_MAX_BUF_LEN;
    for(int i = 0; i < num; ++i)
    {
        readSize = read(fd, (void *) content, CTA_MAX_BUF_LEN);
        if (readSize <= -1)
        {
            ALOGE("[ERROR][CTA5]calculateFileSig fail, read(big) fail:%s", strerror(errno));
            OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
            return sig;
        }
        ret = MD5_Update(&c, (const unsigned char *) content, (size_t) readSize);
    }
    if(remainder > 0)
    {
        readSize = read(fd, (void *) content, remainder);
        if (readSize <= -1)
        {
            ALOGE("[ERROR][CTA5]calculateFileSig fail, read(big remaind) fail:%s", strerror(errno));
            OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
            return sig;
        }
        ret = MD5_Update(&c, (const unsigned char *) content, (size_t) readSize);
    }
    ret = MD5_Final((unsigned char *) out, &c);
    memcpy(mContentSig, out, sizeof(out));
    sig.append((const char *)out, sizeof(out));
    OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
    writeFileSig(fd);
    return sig;
}

/**
 * Write file signature to fd's header
 */
void Cta5FileHeader::writeFileSig(int fd)
{
    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]writeFileSig fail, lseek begin fail:%s", strerror(errno));
        return ;
    }
    int fileSigPos = Cta5FileHeader::CTA5_FILE_HEADER_LEN - sizeof(mContentSig);
    if (-1 == lseek(fd, fileSigPos, SEEK_SET))
    {
        ALOGE("[ERROR][CTA5]writeFileSig fail, lseek fileSigPos fail:%s", strerror(errno));
        return ;
    }
    int writeLen = write(fd, mContentSig, sizeof(mContentSig));
    if(writeLen == -1)
    {
        ALOGE("[ERROR][CTA5]writeFileSig fail,write fail:%s", strerror(errno));
    }
    return;
}

uint64_t Cta5FileHeader::getContentPos()
{
    return CTA5_FILE_HEADER_LEN;
}

uint64_t Cta5FileHeader::getContentLen()
{
    return mTotalLen - CTA5_FILE_HEADER_LEN;
}

//If the CTA5 file is partial encrypted, then return true
bool Cta5FileHeader::isPartialEncryptedFile()
{
    return mIsPartialEncrypted;
}

uint64_t Cta5FileHeader::getClearHeaderOffset()
{
    return mPartialEncryptHeader.getClearHeaderOffset();
}

uint64_t Cta5FileHeader::getClearHeaderLen()
{
    return mPartialEncryptHeader.getClearHeaderLen();
}

uint64_t Cta5FileHeader::getCipherHeaderOffset()
{
    return mPartialEncryptHeader.getCipherHeaderOffset();
}

uint64_t Cta5FileHeader::getCipherHeaderLen()
{
    return mPartialEncryptHeader.getCipherHeaderLen();
}

bool Cta5FileHeader::setClearHeaderOffset(uint64_t offset)
{
    return mPartialEncryptHeader.setClearHeaderOffset(offset);
}

bool Cta5FileHeader::setClearHeaderLen(uint64_t len)
{
    return mPartialEncryptHeader.setClearHeaderLen(len);
}

bool Cta5FileHeader::setCipherHeaderOffset(uint64_t offset)
{
    return mPartialEncryptHeader.setCipherHeaderOffset(offset);
}

bool Cta5FileHeader::setCipherHeaderLen(uint64_t len)
{
    return mPartialEncryptHeader.setCipherHeaderLen(len);
}

String8 Cta5FileHeader::getCtaHeader()
{
    String8 dumpStr;
    uint8_t header[CTA5_FILE_HEADER_LEN];
    memset(header, 0, sizeof(header));
    int pos = 0;
    memcpy(header + pos, mMagic, sizeof(mMagic));
    pos += sizeof(mMagic);
    memcpy(header + pos, mMimeType, sizeof(mMimeType));
    pos += sizeof(mMimeType);
    memcpy(header + pos, mReserved, sizeof(mReserved));
    pos += sizeof(mReserved);
    String8 keyInfoStr = mKeyInfo[0].getKeyInfo();
    memcpy(header + pos, keyInfoStr.string(), KeyInfo::KEYINFO_SIZE);
    pos += KeyInfo::KEYINFO_SIZE;
    keyInfoStr = mKeyInfo[1].getKeyInfo();
    memcpy(header + pos, keyInfoStr.string(), KeyInfo::KEYINFO_SIZE);
    pos += KeyInfo::KEYINFO_SIZE;
    /////////////////
    uint64_t totalLen = Cta5FileUtil::hosttonetll(mTotalLen);
    memcpy(header + pos, &totalLen, sizeof(totalLen));
    pos += sizeof(totalLen);
    uint64_t headerLen = Cta5FileUtil::hosttonetll(mHeaderLen);
    memcpy(header + pos, &headerLen, sizeof(headerLen));
    pos += sizeof(headerLen);

    uint32_t isPartialEncrypted = Cta5FileUtil::hosttonetl(mIsPartialEncrypted);
    memcpy(header + pos, &isPartialEncrypted, sizeof(isPartialEncrypted));
    pos += sizeof(isPartialEncrypted);
    memcpy(header + pos, mPartialEncryptHeader.getPartialEncryptHeader().string(),
            PartialEncryptHeader::PARTIAL_ENCRYPT_HEADER_SIZE);
    pos += PartialEncryptHeader::PARTIAL_ENCRYPT_HEADER_SIZE;

    //The signature should place at the end of the header
    memcpy(header + pos, mHeaderSig, sizeof(mHeaderSig));
    pos += sizeof(mHeaderSig);
    memcpy(header + pos, mContentSig, sizeof(mContentSig));
    pos += sizeof(mContentSig);

    /*printf("mHeaderSig = \n");
    for (size_t i = 0; i < sizeof(mHeaderSig); ++i)
    {
        printf("%02x", mHeaderSig[i]);
    }
    printf("\n");

    printf("mContentSig = \n");
    for (size_t i = 0; i < sizeof(mContentSig); ++i)
    {
        printf("%02x", mContentSig[i]);
    }
    printf("\n");*/

    String8 str8;
    str8.append((const char*)header, CTA5_FILE_HEADER_LEN);
    /*dumpStr.clear();
    for (size_t i = 0; i < str8.length(); ++i)
    {
        dumpStr.appendFormat("%02x", str8[i]);
    }
    ALOGD("getCtaHeader dumpStr(cta header) = %s", dumpStr.string());
    printf("header = \n");
    for (size_t i = 0; i < CTA5_FILE_HEADER_LEN; ++i)
    {
        printf("%02x", header[i]);
    }
    printf("\n");*/
    return str8;
}

uint64_t Cta5FileHeader::getCtaHeaderLen()
{
    return CTA5_FILE_HEADER_LEN;
}

/*bool Cta5FileHeader::isCta5File(int fd)
{
    if (memcmp((const void *) mMagic, (const void *) CTA_MAGIC,
            sizeof(mMagic)))
    {
        ALOGE("invalid magic");
        return false;
    }
    if (memcmp((const void *) mMimeType, (const void *) CTA_MIME_PREFIX,
            sizeof(CTA_MIME_PREFIX) - 1))
    {
        ALOGE("invalid mime");
        return false;
    }
    return true;
}*/

String8 Cta5FileHeader::getMimeType()
{
    String8 mime = String8::format("%s",mMimeType);
    return mime;
}

String8 Cta5FileHeader::getOriginalMimeType()
{
    int mimePrefixLen = strlen(CTA_MIME_PREFIX);
    String8 mime = String8::format("%s", mMimeType + mimePrefixLen);
    ALOGD("original mime = %s", mime.string());
    return mime;
}

bool Cta5FileHeader::changeKey(String8 oldKey, String8 newKey)
{
    //First check if oldKey is valid
    if(!mCurrentKeyInfo->isKeyValid(oldKey))
    {
        ALOGE("The old key is invalid, so refuse to change key");
        return false;
    }
    //Do copy
    (*mNextKeyInfo) = (*mCurrentKeyInfo);
    //Copy data from current keyinfo to next keyinfo
    //mNextKeyInfo->setKeyInfo(mCurrentKeyInfo->getKeyInfo().string());
    mNextKeyInfo->changeKeyInfo(newKey);
    //Exchange currentKeyInfo and nexKeyInfo
    KeyInfo *pKeyInfo = mNextKeyInfo;
    mNextKeyInfo = mCurrentKeyInfo;
    mCurrentKeyInfo = pKeyInfo;
    calculateHeaderSig();
    return true;
}

uint8_t *Cta5FileHeader::getContentKey()
{
    return mCurrentKeyInfo->getContentKey();
}

bool Cta5FileHeader::isKeyValid(String8 key)
{
    return mCurrentKeyInfo->isKeyValid(key);
}

KeyInfo *Cta5FileHeader::getCurrentKeyInfo()
{
    return mCurrentKeyInfo;
}

//return keyinfo array's address
KeyInfo *Cta5FileHeader::getKeyInofs()
{
    return mKeyInfo;
}

void Cta5FileHeader::dump()
{
    String8 str;
    str.appendFormat("Cta5FileHeader:[mMimeType:%s", mMimeType);
    str.append(",mMagic:");
    for (uint32_t i = 0; i < sizeof(mMagic); ++i)
    {
        str.appendFormat("%c", mMagic[i]);
    }
    mKeyInfo[0].dump();
    mKeyInfo[1].dump();
    str.appendFormat(",mIsPartialEncrypted=%d", mIsPartialEncrypted);
    mPartialEncryptHeader.dump();
    str.appendFormat(",mTotalLen=%ld", (long) mTotalLen);
    str.appendFormat(",HeaderLen=%ld]", (long) mHeaderLen);
    ALOGD("%s", str.string());
}

//////////////////
//CTA5 file format
Cta5File::Cta5File(int fd, String8 key) :
        mCta5FileHeader(fd, key)
{
    mFd = fd;
    mNeedCancel = false;
    mCipherFd = -1;
    mClearFd = -1;
    ALOGD("Cta5File constructor from fd");
}

Cta5File::Cta5File(String8 mimeType, String8 cid __attribute__((unused)),
                   String8 dcfFlHeaders __attribute__((unused)), uint64_t datatLen,
        String8 key) :
        mCta5FileHeader(mimeType, datatLen, key)
{
    mNeedCancel = false;
    mCipherFd = -1;
    mClearFd = -1;
    memcpy(mIv, IV, sizeof(mIv));
}

Cta5File::Cta5File(String8 mimeType, String8 cid __attribute__((unused)),
                   String8 dcfFlHeaders __attribute__((unused)),
                   uint64_t datatLen,
        String8 key, bool needPartialEncrypt) :
        mCta5FileHeader(mimeType, datatLen, key, needPartialEncrypt)
{
    mNeedCancel = false;
    mCipherFd = -1;
    mClearFd = -1;
    mFd = -1;
    memcpy(mIv, IV, sizeof(mIv));
}

Cta5File::Cta5File(String8 mimeType, uint64_t datatLen, String8 key, bool needPartialEncrypt) :
        mCta5FileHeader(mimeType, datatLen, key, needPartialEncrypt)
{
    mNeedCancel = false;
    mCipherFd = -1;
    mClearFd = -1;
    mFd = -1;
    memcpy(mIv, IV, sizeof(mIv));
}

Cta5File::~Cta5File()
{
    if(mFd != -1)
    {
        ALOGD("~Cta5File close mFd");
        close(mFd);
    }
    mFd = -1;
    if (mCipherFd != -1)
    {
        ALOGD("~Cta5File close mCipherFd");
        close(mCipherFd);
    }
    mCipherFd = -1;
    if (mClearFd != -1)
    {
        ALOGD("~Cta5File close mClearFd");
        close(mClearFd);
    }
    mClearFd = -1;
    mNeedCancel = false;
}

//If the CTA5 file is partial encrypted, then return true
bool Cta5File::isPartialEncryptedFile()
{
    return mCta5FileHeader.isPartialEncryptedFile();
}

uint64_t Cta5File::getClearHeaderOffset()
{
    return mCta5FileHeader.getClearHeaderOffset();
}

uint64_t Cta5File::getClearHeaderLen()
{
    return mCta5FileHeader.getClearHeaderLen();
}

uint64_t Cta5File::getCipherHeaderOffset()
{
    return mCta5FileHeader.getCipherHeaderOffset();
}

uint64_t Cta5File::getCipherHeaderLen()
{
    return mCta5FileHeader.getCipherHeaderLen();
}

bool Cta5File::setClearHeaderOffset(uint64_t offset)
{
    return mCta5FileHeader.setClearHeaderOffset(offset);
}

bool Cta5File::setClearHeaderLen(uint64_t len)
{
    return mCta5FileHeader.setClearHeaderLen(len);
}

bool Cta5File::setCipherHeaderOffset(uint64_t offset)
{
    return mCta5FileHeader.setCipherHeaderOffset(offset);
}

bool Cta5File::setCipherHeaderLen(uint64_t len)
{
    return mCta5FileHeader.setCipherHeaderLen(len);
}

bool Cta5File::isCta5File(int fd)
{
    bool ret = isCta5NormalFile(fd) || isCta5MultimediaFile(fd);
    return ret;
}

bool Cta5File::isCta5MultimediaFile(int fd)
{
    uint8_t magic[CTA_MAGIC_LEN+1] = { 0 };
    if (-1 == lseek(fd, -CTA_MAGIC_LEN, SEEK_END))
    {
        ALOGE("Cta5File::isCta5MultimediaFile false, lseek fail");
        return false;
    }
    int readSize = read(fd, magic, CTA_MAGIC_LEN);
    if (readSize != CTA_MAGIC_LEN)
    {
        ALOGE("Cta5File::isCta5MultimediaFile false, bad file");
        return false;
    }
    if (memcmp(magic, CTA_MM_MAGIC, CTA_MAGIC_LEN) != 0)
    {
        ALOGE("Cta5File::isCta5MultimediaFile false, bad magic:%s",magic);
        return false;
    }
    return true;
}

bool Cta5File::isCta5NormalFile(int fd)
{
    uint8_t magic[CTA_MAGIC_LEN+1] = { 0 };
    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        ALOGE("Cta5File::isCta5NormalFile false, lseek fail");
        return false;
    }
    int readSize = read(fd, magic, CTA_MAGIC_LEN);
    if (readSize != CTA_MAGIC_LEN)
    {
        ALOGE("Cta5File::isCta5NormalFile false, bad file");
        return false;
    }
    if (memcmp(magic, CTA_MAGIC, CTA_MAGIC_LEN) != 0)
    {
        ALOGE("Cta5File::isCta5NormalFile false, bad magic:%s",magic);
        return false;
    }
    return true;
}

String8 Cta5File::getMimeType()
{
    return mCta5FileHeader.getMimeType();
}

String8 Cta5File::getOriginalMimeType()
{
    return mCta5FileHeader.getOriginalMimeType();
}

bool Cta5File::isKeyValid(String8 key)
{
    ALOGD("Cta5File::isKeyValid");
    return mCta5FileHeader.isKeyValid(key);
}

bool Cta5File::changeKey(String8 oldKey, String8 newKey)
{
    ALOGD("changeKey oldkey = %s, newKey = %s", oldKey.string(), newKey.string());
    bool ret = mCta5FileHeader.changeKey(oldKey, newKey);
    //Write the result to file
    //find out the pos to be writen
    int pos = CTA_MAGIC_LEN + CTA_MAX_MIEM_LEN + 128;
    int offset = 0;
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
    if (-1 == write(mFd, mCta5FileHeader.getCurrentKeyInfo()->getKeyInfo().string(), KeyInfo::KEYINFO_SIZE))
    {
        ALOGE("[ERROR][CTA5]ChangeKey fail- write failed:%s", strerror(errno));
        return false;
    }
    return ret;
}

uint8_t *Cta5File::getKey()
{
    //return mKey;
    return getContentKey();
}


//Get the iv for aes_128_cbc encrypt/decrypt
uint8_t *Cta5File::getIv()
{
    return mIv;
}
//Get the cta5 file's filedescriptor
int Cta5File::getFd()
{
    return mFd;
}

//When you construct a cta5 file header and want to dump it to file
//Please call this api to get header data
String8 Cta5File::getCta5FileHeader()
{
    return mCta5FileHeader.getCtaHeader();
}

String8 Cta5File::calculateFileSig(int fd)
{
    return this->mCta5FileHeader.calculateFileSig(fd);
}

/**
 * Now it's same as getCta5FileHeader
 */
String8 Cta5File::getHeader()
{
    return mCta5FileHeader.getCtaHeader();
}

//get the position of real content, the content is encrypted
uint64_t Cta5File::getContentPos()
{
    return mCta5FileHeader.getContentPos();
}

uint64_t Cta5File::getContentLen()
{
    return mCta5FileHeader.getContentLen();
}

bool Cta5File::cancel()
{
    ALOGD("hongen cta5File cancel: mNeedCancel=[%d]",mNeedCancel);
    mNeedCancel = true;
    return true;
}

bool Cta5File::isNeedCancel()
{
    ALOGD("hongen cta5File isNeedCancel: mNeedCancel=[%d]",mNeedCancel);
    return mNeedCancel;
}

int Cta5File::getClearFd()
{
    return mClearFd;
}

int Cta5File::getCipherFd()
{
    return mCipherFd;
}

void Cta5File::setClearFd(int fd)
{
    mClearFd = fd;
}

void Cta5File::setCipherFd(int fd)
{
    mCipherFd = fd;
}

uint8_t *Cta5File::getContentKey()
{
    return mCta5FileHeader.getContentKey();
}

void Cta5File::dump()
{
    this->mCta5FileHeader.dump();
}

int64_t Cta5File::pread(void* buf __attribute__((unused)),
                        uint64_t size __attribute__((unused)),
                        off64_t offset __attribute__((unused)))
{
    ALOGD("Cta5File::pread no implementation");
    return 0;
}

bool Cta5File::notify(const Vector<DrmCtaUtil::Listener> *infoListener, String8 progress)
{
    /*for (Vector<DrmCtaUtil::Listener>::const_iterator iter = infoListener->begin();
            iter != infoListener->end(); ++iter)
    {
        DrmInfoEvent event(iter->GetUniqueId(), 10001,
                progress);
        iter->GetListener()->onInfo(event);
    }*/
    //ALOGD("hongen encrypt: notify-progress=[%s]",progress.string());
    for (size_t i = 0; i < infoListener->size(); i++)
    {
        DrmInfoEvent event((*infoListener)[i].GetUniqueId(), DrmDef::CTA5_CALLBACK, progress);
//        ALOGD("hongen encrypt: notify---,");
        (*infoListener)[i].GetListener()->onInfo(event);
    }
    return true;
}
