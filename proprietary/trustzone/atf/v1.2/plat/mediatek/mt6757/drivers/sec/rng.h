#ifndef _RNG_H_
#define _RNG_H_

#define TRNG_MAGIC	0x74726e67

uint32_t plat_get_true_rnd(uint32_t *val);
uint32_t plat_get_rnd(uint32_t *val);

#endif /* _RNG_H_ */

