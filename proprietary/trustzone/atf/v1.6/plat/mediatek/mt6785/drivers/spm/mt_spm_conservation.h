#ifndef __MT_SPM_CONSERVATION_H__
#define __MT_SPM_CONSERVATION_H__

#include <mt_spm_internal.h>

int spm_conservation(int stateId, unsigned int ext_opand
	, struct spm_lp_scen *spm_lp, int resource_req);
void spm_conservation_finish(int stateId, unsigned int ext_opand
	, struct spm_lp_scen *spm_lp);

#endif /* __MT_SPM_CONSERVATION_H__ */
