
#include "libnvram_sec.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//typedef struct
//{
//   unsigned char data[NVRAM_CIPHER_LEN];
//   unsigned int ret;sss

//} META_CONTEXT;

/******************************************************************************
 *  EXPORT FUNCTIONS
 ******************************************************************************/
int sec_nvram_enc (META_CONTEXT *meta_ctx) {
	return 0;
}
int sec_nvram_dec (META_CONTEXT *meta_ctx) {
	return 0;
}
