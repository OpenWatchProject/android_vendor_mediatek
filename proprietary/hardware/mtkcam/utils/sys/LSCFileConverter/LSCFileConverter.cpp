#include <stdio.h>
#include <string>

#define SHADING_TABLE_SIZE (24620)

struct ILscBuf
{
	int i4ImgWd;
	int i4ImgHt;
	int i4BlkX;
	int i4BlkY;
	int i4BlkW;
	int i4BlkH;
	int i4BlkLastW;
	int i4BlkLastH;
};

struct ConfigBlk
{
	ConfigBlk()
		: i4BlkX(0), i4BlkY(0), i4BlkW(0), i4BlkH(0), i4BlkLastW(0), i4BlkLastH(0)
	{}

	ConfigBlk(int i4ImgWd, int i4ImgHt, int i4GridX, int i4GridY)
	{
		i4BlkX = i4GridX - 2;
		i4BlkY = i4GridY - 2;
		i4BlkW = (i4ImgWd) / (2 * (i4BlkX + 1));
		i4BlkH = (i4ImgHt) / (2 * (i4BlkY + 1));
		i4BlkLastW = i4ImgWd / 2 - (i4BlkX*i4BlkW);
		i4BlkLastH = i4ImgHt / 2 - (i4BlkY*i4BlkH);
	}

	ConfigBlk(int _i4BlkX, int _i4BlkY, int _i4BlkW, int _i4BlkH, int _i4BlkLastW, int _i4BlkLastH)
		: i4BlkX(_i4BlkX), i4BlkY(_i4BlkY), i4BlkW(_i4BlkW), i4BlkH(_i4BlkH), i4BlkLastW(_i4BlkLastW), i4BlkLastH(_i4BlkLastH)
	{}

	int i4BlkX;
	int i4BlkY;
	int i4BlkW;
	int i4BlkH;
	int i4BlkLastW;
	int i4BlkLastH;
};

struct ILscTableConfig
{
	ILscTableConfig()
		: i4ImgWd(0), i4ImgHt(0), i4GridX(0), i4GridY(0), rCfgBlk()
	{}

	int i4ImgWd;
	int i4ImgHt;
	int i4GridX;
	int i4GridY;
	ConfigBlk rCfgBlk;
};

using namespace std;

int main(int argc, char **argv)
{
	FILE *myFptr=NULL;
	unsigned char _readBuf[SHADING_TABLE_SIZE];
	const char *outputFilename;

	if (argc < 2) 
	{
		printf("command <source> [output]\n");
		printf("Must: <source> source file name ex: xxx.bin\n");
		printf("Option: [output] Ouput file name\n");
		return 0;
	}
	if (argc >= 3) {
		outputFilename = argv[2];
	}
	

    string inputFilenameS =argv[1];
		
	printf("input name %s\n", inputFilenameS.c_str());

	//printf("111  %d\n", strncmp(inputFilename, ".", 1));


	if(inputFilenameS.find(".LSC")!=string::npos || inputFilenameS.find(".lsc") != string::npos)
	{
		char *inputStr=argv[1];
		myFptr = fopen(argv[1], "rb");

		inputStr[ strlen(inputStr) - 4] = '\0';
		outputFilename = inputStr;
	}
	else
	{
		printf("invalid file name\n");
		return 0;
	}
	

	if (myFptr != NULL)
	{
		//read data
		fread(_readBuf, SHADING_TABLE_SIZE,1, myFptr);

		FILE *fhwtbl, *fsdblk;
		
		char strFilename1[512];
		char strFilename2[512];

		sprintf(strFilename1, "%s.sdblk", outputFilename);
		fsdblk = fopen(strFilename1, "wb");
		if (fsdblk == NULL)
		{
			printf("Can't open: %s", strFilename1);
			return -1;
		}

		sprintf(strFilename2, "%s.hwtbl", outputFilename);
		fhwtbl = fopen(strFilename2, "wb");
		if (fhwtbl == NULL)
		{
			printf("Can't open: %s", strFilename2);
			return -1;
		}

		//load config
		ILscTableConfig _loadConfig;
		memcpy(&_loadConfig, &_readBuf[0], sizeof(ILscTableConfig));

		//convert config
		ILscBuf _tableConfig;
		_tableConfig.i4BlkX = _loadConfig.i4GridX - 2;
		_tableConfig.i4BlkY = _loadConfig.i4GridY - 2;
		_tableConfig.i4BlkW = _loadConfig.i4ImgWd/(2*(_tableConfig.i4BlkX +1));
		_tableConfig.i4BlkH = _loadConfig.i4ImgHt/(2*(_tableConfig.i4BlkY +1));
		_tableConfig.i4BlkLastW = _loadConfig.i4ImgWd/2 - (_tableConfig.i4BlkX*_tableConfig.i4BlkW);
		_tableConfig.i4BlkLastH = _loadConfig.i4ImgHt/2 - (_tableConfig.i4BlkY*_tableConfig.i4BlkH);


		unsigned int* pData = (unsigned int *)&_readBuf[sizeof(ILscTableConfig) + 4];//RsvdData is 4 bytes

		fprintf(fsdblk, " %8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
			0 /*LscConfig.ctl1.bits.SDBLK_XOFST*/,
			0 /*LscConfig.ctl1.bits.SDBLK_YOFST*/,
			_tableConfig.i4BlkW /*LscConfig.ctl2.bits.LSC_SDBLK_WIDTH*/,
			_tableConfig.i4BlkH /*LscConfig.ctl3.bits.LSC_SDBLK_HEIGHT*/,
			_tableConfig.i4BlkX /*LscConfig.ctl2.bits.LSC_SDBLK_XNUM*/,
			_tableConfig.i4BlkY /*LscConfig.ctl3.bits.LSC_SDBLK_YNUM*/,
			_tableConfig.i4BlkLastW /*LscConfig.lblock.bits.LSC_SDBLK_lWIDTH*/,
			_tableConfig.i4BlkLastH /*LscConfig.lblock.bits.LSC_SDBLK_lHEIGHT*/);

		int x_num = _tableConfig.i4BlkX + 1;
		int y_num = _tableConfig.i4BlkY + 1;
		int numCoef = x_num * y_num * 4 * 6;

		int i;
		for (i = numCoef - 1; i >= 0; i -= 6)
		{
			unsigned int coef1, coef2;
			unsigned int val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			val = *pData++;
			coef2 = (val & 0xFFFF0000) >> 16;
			coef1 = (val & 0x0000FFFF);
			fprintf(fsdblk, " %8d %8d", coef1, coef2);
			fprintf(fhwtbl, "0x%08x, ", val);

			fprintf(fhwtbl, "\n");
			fprintf(fsdblk, "\n");
		}

		fclose(fhwtbl);
		fclose(fsdblk);

		printf("Output files: %s and %s\n", strFilename1, strFilename2);
	}
	else 
	{
		printf("can't open file");
	}

    return 0;
}

