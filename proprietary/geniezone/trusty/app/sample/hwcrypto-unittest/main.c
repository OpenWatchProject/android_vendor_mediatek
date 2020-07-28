/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Tests:
 * generic:
 * - no session / invalid session
 * - closed session
 *
 * hwkey:
 * - derive twice to same result
 * - derive different, different result
 * - keyslot, invalid slot
 *
 * rng:
 *
 */

#include <string.h>
#include <stdlib.h>

#include <trusty_unittest.h>
#include <lib/hwkey/hwkey.h>
#include <lib/rng/trusty_rng.h>

#define LOG_TAG "hwcrypto_unittest"

#define RPMB_STORAGE_AUTH_KEY_ID "com.android.trusty.storage_auth.rpmb"
#define HWCRYPTO_UNITTEST_KEYBOX_ID "com.android.trusty.hwcrypto.unittest.key32"

#define STORAGE_AUTH_KEY_SIZE 	32

static const uint8_t UNITTEST_KEYSLOT[] =  "unittestkeyslotunittestkeyslotun";

/**
 * Implement this hook for device specific hwkey tests
 */
__WEAK void run_device_hwcrypto_unittest(void) { }

static hwkey_session_t hwkey_session_;

static void generic_invalid_session(void)
{
	TEST_BEGIN(__func__);

	const uint8_t src_data[] = "thirtytwo-bytes-of-nonsense-data";
	static const uint32_t size = sizeof(src_data);
	uint8_t dest[size];

	hwkey_session_t invalid = INVALID_IPC_HANDLE;
	uint32_t kdf_version = HWKEY_KDF_VERSION_BEST;

	// should fail immediately
	long rc = hwkey_derive(invalid, &kdf_version, src_data, dest, size);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "generic - bad handle");

	TEST_END
}

static void generic_closed_session(void)
{
	TEST_BEGIN(__func__);

	const uint8_t src_data[] = "thirtytwo-bytes-of-nonsense-data";
	static const uint32_t size = sizeof(src_data);;
	uint8_t dest[size];
	uint32_t kdf_version = HWKEY_KDF_VERSION_BEST;

	long rc = hwkey_open();
	EXPECT_GT_ZERO (rc + 1, "generic -  open");

	hwkey_session_t session = (hwkey_session_t) rc;
	hwkey_close(session);

	// should fail immediately
	rc = hwkey_derive(session, &kdf_version, src_data, dest, size);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "generic - closed handle");

	TEST_END
}

static void hwkey_derive_repeatable(void)
{
	TEST_BEGIN(__func__);

	static const uint32_t size = 32;
	const uint8_t src_data[] = "thirtytwo-bytes-of-nonsense-data";
	uint8_t dest[size];
	uint8_t dest2[size];
	uint32_t kdf_version = HWKEY_KDF_VERSION_BEST;

	memset(dest, 0, size);
	memset(dest2, 0, size);

	/* derive key once */
	long rc = hwkey_derive(hwkey_session_, &kdf_version, src_data, dest, size);
	EXPECT_EQ (NO_ERROR, rc, "derive repeatable - initial derivation");
	EXPECT_NE (HWKEY_KDF_VERSION_BEST, kdf_version, "derive repeatable - kdf version");

	/* derive key again */
	rc = hwkey_derive(hwkey_session_, &kdf_version, src_data, dest2, size);
	EXPECT_EQ (NO_ERROR, rc, "derive repeatable - second derivation");

	/* ensure they are the same */
	rc = memcmp(dest, dest2, size);
	EXPECT_EQ (0, rc, "derive repeatable - equal");
	rc = memcmp(dest, src_data, size);
	EXPECT_NE (0, rc, "derive repeatable - same as seed");

	TEST_END
}

static void hwkey_derive_different(void)
{
	TEST_BEGIN(__func__);

	static const uint32_t size = 32;
	const uint8_t src_data[] = "thirtytwo-bytes-of-nonsense-data";
	const uint8_t src_data2[] = "thirtytwo-byt3s-of-nons3ns3-data";

	uint8_t dest[size];
	uint8_t dest2[size];
	uint32_t kdf_version = HWKEY_KDF_VERSION_BEST;

	memset(dest, 0, size);
	memset(dest2, 0, size);

	/* derive key once */
	long rc = hwkey_derive(hwkey_session_, &kdf_version, src_data, dest, size);
	EXPECT_EQ (NO_ERROR, rc, "derive not repeatable - initial derivation");
	EXPECT_NE (HWKEY_KDF_VERSION_BEST, kdf_version, "derive not repeatable - kdf version");

	/* derive key again, with different source data */
	rc = hwkey_derive(hwkey_session_, &kdf_version, src_data2, dest2, size);
	EXPECT_EQ (NO_ERROR, rc, "derive not repeatable - second derivation");

	/* ensure they are not the same */
	rc = memcmp(dest, dest2, size);
	EXPECT_NE (0, rc, "derive not repeatable - equal");
	rc = memcmp(dest, src_data, size);
	EXPECT_NE (0, rc, "derive not repeatable - equal to source");
	rc = memcmp(dest2, src_data2, size);
	EXPECT_NE (0, rc, "derive not repeatable - equal to source");

	TEST_END
}

static void hwkey_derive_zero_length(void)
{
	TEST_BEGIN(__func__);

	static const uint32_t size = 0;
	const uint8_t *src_data = NULL;
	uint8_t *dest = NULL;
	uint32_t kdf_version = HWKEY_KDF_VERSION_BEST;

	/* derive key once */
	long rc = hwkey_derive(hwkey_session_, &kdf_version, src_data, dest, size);
	EXPECT_EQ (ERR_NOT_VALID, rc, "derive zero length");

	TEST_END
}

static void hwkey_get_storage_auth(void)
{
	TEST_BEGIN(__func__);

	uint32_t actual_size = STORAGE_AUTH_KEY_SIZE;
	uint8_t storage_auth_key[STORAGE_AUTH_KEY_SIZE];
	long rc = hwkey_get_keyslot_data(hwkey_session_, RPMB_STORAGE_AUTH_KEY_ID,
			storage_auth_key, &actual_size);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "auth key accessible when it shouldn't be");

	TEST_END
}

static void hwkey_get_keybox(void)
{
	TEST_BEGIN(__func__);

	uint32_t actual_size = strlen(HWCRYPTO_UNITTEST_KEYBOX_ID) + 1;
	uint8_t dest[actual_size];
	long rc = hwkey_get_keyslot_data(hwkey_session_, HWCRYPTO_UNITTEST_KEYBOX_ID,
			dest, &actual_size);

#ifdef WITH_HWCRYPTO_UNITTEST
	EXPECT_EQ (NO_ERROR, rc, "get hwcrypto-unittest keybox");
	rc = memcmp(UNITTEST_KEYSLOT, dest, strlen(UNITTEST_KEYSLOT));
	EXPECT_EQ (0, rc, "get storage auth key invalid");
#else
	EXPECT_EQ (ERR_NOT_FOUND, rc, "get hwcrypto-unittest keybox");
#endif

	TEST_END
}

static void run_hwkey_tests(void)
{
	TLOGI("WELCOME TO HWKEY UNITTEST!\n");
	long rc = hwkey_open();
	if (rc < 0) {
		TLOGI("err (%d) opening hwkey session\n", rc);
		return;
	}

	hwkey_session_ = (hwkey_session_t) rc;

	generic_invalid_session();
	generic_closed_session();

	hwkey_derive_repeatable();
	hwkey_derive_different();
	hwkey_derive_zero_length();
	hwkey_get_storage_auth();
	hwkey_get_keybox();

	hwkey_close(hwkey_session_);

	/* run device specific tests if available */
	run_device_hwcrypto_unittest();
}


/***********************   HWRNG  UNITTEST  *******************************************/

static uint32_t _hist[256];
static uint8_t  _rng_buf[1024];

static void hwrng_update_hist(uint8_t *data, uint cnt)
{
	for (uint i = 0; i < cnt; i++) {
		_hist[data[i]]++;
	}
}

static void hwrng_show_data(const void *ptr, size_t len)
{
	addr_t address = (addr_t)ptr;
	size_t count;
	size_t i;

	fprintf(stderr, "Dumping first hwrng request:\n");
	for (count = 0 ; count < len; count += 16) {
		for (i=0; i < MIN(len - count, 16); i++) {
			fprintf(stderr, "0x%02hhx ", *(const uint8_t *)(address + i));
		}
		fprintf(stderr, "\n");
		address += 16;
	}
}

static void run_hwrng_show_data_test(void)
{
	int rc;

	TEST_BEGIN(__func__);

	rc = trusty_rng_hw_rand(_rng_buf, 32);
	EXPECT_EQ (NO_ERROR, rc, "hwrng test");
	if (rc == NO_ERROR) {
		hwrng_show_data(_rng_buf, 32);
	}

	TEST_END
}

static void run_hwrng_var_rng_req_test(void)
{
	int rc;
	uint i;
	size_t req_cnt;

	TEST_BEGIN(__func__);

	/* Issue 100 hwrng requests of variable sizes */
	for (i = 0; i < 100; i++ ) {
		req_cnt = ((size_t)rand() % sizeof(_rng_buf)) + 1;
		rc = trusty_rng_hw_rand(_rng_buf, req_cnt);
		EXPECT_EQ (NO_ERROR, rc, "hwrng test");
		if (rc != NO_ERROR) {
			TLOGI("trusty_rng_hw_rand returned %d\n", rc);
			continue;
		}
	}

	TEST_END
}

static void run_hwrng_stats_test(void)
{
	int rc;
	uint i;
	size_t req_cnt;
	uint32_t exp_cnt;
	uint32_t cnt = 0;
	uint32_t ave = 0;
	uint32_t dev = 0;

	TEST_BEGIN(__func__);

	/* issue 100x256 bytes requests */
	req_cnt = 256;
	exp_cnt = 1000 * req_cnt;
	memset(_hist, 0, sizeof(_hist));
	for (i = 0; i < 1000; i++ ) {
		rc = trusty_rng_hw_rand(_rng_buf, req_cnt);
		EXPECT_EQ (NO_ERROR, rc, "hwrng test");
		if (rc != NO_ERROR) {
			TLOGI("trusty_rng_hw_rand returned %d\n", rc);
			continue;
		}
		hwrng_update_hist(_rng_buf, req_cnt);
	}

	/* check hwrng stats */
	for (i = 0; i < 256; i++)
		cnt += _hist[i];
	ave = cnt / 256;
	EXPECT_EQ(exp_cnt, cnt, "hwrng ttl sample cnt");
	EXPECT_EQ(1000, ave, "hwrng eve sample cnt");

	/**
	 * Ideally data should be uniformly distributed
	 * Calculate average deviation from ideal model
	 */
	for (i = 0; i < 256; i++) {
		int val = _hist[i] - ave;
		if (val < 0)
			val = -val;
		dev += val;
	}
	dev /= 256;

	/* Check if everage deviation is within 5% of ideal model
	 * which is fairly arbitrary requirement. It could be useful
	 * to alert is something terribly wrong with rng source.
	 */
	EXPECT_GT(50, dev, "average dev");

	TEST_END
}

static void run_hwrng_tests(void)
{
	TLOGI("WELCOME TO HWRNG UNITTEST!\n");
	run_hwrng_show_data_test();
	run_hwrng_var_rng_req_test();
	run_hwrng_stats_test();
}

static void run_all_tests(void) {
	run_hwrng_tests();
	run_hwkey_tests();
}

int main(void) {
	run_all_tests();
}

