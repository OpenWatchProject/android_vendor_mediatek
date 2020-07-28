/* An example test TA implementation.
 */

#ifndef __TRUSTZONE_TA_TEST__
#define __TRUSTZONE_TA_TEST__

#define TZ_TA_TEST_UUID   "0d5fe516-821d-11e2-bdb4-d485645c4310"

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for Test TA */
#define TZCMD_TEST_ADD           0
#define TZCMD_TEST_MUL           1
#define TZCMD_TEST_ADD_MEM       2
#define TZCMD_TEST_DO_A          3
#define TZCMD_TEST_DO_B          4
#define TZCMD_TEST_SLEEP         5
#define TZCMD_ECHO_MEM_TEST 	6
//#define TZCMD_TEST_DELAY         6
#define TZCMD_TEST_DO_C          7
#define TZCMD_TEST_DO_D          8
#define TZCMD_TEST_SECUREFUNC    9
#define TZCMD_TEST_CP_SBUF2NBUF 10
#define TZCMD_TEST_CP_NBUF2SBUF 11
#define TZCMD_TEST_THREAD       12
#define TZCMD_TEST_TOMCRYPT     13
#define TZCMD_TEST_SPINLOCK     14
#define TZCMD_TEST_NOP          15
#define TZCMD_TEST_RPMB         16
#define TZCMD_TEST_CRYPTO_RSA   17
#define TZCMD_TEST_RNG          18
#define TZCMD_TEST_RTC          19
#define TZCMD_TEST_VERSION      20
#define TZCMD_TEST_CELLINFO     21
#define TZCMD_TEST_INTERNAL_IPC 22
#define TZCMD_TEST_VFP          29
#define TZCMD_MTEE_VERSION  	33
#define TZCMD_TEST_EXIT  	36


#define TZCMD_TEST_SYSCALL 0x1234
#define TZCMD_GET_MUTEX 0x2345
#define TZCMD_SEND_MUTEX 0x2346
#define TZCMD_TEST_MUTEX 0x2347

#define TZCMD_TEST_MULTI_THREAD 0x3000
#define TZCMD_TEST_PTHREAD      0x3001

#define TZCMD_TEST_CHM_IMAGE_RECOGNITION 0x4000
#define TZCMD_TEST_SHM_IMAGE_RECOGNITION 0x4001

#define TZCMD_SHARED_MEM_TEST 0x5588

#define TZCMD_MEM_MUL_TEST 0x6000
#define TZCMD_DMA_TEST 0x6100
#define TZCMD_MEM_STRESS_TEST 0x6001
#define TZCMD_INT_IPC_TEST 0x7000
#define TZCMD_ABORT_TEST 0x9876
#define TZCMD_TEST_CHUNKMEM_TEST 0x9997
#define TZCMD_TEST_ALLUT 0x9999

#define TZCMD_TEST_SCM_WRITE 0x8000
#define TZCMD_TEST_SCM_READ 0x8001
#define TZCMD_TEST_SHM_IPC 0x8002
#define TZCMD_TEST_CHMEM_QUERY_PA 0x8003

#define TZCMD_CONNECT_CA_to_HA 0x9000
#define TZCMD_TEST_SCAMERA_CA_to_HA 0x9001
#define TZCMD_to_HA_mtee_kernel_service 0x9002
#define TZCMD_to_HA_scamera_sample 0x9003
#define TZCMD_TEST_VREG_FROM_HA 0x9004


#endif /* __TRUSTZONE_TA_TEST__ */
