#include <stdint.h>
#include <stdio.h>
#include <trusty_std.h>
#include <err.h>
#include <tz_private/log.h>

#define LOG_TAG "matrix"

#define SAMPLE_SYS_FD 4

uint64_t test_matrix_64(void)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;
	uint32_t times = 0;
	uint64_t matrix[128][128] = {{0}};
	uint64_t result[128][128] = {{0}};

	for(i = 0;i < 128;i++){
		for(j = 0;j < 128;j++){
			matrix[i][j] = 1;
		}
	}

	for(times=0;times<100;times++){
		for(i=0;i<128;i++){
			for(j=0;j<128;j++){
				for(k=0;k<128;k++){
					result[i][j] =result[i][j]+matrix[i][k]*matrix[k][j];
				}
			}
		}

	}

	return result[0][0]+result[127][127];
}

uint32_t test_matrix_32(void)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;
	uint32_t times = 0;
	uint32_t matrix32[128][128] = {{0}};
	uint32_t result32[128][128] = {{0}};

	for (i = 0; i < 128; i++) {
		for (j = 0; j < 128; j++)
			matrix32[i][j] = 1;
	}

	for (times = 0; times < 100; times++) {
		for (i = 0; i < 128; i++) {
			for (j = 0; j < 128; j++) {
				for (k = 0; k < 128; k++)
					result32[i][j] = result32[i][j] + matrix32[i][k] * matrix32[k][j];
			}
		}
	}

	return result32[0][0] + result32[127][127];
}

#define matrix_size 128
uint64_t matrix64_to_kernel(void)
{
	int rc;
	uint32_t i = 0;
	uint32_t j = 0;
	uint64_t matrix[matrix_size][matrix_size];

	for(i = 0;i < matrix_size;i++){
		for(j = 0;j < matrix_size;j++){
			matrix[i][j] = 1;
		}
	}

	rc = ioctl(SAMPLE_SYS_FD, 1, matrix);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl failed. rc = %d\n", rc);
		return ERR_GENERIC;
	}

	return matrix[0][0] + matrix[matrix_size-1][matrix_size-1];

}

uint64_t matrix32_to_kernel(void)
{
	int rc;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t matrix[matrix_size][matrix_size];

	for(i = 0;i < matrix_size;i++){
		for(j = 0;j < matrix_size;j++){
			matrix[i][j] = 1;
		}
	}

	rc = ioctl(SAMPLE_SYS_FD, 2, matrix);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl failed. rc = %d\n", rc);
		return ERR_GENERIC;
	}

	return matrix[0][0] + matrix[matrix_size-1][matrix_size-1];

}


