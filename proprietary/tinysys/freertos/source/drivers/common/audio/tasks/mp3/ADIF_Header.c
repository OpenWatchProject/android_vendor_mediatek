#include "ADIF_Header.h"

#if ENABLE_TEST_ADIF_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#endif //#if ENABLE_TEST_ADIF_HEADER

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
//#include <stdarg.h>

const unsigned char AdifHeaderTemplate[ADIF_HEADER_TEMPLATE_LEN] = {
	0x41, 0x44, 0x49, 0x46, 0x00, 0x00, 0x00, 0x01,
	0xFF, 0xFF, 0xE0, 0x80, 0x80, 0x00, 0x04, 0x00,
	0x01, 0x00
};


// return: 0 => error
//         1 => OK
// input:
//         bitrate: range between 0~0x007FFFFF
//         samplerate: number in Hz, ex: 48000, 44100, 32000...
//         adif_header_array: output ADIF header, the size should be 18byte(ADIF_HEADER_TEMPLATE_LEN)
int createAdifHeader(unsigned int bitrate, unsigned int samplerate, unsigned char *adif_header_array)
{
	int i;
	int ret = 1;
	int samplerateindex = 3;

	if(bitrate > 0x007FFFFF)
	{
		return 0;
	}

	switch(samplerate)
	{
	case 96000:
		samplerateindex = 0;
		break;
	case 88200:
		samplerateindex = 1;
		break;
	case 64000:
		samplerateindex = 2;
		break;
	case 48000:
		samplerateindex = 3;
		break;
	case 44100:
		samplerateindex = 4;
		break;
	case 32000:
		samplerateindex = 5;
		break;
	case 24000:
		samplerateindex = 6;
		break;
	case 22050:
		samplerateindex = 7;
		break;
	case 16000:
		samplerateindex = 8;
		break;
	case 12000:
		samplerateindex = 9;
		break;
	case 11025:
		samplerateindex = 10;
		break;
	case 8000:
		samplerateindex = 11;
		break;
	case 7350:
		samplerateindex = 12;
		break;
	default :
		ret = 0;
		break;
	}

	if(ret == 0)
	{
		return 0;
	}

	for(i=0; i<ADIF_HEADER_TEMPLATE_LEN; i++)
	{
		adif_header_array[i] = AdifHeaderTemplate[i];
	}

	adif_header_array[11] |= (samplerateindex << 3);
	adif_header_array[4] |= ( (unsigned char)(bitrate >> 19) );
	adif_header_array[5] |= ( (unsigned char)(bitrate >> 11) );
	adif_header_array[6] |= ( (unsigned char)(bitrate >> 3) );
	adif_header_array[7] |= ( (unsigned char)(bitrate << 5) );

	return 1;
}


int addAdifHeader(unsigned int bitrate, unsigned int samplerate, unsigned char *adif_header_array, void *pBsBuf, int BsBufLen, void *pOutBuf, int *OutBufLen)
{
	int i;
	int ret;
	unsigned char *dest_ptr = (unsigned char *)pOutBuf;
	ret = createAdifHeader(bitrate, samplerate, adif_header_array);
	if(ret == 0)
	{
		return 0;
	}

	for(i=0; i<ADIF_HEADER_TEMPLATE_LEN; i++)
	{
		dest_ptr[i] = adif_header_array[i];
	}
	dest_ptr = dest_ptr + ADIF_HEADER_TEMPLATE_LEN;
	memcpy((void *)dest_ptr, pBsBuf, BsBufLen);
	*OutBufLen = BsBufLen + ADIF_HEADER_TEMPLATE_LEN;

	return(1);
}


#if ENABLE_TEST_ADIF_HEADER
#define TEST_LENGTH 100
int main(int argc, char **argv)
{
	int i, ret;
	unsigned char adif_header_array[ADIF_HEADER_TEMPLATE_LEN];
	unsigned int bitrate, samplerate;
	unsigned char  pBsBuf[TEST_LENGTH];
	unsigned char pOutBuf[TEST_LENGTH+ADIF_HEADER_TEMPLATE_LEN];
	int OutBufLen;

	if(argc != 3)
	{
		printf("Uasge:%s bitrate samplerate");
	}

	bitrate = atoi(argv[1]);
	samplerate = atoi(argv[2]);

	for(i=0; i<TEST_LENGTH; i++)
	{
		pBsBuf[i] = i;
	}

	//createAdifHeader(bitrate, samplerate, adif_header_array);
	ret = addAdifHeader(bitrate, samplerate, adif_header_array, pBsBuf, TEST_LENGTH, pOutBuf, &OutBufLen);
	printf("ret = %d\n", ret);
	printf("OutBufLen = %d\n", OutBufLen);

	for(i=0; i<OutBufLen; i++)
	{
		if( (i%16) == 0 )
		{
			printf("\n");
		}
		printf("%02X ", pOutBuf[i]);
	}
}
#endif //#if ENABLE_TEST_ADIF_HEADER

