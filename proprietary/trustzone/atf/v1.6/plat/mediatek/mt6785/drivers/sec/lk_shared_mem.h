#ifndef _LK_SHARED_MEM_H_
#define _LK_SHARED_MEM_H_

#include <memory_layout.h>
/*
 *  shared memory layout
 *  |           |           |     |         |           |       |         |
 *  | i_key_pad |  message  | ... | padding | o_key_pad | hash1 | padding | ...
 *  |           |           |     |         |           |       |         |
 *    block_size   msg_size          128B     block_size  sha256   128B
 *
 *  |       |         |    |           |           |
 *  | hash2 | padding | iv | rpmb key  | operation |
 *  |       |         |    |           |           |
 *   sha256   128B     iv   block_size   mass data
 */

#define LK_SHARED_MEM_ADDR   (ATF_HMAC_BASE)   /* fixed and pre-allocated in lk */
#define LK_SHARED_MEM_SIZE   (0x1000)          /* 4KB*/
#define SHA256_SIZE          (32)              /* size in bytes */
#define HMAC_BLOCK_SIZE      (64)              /* fixed 64 bytes in hamc sha256 case */
#define I_KEY_PAD_SIZE       (HMAC_BLOCK_SIZE) /* size in bytes */
#define O_KEY_PAD_SIZE       (HMAC_BLOCK_SIZE) /* size in bytes */
#define MSG_MAX_SIZE         (3072 - I_KEY_PAD_SIZE - SHA256_PADDING_MAX)
#define SHA256_PADDING_MAX   (128)

#define I_KEY_PAD_OFFSET     (0)
#define MSG_OFFSET           (I_KEY_PAD_SIZE)
#define O_KEY_PAD_OFFSET     (MSG_OFFSET + MSG_MAX_SIZE + SHA256_PADDING_MAX)
#define HASH1_OFFSET         (O_KEY_PAD_OFFSET + O_KEY_PAD_SIZE)
#define HASH2_OFFSET         (HASH1_OFFSET + SHA256_SIZE + SHA256_PADDING_MAX)
#define IV_OFFSET            (HASH2_OFFSET + SHA256_SIZE + SHA256_PADDING_MAX)
#define SHA256_OP_OFFSET     (IV_OFFSET + SHA256_SIZE)

#endif  /* _LK_SHARED_MEM_H_*/
