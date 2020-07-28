#ifndef EXTRACT_H_
#define EXTRACT_H_

#define EXTRACT_PRIMITIVE(type, dest) \
	do { \
		if (p + sizeof(type) > pend) { \
			ALOGE("Too short input"); \
			goto error; \
		} \
		memcpy(&dest, p, sizeof(type)); \
		p += sizeof(type); \
	} while (0)
#define EXTRACT_BYTES(dest, len) \
	do { \
		if (p + (len) > pend) { \
			ALOGE("Too short input, need %uB, remain %zdB, %p, %p", (len), pend - p, p + (len), pend); \
			goto error; \
		} \
		memcpy(dest, p, (len)); \
		p += (len); \
	} while (0)
#define EXTRACT_BYTES_NOCOPY(dest, len) \
	do { \
		if (p + len > pend) { \
			ALOGE("Too short input, need %uB, remain %zdB", (len), pend - p); \
			goto error; \
		} \
		dest = p; \
		p += len; \
	} while (0)
#define EXTRACT_PRIMITIVE_ERR(type, dest, error) \
	do { \
		if (p + sizeof(type) > pend) { \
			ALOGE("Too short input"); \
			goto error; \
		} \
		memcpy(&dest, p, sizeof(type)); \
		p += sizeof(type); \
	} while (0)
#define EXTRACT_BYTES_ERR(dest, len, error) \
	do { \
		if (p + (len) > pend) { \
			ALOGE("Too short input, need %uB, remain %zdB", (len), pend - p); \
			goto error; \
		} \
		memcpy(dest, p, (len)); \
		p += (len); \
	} while (0)
#define EXTRACT_BYTES_NOCOPY_ERR(dest, len, error) \
	do { \
		if (p + len > pend) { \
			ALOGE("Too short input, need %uB, remain %zdB", (len), pend - p); \
			goto error; \
		} \
		dest = p; \
		p += len; \
	} while (0)

#define PACK_PRIMITIVE_ERR(type, src, error) \
	do { \
		if (p + sizeof(type) > pend) { \
			ALOGE("Too short buffer"); \
			goto error; \
		} \
		memcpy(p, &src, sizeof(type)); \
		p += sizeof(type); \
	} while (0)
#define PACK_BYTES_ERR(src, len, error) \
	do { \
		if (p + len > pend) { \
			ALOGE("Too short buffer"); \
			goto error; \
		} \
		memcpy(p, src, len); \
		p += len; \
	} while (0)
#define PACK_EXP_ERR(type, exp, error) \
	do { \
		type tmp = exp; \
		if (p + sizeof(type) > pend) { \
			ALOGE("Too short buffer"); \
			goto error; \
		} \
		memcpy(p, &tmp, sizeof(type)); \
		p += sizeof(type); \
	} while (0)
#define PACK_BN_ERR(bn, error) \
	do { \
		if (p + sizeof(uint32_t) + BN_num_bytes(bn) > pend) { \
			ALOGE("Too short buffer"); \
			goto error; \
		} \
		PACK_EXP_ERR(uint32_t, BN_num_bytes(bn), error); \
		BN_bn2bin(bn, p); \
		p += BN_num_bytes(bn); \
	} while (0)
#define PACK_PRIMITIVE(type, src) PACK_PRIMITIVE_ERR(type, src, error)
#define PACK_BYTES(src, len) PACK_BYTES_ERR(src, len, error)
#define PACK_EXP(type, exp) PACK_EXP_ERR(type, exp , error)

#endif
