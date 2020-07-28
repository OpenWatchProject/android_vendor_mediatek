#ifndef __MAL_CFG_H__
#define __MAL_CFG_H__


#include <limits.h>
#include <stdbool.h>

// Compiler flags, NEED_TO_BE NOTICED, set by the compiler
// N/A

// Type definitions
typedef struct mal mal_t;
typedef struct mal* mal_ptr_t;
typedef struct mal_cfg_v1 mal_cfg_v1_t;
typedef struct mal_cfg_v1* mal_cfg_v1_ptr_t;
typedef enum mal_cfg_type
{
    mal_cfg_type_none,
    mal_cfg_type_ver,
    mal_cfg_type_id,
    mal_cfg_type_md_id,
    mal_cfg_type_sim_id,
    mal_cfg_type_max = UINT_MAX
}mal_cfg_type_t;

// Macros, NEED_TO_BE_NOTICED, the maximal size should be constrained
#define SYS_FAIL    (-1)
#define SYS_SUCC    (0)

#define MAL_SUCC    (SYS_SUCC)
#define MAL_FAIL    (SYS_FAIL)
#define MAL_VER    (1)

// API, NEED_TO_BE_NOTICED, certain APIs have not been fully-implemented for future extensions
// ==== NEED_TO_BE_NOTICED, temporary and going to remove ====
#define mal_init(mal_ptr, num_of_cfg, args ...)    (unlikely((mal_ptr) == NULL) ? NULL : ((mal_ptr)->is_once = false, mal_reset((mal_ptr)), mal_set((mal_ptr), (num_of_cfg), ## args)))
// #define mal_exit(mal_ptr)    (unlikely((mal_ptr) == NULL) ? NULL : ((mal_ptr)->is_once = true, mal_free_conn((mal_ptr), mdfx_connmngr_get_conn_by_id((mal_ptr)->conn_id)), mal_reset((mal_ptr))))
// ==== NEED_TO_BE_NOTICED, temporary and going to remove ====
//#define mal_init(mal_ptr, num_of_cfg, args ...)    (unlikely((mal_ptr) == NULL) ? NULL : (mal_reset((mal_ptr)), mal_set((mal_ptr), (num_of_cfg), ## args)))
//#define mal_exit(mal_ptr)    (unlikely((mal_ptr) == NULL) ? NULL : mal_reset((mal_ptr)))

#pragma GCC diagnostic ignored "-Waddress-of-temporary"
#define mal_once(num_of_cfg, args ...)    (mal_set(mal_reset(&((mal_t){.is_once = true})), (num_of_cfg), ## args))

// Implementation
// => MAL configurations
struct mal_cfg_v1
{
    int md_id;
    int sim_id;
};

// => Abstract Data Type
struct mal
{
    union
    {
        mal_cfg_v1_t v1;
    }cfg;

    // General variables
    // => Public
    int ver;
    // => Private
    bool is_once;
    int conn_id;
    bool is_conn;
    // Individual variables which must be private
    // N/A
};

#endif
