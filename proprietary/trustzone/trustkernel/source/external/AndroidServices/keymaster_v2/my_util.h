#ifndef MY_UTIL_H_
#define MY_UTIL_H_

static inline keymaster_key_param_t* keymaster_get_tag_unique(const keymaster_key_param_set_t *set, keymaster_tag_t tag) {
	keymaster_key_param_t *res = NULL, *now = set->params, *end = set->params + set->length;
	while (now < end) {
		if (now->tag == tag) {
			if (res == NULL)
				res = now;
			else
				return NULL;
		}
		now++;
	}
	
	return res;
}

#endif
