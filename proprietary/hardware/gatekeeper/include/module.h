#ifndef _GATEKEEPER_MODULE_H_
#define _GATEKEEPER_MODULE_H_

#define GATEKEEPER_SOFTWARE_MODULE_NAME "Soft Gatekeeper HAL"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
   That's the only function used by module.c. Name of this function must
   not be mangled when compiled by C++ compiler.
   -------------------------------------------------------------------------*/
int soft_gatekeeper_open(const struct hw_module_t* module, const char* id,
            struct hw_device_t** device);

int soft_gatekeeper_close(hw_device_t *hw);

#ifdef __cplusplus
}
#endif

#endif /* _GATEKEEPER_MODULE_H_ */
