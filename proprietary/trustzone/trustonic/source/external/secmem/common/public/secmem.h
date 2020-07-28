#ifndef SECMEM_H
#define SECMEM_H

#include <sys/ioctl.h>

#define SECMEM_NAME     "secmem"
#define SECMEM_DEV      "/proc/secmem0"

#define MAX_NAME_SIZE   32

struct secmem_param {
    uint64_t alignment;      /* IN */
    uint64_t size;           /* IN */
    unsigned int refcount;   /* INOUT */
    uint64_t sec_handle;     /* OUT */
    uint32_t    id;
    uint8_t     *owner;
};

struct secmem_param_ioctl {
    uint64_t alignment;      /* IN */
    uint64_t size;           /* IN */
    unsigned int refcount;   /* INOUT */
    uint64_t sec_handle;     /* OUT */
    uint32_t    id;
    uint8_t     owner[MAX_NAME_SIZE];
    uint32_t    owner_len;
};

#define SECMEM_IOC_MAGIC      'T'
#define SECMEM_MEM_ALLOC      _IOWR(SECMEM_IOC_MAGIC, 1, struct secmem_param_ioctl)
#define SECMEM_MEM_REF        _IOWR(SECMEM_IOC_MAGIC, 2, struct secmem_param_ioctl)
#define SECMEM_MEM_UNREF      _IOWR(SECMEM_IOC_MAGIC, 3, struct secmem_param_ioctl)
#define SECMEM_MEM_ALLOC_TBL  _IOWR(SECMEM_IOC_MAGIC, 4, struct secmem_param_ioctl)
#define SECMEM_MEM_UNREF_TBL  _IOWR(SECMEM_IOC_MAGIC, 5, struct secmem_param_ioctl)
#define SECMEM_MEM_USAGE_DUMP _IOWR(SECMEM_IOC_MAGIC, 6, struct secmem_param_ioctl)
#define SECMEM_MEM_DUMP_INFO  _IOWR(SECMEM_IOC_MAGIC, 7, struct secmem_param_ioctl)

#define SECMEM_IOC_MAXNR      (10)

#endif /* end of SECMEM_H */
