/*
 * This header file defines the implementation-dependent types,
 * constants and macros for all the Trusted Foundations implementations
 * of the TEE Client API
 */
#ifndef __TEE_CLIENT_API_IMP_H__
#define __TEE_CLIENT_API_IMP_H__

#define TEEC_MEM_INOUT (TEEC_MEM_INPUT | TEEC_MEM_OUTPUT)

struct tee_client;

struct teec_context_imp {
	u64    client; // FIXME!! linux kernel use struct tee_client *
};

struct teec_session_imp {
	u32			session_id;
	struct teec_context_imp context;
	u8			active;  // FIXME!! linux kernel use bool
};

struct teec_shared_memory_imp {
	struct tee_client *client;
	u8 implementation_allocated;  // FIXME!! linux kernel use bool
};

struct teec_operation_imp {
	u64 session; // FIXME!! linux kernel use struct teec_session_imp *
};

/*
 * There is no natural, compile-time limit on the shared memory, but a specific
 * implementation may introduce a limit (in particular on TrustZone)
 */
#define TEEC_CONFIG_SHAREDMEM_MAX_SIZE ((size_t)0xFFFFFFFF)

#define TEEC_PARAM_TYPES(entry0_type, entry1_type, entry2_type, entry3_type) \
	((entry0_type) | ((entry1_type) << 4) | \
	((entry2_type) << 8) | ((entry3_type) << 12))

#endif /* __TEE_CLIENT_API_IMP_H__ */
