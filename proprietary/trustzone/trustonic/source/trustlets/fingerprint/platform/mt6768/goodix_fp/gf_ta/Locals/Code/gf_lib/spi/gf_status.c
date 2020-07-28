/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include "gf_status.h"

#define GF_SINGLE_CHIP_CFG_LEN     120
#define GF_MULTI_CHIP_CFG_LEN      249

/**
 * @brief get chip status through config
 * @details single-chip or multi-chip;
 *          encrypt or not;
 *          floating or not;
 *          pixel cancel or not;
 *          coating or not;
 *          selectBmpThreshold value;
 *
 * @param cfg config start address
 * @param cfg_len config length
 * @param algoCfg status here
 * @return 0-success; 1-error config size;
 * @author Ted Lin-<linjinhui@goodix.com>
 * @date 2015-10-29
 */
gf_decode_ret_t gf_decodeAlgoCfgFromFwCfg(uint8_t *cfg, uint32_t cfg_len, gf_algo_cfg_t *algoCfg)
{
    uint8_t is_318 = 0;

    if((cfg_len != GF_SINGLE_CHIP_CFG_LEN) && (cfg_len != GF_MULTI_CHIP_CFG_LEN)){
        return GF_DECODE_ERROR_CFG_LEN;
    }

    switch(cfg_len){
        // single-chip
        case GF_SINGLE_CHIP_CFG_LEN:
            algoCfg->isFloating         = 0;
            algoCfg->isEnableEncrypt    = !(cfg[99] & 0x01);
            algoCfg->isPixelCancel      = !(cfg[79] & 0x10);
            algoCfg->isCoating          = !(cfg[90] >= 0x80);
            is_318                      = !(cfg[18] > 0x50);
            break;

        // multi-chip
        case GF_MULTI_CHIP_CFG_LEN:
            algoCfg->isFloating         = 1;
            algoCfg->isEnableEncrypt    = !(cfg[18] & 0x40);
            algoCfg->isPixelCancel      = !(cfg[145] & 0x10);
            algoCfg->isCoating          = !(cfg[164] > 0x40);
            is_318                      = !(cfg[192] > 0x31);
            break;
        default:
            break;
    }

    // 318M\518M\318
    if(is_318 && algoCfg->isCoating){
        algoCfg->selectBmpThreshold = 300;
    }
    // 316\316M
    if(!is_318 && algoCfg->isCoating){
        algoCfg->selectBmpThreshold = 400;
    }
    // 516\516M\816M
    if(!algoCfg->isCoating){
        algoCfg->selectBmpThreshold = 600;
    }

    return GF_DECODE_SUCESS;
}

