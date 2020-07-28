#ifndef __SMARTPA_PARAM_H__
#define __SMARTPA_PARAM_H__

#include <stdint.h>
#include <AudioSmartPaParam.h>
#include "rt_spa_param.h"

#pragma pack(push, 1)

struct sec_header {
	char tag[8];
	uint16_t BHV;
	uint16_t MSHV;
	uint32_t TBS;
	char ICTagString[16];
	char BinDesc[96];
	char date[16];
	uint32_t EHO;
	uint32_t EHS;
	uint32_t EDS;
	uint32_t CRC;
	char AuthorInfo[80];
	uint8_t upub_key[144];
	uint8_t uinfo_sig[128];
	uint8_t hinfo_sig[128];
};

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */
void dynamic_link_arsi_assign_lib_fp(AurisysLibInterface *lib);
int mtk_smartpa_param_init(struct SmartPAParamOps *mSmartPAParam);
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* #ifndef __SMARTPA_PARAM_H__ */
