#define LOG_TAG "KMSetkey"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <type_traits>

#include <log/log.h>
#include <cutils/log.h>

#include <kmsetkey.h>
#include <kmsetkey_ipc.h>

#define SINGLE_RUN 1

int32_t ree_benchmark(const char *peakb, const bool finish)
{
	int32_t rc, tries, rand_tries;
	uint32_t out_size;
	uint8_t out[MAX_MSG_SIZE];
	struct kmsetkey_msg *msg;
	struct timespec t1, t2;
	double time_diff;

	if (!finish)
		return 0;

	rc = kmsetkey_connect();
	if (rc < 0) {
		ALOGE("kmsetkey_connect failed: %d\n", rc);
		printf("kmsetkey_connect failed: %d\n", rc);
		return rc;
	}

	switch (peakb[0]) {
	case 'R':
#if SINGLE_RUN
		if (peakb[1] == 'K') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(RSA_KEY, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("RSA_KEY: time_diff = %lf ms\n", time_diff);
			printf("RSA_KEY: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'S') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(RSA_SIGN, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("RSA_SIGN: time_diff = %lf ms\n", time_diff);
			printf("RSA_SIGN: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'V') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(RSA_VERI, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("RSA_VERI: time_diff = %lf ms\n", time_diff);
			printf("RSA_VERI: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'T') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(RSA_TEST, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("RSA_TEST: time_diff = %lf ms\n", time_diff);
			printf("RSA_TEST: time_diff = %lf ms\n", time_diff);
		} else {
			rc = -1;
		}

		//msg = (struct kmsetkey_msg *)out;
		//memcpy(&tries, msg->payload, sizeof(int32_t));
		//memcpy(&rand_tries, msg->payload + sizeof(int32_t), sizeof(int32_t));
		//ALOGE("kmsetkey_call: tries = %d, rand_tries = %d\n", tries, rand_tries);
		//printf("kmsetkey_call: tries = %d, rand_tries = %d\n", tries, rand_tries);
#else
		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(RSA_KEY, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("RSA_KEY: time_diff = %lf ms\n", time_diff);
		printf("RSA_KEY: time_diff = %lf ms\n", time_diff);

		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(RSA_SIGN, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("RSA_SIGN: time_diff = %lf ms\n", time_diff);
		printf("RSA_SIGN: time_diff = %lf ms\n", time_diff);

		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(RSA_VERI, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("RSA_VERI: time_diff = %lf ms\n", time_diff);
		printf("RSA_VERI: time_diff = %lf ms\n", time_diff);
#endif
		break;
	case 'E':
#if SINGLE_RUN
		if (peakb[1] == 'K') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(ECC_KEY, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("ECC_KEY: time_diff = %lf ms\n", time_diff);
			printf("ECC_KEY: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'S') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(ECC_SIGN, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("ECC_SIGN: time_diff = %lf ms\n", time_diff);
			printf("ECC_SIGN: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'V') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(ECC_VERI, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("ECC_VERI: time_diff = %lf ms\n", time_diff);
			printf("ECC_VERI: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'T') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(ECC_TEST, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("ECC_TEST: time_diff = %lf ms\n", time_diff);
			printf("ECC_TEST: time_diff = %lf ms\n", time_diff);
		} else {
			rc = -1;
		}
#else
		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(ECC_KEY, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("ECC_KEY: time_diff = %lf ms\n", time_diff);
		printf("ECC_KEY: time_diff = %lf ms\n", time_diff);

		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(ECC_SIGN, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("ECC_SIGN: time_diff = %lf ms\n", time_diff);
		printf("ECC_SIGN: time_diff = %lf ms\n", time_diff);

		out_size = MAX_MSG_SIZE;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		rc = kmsetkey_call(ECC_VERI, true, NULL, 0, out, &out_size);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
		time_diff -= t1.tv_nsec;
		time_diff /= 1000000;
		ALOGE("ECC_VERI: time_diff = %lf ms\n", time_diff);
		printf("ECC_VERI: time_diff = %lf ms\n", time_diff);
#endif
		break;
	case 'A':
		if (peakb[1] == '1') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(AES_128_ENC, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("AES_128_ENC: time_diff = %lf ms\n", time_diff);
			printf("AES_128_ENC: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == '2') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(AES_256_ENC, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("AES_256_ENC: time_diff = %lf ms\n", time_diff);
			printf("AES_256_ENC: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'T') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(AES_TEST, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("AES_256_TEST: time_diff = %lf ms\n", time_diff);
			printf("AES_256_TEST: time_diff = %lf ms\n", time_diff);
		} else {
			rc = -1;
		}
		break;
	case 'S':
		if (peakb[1] == 'H') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(SHA_HASH, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("SHA_HASH: time_diff = %lf ms\n", time_diff);
			printf("SHA_HASH: time_diff = %lf ms\n", time_diff);
		} else if (peakb[1] == 'T') {
			out_size = MAX_MSG_SIZE;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			rc = kmsetkey_call(SHA_TEST, true, NULL, 0, out, &out_size);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			time_diff = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec;
			time_diff -= t1.tv_nsec;
			time_diff /= 1000000;
			ALOGE("SHA_TEST: time_diff = %lf ms\n", time_diff);
			printf("SHA_TEST: time_diff = %lf ms\n", time_diff);
		} else {
			rc = -1;
		}
		break;
	default:
		rc = -1;
		break;
	}

	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d\n", rc);
		printf("kmsetkey_call failed: %d\n", rc);
	}

exit:
	kmsetkey_disconnect();
	return rc < 0 ? rc : 0;
}
