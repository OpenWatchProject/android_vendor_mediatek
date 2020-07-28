#ifndef MTK_AURISYS_CONFIG_PARSER_H
#define MTK_AURISYS_CONFIG_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct aurisys_config_t;
struct aurisys_config_dsp_t;



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

void trans_aurisys_config_dsp_to_hal(
    struct aurisys_config_dsp_t *dsp,
    struct aurisys_config_t **hal);


void delete_aurisys_config(struct aurisys_config_t *aurisys_config);




#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_CONFIG_PARSER_H */

