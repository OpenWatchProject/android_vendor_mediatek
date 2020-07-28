#ifndef MTK_AURISYS_LIB_HANDLER_H
#define MTK_AURISYS_LIB_HANDLER_H

#include <stdbool.h>

#include <uthash.h> /* uthash */

#include <audio_lock.h>

#include <aurisys_scenario.h>

#include <arsi_type.h>

#include <audio_pool_buf_handler.h> /* TODO: hide */



#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct AurisysLibInterface;
struct aurisys_config_t;
struct aurisys_adb_command_t;
struct PcmDump_t;
struct aurisys_lib_manager_config_t;
struct aurisys_library_name_t;
struct aurisys_library_config_t;


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */


/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */

typedef struct aurisys_lib_handler_t {
    char *lib_name; /* key for manager */
    void *self; /* key for adb cmd */

    struct alock_t *lock;

    arsi_lib_config_t lib_config;    /* private lib_config */

    const arsi_task_config_t *task_config;  /* only pointer to manager's task cfg */
    struct AurisysLibInterface *api;        /* only pointer to library_config's api */
    char *param_path;                       /* only pointer to library_config's param path */
    char *lib_dump_path;                    /* only pointer to library_config's lib dump path */
    const char *custom_info;                /* custom info*/

    data_buf_t param_buf;

    data_buf_t working_buf;
    void *arsi_handler;

    audio_pool_buf_formatter_t ul_pool_formatter;
    audio_pool_buf_t           ul_pool_in;
    audio_pool_buf_t           ul_pool_out;
    uint32_t                   ul_process_size_per_time;
    audio_ringbuf_t            ul_smoother;
    uint32_t                   ul_left_raw_in_size;

    audio_pool_buf_formatter_t dl_pool_formatter;
    audio_pool_buf_t           dl_pool_in;
    audio_pool_buf_t           dl_pool_out;
    uint32_t                   dl_process_size_per_time;
    audio_ringbuf_t            dl_smoother;
    uint32_t                   dl_left_raw_in_size;

    audio_pool_buf_t          *aec_pool_in;
    audio_buf_t               *p_ul_ref_buf_aec; /* only pointer to aec in lib_config->p_ul_ref_bufs */

    char *linear_buf_ul;
    char *linear_buf_ul_aec;
    char *linear_buf_dl;

    struct PcmDump_t *pcm_dump_ul_in;
    struct PcmDump_t *pcm_dump_ul_out;
    struct PcmDump_t *pcm_dump_aec;

    struct PcmDump_t *pcm_dump_dl_in;
    struct PcmDump_t *pcm_dump_dl_out;

    struct PcmDump_t *lib_dump;
    data_buf_t        lib_dump_buf;

    bool pad_flag;


    bool     *enable_log;       /* only pointer to component's enhancement_mode */
    bool     *enable_raw_dump;  /* only pointer to component's enhancement_mode */
    bool     *enable_lib_dump;  /* only pointer to component's enhancement_mode */
    uint32_t *enhancement_mode; /* only pointer to component's enhancement_mode */

    bool raw_dump_enabled; /* fopen or not */
    bool lib_dump_enabled; /* fopen or not */

    debug_log_fp_t debug_log_fp;

    uint32_t dsp_task_scene; /* hifi3 task scene */

    struct aurisys_lib_handler_t **head; /* point to the parent component's lib_handler_list_for_adb_cmd */
    UT_hash_handle hh_component; /* makes this structure hashable for adb cmd */

    UT_hash_handle hh_all;       /* makes this structure hashable for manager */
    UT_hash_handle hh_ul_list;   /* makes this structure hashable for manager */
    UT_hash_handle hh_dl_list;   /* makes this structure hashable for manager */
} aurisys_lib_handler_t;


/*
 * =============================================================================
 *                     hook function
 * =============================================================================
 */


/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

void aurisys_lib_handler_c_file_init(void);
void aurisys_lib_handler_c_file_deinit(void);


void aurisys_create_lib_handler_list_xlink(
    struct aurisys_library_name_t *name_list,
    struct aurisys_library_config_t *library_config_list,
    struct aurisys_lib_handler_t **lib_handler_list,
    uint32_t *num_library_hanlder,
    const struct aurisys_lib_manager_config_t *manager_config);

void aurisys_destroy_lib_handler_list(aurisys_lib_handler_t **handler_list);


int aurisys_arsi_parsing_param_file(aurisys_lib_handler_t *lib_handler);
void aurisys_arsi_create_handler(aurisys_lib_handler_t *lib_handler);
void aurisys_arsi_destroy_handler(aurisys_lib_handler_t *lib_handler);

uint32_t aurisys_arsi_process_ul_only(aurisys_lib_handler_t *lib_handler);
uint32_t aurisys_arsi_process_dl_only(aurisys_lib_handler_t *lib_handler);
uint32_t aurisys_arsi_process_ul_and_dl(aurisys_lib_handler_t *lib_handler);

int aurisys_arsi_set_ul_digital_gain(
    aurisys_lib_handler_t *lib_handler,
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain);

int aurisys_arsi_set_dl_digital_gain(
    aurisys_lib_handler_t *lib_handler,
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain);

int aurisys_arsi_set_ul_enhance(
    aurisys_lib_handler_t *lib_handler,
    const uint8_t b_enhance_on);

int aurisys_arsi_set_dl_enhance(
    aurisys_lib_handler_t *lib_handler,
    const uint8_t b_enhance_on);

int aurisys_arsi_set_ul_mute(
    aurisys_lib_handler_t *lib_handler,
    const uint8_t b_mute_on);

int aurisys_arsi_set_dl_mute(
    aurisys_lib_handler_t *lib_handler,
    const uint8_t b_mute_on);

int aurisys_arsi_run_adb_cmd(
    aurisys_lib_handler_t *lib_handler,
    struct aurisys_adb_command_t *adb_cmd);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_LIB_HANDLER_H */

