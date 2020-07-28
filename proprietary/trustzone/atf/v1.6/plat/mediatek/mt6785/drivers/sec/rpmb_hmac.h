#ifndef _RPMB_DRV_H_
#define _RPMB_DRV_H_

typedef unsigned char u8;

int32_t rpmb_init(void);
int32_t rpmb_uninit(void);
int32_t rpmb_hmac(uint32_t offset, uint32_t size);

#define MTK_RPMB_IN_LK                  (0)
#define MTK_RPMB_LEAVING_LK             (MTK_RPMB_IN_LK+1)

#endif  /* _RPMB_DRV_H_*/
