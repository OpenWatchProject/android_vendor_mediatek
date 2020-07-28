#ifndef __MT_SPM_SYSPLL_H__
#define __MT_SPM_SYSPLL_H__

void spm_syspll_args(uint64_t arg1, uint64_t arg2, uint64_t arg3);
void spm_syspll(int StateId);
void spm_syspll_finish(int StateId);

#endif /* __MT_SPM_SYSPLL_H__ */
