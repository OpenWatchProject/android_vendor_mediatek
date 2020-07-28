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
struct aurisys_adb_command_t;



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

int init_aurisys_controller(void *cfg_buf, uint32_t cfg_size);
int deinit_aurisys_controller(void);


struct aurisys_lib_manager_t *create_aurisys_lib_manager(
    const struct aurisys_lib_manager_config_t *manager_config);

int destroy_aurisys_lib_manager(struct aurisys_lib_manager_t *manager);


bool aurisys_set_parameter(struct aurisys_adb_command_t *adb_cmd);
bool aurisys_get_parameter(
    struct aurisys_adb_command_t *adb_cmd,
    void *wb_addr,
    uint32_t wb_mem_size,
    uint32_t *wb_data_size);



void set_aurisys_on(const bool enable);
bool get_aurisys_on();



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_CONTROLLER_H */

