/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_FDT_H__
#define __GF_FDT_H__

// number of area row & number of area col
#define AREA_ROW_NUM       (8)
#define AREA_COL_NUM       (8)

// chip specific parameter for E
#define IMAGE_ROW_E       (64)        // number of chip row pixel
#define IMAGE_COL_E       (176)       // number of chip column pixel

// chip specific parameter for F
#define IMAGE_ROW_F       (88)        // number of chip row pixel
#define IMAGE_COL_F       (108)       // number of chip column pixel

// chip specific parameter for G
#define IMAGE_ROW_G       (54)        // number of chip row pixel
#define IMAGE_COL_G       (176)       // number of chip column pixel

// chip specific parameter for L
#define IMAGE_ROW_L       (112)        // number of chip row pixel
#define IMAGE_COL_L       (132)       // number of chip column pixel

typedef enum {
    GF_FDT_IMAGE, // FDT down confirm by Image rawdata
    GF_FDT_NAV // FDT down confirm by Nav rawdata
} gf_fdt_type_t;

typedef enum {
    GF_FDT_RET_TEMP = 0, // FDT INT is caused by temperature rise
    GF_FDT_RET_FINGER = 1, // FDT INT is caused by finger touch
    GF_FDT_RET_VOID = 2, // FDT INT is caused by touch & release too quickly to capture a void image
    GF_FDT_RET_BAD = 3,//image base is bad, we should update it
    GF_FDT_RET_NONE = 4,
} gf_fdt_ret_t;

typedef struct {
    uint8_t fdt_area_num;
    uint8_t fdt_threshold;
    uint8_t nav_row_start;
    uint8_t nav_row_step;
    uint8_t nav_row_num;
    uint16_t fdt_tx_num;
    uint16_t nav_tx_num;
    uint16_t image_tx_num;
    uint32_t image_row;
    uint32_t image_col;
} gf_fdt_params_t;

/**
 * \brief Check the finger press event's validity. FDT down INT by finger touch or temperature increase
 *
 * \param fdt_type[in] fdt type. #gf_fdt_type_t
 *
 * \param raw_data[in] image/nav rawdata after FDT down
 *
 *\param fdt_event_type[out] finger press type. #gf_fdt_ret_t
 *
 * \return If success, return #GF_SUCCESS,otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_fdt_get_event_type(gf_fdt_type_t fdt_type, uint16_t* raw_data, gf_fdt_ret_t* fdt_ret);

/**
 * \brief Init fdt event check parameter
 *
 * \return If success, return #GF_SUCCESS,otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *
 */
gf_error_t gf_fdt_init(gf_chip_type_t chip_type, gf_fdt_params_t* init_params);

#endif //__GF_FDT_H__
