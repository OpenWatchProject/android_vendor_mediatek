#ifndef _GP_H_
#define _GP_H_

#ifdef __cplusplus
extern "C" {
#endif

void hex_dump(const char* desc, void* addr, int len);

TEE_Result read_file(char* path, uint8_t* data, size_t data_len);

TEE_Result write_file(char* path, uint8_t* data, size_t data_len);

TEE_Result delete_file(char* path);

TEE_Result sha(uint32_t algo, uint8_t* msg, size_t msg_len, uint8_t* digest, size_t* digest_len);

TEE_Result rsa_sign_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len, uint8_t* sig,
        size_t* sign_len);

TEE_Result rsa_verify_digest(rsa_key_t* key, uint8_t* digest, size_t digest_len, uint8_t* sig,
        size_t sig_len);

TEE_Result rsa_key_gen(rsa_key_t* key, size_t bits);

#ifdef __cplusplus
}
#endif

#endif
