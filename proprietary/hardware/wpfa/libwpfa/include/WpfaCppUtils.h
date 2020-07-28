#ifndef WPFACPPUTILS_H
#define WPFACPPUTILS_H



#ifdef __cplusplus
extern "C" {
#endif

#define CPP_UTIL_TAG "WpfaCppUtils"

void initialRuleContainer();
int addFilter(int fid, WPFA_filter_reg_t filter);
int removeFilter(int fid);
WPFA_filter_reg_t* getFilterFromMap(int fid);
int getFilterMapSize();
int findFilterById(int fid);
void dumpAllFilterId();
void deinitialRuleContainer();
void initialA2MRingBuffer();
void initialA2MRingBuffer();
void readRingBuffer();
void writeRingBuffer(unsigned char *data ,int len);
void wpfaDriverInit();
void wpfaDriverCbRegister();
int a2mWpfaInitNotify();
int a2mWpfaVersionNotify();


#ifdef __cplusplus
}
#endif



#endif /* end of WPFACPPUTILS_H */

