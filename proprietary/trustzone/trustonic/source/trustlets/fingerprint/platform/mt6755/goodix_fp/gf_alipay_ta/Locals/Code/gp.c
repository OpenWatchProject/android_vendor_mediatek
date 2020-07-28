#include <taStd.h>
#include <tee_internal_api.h>

#include <comm.h>
#include <gp.h>

void hex_dump(const char* desc, void* addr, int len) {
    uint8_t buff[17];
    uint8_t* pc = (uint8_t*) addr;
    int i = 0;

    if (desc != NULL)
        TEE_LogPrintf("[" LOG_TAG "]%s: (len: 0x%04x)\n", desc, len);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                TEE_LogPrintf("  %s\n", buff);
            // Output the offset.
            TEE_LogPrintf("[" LOG_TAG "]  %04x ", i);
        }
        // Now the hex code for the specific character.
        TEE_LogPrintf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        TEE_LogPrintf("   ");
        i++;
    }

    // And print the final ASCII bit.
    TEE_LogPrintf("  %s\n", buff);
}

TEE_Result read_file(char* path, uint8_t* data, size_t data_len) {
    LOG_DEBUG("read_file: %s\n", path);
    TEE_ObjectHandle handle = NULL;
    TEE_Result ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, path, strlen(path),
            TEE_DATA_FLAG_ACCESS_READ, &handle);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_OpenPersistentObject failed: 0x%08x\n", ret);
        return TEE_ERROR_GENERIC;
    }
    if (data != NULL && data_len > 0) {
        ret = TEE_ReadObjectData(handle, data, data_len, (uint32_t*) &data_len);
        if (ret != TEE_SUCCESS) {
            LOG_ERROR("TEE_ReadObjectData failed: 0x%08x\n", ret);
            ret = TEE_ERROR_GENERIC;
        }
    }

    TEE_CloseObject(handle);
    return ret;
}

TEE_Result write_file(char* path, uint8_t* data, size_t data_len) {
    LOG_DEBUG("write_file: %s\n", path);
    TEE_ObjectHandle handle = NULL;
    TEE_Result ret = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, path, strlen(path),
            TEE_DATA_FLAG_CREATE | TEE_DATA_FLAG_ACCESS_WRITE, NULL, NULL, 0, &handle);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_CreatePersistentObject failed: %08x\n", ret);
        return ret;
    }
    if (data != NULL && data_len > 0) {
        ret = TEE_WriteObjectData(handle, data, data_len);
        if (ret != TEE_SUCCESS) {
            LOG_ERROR("TEE_WriteObjectData failed: %08x\n", ret);
        }
    }

    TEE_CloseObject(handle);
    return ret;
}

TEE_Result delete_file(char* path) {
    LOG_DEBUG("delete_file: %s\n", path);
    TEE_ObjectHandle handle = NULL;
    TEE_Result ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, path, strlen(path),
            TEE_DATA_FLAG_ACCESS_WRITE_META, &handle);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_OpenPersistentObject failed: %08x\n", ret);
        return ret;
    }

    TEE_CloseAndDeletePersistentObject(handle);
    return TEE_SUCCESS;
}

TEE_Result sha(uint32_t algo, uint8_t* msg, size_t msg_len, uint8_t* digest, size_t* digest_len) {
    TEE_OperationHandle digestop = NULL;
    TEE_Result ret = TEE_AllocateOperation(&digestop, algo, TEE_MODE_DIGEST, 0);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateOperation failed: %08x\n", ret);
        return ret;
    }

    TEE_DigestUpdate(digestop, msg, msg_len);
    ret = TEE_DigestDoFinal(digestop, NULL, 0, digest, digest_len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_DigestDoFinal failed: %08x\n", ret);
    }

    TEE_FreeOperation(digestop);
    return ret;
}

TEE_Result rsa_sign_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len, uint8_t* sig,
        size_t* sign_len) {
    TEE_ObjectHandle keyobj = NULL;
    TEE_OperationHandle signop = NULL;

    TEE_Result ret = TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, 8 * RSA2048_LEN, &keyobj);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateTransientObject failed: %08x\n", ret);
        return ret;
    }

    TEE_Attribute attr[3];
    TEE_InitRefAttribute(&attr[0], TEE_ATTR_RSA_MODULUS, key->n.buf, key->n.len);
    TEE_InitRefAttribute(&attr[1], TEE_ATTR_RSA_PUBLIC_EXPONENT, key->e.buf, key->e.len);
    TEE_InitRefAttribute(&attr[2], TEE_ATTR_RSA_PRIVATE_EXPONENT, key->d.buf, key->d.len);
    ret = TEE_PopulateTransientObject(keyobj, attr, 3);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_PopulateTransientObject failed: %08x\n", ret);
        goto out;
    }

    ret = TEE_AllocateOperation(&signop, TEE_ALG_RSASSA_PKCS1_V1_5_SHA256, TEE_MODE_SIGN,
            8 * RSA2048_LEN);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateOperation failed: %08x\n", ret);
        goto out;
    }

    TEE_SetOperationKey(signop, keyobj);
    ret = TEE_AsymmetricSignDigest(signop, NULL, 0, digest, digest_len, sig, sign_len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AsymmetricSignDigest failed: %08x\n", ret);
        goto out;
    }

    out: if (keyobj != NULL) {
        TEE_FreeTransientObject(keyobj);
        keyobj = NULL;
    }
    if (signop != NULL) {
        TEE_FreeOperation(signop);
        signop = NULL;
    }
    return ret;
}

TEE_Result rsa_verify_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len, uint8_t* sig,
        size_t sig_len) {
    TEE_ObjectHandle keyobj = NULL;
    TEE_OperationHandle verifyop = NULL;

    TEE_Result ret = TEE_AllocateTransientObject(TEE_TYPE_RSA_PUBLIC_KEY, 8 * RSA2048_LEN, &keyobj);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateTransientObject failed: %08x\n", ret);
        return ret;
    }

    TEE_Attribute attr[2];
    TEE_InitRefAttribute(&attr[0], TEE_ATTR_RSA_MODULUS, key->n.buf, key->n.len);
    TEE_InitRefAttribute(&attr[1], TEE_ATTR_RSA_PUBLIC_EXPONENT, key->e.buf, key->e.len);
    ret = TEE_PopulateTransientObject(keyobj, attr, 2);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_PopulateTransientObject failed: %08x\n", ret);
        goto out;
    }

    ret = TEE_AllocateOperation(&verifyop, TEE_ALG_RSASSA_PKCS1_V1_5_SHA256, TEE_MODE_VERIFY,
            8 * RSA2048_LEN);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateOperation failed: %08x\n", ret);
        goto out;
    }

    TEE_SetOperationKey(verifyop, keyobj);
    ret = TEE_AsymmetricVerifyDigest(verifyop, NULL, 0, digest, digest_len, sig, sig_len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AsymmetricVerifyDigest failed: %08x\n", ret);
        goto out;
    }

    out: if (keyobj != NULL) {
        TEE_FreeTransientObject(keyobj);
        keyobj = NULL;
    }
    if (verifyop != NULL) {
        TEE_FreeOperation(verifyop);
        verifyop = NULL;
    }
    return ret;
}

TEE_Result rsa_key_gen(rsa_key_t* key, size_t bits) {
    TEE_ObjectHandle keyobj = NULL;
    TEE_Result ret = TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, bits, &keyobj);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_AllocateTransientObject failed: %08x\n", ret);
        return ret;
    }

    ret = TEE_GenerateKey(keyobj, bits, NULL, 0);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_GenerateKey failed: %08x\n", ret);
        goto out;
    }

    ret = TEE_GetObjectBufferAttribute(keyobj, TEE_ATTR_RSA_MODULUS, key->n.buf, &key->n.len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR("TEE_GetObjectBufferAttribute for MODULUS failed: %08x\n", ret);
    }
    ret = TEE_GetObjectBufferAttribute(keyobj, TEE_ATTR_RSA_PUBLIC_EXPONENT, key->e.buf,
            &key->e.len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR( "TEE_GetObjectBufferAttribute for PUBLIC EXPONENT failed: %08x\n", ret);
    }
    ret = TEE_GetObjectBufferAttribute(keyobj, TEE_ATTR_RSA_PRIVATE_EXPONENT, key->d.buf,
            &key->d.len);
    if (ret != TEE_SUCCESS) {
        LOG_ERROR( "TEE_GetObjectBufferAttribute for PRIVATE EXPONENT failed: %08x\n", ret);
    }

    out: if (keyobj != NULL) {
        TEE_FreeTransientObject(keyobj);
        keyobj = NULL;
    }
    return ret;
}
