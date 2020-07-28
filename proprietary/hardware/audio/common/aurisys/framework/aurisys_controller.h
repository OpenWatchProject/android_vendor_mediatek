#ifndef MTK_AURISYS_CONTROLLER_H
#define MTK_AURISYS_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>



#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct aurisys_lib_manager_t;
struct aurisys_lib_manager_config_t;
struct arsi_task_config_t;



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

int init_aurisys_controller(void);
int deinit_aurisys_controller(void);

int wrap_config_to_dsp(void);


struct aurisys_lib_manager_t *create_aurisys_lib_manager(
    const struct aurisys_lib_manager_config_t *manager_config);

int destroy_aurisys_lib_manager(struct aurisys_lib_manager_t *manager);


int aurisys_set_parameter(const char *key_value_pair);
char *aurisys_get_parameter(const char *key);
int get_aurisys_set_raram_result(void);



void set_aurisys_on(const bool enable);
bool get_aurisys_on();



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_CONTROLLER_H */

