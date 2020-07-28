#ifndef _ADIFHEADER_H_
#define _ADIFHEADER_H_

#define ENABLE_TEST_ADIF_HEADER 0

#define ADIF_HEADER_TEMPLATE_LEN (18)
int createAdifHeader(unsigned int bitrate, unsigned int samplerate, unsigned char *adif_header_array);
int    addAdifHeader(unsigned int bitrate, unsigned int samplerate, unsigned char *adif_header_array, void *pBsBuf, int BsBufLen, void *pOutBuf, int *OutBufLen);

#endif //#ifndef _ADIFHEADER_H_
