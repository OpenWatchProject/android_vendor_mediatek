#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "libnvram.h"
#include "libnvram_log.h"
#include "MetaPub.h"
#include "Meta_APEditor_Para.h"
#include "errno.h"
#if 1
#include <vendor/mediatek/hardware/nvram/1.1/INvram.h>
#include "cutils/log.h"
#include <vector>
#include <stdint.h>
#include <android-base/logging.h>

using std::string;


static void covertArray2Vector(const char* in, int len, std::vector<uint8_t>& out) {
    out.clear();
    for(int i = 0; i < len; i++) {
       out.push_back(in[i]);
    }
}
static void covertVector2Array(std::vector<uint8_t> in, char* out) {
	int size = in.size();
	for(int i = 0; i < size; i++) {
		out[i] = in.at(i);
	}
}
#endif


//-----------------------------------------------------------------------------
bool META_Editor_Init(void)
{
	return true;
}
//-----------------------------------------------------------------------------
bool META_Editor_Deinit(void)
{
	return true;
}

//readFileById(int32_t lid, uint16_t para)			generates (string data, int32_t readsize, int8_t retval);

//-----------------------------------------------------------------------------
bool META_Editor_ReadFile_OP(FT_AP_Editor_read_req *pReq)
{
#if 1
	FT_AP_Editor_read_cnf kCnf;
	F_INFO kFileInfo;
	int iNvmRecSize = 0, iReadSize;
	F_ID iFD;
	char* pBuffer = NULL;
	char* ptempbuf = NULL;
	bool IsRead =true;
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();

    if (client == NULL) {
	 LOG(INFO) << "client is NULL";
    }

	char *nvrambuff=NULL;
	std::vector<uint8_t> nvramvec;
	int ret=0;
    int Readsize = 0;
	std::string readnvram;
	int i =0;
	memset(&kCnf, 0, sizeof(FT_AP_Editor_read_cnf));
	kCnf.header.id 		= pReq->header.id + 1;
	kCnf.header.token 	= pReq->header.token;
	kCnf.file_idx 		= pReq->file_idx;
	kCnf.para 			= pReq->para;

	auto callback = [&] (const std::string& data, int32_t readsize, int8_t retval) {
		 readnvram=data;
		 ret=(int)retval;
		 Readsize=(int)readsize;
		//NVRAM_LOG("readnvram %s readsize %d retval%d\n", readnvram, readsize, retval);
		LOG(INFO) << "readnvram@1:" << readnvram;
		
	};
	client->readFileBylid(pReq->file_idx,pReq->para,callback);
	NVRAM_LOG("ret=%d, Readsize=%d\n", ret, Readsize);
#if 0	
	iFD = NVM_GetFileDesc(pReq->file_idx,&(kFileInfo.i4RecSize),&(kFileInfo.i4RecNum),IsRead);
	if (iFD.iFileDesc == -1) 
    {
		NVRAM_LOG("Error AP_Editor_ReadFile can't open file: file index-%d, %d\n", pReq->file_idx, iNvmRecSize);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		return false;		
	}
	iNvmRecSize = kFileInfo.i4RecSize;
    
	if (pReq->para > kFileInfo.i4RecNum) 
    {
		NVRAM_LOG("Error AP_Editor_ReadFile para: %d, %d\n", pReq->file_idx, pReq->para);
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		return false;
	}

    /* Open NVRAM realted files */
	pBuffer = (char*)malloc(iNvmRecSize);
    if (pBuffer == NULL) {
        NVRAM_LOG("Error AP_Editor_ReadFile:memory malloc error\r\n");
        return false;
    }
	lseek(iFD.iFileDesc, (pReq->para - 1) * iNvmRecSize, SEEK_CUR);
	iReadSize=read(iFD.iFileDesc, pBuffer, iNvmRecSize);
	if(iNvmRecSize != iReadSize){
		NVRAM_LOG("Error AP_Editor_ReadFile :Read size not match:iReadSize(%d),iNvmRecSize(%d),error:%s\n",iReadSize,iNvmRecSize,strerror(errno));
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		free(pBuffer);
		return false;
	}
		
	NVM_CloseFileDesc(iFD);
	
#endif
	kCnf.read_status = META_STATUS_SUCCESS;
	kCnf.status = META_SUCCESS;
	pBuffer = (char*)malloc(Readsize);
	ptempbuf = (char*)malloc(2*Readsize+1);
	//char *cstr = new char[str.length() + 1];	
	strncpy(ptempbuf, readnvram.c_str(), 2*Readsize);
	ptempbuf[2*Readsize] = '\0';

	NVRAM_LOG("ptempbuf[0]%x, ptempbuf[1]%x, ptempbuf[2]%x, ptempbuf[3]%x, ptempbuf[4]%x, ptempbuf[5]%x, ptempbuf[6]%x, ptempbuf[7]%x, ptempbuf[8]%x \n",
		ptempbuf[0],ptempbuf[1],ptempbuf[2],ptempbuf[3],ptempbuf[4],ptempbuf[5],ptempbuf[6],ptempbuf[7],ptempbuf[8]);
	
	for(i=0;i<2*Readsize; i+=2)
		{
		sscanf(ptempbuf+i, "%02X", &pBuffer[i/2]);
		}
	NVRAM_LOG("pBuffer[0]%x, pBuffer[1]%x, pBuffer[2]%x, pBuffer[3]%x, pBuffer[4]%x, pBuffer[5]%x, pBuffer[6]%x, pBuffer[7]%x, pBuffer[8]%x \n",
		pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3],pBuffer[4],pBuffer[5],pBuffer[6],pBuffer[7],pBuffer[8]);
	

	//covertVector2Array(readnvram,pBuffer);

	//WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), pBuffer, iNvmRecSize);
	WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), pBuffer, Readsize);

	NVRAM_LOG("AP_Editor_ReadFile result: file_idx ~ %d para ~ %d read ~ %d\n", pReq->file_idx, pReq->para, Readsize);
	free(pBuffer);
#endif
	return true;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_write_cnf	META_Editor_WriteFile_OP(
	FT_AP_Editor_write_req *pReq,
    char *peer_buf,
    unsigned short peer_len)
{
	FT_AP_Editor_write_cnf kCnf;
	F_INFO kFileInfo;
	int iNvmRecSize = 0, iWriteSize;
	F_ID iFD;
	bool IsRead = false;
    std::vector<uint8_t> nvramvec;
    int ret=0;
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();

	memset(&kCnf, 0, sizeof(FT_AP_Editor_write_cnf));
	kCnf.file_idx 		= pReq->file_idx;
	kCnf.para 			= pReq->para;
	
	if ((peer_buf == NULL) || (peer_len == 0)) {
		NVRAM_LOG("Error AP_Editor_WriteFile Peer Buffer Error\n");
		kCnf.status = META_FAILED;
		return kCnf;
	}
	
#if 0	
	iFD = NVM_GetFileDesc(pReq->file_idx,&(kFileInfo.i4RecSize),&(kFileInfo.i4RecNum),IsRead);
	if (iFD.iFileDesc == -1) {
		NVRAM_LOG("Error AP_Editor_WriteFile can't open file: file index-%d, %d\n", 
			pReq->file_idx, iNvmRecSize);
		kCnf.status = META_FAILED;
		return kCnf;
	}
	iNvmRecSize = kFileInfo.i4RecSize;
	if ((pReq->para > kFileInfo.i4RecNum) || (peer_len > kFileInfo.i4RecSize)) {
		NVRAM_LOG("Error AP_Editor_WriteFile para: %d, %d, %d\n", pReq->file_idx, pReq->para, peer_len);
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		return kCnf;

	}
	lseek(iFD.iFileDesc, (pReq->para - 1) * iNvmRecSize, SEEK_CUR);
	iWriteSize = write(iFD.iFileDesc, peer_buf, iNvmRecSize);
	if(iNvmRecSize != iWriteSize){
		NVRAM_LOG("Error AP_Editor_WriteFile :Write size not match:iWriteSize(%d),iNvmRecSize(%d),error:%s\n",iWriteSize,iNvmRecSize,strerror(errno));
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		return kCnf;
	}
	NVM_CloseFileDesc(iFD);
#else
auto callback = [&] (int32_t writesize, int8_t retval) {
	 ret=(int)retval;
	 iWriteSize=(int)writesize;
};
covertArray2Vector(peer_buf,peer_len,nvramvec);


client->writeFileBylid(pReq->file_idx,pReq->para,nvramvec,callback);
NVRAM_LOG("ret=%d, iWriteSize=%d\n", ret, iWriteSize);


#endif

	kCnf.write_status = META_STATUS_SUCCESS;
if(ret == 0)
	kCnf.status = META_SUCCESS;
else
	kCnf.status = META_FAILED;

	NVRAM_LOG("AP_Editor_WriteFile result: file_idx-%d para-%d write-%d\n", pReq->file_idx, pReq->para, iWriteSize);
	NVRAM_LOG("AddBackupFileNum Begin");
	//NVM_AddBackupFileNum(pReq->file_idx);
	ret=client->AddBackupFileNum(pReq->file_idx);
	NVRAM_LOG("AddBackupFileNum End ret=%d",ret);
	return kCnf;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_reset_cnf	META_Editor_ResetFile_OP(FT_AP_Editor_reset_req *pReq)
{
	FT_AP_Editor_reset_cnf kCnf;
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();

	memset(&kCnf, 0, sizeof(FT_AP_Editor_reset_cnf));
	//if (!NVM_ResetFileToDefault(pReq->file_idx)) 
	if (!client->ResetFileToDefault(pReq->file_idx))
    {
		printf("Error AP_Editor_ResetFile\n");
		kCnf.status = META_FAILED;
		return kCnf;
	}

	kCnf.status = META_SUCCESS;
	return kCnf;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_reset_cnf	META_Editor_ResetAllFile_OP(FT_AP_Editor_reset_req *pReq)
{
	int i;
	FT_AP_Editor_reset_cnf kCnf;
	FT_AP_Editor_reset_req kReq;
	#if 0
	F_INFO kFileInfo = NVM_ReadFileVerInfo(0);
	#else
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();
    int FileInfoRecSize = 0, FileInfoRecNum= 0, FileInfoMaxFileLid = 0;
	auto callback = [&] (int32_t i4RecSize, int32_t i4RecNum, int32_t i4MaxFileLid) {
	 FileInfoRecSize=(int)i4RecSize;
	 FileInfoRecNum=(int)i4RecNum;
	 FileInfoMaxFileLid=(int)i4MaxFileLid;
};
    client->ReadFileVerInfo(0,callback);
	NVRAM_LOG("FileInfoRecSize %d, FileInfoRecNum %d, FileInfoMaxFileLid %d", FileInfoRecSize,FileInfoRecNum,FileInfoMaxFileLid);
    #endif
	memset(&kCnf, 0, sizeof(FT_AP_Editor_reset_cnf));
	memset(&kReq, 0, sizeof(FT_AP_Editor_reset_req));
    
	if ((pReq->file_idx != 0xFCCF) || (pReq->reset_category != 0xFC)) 
    {
		kCnf.status = META_FAILED;
		NVRAM_LOG("Error AP_Editor_ResetAllFile para is wrong - %d", pReq->file_idx);
		return kCnf;
	}

	//for (i = 0; i < kFileInfo.i4MaxFileLid; ++i) 
	for (i = 0; i < FileInfoMaxFileLid; ++i) 
    {
		kReq.file_idx = i;
		kCnf = META_Editor_ResetFile_OP(&kReq);
		if (kCnf.status == META_FAILED) {
			NVRAM_LOG("Error AP_Editor_ResetAllFile: file_idx-%d\n", kReq.file_idx);
			return kCnf;			
		}
	}

	return kCnf;
}
//-----------------------------------------------------------------------------
