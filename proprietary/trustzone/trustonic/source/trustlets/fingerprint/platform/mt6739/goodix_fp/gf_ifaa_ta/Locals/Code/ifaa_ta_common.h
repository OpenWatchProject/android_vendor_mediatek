/**
 * @(#) ifaa_ta_common module  1.0 2015/12/12
 *
 * Copyright (c) 2014, IFAA and/or its affiliates. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * -Redistribution of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 *
 * -Redistribution in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * Neither the name of IFAA or the names of contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. ALIPAY, INC.
 * AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
 * AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL ALIPAY OR ITS LICENSORS BE LIABLE FOR ANY LOST
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF ALIPAY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * You acknowledge that this software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 *
 * ***********************************************************************************
 *
 * This head file defines the interfaces that library presents to client programs, and
 * is intended for call from Trusted Execution Application.
 */


#ifndef __IFAA_TA_COMMON_H__
#define __IFAA_TA_COMMON_H__


#include <stdint.h>

typedef enum {
    IFAA_ERR_SUCCESS                     = 0x00000000,   // 成功
    IFAA_ERR_UNKNOWN                     = 0x7A000001,   // 未知错误
    IFAA_ERR_BAD_ACCESS                  = 0x7A000002,   // 访存错误
    IFAA_ERR_BAD_PARAM                   = 0x7A000003,   // 错误的入参
    IFAA_ERR_UNKNOWN_CMD                 = 0x7A000004,   // 未知的命令
    IFAA_ERR_BUF_TO_SHORT                = 0x7A000005,   // 出参buffer不够
    IFAA_ERR_OUT_OF_MEM                  = 0x7A000006,   // 内存分配失败
    IFAA_ERR_TIMEOUT                     = 0x7A000007,   // 超时
    IFAA_ERR_HASH                        = 0x7A000008,   // HASH失败
    IFAA_ERR_SIGN                        = 0x7A000009,   // 签名失败
    IFAA_ERR_VERIFY                      = 0x7A00000A,   // 验签失败
    IFAA_ERR_KEY_GEN                     = 0x7A00000B,   // 生成秘钥失败
    IFAA_ERR_READ                        = 0x7A00000C,   // 读文件失败
    IFAA_ERR_WRITE                       = 0x7A00000D,   // 写文件失败
    IFAA_ERR_ERASE                       = 0x7A00000E,   // 删除文件失败
    IFAA_ERR_NOT_MATCH                   = 0x7A00000F,   // 生物特征不匹配
    IFAA_ERR_GEN_RESPONSE                = 0x7A000010,   // 生成response失败
    IFAA_ERR_GET_DEVICEID                = 0x7A000011,   // 获取设备ID失败
    IFAA_ERR_GET_LAST_IDENTIFIED_RESULT  = 0x7A000012,   // 获取最近一次认证结果失败
    IFAA_ERR_AUTHENTICATOR_SIGN          = 0x7A000013,   // Authenticator证书签名失败
    IFAA_ERR_GET_ID_LIST                 = 0x7A000014,   // 获取对应的id list失败
    IFAA_ERR_GET_AUTHENTICATOR_VERSION   = 0x7A000015,   // 获取对应的Authenticator的版本号错误
    IFAA_ERR_UN_INITIALIZED              = 0x7A000016,   // 未初始化错误

    // more in considering....
} IFAA_Result;


/////////////////////////////////////////////////////////////
/// Sensor厂商接口(仅定义参数和返回值，不约定整个签名)
/////////////////////////////////////////////////////////////

/**
 * 获取最近一次的认证ID
 * @param buf_id: 返回的最近一次认证结果的id序列化后的buffer(出参)
 * @param id_len: buf_id的长度(入参+出参)
 */
typedef IFAA_Result (*IFAA_LastIdentifiedResultGetter)(uint8_t *buf_id, uint32_t *id_len);

/**
 * 获取对应Authenticator的version
 * @param buf_version: 对应的Authenticator的版本号序列化后的buffer(出参)
 * @param version_len: buf_version的长度(入参+出参)
 */
typedef IFAA_Result (*IFAA_AuthenticatorVersionGetter)(int32_t *version);

/**
 * 获取系统中Enroll的id list
 * @param buf_id_list: 系统中录入的对应的生物特征的id列表对应的buffer
 * 格式：
 *      +-------------------------------------+
 *      |id1_len | id_1 | id2_len | id_2 | ...|
 *      +-------------------------------------+
 * @param id_list_len: 返回的buffer的长度(入参+出参)
 */
typedef IFAA_Result (*IFAA_IdListGetter)(uint8_t *buf_id_list, uint32_t *id_list_len);


typedef enum {
    IFAA_NO = 0,
    IFAA_YES
} IFAABoolean;

/**
 * id的比较符
 * @param buf_l: 待比较的buffer
 * @param buf_r: 被比较的buffer
 */
typedef IFAABoolean (*IFAA_IdEquator)(const uint8_t *buf_l, const uint8_t *buf_r, uint32_t buf_len);


/////////////////////////////////////////////////////////////
/// TA接口
/////////////////////////////////////////////////////////////

/**
 * 入口函数接受的command
 */
typedef enum {
    IFAA_TA_CMD_GET_DEVICE_ID          = 0x01,  // 获取设备ID
    IFAA_TA_CMD_REGISTER,                       // 注册
    IFAA_TA_CMD_AUTHENTICATE,                   // 验证
    IFAA_TA_CMD_DEREGISTER,                     // 注销
    IFAA_TA_CMD_QUERY_STATUS,                   // 查询本地免密开通状态
    IFAA_TA_CMD_GEN_ASYMMETRIC_KEY,             // 预生成非对称秘钥
    IFAA_TA_CMD_GET_TA_VERSION,                 // 获取TA版本号

#ifdef __IFAA_TEST_MODE  // 测试时使用
    ///// 文件操作
    IFAA_TA_CMD_READ_FILE,                      // 读文件
    IFAA_TA_CMD_WRITE_FILE,                     // 写文件
    IFAA_TA_CMD_DELETE_FILE,                    // 删除文件

    IFAA_INTERNAL_GET_LAST_IDENTIFIED_RESULT,   // 获取最近一次认证结果ID
    IFAA_INTERNAL_GET_AUTHENTICATOR_VERSION,    // 获取authenticator的版本
    IFAA_INTERNAL_GET_ID_LIST,                  // 获取idlist
    IFAA_INTERNAL_BIO_ID_COMPARE,


    IFAA_INTERNAL_SHA256,                       // SHA256
    IFAA_INTERNAL_SIGN,                         // 签名
    IFAA_INTERNAL_VERIFY,                       // 验证签名
    IFAA_INTERNAL_KEY_GENERATE,                 // 生成公私钥
    IFAA_INTERNAL_HMAC_SHA1,                    // HmacSha1

    IFAA_INTERNAL_AUTHENTICATOR_SIGN,           // 校验器签名(可暂不实现)
    IFAA_INTERNAL_AUTHENTICATOR_VERIFY,         // 校验器验签
#endif
} ifaa_ta_cmd;


typedef enum {
    IFAA_BIO_FINGERPRINT = 1 << 0,              // 指纹
    IFAA_BIO_IRIS        = 1 << 1,              // 虹膜
    IFAA_BIO_FACE        = 1 << 2,              // 人脸

    // more
} IFAA_BioType;


typedef enum {
    IFAA_ENTRYLAST_IDENTIFIED_RESULT_GETTER    = 0x01,  // 获取最近一次认证通过的id
    IFAA_ENTRY_AUTHENTICATOR_VERSION_GETTER,            // 获取对应Authenticator的版本
    IFAA_ENTRY_ID_LIST_GETTER,                          // 获取系统中录入的生物特征的id列表
    IFAA_ENTRY_EQUATOR                                  // 生物特征id的比较符
    // more
} IFAA_TaEntry;


/**
 * TA本地免密入口
 * param buf_in: 输入buffer
 * 格式：
 *     +----------------------------------------------------------+
 *     | version | sig_len | sig | pkg_len | pkg | cmd | params.. |
 *     +----------------------------------------------------------+
 * param in_len: 输入buffer的长度
 *
 * param_buf_out: 输出buffer(出参)
 * 格式：
 *     +----------------------------------+
 *     | TEE_Result | total_len | value.. |
 *     +----------------------------------+
 * param out_len: 输出buffer的长度(入参 + 出参)
 */
IFAA_Result IFAA_TaInvokeCmd(uint8_t *buf_in, uint32_t in_len, uint8_t *buf_out, uint32_t *out_len);


/**
 * 添加本地免密Entry
 * @param bioType: 生物验证类型
 * @param entry: 槽位编号
 * @param func: 函数入口地址
 */
IFAA_Result IFAA_TaAddEntry(IFAA_BioType bioType, IFAA_TaEntry entry, void* func);

/**
 *获取本地免密Entry
 * @param bioType:  生物验证类型
 * @param entry:    槽位编号
 * @param func:     函数的入口地址(出参)
 */
IFAA_Result IFAA_TaGetEntry(IFAA_BioType bioType, IFAA_TaEntry entry, void**func);


/////////////////////////////////////////////////////////////
/// TEE接口部分:加解密
/////////////////////////////////////////////////////////////

/**
 * SHA256
 * @param msg:          待hash的明文
 * @param msg_len:      明文长度
 * @param digest:       产生的摘要(出参)
 * @param digest_len:   摘要长度(入参+出参)
 */
IFAA_Result IFAA_Sha256(const uint8_t* msg, uint32_t msg_len,
        uint8_t* digest, uint32_t* digest_len);

typedef struct {
    uint8_t*    buf;
    uint32_t      len;
} vlb_t;

typedef struct {
    vlb_t n;
    vlb_t d;
    vlb_t e;
} IFAA_RsaKey;


///**
// * IFAA自定义证书格式
// * +---------------------------------------------+
// * | n_len | n | e_len | e | sig_len | signature |
// * +---------------------------------------------+
// */
// typedef struct {
//    /**
//     * 公钥格式
//     * +-----------------------+
//     * | n_len | n | e_len | e |
//     * +-----------------------+
//     */
//    vlb_t pub_key;      //公钥buffer
//    vlb_t sig;          //签名
//} IFAA_Certificate;


/**
 * 证书统一编码格式 tag-union
 * +---------------------------------------+
 * | cert_enc_alg | content of certificate |
 * +---------------------------------------+
 */
typedef struct x509 x509;  // forward declaration
#if 0
typedef struct {
    IFAA_CertEncodeAlgorithm cert_enc_alg;  // 证书类型

    union {
         //////////////////////////////////////////////////
         // IFAA自定义证书序列化格式
         // +---------------------------------------------+
         // | n_len | n | e_len | e | sig_len | signature |
         // +---------------------------------------------+
         //////////////////////////////////////////////////
        struct {
            /**
             * 公钥序列化格式
             * +-----------------------+
             * | n_len | n | e_len | e |
             * +-----------------------+
             */
            struct {
                vlb_t n;    // 大整数n
                vlb_t e;    // 大整数e
            } pub_key;
            vlb_t sig;      // 签名
        } ifaa_cert;

        //////////////////////////////////////////////////
        // X509标准证书,格式暂参考DER编码
        //////////////////////////////////////////////////
        struct {
            x509 *certificate;
        } x509_cert;

        // more others...
    } body;
} IFAA_Certificate;



/**
 * 签名
 * @param key:          秘钥
 * @param digest:       摘要
 * @param digest_len:   摘要的长度
 * @param sig:          长生的前面(出参)
 * @param sig_len:      签名的长度(入参+出参)
 */
IFAA_Result IFAA_RsaSignDigest(const IFAA_RsaKey* key,
                               const uint8_t* digest, uint32_t digest_len,
                               uint8_t* sig, uint32_t* sig_len);

/**
 * 验签
 * @param key:          公钥
 * @param digest:       摘要
 * @param digest_len:   摘要长度
 * @param sig:          签名
 * @param sig_len:      签名长度
 */
IFAA_Result IFAA_RsaVerifyDigest(const IFAA_RsaKey* key,
                                 const uint8_t* digest, uint32_t digest_len,
                                 const uint8_t* sig, uint32_t sig_len);


typedef enum {
    RSA_BITS_2048,
    RSA_BITS_4096,
} IFAA_RsaBit;

/**
 * 生成RSA秘钥
 * @param bits: 秘钥长度
 * @param key:  生成的秘钥(出参)
 */
IFAA_Result IFAA_RsaKeyGenerate(IFAA_RsaBit bits, IFAA_RsaKey *key);


/**
 * HMAC_SHA1
 */
IFAA_Result IFAA_HmacSha1(const uint8_t *msg, uint32_t msg_len,
                          const uint8_t *key, uint32_t key_len,
                          uint8_t *mac);

/**
 * 获取设备ID
 * @param device_id:        设备id的buffer(出参)
 * @param device_id_len:    设备id的长度
 */
IFAA_Result IFAA_GetDeviceId(uint8_t *device_id, uint32_t *device_id_len);


/**
 * 获取TA版本
 * @param device_id:        版本号的buffer(出参)
 * @param device_id_len:    版本号的长度
 */
IFAA_Result IFAA_GetTaVersion(uint8_t *version, uint32_t *version_len);


/**
 * 证书验签
 * @param digest:       摘要
 * @param digest_len:   摘要长度
 * @param signature:    签名
 * @param sig_len:      签名长度
 * @param cert_chain:   证书链
 * @param cert_count:   证书链中证书的个数
 */
IFAA_Result IFAA_AuthenticatorVerifyDigest(const uint8_t *digest, uint32_t digest_len,
                                           const uint8_t *signature, uint32_t sig_len,
                                           const IFAA_Certificate *cert_chain, uint32_t cert_count);

/**
 * 证书签名
 * @param digest:       摘要
 * @param digest_len:   摘要长度
 * @param signature:    签名
 * @param sig_len:      签名长度
 */
IFAA_Result IFAA_AuthenticatorSignDigest(const uint8_t* digest, uint32_t digest_len,
                                         uint8_t* signature, uint32_t* sig_len);

// IFAA_Result IFAA_AuthenticatorSignDigest(const uint8_t* digest, uint32_t digest_len,
//                                         const uint8_t* cert_chain, uint32_t cert_chain_len,
//                                         uint8_t* signature, uint32_t* sig_len);


/////////////////////////////////////////////////////////////
/// TEE接口部分:文件操作
/////////////////////////////////////////////////////////////

/**
 * 写入文件
 * @param path: 文件路径
 * @param data: 写入的数据
 * @param len:  数据长度
 */
IFAA_Result IFAA_WriteFile(const char *path, const uint8_t *data, uint32_t len);

/**
 * 读取文件
 * @param path: 文件路径
 * @param data: 文件读取的缓存buffer
 * @param len:  读取的长度
 */
IFAA_Result IFAA_ReadFile(const char *path,  uint8_t *data, uint32_t *len);

/**
 * 删除文件
 * @param path: 文件路径
 */
IFAA_Result IFAA_DeleteFile(const char *path);

#endif

#endif  // __IFAA_TA_COMMON_H__
