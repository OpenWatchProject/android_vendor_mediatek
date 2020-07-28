#ifndef _RNG_H_
#define _RNG_H_

uint32_t plat_get_true_rnd(uint32_t *val);
uint32_t plat_get_rnd(uint32_t *val);
uint32_t plat_get_rnd_4(uint32_t x1, uint32_t *val0, uint32_t *val1,
		uint32_t *val2, uint32_t *val3);

#endif /* _RNG_H_ */

