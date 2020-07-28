#include "comm.h"

TEE_Result read_file(char* path, uint8_t* data, size_t data_len) {
    LOG_DEBUG("read_file: %s\n", path);
    TEE_Result ret = TEE_SUCCESS;
    int fd = qsee_sfs_open(path, O_RDONLY);
    if (fd == 0) {
        LOG_ERROR("qsee_sfs_open failed: 0x%08X\n", qsee_sfs_error(fd));
        return TEE_ERR_READ_FILE;
    }
    if (data != NULL && data_len > 0) {
        int count = qsee_sfs_read(fd, (char*)data, data_len);
        if (count <= 0) {
            LOG_ERROR("qsee_sfs_read failed: 0x%08X\n", qsee_sfs_error(fd));
            ret = TEE_ERR_READ_FILE;
        }
    }
    int status = qsee_sfs_close(fd);
    if (status != 0) {
        LOG_ERROR("qsee_sfs_close failed: 0x%08X\n", qsee_sfs_error(fd));
        ret = TEE_ERR_READ_FILE;
    }
    return ret;
}

TEE_Result write_file(char* path, uint8_t* data, size_t data_len) {
    LOG_DEBUG("write_file: %s\n", path);
    TEE_Result ret = TEE_SUCCESS;
    int fd = qsee_sfs_open(path, O_CREAT | O_WRONLY);
    if (fd == 0) {
        LOG_ERROR("qsee_sfs_open failed: 0x%08X\n", qsee_sfs_error(fd));
        return TEE_ERR_WRITE_FILE;
    }
    if (data != NULL && data_len > 0) {
        int count = qsee_sfs_write(fd, (const char*)data, data_len);
        if (count != data_len) {
            LOG_ERROR("qsee_sfs_write failed: 0x%08X\n", qsee_sfs_error(fd));
            ret = TEE_ERR_WRITE_FILE;
        }
    }
    int status = qsee_sfs_close(fd);
    if (status != 0) {
        LOG_ERROR("qsee_sfs_close failed: 0x%08X\n", qsee_sfs_error(fd));
        ret = TEE_ERR_WRITE_FILE;
    }
    return ret;
}

TEE_Result delete_file(char* path) {
    LOG_DEBUG("delete_file: %s\n", path);
    int ret = qsee_sfs_rm(path);
    if (ret != 0) {
        LOG_ERROR("qsee_sfs_rm failed\n");
        return TEE_ERR_DELETE_FILE;
    }
    return TEE_SUCCESS;
}

TEE_Result sha256(uint8_t* msg, size_t msg_len, uint8_t* digest,
                  size_t* digest_len) {
    ENTER_FUNC
    int ret = qsee_hash(QSEE_HASH_SHA256, msg, msg_len, digest, *digest_len);
    if (ret < 0) {
        LOG_ERROR("qsee_hash failed\n");
        return TEE_ERR_SHA256;
    }
    LEAVE_FUNC
    return TEE_SUCCESS;
}

/* Endianness converter Macro */
#define htonl(x)                            \
    (((((uint32_t)(x)&0x000000FFU) << 24) | \
      (((uint32_t)(x)&0x0000FF00U) << 8) |  \
      (((uint32_t)(x)&0x00FF0000U) >> 8) |  \
      (((uint32_t)(x)&0xFF000000U) >> 24)))

static int copy_S_BIGINT(uint8_t* dst, uint32_t dst_len, QSEE_S_BIGINT* src,
                         uint32_t max_len) {
    uint32_t i = 0;
    uint32_t array_length = 0;

    if (dst_len > max_len) {
        return -1;
    }

    array_length = dst_len / 4;
    for (i = 0; i < array_length; i++) {
        *((uint32_t*)dst + i) = htonl(src->bi.a[array_length - 1 - i]);
    }
    return 0;
}

static void init_key(QSEE_S_BIGINT** key, uint8_t* buf, size_t size) {
    *key = (QSEE_S_BIGINT*)TEE_Malloc(sizeof(QSEE_S_BIGINT),
                                      TEE_ALLOCATION_HINT_ZEROED);
    if (*key != NULL) {
        qsee_BIGINT_read_unsigned_bin(&(*key)->bi, buf, size);
    }
}

TEE_Result rsa_sign_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len,
                           uint8_t* sig, size_t* sig_len) {
    TEE_Result ret = TEE_SUCCESS;
    QSEE_RSA_KEY rsa_key = {0};
    init_key(&(rsa_key.N), key->n.buf, key->n.len);
    init_key(&(rsa_key.d), key->d.buf, key->d.len);
    init_key(&(rsa_key.e), key->e.buf, key->e.len);

    rsa_key.type = QSEE_RSA_KEY_PRIVATE;
    int status = qsee_rsa_sign_hash(&rsa_key, QSEE_RSA_PAD_PKCS1_V1_5_SIG, NULL,
                                    QSEE_HASH_IDX_SHA256, digest, digest_len,
                                    sig, (int*)sig_len);
    if (status != 0) {
        LOG_ERROR("qsee_rsa_sign_hash failed\n");
        ret = TEE_ERR_RSA_SIGN;
    }

    if (rsa_key.N != NULL) TEE_Free(rsa_key.N);
    if (rsa_key.d != NULL) TEE_Free(rsa_key.d);
    if (rsa_key.e != NULL) TEE_Free(rsa_key.e);

    return ret;
}

TEE_Result rsa_verify_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len,
                             uint8_t* sig, size_t sig_len) {
    TEE_Result ret = TEE_SUCCESS;
    QSEE_RSA_KEY rsa_key = {0};
    init_key(&(rsa_key.N), key->n.buf, key->n.len);
    init_key(&(rsa_key.d), key->d.buf, key->d.len);
    init_key(&(rsa_key.e), key->e.buf, key->e.len);

    rsa_key.type = QSEE_RSA_KEY_PUBLIC;
    int status = qsee_rsa_verify_signature(
        &rsa_key, QSEE_RSA_PAD_PKCS1_V1_5_SIG, NULL, QSEE_HASH_IDX_SHA256,
        digest, digest_len, sig, sig_len);
    if (status != 0) {
        LOG_ERROR("qsee_rsa_verify_signature failed\n");
        ret = TEE_ERR_RSA_VERIFY;
    }

    if (rsa_key.N != NULL) TEE_Free(rsa_key.N);
    if (rsa_key.d != NULL) TEE_Free(rsa_key.d);
    if (rsa_key.e != NULL) TEE_Free(rsa_key.e);

    return ret;
}

TEE_Result rsa_key_gen(rsa_key_t* key, size_t bits) {
    TEE_Result ret = TEE_SUCCESS;
    QSEE_RSA_KEY rsa_key = {0};
    uint8_t pub_exp[] = {0x00, 0x00, 0x01, 0x00, 0x01};

    do {
        if (qsee_util_init_s_bigint(&(rsa_key.e)) ||
            qsee_util_init_s_bigint(&(rsa_key.d)) ||
            qsee_util_init_s_bigint(&(rsa_key.N)) ||
            qsee_util_init_s_bigint(&(rsa_key.p)) ||
            qsee_util_init_s_bigint(&(rsa_key.q))) {
            LOG_ERROR("qsee_util_init_s_bigint failed.\n");
            ret = TEE_ERR_RSA_KEY_GEN;
            break;
        }

        rsa_key.type = QSEE_RSA_KEY_PRIVATE_PUBLIC;

        int status =
            qsee_rsa_key_gen(&rsa_key, bits / 8, pub_exp, sizeof(pub_exp));
        if (status != 0) {
            LOG_ERROR("qsee_rsa_key_gen failed.\n");
            ret = TEE_ERR_RSA_KEY_GEN;
            break;
        }

        if (copy_S_BIGINT(key->n.buf, QSEE_RSA_KEY_SIZE(rsa_key.N), rsa_key.N,
                          key->n.len) ||
            copy_S_BIGINT(key->d.buf, QSEE_RSA_KEY_SIZE(rsa_key.d), rsa_key.d,
                          key->d.len) ||
            copy_S_BIGINT(key->e.buf, QSEE_RSA_KEY_SIZE(rsa_key.e), rsa_key.e,
                          key->e.len)) {
            LOG_ERROR("copy_S_BIGINT failed.\n");
            ret = TEE_ERR_RSA_KEY_GEN;
            break;
        }

        key->n.len = QSEE_RSA_KEY_SIZE(rsa_key.N);
        key->d.len = QSEE_RSA_KEY_SIZE(rsa_key.d);
        key->e.len = QSEE_RSA_KEY_SIZE(rsa_key.e);
    } while (false);

    if (rsa_key.d != NULL) qsee_util_free_s_bigint(rsa_key.d);
    if (rsa_key.N != NULL) qsee_util_free_s_bigint(rsa_key.N);
    if (rsa_key.p != NULL) qsee_util_free_s_bigint(rsa_key.p);
    if (rsa_key.q != NULL) qsee_util_free_s_bigint(rsa_key.q);
    if (rsa_key.e != NULL) qsee_util_free_s_bigint(rsa_key.e);

    return ret;
}

TEE_Result hmac_sha1(uint8_t* msg, size_t msg_len, uint8_t* key, size_t key_len,
                     uint8_t* mac) {
    int ret = qsee_hmac(QSEE_HMAC_SHA1, msg, msg_len, key, key_len, mac);
    if (ret < 0) {
        LOG_ERROR("qsee_hmac failed\n");
        return TEE_ERR_HMAC_SHA1;
    }
    return TEE_SUCCESS;
}

