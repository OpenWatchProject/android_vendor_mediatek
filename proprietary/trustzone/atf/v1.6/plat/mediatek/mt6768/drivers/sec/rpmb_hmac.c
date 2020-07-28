#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <lk_shared_mem.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform_def.h>
#include <platform.h>
#include <sha2_export.h>
#include <stdint.h>
#include <string.h>
#include <rpmb_hmac.h>
#include <xlat_tables_v2.h>
#include <tz_tags.h>
#include <memory_layout.h>


/* #define MTK_RPMB_DEBUG */
/* #define MTK_HMAC_TEST_VECTOR */
#define RPMB_KEY_SIZE                (32)        /* fixed 32 bytes in rpmb case */


/* sync with preloader tz_mem.h, platform-dependent code */
#define ATF_BOOT_ARG_ADDR       (ATF_ATF_BASE)
#define TEE_BOOT_ARG_ADDR       (ATF_BOOT_ARG_ADDR + 0x100)
#define TEE_PARAMETER_ADDR      (TEE_BOOT_ARG_ADDR + 0x100)


/* copied from preloader tz_mem.h */
struct sec_mem_arg_t {
	unsigned int magic;             /* Magic number */
	unsigned int version;           /* version */
	unsigned long long svp_mem_start;     /* MM sec mem pool start addr. */
	unsigned long long svp_mem_end;       /* MM sec mem pool end addr. */
	unsigned int tplay_table_start; /* tplay handle-to-physical table start */
	unsigned int tplay_table_size;  /* tplay handle-to-physical table size */
	unsigned int tplay_mem_start;   /* tplay physcial memory start address for crypto operation */
	unsigned int tplay_mem_size;    /* tplay phsycial memory size for crypto operation */
	unsigned int secmem_obfuscation;/* MM sec mem obfuscation or not */
	unsigned int msg_auth_key[8];   /* size of message auth key is 32bytes(256bits) */
	unsigned int rpmb_size;         /* size of rpmb partition */
	unsigned int shared_secmem;     /* indicate if the memory is shared between REE and TEE */
	unsigned int m4u_mem_start;     /* reserved start address of secure memory for m4u */
	unsigned int m4u_mem_size;      /* reserved size of secure memory for m4u */
	unsigned int cmdq_mem_start;    /* reserved start address of secure memory for cmdq */
	unsigned int cmdq_mem_size;     /* reserved size of secure memory for cmdq */
};

static int rpmb_is_in_lk = MTK_RPMB_IN_LK;
static uint8_t rpmbk[HMAC_BLOCK_SIZE] = {0};   /* it has to be block_size(2*key_size) */


#ifdef MTK_RPMB_DEBUG
/* dump buffer content  */
static void dump_buf(uint8_t *buf, uint32_t size)
{
	int i;

	for (i = 0; i < size/8; i++) {
		INFO("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[0+i*8], buf[1+i*8], buf[2+i*8], buf[3+i*8],
				buf[4+i*8], buf[5+i*8], buf[6+i*8], buf[7+i*8]);
	}

	for (i = 0; i < size%8; i++)
		INFO("%02x\n", buf[((size/8)*8) + i]);
}
#endif


/* must be called after  */
static int32_t read_rpmbk(void)
{

	uint32_t key_size = RPMB_KEY_SIZE;
	u8 *rpmb_key_param = NULL;

	/* rpmb key generated in preloader are of length 32 bytes, < block_size (64bytes) */
	if (IS_TAG_FMT((struct tz_tag *)(TEE_PARAMETER_ADDR))) {
		struct tz_tag *flash_dev_tag = find_tag(TEE_PARAMETER_ADDR, TZ_TAG_FLASH_DEV_INFO);

		if (!IS_NULL_TAG(flash_dev_tag))
			rpmb_key_param = (u8 *)&flash_dev_tag->u.flash_dev_cfg.rpmb_msg_auth_key;
	} else {
		struct sec_mem_arg_t *sec_mem_ptr = (struct sec_mem_arg_t *) (TEE_PARAMETER_ADDR);

		rpmb_key_param = (u8 *)&sec_mem_ptr->msg_auth_key;
	}

#ifdef MTK_RPMB_DEBUG
	INFO("dump original rpmb key in tee para:\n");
	dump_buf((u8 *)rpmb_key_param, 32);
#endif

	if (rpmb_key_param != NULL)
		memcpy(rpmbk, rpmb_key_param, RPMB_KEY_SIZE);
	else
		ERROR("rpmb key param is null!\n");

	/* keys shorter than blocksize are zero-padded */
	if (key_size < HMAC_BLOCK_SIZE)
		memset(rpmbk+key_size, 0x0, (HMAC_BLOCK_SIZE-key_size));

#ifdef MTK_RPMB_DEBUG
	INFO("rpmb key:\n");
	dump_buf(rpmbk, HMAC_BLOCK_SIZE);
#endif
	return MTK_SIP_E_SUCCESS;
}

/* clear rpmb key after HMAC is calculated */
static int32_t clear_rpmbk(void)
{
	memset(rpmbk, 0x0, HMAC_BLOCK_SIZE);
	return MTK_SIP_E_SUCCESS;
}

static int32_t reinit_key_pad(uint8_t *i_key_pad, uint8_t *o_key_pad)
{
	/* reinit o_key_pad */
	memset(o_key_pad, 0x5C, HMAC_BLOCK_SIZE);

	/* reinit i_key_pad */
	memset(i_key_pad, 0x36, HMAC_BLOCK_SIZE);

	return MTK_SIP_E_SUCCESS;
}

/*
 * rpmb_init: set shared memory region with lk.
 * return 0 for success, otherwise negative status code.
 */
int32_t rpmb_init(void)
{
	uint32_t phy_addr = LK_SHARED_MEM_ADDR;
	int ret_val = 0;

#ifdef MTK_RPMB_DEBUG
	uint64_t *shared_mem_ptr;
	uint64_t offset = 0;

	set_uart_flag();
	INFO("[%s] in rpmb shared mem smc, phy_addr:0x%x rpmb_is_in_lk:0x%x!!\n",
			__func__, phy_addr, rpmb_is_in_lk);
#endif

	/* return err if it's already invoked in lk */
	if (rpmb_is_in_lk == MTK_RPMB_LEAVING_LK) {
		INFO("[%s] no longer in lk, return immediately\n", __func__);
		return MTK_SIP_E_NOT_SUPPORTED;
	} else if (rpmb_is_in_lk == MTK_RPMB_IN_LK) {
		INFO("[%s] invoked from lk\n", __func__);
	}

	/* map physcial memory for 4KB size.
	 * assert inside mmap_add_dynamic_region if error occurs. no need to check return
	 * value in this case
	 */

	ret_val = mmap_add_dynamic_region((uint64_t)(phy_addr & ~(PAGE_SIZE_MASK)),
			(uint64_t)(phy_addr & ~(PAGE_SIZE_MASK)),
			PAGE_SIZE,
			MT_DEVICE | MT_RW | MT_NS);

	if (ret_val) {
		ERROR("[%s] dynammic mmap fail, ret=%d\n", __func__, ret_val);
		assert(!ret_val);
	}

	/* flush phy_addr content  */
	flush_dcache_range((uint64_t)(phy_addr & ~(PAGE_SIZE_MASK)), (uint64_t)PAGE_SIZE);


#ifdef MTK_RPMB_DEBUG

	shared_mem_ptr = (uint64_t *)(uintptr_t)(phy_addr);
	offset = phy_addr - (phy_addr & ~(PAGE_SIZE_MASK));
	INFO("[%s] in rpmb shared mem smc, aligned phy_addr:0x%x offset:0x%x!!\n",
			__func__, (phy_addr & ~(PAGE_SIZE_MASK)), offset);
	INFO("[%s] dump content 0x%x set in lk nwd:\n", __func__, shared_mem_ptr);
	unsigned int i = 0;

	for (i = 0; i < 2; i++, shared_mem_ptr++)
		INFO("0x%16lx:0x%16lx\n", (shared_mem_ptr), *(shared_mem_ptr));

	clear_uart_flag();
#endif
	return MTK_SIP_E_SUCCESS;
}

/*
 * rpmb_uninit: mark as leaving lk, avoid request from kernel. ToDo:release the shared memory region with lk
 * return 0 for success, otherwise negative status code.
 */
int32_t rpmb_uninit(void)
{
	uint32_t phy_addr = LK_SHARED_MEM_ADDR;
	int ret_val = 0;

	/* ToDo: ummap the shared memory with LK for RPMB
	 * once unmap API is available in ATF
	 */
	ret_val = mmap_remove_dynamic_region((uint64_t)(phy_addr & ~(PAGE_SIZE_MASK)),
								PAGE_SIZE);
	if (ret_val) {
		ERROR("[%s] dynammic unmap fail, ret=%d\n", __func__, ret_val);
		assert(!ret_val);
	}

#ifdef MTK_RPMB_DEBUG
	set_uart_flag();
	INFO("[%s] mark leaving lk\n", __func__);
#endif
	/* mark leaving lk, kernel can't making smc call for HMAC computation */
	rpmb_is_in_lk = MTK_RPMB_LEAVING_LK;

#ifdef MTK_RPMB_DEBUG
	clear_uart_flag();
#endif
	return MTK_SIP_E_SUCCESS;
}

/*
 * hmac_sha256: calculate HMAC with sepcified key and msg.
 * @key: specified key
 * @key_size: the size of key for hmac, here we don't consider key_size >  block_size
 * @msg: specified msg
 * @msg_len: the size of msg.
 * @mac: mac result, here the size is 32 bytes for sha256.
 * return 0 for success, otherwise negative status code.
 */
static int32_t hmac_sha256(uint8_t *key, uint32_t key_size, uint8_t *msg,
		uint32_t msg_len, uint8_t *mac){

	int i = 0;
	uint8_t *i_key_pad_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + I_KEY_PAD_OFFSET);
	uint8_t *o_key_pad_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + O_KEY_PAD_OFFSET);
	uint8_t *hash1_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + HASH1_OFFSET);
	uint8_t *hash2_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + HASH2_OFFSET);

#ifdef MTK_RPMB_DEBUG
	set_uart_flag();
	INFO("[%s] entering...\n", __func__);

	INFO("key:\n");
	dump_buf(key, key_size);

	INFO("msg addr:0x%x msg:\n", msg);
	dump_buf(msg, msg_len);
#endif

	/* check valid range */
	if (msg_len > MSG_MAX_SIZE) {
		INFO("[%s] size over maximum allowed size\n", __func__);
		return MTK_SIP_E_INVALID_RANGE;
	}

	if ((msg_len == 0) || (key_size == 0))
		INFO("[%s] key or msg size is zero\n", __func__);

	/* the key is always of fixed length in our use case.(HMAC-SHA256)
	 *  so no need to check key length
	 */
	reinit_key_pad(i_key_pad_ptr, o_key_pad_ptr);

#ifdef MTK_RPMB_DEBUG

	INFO("i_key_pad:\n");
	dump_buf(i_key_pad_ptr, HMAC_BLOCK_SIZE);

	INFO("o_key_pad:\n");
	dump_buf(o_key_pad_ptr, HMAC_BLOCK_SIZE);

#endif


	if (key_size > HMAC_BLOCK_SIZE) {
		/* do not consider this case in this implementation,
		 * key is always < HMAC_BLOCK_SIZE
		 */
	}

	/* key xorred with o_key_pad & i_key_pad,
	 * rest is filled with 0x36(inner) or 0x5C(outter)
	 */
	for (i = 0; i < key_size; i++) {
		o_key_pad_ptr[i] = o_key_pad_ptr[i] ^ key[i];
		i_key_pad_ptr[i] = i_key_pad_ptr[i] ^ key[i];
	}

#ifdef MTK_RPMB_DEBUG

	INFO("key_size:%d\n", key_size);
	INFO("i_key_pad[with key]:\n");
	dump_buf(i_key_pad_ptr, HMAC_BLOCK_SIZE);

	INFO("o_key_pad[with key]:\n");
	dump_buf(o_key_pad_ptr, HMAC_BLOCK_SIZE);

#endif

	/*
	 *  shared memory layout
	 *  |           |           |     |            |       |         |       |         |    |                 |
	 *  | i_key_pad |  message  | ... | o_key_pad  | hash1 | padding | hash2 | padding | iv |sha256 operation |
	 *  |           |           |     |            |       |         |       |         |    |                 |
	 *    i_key_size   msg_size         o_key_size   sha256   128B     sha256   128B     iv  sha256 mass data |
	 *
	 */

#ifdef MTK_RPMB_DEBUG
	INFO("compute first hash ==> hash(i_key_pad ¡ü message)\n");
#endif

	/* compute first hash ==> hash(i_key_pad ¡ü message) */
	sha256(i_key_pad_ptr, (msg_len + HMAC_BLOCK_SIZE), hash1_ptr);

#ifdef MTK_RPMB_DEBUG
	INFO("dump first hash:\n");
	dump_buf(hash1_ptr, SHA256_SIZE);
	INFO("compute second hash ==> hash(o_key_pad ¡ühash1)\n");
#endif

	/* compute second hash ==> hash(o_key_pad ¡ühash1) */
	sha256(o_key_pad_ptr, (HMAC_BLOCK_SIZE + SHA256_SIZE), hash2_ptr);

#ifdef MTK_RPMB_DEBUG
	INFO("dump second hash:\n");
	dump_buf(hash2_ptr, SHA256_SIZE);
#endif

	memcpy(mac, hash2_ptr, SHA256_SIZE);
	return MTK_SIP_E_SUCCESS;
}

#ifdef MTK_HMAC_TEST_VECTOR
#define MTK_HMAC_TEST_VECTOR1 (1)
#define MTK_HMAC_TEST_VECTOR2 (0)
#define MTK_HMAC_TEST_VECTOR3 (0)

static int32_t hmac_test(void)
{

	uint32_t key_size = 0;
	uint32_t msg_size = 0;
	uint8_t *msg_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + MSG_OFFSET);
	uint8_t *mac_result = (uint8_t *)(LK_SHARED_MEM_ADDR + HASH2_OFFSET);

#if MTK_HMAC_TEST_VECTOR1
	/* Test vector 1 */
	/* refer to IETF RFC 4231 Test Case 1, https://tools.ietf.org/html/rfc4231#section-4.1 */
	/* Answer: b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7 */
	key_size = 20;
	memset(rpmbk, 0x0b, key_size);
	msg_size = 8;
	char test_str[8] = {0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65};

	memcpy(msg_ptr, test_str, msg_size);
#endif

#if MTK_HMAC_TEST_VECTOR2
	/* Test vector 2 */
	/* refer to IETF RFC 4231 Test Case 2, https://tools.ietf.org/html/rfc4231#section-4.1 */
	/* Answer: 5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843 */
	char test_str[28] = {0x77, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6f, 0x20, 0x79,
		0x61, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20, 0x66, 0x6f,
		0x72, 0x20, 0x6e, 0x6f, 0x74, 0x68, 0x69, 0x6e, 0x67,
		0x3f};
	char key[4] = {0x4a, 0x65, 0x66, 0x65};

	key_size = 4;
	msg_size = 28;
	memcpy(rpmbk, key, key_size);
	memcpy(msg_ptr, test_str, msg_size);
#endif

#if MTK_HMAC_TEST_VECTOR3
	/* Test vector 3 */
	/* refer to IETF RFC 4231 Test Case 3, https://tools.ietf.org/html/rfc4231#section-4.1 */
	/* Answer: 773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe*/
	key_size = 20;
	memset(rpmbk, 0xaa, key_size);
	msg_size = 50;
	memset(msg_ptr, 0xdd, msg_size);
#endif

	hmac_sha256(rpmbk, key_size, msg_ptr, msg_size, mac_result);

	/* compute second hash ==> hash(o_key_pad ¡ühash1) */
	INFO("dump test vector result\n");
	dump_buf(mac_result, SHA256_SIZE);

	return MTK_SIP_E_SUCCESS;
}

#endif

/*
 * rpmb_hmac: calculate HMAC with sepcified offset and size in shared memory.
 * @offset: offset of the shared memory
 * @size: the size of data that needs to calculate HMAC
 * return 0 for success, otherwise negative status code.
 */
int32_t rpmb_hmac(uint32_t offset, uint32_t size)
{
	/*
	 *  shared memory layout
	 *  |           |           |     |            |       |         |       |         |    |                 |
	 *  | i_key_pad |  message  | ... | o_key_pad  | hash1 | padding | hash2 | padding | iv |sha256 operation |
	 *  |           |           |     |            |       |         |       |         |    |                 |
	 *    i_key_size   msg_size         o_key_size   sha256   128B     sha256   128B     iv  sha256 mass data |
	 *
	 */
	uint8_t *msg_ptr = (uint8_t *)(LK_SHARED_MEM_ADDR + MSG_OFFSET);
	uint8_t *mac_result = (uint8_t *)(LK_SHARED_MEM_ADDR + HASH2_OFFSET);

#ifdef MTK_RPMB_DEBUG
	set_uart_flag();
	INFO("[%s] entering...\n", __func__);

#endif

	/* return err if it's already invoked in lk */
	if (rpmb_is_in_lk == MTK_RPMB_LEAVING_LK) {
		INFO("[%s] no longer in lk, return immediately\n", __func__);
		return MTK_SIP_E_NOT_SUPPORTED;
	} else if (rpmb_is_in_lk == MTK_RPMB_IN_LK) {
		INFO("[%s] invoked from lk\n", __func__);
	}

	/* check valid range */
	if (size > MSG_MAX_SIZE) {
		INFO("[%s] size over maximum allowed size\n", __func__);
		return MTK_SIP_E_INVALID_RANGE;

	}

	/* not allow empty msg in our case */
	if (size == 0) {
		INFO("[%s] size is zero\n", __func__);
		return MTK_SIP_E_INVALID_RANGE;
	}

#ifdef MTK_HMAC_TEST_VECTOR
	hmac_test();
	return MTK_SIP_E_SUCCESS;
#endif

	read_rpmbk();

	hmac_sha256(rpmbk, RPMB_KEY_SIZE, msg_ptr, size, mac_result);

#ifdef MTK_RPMB_DEBUG
	INFO("dump final mac:\n");
	dump_buf(mac_result, SHA256_SIZE);
#endif

	clear_rpmbk();

#ifdef MTK_RPMB_DEBUG
	INFO("[%s] leaving...\n", __func__);
	clear_uart_flag();
#endif
	return MTK_SIP_E_SUCCESS;

	/*
	 * lk is running as single threaded, so no need to consider sync issue in
	 * shared memory at the moment.
	 */
}

