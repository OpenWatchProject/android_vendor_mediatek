#ifdef FT_DRM_KEY_MNG_FEATURE
#ifdef FT_DRM_KEY_MNG_TEE_FEATURE
#include "Keymanage.h"
#include <vendor/mediatek/hardware/keymanage/1.0/IKeymanage.h>
using namespace vendor::mediatek::hardware::keymanage::V1_0;
#else
#include "Keyinstall.h"
#include <vendor/mediatek/hardware/keyinstall/1.0/IKeyinstall.h>
using namespace vendor::mediatek::hardware::keyinstall::V1_0;
#endif
#endif
#include <vendor/mediatek/hardware/nvram/1.1/INvram.h>
#include <string.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <cutils/properties.h>

#include <fcntl.h>
#include <unistd.h>

#include <cutils/sockets.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <dlfcn.h>
#include <time.h>
#include "sysenv_utils.h"

#include "FtModule.h"
#include "LogDefine.h"
#include "Context.h"
#include "PortInterface.h"
#include "PortHandle.h"
#include "hardware/ccci_intf.h"
#include "MSocket.h"

#define SIM_SWITCH_MODE_FILE  "/sys/mtk_ssw/mode"
#define SIM_SWITCH_MODE_STR_LEN 8

#define MTK_MDDB_PATH 	"vendor/etc/mddb"
#define MTK_APDB_PATH   "vendor/etc/apdb"

#ifndef FT_CNF_OK
#define FT_CNF_OK 0
#endif
#ifndef FT_CNF_FAIL
#define FT_CNF_FAIL 1
#endif

#if defined(__LP64__)
#define CCAP_LIB_PATH "/system/lib64/libccap.so"
#else
#define CCAP_LIB_PATH "/system/lib/libccap.so"
#endif


#ifdef FT_WIFI_FEATURE

FtModWifi::FtModWifi(void)
	:CmdTarget(FT_WIFI_REQ_ID)
{
}

FtModWifi::~FtModWifi(void)
{
}

void FtModWifi::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_WIFI_OP ");
    META_WIFI_OP((FT_WM_WIFI_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_GPS_FEATURE

FtModGPS::FtModGPS(void)
	:CmdTarget(FT_GPS_REQ_ID)
{
}

FtModGPS::~FtModGPS(void)
{
}

void FtModGPS::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_GPS_OP ");

    int ATM_Test = ((NORMAL_BOOT == getBootMode()) && (1 == getNormalModeTestFlag()))? 1:0;
    META_GPS_OP((GPS_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen(), ATM_Test);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef FT_NFC_FEATURE
FtModNFC::FtModNFC(void)
	:CmdTarget(FT_NFC_REQ_ID)
{
}

FtModNFC::~FtModNFC(void)
{
}

void FtModNFC::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NFC_OP ");

	if(getInitState())
    META_NFC_OP((NFC_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}

int FtModNFC::init(Frame*)
{
	if (META_NFC_init() != 0)
	{	
		 META_NFC_deinit();
		 return false;
	}
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_FM_FEATURE

FtModFM::FtModFM(void)
	:CmdTarget(FT_FM_REQ_ID)
{
}

FtModFM::~FtModFM(void)
{

}

void FtModFM::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_FM_OP ");
    META_FM_OP((FM_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}

int FtModFM::init(Frame*)
{
	META_FM_init();
	return true;

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_BT_FEATURE

FtModBT::FtModBT(void)
	:CmdTarget(FT_BT_REQ_ID)
{
}

FtModBT::~FtModBT(void)
{
}

void FtModBT::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_BT_OP ");

    META_BT_OP((BT_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_AUDIO_FEATURE

FtModAudio::FtModAudio(void)
	:CmdTarget(FT_L4AUD_REQ_ID)
{
}

FtModAudio::~FtModAudio(void)
{
}

void FtModAudio::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_Audio_OP ");

    META_Audio_OP((FT_L4AUD_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}

int FtModAudio::init(Frame*)
{
	META_Audio_init();
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_CCAP_FEATURE

FtModCCAP::FtModCCAP(void)
	:CmdTarget(FT_CCT_REQ_ID)
{
}

FtModCCAP::~FtModCCAP(void)
{
}

void FtModCCAP::exec(Frame *pFrm)
{
    static void *CcapHwhndl = NULL;
    
    static int (*func_ccap_init)() = NULL;
    static void (*func_ccap_op)(const void*, void*, void**, unsigned short*, void**, unsigned short*) = NULL;
    static void (*func_ccap_const)(int*, int*, int*, int*, int*) = NULL;
    static void (*func_ccap_set_error)(const void*, void*) = NULL;
    static int (*func_ccap_get_req_op)(const void*) = NULL;
    
    static int cnf_size = 0;
    static int FT_CCT_OP_SUBPREVIEW_LCD_START = 0;
    static int FT_CCT_OP_SUBPREVIEW_LCD_STOP = 0;
    
    if(CcapHwhndl == NULL)
    {
        CcapHwhndl = dlopen(CCAP_LIB_PATH, RTLD_NOW);
        
        if(CcapHwhndl == NULL)
        {
            META_LOG("[Meta][FT] link libccap.so fail ");	
            return;
        }
        else
        {
            func_ccap_init = (int(*)()) dlsym(CcapHwhndl, "intf_ccap_init");
            func_ccap_op = (void(*)(const void*, void*, void**, unsigned short*, void**, unsigned short*)) dlsym(CcapHwhndl, "intf_ccap_op");
            func_ccap_const = (void(*)(int*, int*, int*, int*, int*)) dlsym(CcapHwhndl, "intf_ccap_const");
            func_ccap_set_error = (void(*)(const void*, void*)) dlsym(CcapHwhndl, "intf_ccap_set_error");
            func_ccap_get_req_op = (int(*)(const void*)) dlsym(CcapHwhndl, "intf_ccap_get_req_op");
            if (func_ccap_init == NULL || func_ccap_op == NULL || 
                func_ccap_const == NULL || func_ccap_set_error == NULL ||
                func_ccap_get_req_op == NULL )
            {
                META_LOG("[Meta][FT] link libccap.so function pointers fail ");	
                dlclose(CcapHwhndl);
                CcapHwhndl = NULL;
                return;
            }

            func_ccap_const(NULL, &cnf_size, NULL, &FT_CCT_OP_SUBPREVIEW_LCD_START, &FT_CCT_OP_SUBPREVIEW_LCD_STOP);
        }
    }
    
	CmdTarget::exec(pFrm);
   	
	void *req = (void*)pFrm->localBuf();
	int req_op = func_ccap_get_req_op( req );
	if ((req_op != FT_CCT_OP_SUBPREVIEW_LCD_START) && (req_op !=FT_CCT_OP_SUBPREVIEW_LCD_STOP))
	{
		META_LOG("[Meta][FT] META_CCAP_init ");	
		if (func_ccap_init() == 0)
		{
            void *cnf = malloc(cnf_size);
            if(cnf == NULL)
            {
			    META_LOG("[Meta][FT] FT_CCAP_OP META_CCT_init Fail, malloc Fail. ");
                return;
            }
	        memset(cnf, 0, cnf_size);

            func_ccap_set_error(req, cnf);
			WriteDataToPC(cnf, cnf_size, NULL, 0);

            free(cnf);

			META_LOG("[Meta][FT] FT_CCAP_OP META_CCT_init Fail ");
			return;
		}
	}
	else
    {
   		META_LOG("[Meta][FT] Now is sub Camera, init will be do later");	
   	}

	META_LOG("[Meta][FT] FT_CCAP_OP ");

#if 0
    func_ccap_op( (const void*)req, (void*)pFrm->peerBuf() );
#else
    //sned data & free buffer in meta 
    void *localBuf = NULL, *peerBuf = NULL;
    unsigned short localBufLen = 0, peerBufLen = 0;
    
    func_ccap_op( (const void*)req, (void*)pFrm->peerBuf(), &localBuf, &localBufLen, &peerBuf, &peerBufLen );    
    WriteDataToPC(localBuf, localBufLen, peerBuf, peerBufLen);    
    free(localBuf);
    free(peerBuf);    
#endif
    
    //dlclose(CcapHwhndl);    
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_DRM_KEY_MNG_FEATURE
FtModDRM::FtModDRM(void)
	:CmdTarget(FT_DRMKEY_REQ_ID)
{
}

FtModDRM::~FtModDRM(void)
{

}

void FtModDRM::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

#ifdef FT_DRM_KEY_MNG_TRUSTONIC_FEATURE
	//phone drmkey solution
	META_LOG("[Meta][FT] Trustonic Ft_DRM");
	FT_DRMKEY_INSTALL_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_DRMKEY_INSTALL_CNF));
	//init the header
	ft_cnf.header.id = pFrm->getCmdTarget()->getId() + 1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	ft_cnf.status = DRMKEY_INSTALL_FAIL;

	META_LOG("[DRMKEY] pFrm->localBuf())->header.token: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->header.token);
	META_LOG("[DRMKEY] pFrm->localBuf())->header.id: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->header.id);
	META_LOG("[DRMKEY] pFrm->localBuf())->op: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->op);
	META_LOG("[DRMKEY] pFrm->localBuf())->cmd.set_req.file_size: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->cmd.set_req.file_size);
	META_LOG("[DRMKEY] pFrm->localBuf())->cmd.set_req.stage: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->cmd.set_req.stage);
	META_LOG("[DRMKEY] pFrm->localBuf())->cmd.query_req.req: %d\n", ((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf())->cmd.query_req.req);
	META_LOG("[DRMKEY] =========================================================\n");
	META_LOG("[DRMKEY] ft_cnf.header.token: %d\n", ft_cnf.header.token);
	META_LOG("[DRMKEY] ft_cnf.header.id: %d\n", ft_cnf.header.id);
	META_LOG("[DRMKEY] ft_cnf.op: %d\n", ft_cnf.op);
	META_LOG("[DRMKEY] ft_cnf.status: %d\n", ft_cnf.status);
	META_LOG("[DRMKEY] ft_cnf.result.set_cnf.result: %d\n", ft_cnf.result.set_cnf.result);
	META_LOG("[DRMKEY] ft_cnf.result.keyresult.keycount %d\n", ft_cnf.result.keyresult.keycount);

	META_DRMKEY_INSTALL_OP((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf(), &ft_cnf, (char *)pFrm->peerBuf(), pFrm->peerLen());
	WriteDataToPC(&ft_cnf, sizeof(FT_DRMKEY_INSTALL_CNF), NULL, 0);
	META_LOG("[DRMKEY_HIDL] WriteDataToPC done");
#else
	//tablet drmkey solution
	META_LOG("[Meta][FT] Inhouse Ft_DRM");

        FT_DRMKEY_INSTALL_CNF ft_cnf;
        memset(&ft_cnf, 0, sizeof(FT_DRMKEY_INSTALL_CNF));

        //init the header
        ft_cnf.header.id = pFrm->getCmdTarget()->getId() + 1;
        ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
        ft_cnf.status = DRMKEY_INSTALL_FAIL;

        android::hardware::hidl_vec<uint8_t> data;
        HIDL_FT_DRMKEY_INSTALL_REQ toServer_req;
        HIDL_FT_DRMKEY_INSTALL_CNF toServer_cnf;

        convertREQ2HIDL((FT_DRMKEY_INSTALL_REQ *)pFrm->localBuf(), &toServer_req);

        memset(&toServer_cnf, 0, sizeof(HIDL_FT_DRMKEY_INSTALL_CNF));
        convertCNF2HIDL(&ft_cnf, &toServer_cnf);

        META_LOG("[KM_HIDL] start HIDL");
        android::sp<IKeymanage> client = IKeymanage::getService();
        META_LOG("[KM_HIDL] getService done");

        data.setToExternal(pFrm->peerBuf(), pFrm->peerLen());
        META_LOG("[KM_HIDL] data.setToExternal done");

        auto callback = [&] (const HIDL_FT_DRMKEY_INSTALL_CNF& fromServer_cnf) {
        convertCNF2nonHIDL(&fromServer_cnf, &ft_cnf);
        };

        client->meta_drmkey_install_op(toServer_req, toServer_cnf, data, pFrm->peerLen(), callback);

        META_LOG("[KM_HIDL] hidl_meta_drmkey_install_op done");
        WriteDataToPC(&ft_cnf, sizeof(FT_DRMKEY_INSTALL_CNF), NULL, 0);
        META_LOG("[KM_HIDL] WriteDataToPC done");

#endif

}
#endif

#ifdef FT_GAMMA_FEATURE
FtModGAMMA::FtModGAMMA(void)
    :CmdTarget(FT_GAMMA_REQ_ID)
{
}
FtModGAMMA::~FtModGAMMA(void)
{
}
void FtModGAMMA::exec(Frame *pFrm)
{
    CmdTarget::exec(pFrm);
    META_LOG("[Meta][FT] FT_GAMMA_OP ");
    META_GAMMA_OP((GAMMA_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////+

#ifdef FT_ATTESTATION_KEY_FEATURE
FtModAttestationKey::FtModAttestationKey(void)
	:CmdTarget(FT_ATTESTATIONKEY_REQ_ID)
{

}

FtModAttestationKey::~FtModAttestationKey(void)
{

}

void FtModAttestationKey::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta] FtModAttestationKey");

	int finish = 0;
	FT_ATTESTATIONKEY_INSTALL_CNF  ft_cnf;

	memset(&ft_cnf, 0, sizeof(FT_ATTESTATIONKEY_INSTALL_CNF));
	ft_cnf.status = META_FAILED;


	FT_ATTESTATIONKEY_INSTALL_REQ *req = (FT_ATTESTATIONKEY_INSTALL_REQ *)pFrm->localBuf();

	ft_cnf.header.id = req->header.id +1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.type= req->type;

	META_LOG("[Meta] FtModAttestationKey req->type = %d", req->type);
	switch(req->type)
	{
		case FT_ATTESTATIONKEY_INSTALL_SET:
			{
				META_LOG("[Meta] FtModAttestationKey state = %d", req->cmd.set_req.stage);
				if((req->cmd.set_req.stage & 0x04) == 0x04) //KEY_BLK_EOF
					finish = 1;
				META_LOG("[Meta] call ree_import_attest_keybox to send (%d) byte.", pFrm->peerLen());
				META_LOG("[Meta] ft_cnf.status(B): %d\n", ft_cnf.status);
				META_LOG("[Meta] ft_cnf.result.set_cnf.result(B): %d\n", ft_cnf.result.set_cnf.result);
				ft_cnf.result.set_cnf.result = ree_import_attest_keybox((const uint8_t *)pFrm->peerBuf(), (const uint32_t)pFrm->peerLen(), finish);
				META_LOG("[Meta] ft_cnf.status(A): %d\n", ft_cnf.status);
				META_LOG("[Meta] ft_cnf.result.set_cnf.result(A): %d\n", ft_cnf.result.set_cnf.result);
				if(ft_cnf.result.set_cnf.result == 0)
					ft_cnf.status = META_SUCCESS;
				else
					ft_cnf.status = META_FAILED;
			}
			break;
		default:
			break;
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_ATTESTATIONKEY_INSTALL_CNF), NULL, 0);
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////

#define NVRAM_PEER_MAX_LEN 2000
#define BLK_CREATE	0x01
#define BLK_WRITE	0x02
#define BLK_EOF		0x04

#ifdef FT_NVRAM_FEATURE

FtModNvramBackup::FtModNvramBackup(void)
	:CmdTarget(FT_NVRAM_BACKUP_REQ_ID)
{
}

FtModNvramBackup::~FtModNvramBackup(void)
{
}
void FtModNvramBackup::covertArray2Vector(const char* in, int len, std::vector<uint8_t>& out) {
    out.clear();
    for(int i = 0; i < len; i++) {
       out.push_back(in[i]);
    }
}
void FtModNvramBackup::covertVector2Array(std::vector<uint8_t> in, char* out) {
	int size = in.size();
	for(int i = 0; i < size; i++) {
		out[i] = in.at(i);
	}
}

bool FtModNvramBackup::SendNVRAMFile(unsigned char file_ID, FT_NVRAM_BACKUP_CNF* pft_cnf)
{
	int backupFd;
	int peer_buff_size = 0;
    char* peer_buf = 0;
	bool return_value = FALSE;

	backupFd = open(makepath(file_ID), O_RDWR);

	if(backupFd >= 0)
	{
		META_LOG("[Meta][FT] File%d opens succeed ! ",file_ID);
		unsigned int fileLen = getFileSize(backupFd);

		peer_buf = (char*)malloc(NVRAM_PEER_MAX_LEN);
		memset(peer_buf, 0, NVRAM_PEER_MAX_LEN);

		pft_cnf->block.stage = BLK_CREATE;
		pft_cnf->block.file_ID = file_ID;

		while(!(pft_cnf->block.stage & BLK_EOF))
		{
			peer_buff_size = read(backupFd, peer_buf, NVRAM_PEER_MAX_LEN);

			if(peer_buff_size != -1)
			{
				pft_cnf->status = META_SUCCESS;
				if(peer_buff_size == 0)
				{
					pft_cnf->block.stage |= BLK_EOF;
					META_LOG("[Meta][FT] File%d backups succeed! ",file_ID);
					pft_cnf->block.file_size = fileLen;

					close(backupFd);

					free(peer_buf);

					if(remove(makepath(file_ID)) == 0)
					{
						META_LOG("[Meta][FT] File%d DeleteFile succeed! ",file_ID);
						return_value = TRUE;
						WriteDataToPC(pft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),NULL, 0);
					}
					return return_value;
				}
				else
				{
					pft_cnf->block.stage |= BLK_WRITE;
					if(peer_buff_size != NVRAM_PEER_MAX_LEN)
					{
						META_LOG("[Meta][FT] File%d backups %d data ! ",file_ID,peer_buff_size);
					}
					WriteDataToPC(pft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),peer_buf, peer_buff_size);
					memset(peer_buf,0,NVRAM_PEER_MAX_LEN);
					pft_cnf->block.stage &= ~BLK_CREATE;
				}

			}
			else
			{
				pft_cnf->block.stage |= BLK_EOF;
				META_LOG("[Meta][FT] File%d backups read failed ! ", file_ID);
			}

		}

		free(peer_buf);
		close(backupFd);

	}
	else
	{
		META_LOG("[Meta][FT] File%d backups open failed ! ", file_ID);
	}

	return return_value;

}


void FtModNvramBackup::exec(Frame *pFrm)
{
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();
	std::vector<uint8_t> nvramvec;
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NvramBackup_OP ");

    if (client == NULL) {
     META_LOG("client is NULL");
    }

	FT_NVRAM_BACKUP_CNF ft_cnf;
	int bFileOpResult = 0;
	memset(&ft_cnf, 0, sizeof(FT_NVRAM_BACKUP_CNF));
	//init the header
	ft_cnf.header.id = pFrm->getCmdTarget()->getId() + 1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	ft_cnf.status = META_FAILED;

	FT_NVRAM_BACKUP_REQ *req = (FT_NVRAM_BACKUP_REQ *)pFrm->localBuf();

	if (req->count > 0)
	{
		META_LOG("[Meta][FT] Count is %d, backup parts of NvRam!", req->count);
		//bFileOpResult = FileOp_BackupData_Special(req->buffer, req->count, req->mode);
		covertArray2Vector(req->buffer,strlen(req->buffer),nvramvec);
		bFileOpResult = client->BackupData_Special(nvramvec, req->count, req->mode);
	}
	else
	{
		META_LOG("[Meta][FT] Count is %d, backup all NvRam!", req->count);
		//bFileOpResult = FileOp_BackupAll_NvRam();
		bFileOpResult = client->BackupAll_NvRam();
	}
	
	if(bFileOpResult)
	{
	    META_LOG("[Meta][FT] NVM_PcBackup_Get_Data Start ! ");
		if(SendNVRAMFile(0,&ft_cnf))
		{
			META_LOG("[Meta][FT] Send file 0 succeed! ! ");
			//init the header
			ft_cnf.header.id = req->header.id +1;
			ft_cnf.header.token = req->header.token;
			ft_cnf.status = META_FAILED;
			ft_cnf.block.file_size = 0;

			if(SendNVRAMFile(1,&ft_cnf))
			{
				META_LOG("[Meta][FT] Send file 1 succeed! ! ");
				return;
			}
		}
	}
	else
	{
		META_LOG("[Meta][FT] Failed to backup NvRam!");
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),NULL, 0);

}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModNvramRestore::FtModNvramRestore(void)
	:CmdTarget(FT_NVRAM_RESTORE_REQ_ID)
{
}

FtModNvramRestore::~FtModNvramRestore(void)
{
}


void FtModNvramRestore::exec(Frame *pFrm)
{
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NvramRestore_OP ");

    if (client == NULL) {
       META_LOG("client is NULL");
    }

	FT_NVRAM_RESTORE_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_NVRAM_RESTORE_CNF));

	FT_NVRAM_RESTORE_REQ *req = (FT_NVRAM_RESTORE_REQ *)pFrm->localBuf();
	
	//init the header
	ft_cnf.header.id = req->header.id +1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.status = META_FAILED;
	
	int backupFd;
	unsigned int fileLen;
	META_LOG("[Meta][FT] FT_NVRAM_Restore_OP receive block stage %x  file id %d file size %d!",req->block.stage,req->block.file_ID,req->block.file_size);
	if(req->block.stage & BLK_CREATE)
	{
		backupFd = open(makepath(req->block.file_ID), O_RDWR | O_TRUNC | O_CREAT, 0777);
	}
	else
	{
		backupFd = open(makepath(req->block.file_ID), O_RDWR | O_APPEND);
	}
	
	if(backupFd >= 0)
	{
		META_LOG("[Meta][FT] FT_NVRAM_Restore_OP create or open file OK!");
		int sWriten = 0;
		sWriten = write(backupFd,pFrm->peerBuf(),pFrm->peerLen());

		if(sWriten>0)
		{
			ft_cnf.status = META_SUCCESS;
			META_LOG("[Meta][FT] FT_NVRAM_Restore_OP File%d write %d data total data %d!",req->block.file_ID,sWriten,pFrm->peerLen());
			if(req->block.stage & BLK_EOF)
			{
				fileLen = getFileSize(backupFd);
				if(req->block.file_size == fileLen)
				{
					META_LOG("[Meta][FT] FT_NVRAM_Restore_OP write file transfer success! ");
					close(backupFd);
					backupFd = -1;
	
					if(req->block.file_ID == 1)
					{
						//if(!FileOp_RestoreAll_NvRam())
						if(!client->RestoreAll_NvRam())
						{
							ft_cnf.status = META_FAILED;
							META_LOG("[Meta][FT] META_Editor_PcRestore_Set_Data failed! ");
	
						}
					}
				}
				else
				{
					ft_cnf.status = META_FAILED;
					META_LOG("[Meta][FT] FT_NVRAM_Restore_OP file %d size error! / %d ",req->block.file_ID,req->block.file_size);
				}
			}
		}
		else
		{
			META_LOG("[Meta][FT] FT_NVRAM_Restore_OP write file failed! sWriten =%d errno = %d",sWriten,errno);
		}
	
		if(backupFd != -1)
			close(backupFd);
	
	
	}
	else
	{
		META_LOG("[Meta][FT] FT_NVRAM_Restore_OP create or open file failed!");
	}
	
	WriteDataToPC(&ft_cnf, sizeof(FT_NVRAM_RESTORE_CNF),NULL, 0);

}



///////////////////////////////////////////////////////////////////////////////////////////////

FtModNvramReset::FtModNvramReset(void)
	:CmdTarget(FT_NVRAM_RESET_REQ_ID)
{
}

FtModNvramReset::~FtModNvramReset(void)
{
}

void FtModNvramReset::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NvramReset_OP ");
	FT_AP_Editor_reset_cnf ft_cnf;
 
    memset(&ft_cnf, 0, sizeof(FT_AP_Editor_reset_cnf));

	FT_AP_Editor_reset_req *req = (FT_AP_Editor_reset_req *)pFrm->localBuf();

    //if the reset_category and file_idx is 0xfc and 0xfccf, we reset all nvram files.
    if ((req->reset_category == 0xfc )&& (req->file_idx ==0xfccf))
        ft_cnf = META_Editor_ResetAllFile_OP(req);	//reset all files
    else
        ft_cnf = META_Editor_ResetFile_OP(req);		//reset one nvram file

    //fill the ft module header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.status = META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_AP_Editor_reset_cnf),NULL, 0);

}

///////////////////////////////////////////////////////////////////////////////////////////////
FtModNvramRead::FtModNvramRead(void)
	:CmdTarget(FT_NVRAM_READ_REQ_ID)
{
}

FtModNvramRead::~FtModNvramRead(void)
{
}

void FtModNvramRead::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NvramRead_OP ");
 	// just call the inferface of ap_editor lib which will reture the data after reading sucessfully
 	if (!META_Editor_ReadFile_OP((FT_AP_Editor_read_req *)pFrm->localBuf()))
	 	META_LOG("[Meta][FT] FT_APEditorR_OP META Test Fail");

}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModNvramWrite::FtModNvramWrite(void)
	:CmdTarget(FT_NVRAM_WRITE_REQ_ID)
{
}

FtModNvramWrite::~FtModNvramWrite(void)
{
}

void FtModNvramWrite::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

	META_LOG("[Meta][FT] FT_NvramWrite_OP ");

    FT_AP_Editor_write_cnf ft_cnf;

    memset(&ft_cnf, 0, sizeof(FT_AP_Editor_write_cnf));
	FT_AP_Editor_write_req *req = (FT_AP_Editor_write_req *)pFrm->localBuf();

    //// just call the inferface of ap_editor lib
    ft_cnf = META_Editor_WriteFile_OP(req, (char *)pFrm->peerBuf(), pFrm->peerLen());



    //fill the ft module header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.status = META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_AP_Editor_write_cnf),NULL, 0);

}


FtModAPDB::FtModAPDB(void)
    :CmdTarget(FT_APDB_REQ_ID)
{
}

FtModAPDB::~FtModAPDB(void)
{
}

void FtModAPDB::exec(Frame *pFrm)
{
    CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FT_APDB_OP ");

    FT_APDB_CNF ft_cnf;
    memset(&ft_cnf, 0, sizeof(FT_APDB_CNF));

    FT_APDB_REQ *req = (FT_APDB_REQ *)pFrm->localBuf();


    //fill the ft module header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.type= req->type;
    ft_cnf.status = META_FAILED;

    char szAPDBPath[64] = {MTK_APDB_PATH};

    switch(req->type)
    {
        case FT_APDB_OP_QUERYPATH:
            memcpy(ft_cnf.result.query_apdbpath_cnf.apdb_path, szAPDBPath, strlen(szAPDBPath));
            META_LOG("[Meta][FT] FT_APDB_OP FT_APDB_OP_QUERYPATH apdb_path: %s",
            ft_cnf.result.query_apdbpath_cnf.apdb_path);
            ft_cnf.status = META_SUCCESS;
            break;
        default:
            break;
    }

    WriteDataToPC(&ft_cnf, sizeof(FT_APDB_CNF),NULL, 0);

}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////

FtModSysEnv::FtModSysEnv(void)
    :CmdTarget(FT_SYSENV_REQ_ID)
{
}

FtModSysEnv::~FtModSysEnv(void)
{
}

void FtModSysEnv::exec(Frame *pFrm)
{
	
	CmdTarget::exec(pFrm);
	
	META_LOG("[Meta][FT] FT_SYS_ENV_OP");
	
	FT_SYS_ENV_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_SYS_ENV_CNF));	
	FT_SYS_ENV_REQ *req = (FT_SYS_ENV_REQ *)pFrm->localBuf();	
	
	//fill the ft module header
	ft_cnf.header.id = req->header.id +1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.type= req->type;
	ft_cnf.status = META_FAILED;
	const char *val;
    switch(req->type)
    {
        case FT_SYSENV_SET:
            META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_SET name = %s, value = %s",
            req->cmd.sysenv_set_req.name, req->cmd.sysenv_set_req.value);
            if(sysenv_set((char*)req->cmd.sysenv_set_req.name, (char*)req->cmd.sysenv_set_req.value) < 0)
            {
                META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_SET failed!");
            }
            else
            {
                META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_SET succeed!");
                ft_cnf.status = META_SUCCESS;
            }	
            break;
	case FT_SYSENV_GET:
            META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_GET name = %s", req->cmd.sysenv_get_req.name);
            val = sysenv_get((char*)req->cmd.sysenv_get_req.name);
            if(val != NULL)
            {
                META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_GET value = %s", val);
                memcpy(ft_cnf.result.sysenv_get_cnf.value, val, strlen(val));
                ft_cnf.status = META_SUCCESS;
            }
            break;
        case FT_SYSENV_END:
            META_LOG("[Meta][FT] FT_SYS_ENV_OP FT_SYSENV_END");
            break;
    }

	
    WriteDataToPC(&ft_cnf, sizeof(FT_SYS_ENV_CNF),NULL, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModTargetClock::FtModTargetClock(void)
    :CmdTarget(FT_TARGETCLOCK_REQ_ID)
{
}

FtModTargetClock::~FtModTargetClock(void)
{
}

void FtModTargetClock::exec(Frame *pFrm)
{
	
	CmdTarget::exec(pFrm);
	
	META_LOG("[Meta][Clock] FT_TARGET_CLOCK_OP");
	
	FT_TARGETCLOCK_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_TARGETCLOCK_CNF));	
	FT_TARGETCLOCK_REQ *req = (FT_TARGETCLOCK_REQ *)pFrm->localBuf();	
	
	//fill the ft module header
	ft_cnf.header.id = req->header.id +1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.type= req->type;
	ft_cnf.status = META_FAILED;

	switch(req->type)
    {
        case FT_CLOCK_SET:
			if(SetSysClock(&req->cmd.set_clock_req))
			{
				ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_CLOCK_GET:
			break;
        case FT_CLOCK_END:
            META_LOG("[Meta][Clock] FT_TARGET_CLOCK_OP FT_CLOCK_END");
            break;
	}
	
    WriteDataToPC(&ft_cnf, sizeof(FT_TARGETCLOCK_CNF), NULL, 0);
}

unsigned int FtModTargetClock::SetSysClock(SET_TARGET_CLOCK_REQ *req)
{
	int ret = 0;	
	struct tm tmpsec;     
	struct timespec rtime;     
	struct timespec cur_time;

	memset(&tmpsec, 0, sizeof(tm));
	memset(&rtime, 0, sizeof(timespec));
	memset(&cur_time, 0, sizeof(timespec));

	META_LOG("[Meta][Clock] set time to: %d-%02d-%02d %02d-%02d-%02d.%03d", req->year, req->mon, req->day, req->hour, req->min, req->sec, req->ms); 
	if(!IsValidDate(req))
		return ret;
	
	tmpsec.tm_year  = req->year- 1900;
	tmpsec.tm_mon   = req->mon - 1;
	tmpsec.tm_mday  = req->day;
	tmpsec.tm_hour  = req->hour;
	tmpsec.tm_min   = req->min;
	tmpsec.tm_sec   = req->sec;
	tmpsec.tm_isdst = -1;

	rtime.tv_sec = mktime(&tmpsec);
	clock_gettime(CLOCK_REALTIME, &cur_time);
	rtime.tv_nsec = req->ms*1000*1000;
	
	if(clock_settime(CLOCK_REALTIME, &rtime) != 0)
	{
		META_LOG("[Meta][Clock] set target clock failed! ret = %d, err = %d, err string = %s", ret, errno, strerror(errno)); 
		return 0;
	}
	
	META_LOG("[Meta][Clock] set target clock successful!");
	return 1;

}

unsigned int FtModTargetClock::IsValidDate(SET_TARGET_CLOCK_REQ *req)
{
	if((req->year > 2067) || (req->year < 1900))
	{
		META_LOG("[Meta][Clock] invalid year [1900~2067]! year = %d",  req->year);
		return 0;
	}

	if((req->mon > 12) || (req->mon < 1))
	{
		META_LOG("[Meta][Clock] invalid mon [1~12]! mon = %d",  req->mon);
		return 0;
	}
	
	if((req->day > 31) || (req->day < 1))
	{
		META_LOG("[Meta][Clock] invalid day [1~31]! day = %d",  req->day);
		return 0;
	}

	if(req->hour > 23)
	{
		META_LOG("[Meta][Clock] invalid hour [0~23]! hour = %d", req->hour);
		return 0;
	}

	if(req->min > 59)
	{
		META_LOG("[Meta][Clock] invalid min [0~59]! min = %d", req->min);
		return 0;
	}

	if(req->sec > 59)
	{
		META_LOG("[Meta][Clock] invalid sec [0~59]! sec = %d", req->sec);
		return 0;
	}

	if(req->ms > 999)
	{
		META_LOG("[Meta][Clock] invalid ms [0~999]! ms = %d", req->ms);
		return 0;
	}

	return 1;
}


FtModTestAlive::FtModTestAlive(void)
	:CmdTarget(FT_IS_ALIVE_REQ_ID)
{
}

FtModTestAlive::~FtModTestAlive(void)
{
}

void FtModTestAlive::exec(Frame *pFrm)
{
	META_LOG("[Meta][FT] FT_TestAlive");
	FT_IS_ALIVE_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_IS_ALIVE_CNF));

	CmdTarget::exec(pFrm);

	
	//just give the respone.
	ft_cnf.header.id = pFrm->getCmdTarget()->getId()+1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	
	//write bootprof
	writeBootprof((char *)"[META] AP Send FT_TestAlive To PC.");
	
	WriteDataToPC(&ft_cnf, sizeof(FT_IS_ALIVE_CNF),NULL, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//for normal version

#define RELEASE_SW_TOKEN            "ro.vendor.mediatek.version.release"
#define RELEASE_PLATFORM_TOKEN      "ro.board.platform"
#define RELEASE_PRODUCT_TOKEN       "ro.product.name"
#define RELEASE_BUILD_TIME_TOKEN    "ro.build.date"
#define RELEASE_BUILD_DISP_ID_TOKEN "ro.build.display.id"
#define RELEASE_RSC_PROJECT_TOKEN   "ro.boot.rsc"

FtModVersionInfo::FtModVersionInfo(void)
	:CmdTarget(FT_VER_INFO_REQ_ID)
{
}

FtModVersionInfo::~FtModVersionInfo(void)
{
}

void FtModVersionInfo::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);
	META_LOG("[Meta][FT] FT_VersionInfo");
	FT_VER_INFO_CNF ft_cnf;

	memset(&ft_cnf, 0, sizeof(ft_cnf));
	char product[256] = {0};
		
	//initail the value of ft header
	ft_cnf.header.id = pFrm->getCmdTarget()->getId()+1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	ft_cnf.status = META_FAILED;
	
	property_get(RELEASE_SW_TOKEN, (char*)ft_cnf.sw_ver, "");
	META_LOG("[Meta][FT] ft_cnf.sw_ver = %s ", ft_cnf.sw_ver);
	
	property_get(RELEASE_PLATFORM_TOKEN, (char*)ft_cnf.bb_chip, "");
	META_LOG("[Meta][FT] ft_cnf.bb_chip = %s ", ft_cnf.bb_chip);
	
	property_get(RELEASE_BUILD_TIME_TOKEN, (char*)ft_cnf.sw_time, "");
	META_LOG("[Meta][FT] ft_cnf.sw_time = %s ", ft_cnf.sw_time);
		
	property_get(RELEASE_PRODUCT_TOKEN, product, "");
	META_LOG("[Meta][FT] Product Name = %s\n", product);
		
	property_get(RELEASE_RSC_PROJECT_TOKEN, (char*)ft_cnf.rsc_ver, "");
	META_LOG("[Meta][FT] ft_cnf.rsc_ver = %s\n", ft_cnf.rsc_ver);
			
	ft_cnf.status = META_SUCCESS;
		
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);


}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModVersionInfo2::FtModVersionInfo2(void)
	:CmdTarget(FT_VER_INFO_V2_REQ_ID)
{
}

FtModVersionInfo2::~FtModVersionInfo2(void)
{
}

void FtModVersionInfo2::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm); 
	META_LOG("[Meta][FT] FT_VersionInfo2");
	FT_VER_INFO_V2_CNF ft_cnf;
    memset(&ft_cnf, 0, sizeof(ft_cnf));
	char product[256] = {0};
	char disp_id[256] = {0};
		
	//initail the value of ft header
	ft_cnf.header.id = pFrm->getCmdTarget()->getId()+1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	ft_cnf.status = META_FAILED;
	
	property_get(RELEASE_SW_TOKEN, (char*)ft_cnf.sw_ver, "");
	META_LOG("[Meta][FT] ft_cnf.sw_ver = %s ", ft_cnf.sw_ver);
	
	property_get(RELEASE_PLATFORM_TOKEN, (char*)ft_cnf.bb_chip, "");
	META_LOG("[Meta][FT] ft_cnf.bb_chip = %s ", ft_cnf.bb_chip);
	
	property_get(RELEASE_BUILD_TIME_TOKEN, (char*)ft_cnf.sw_time, "");
	META_LOG("[Meta][FT] ft_cnf.sw_timep = %s ", ft_cnf.sw_time);

	property_get(RELEASE_BUILD_DISP_ID_TOKEN, disp_id, "");
	strncpy((char*)ft_cnf.build_disp_id, disp_id, 63);
	META_LOG("[Meta][FT] ft_cnf.build_disp_id = %s ", ft_cnf.build_disp_id);
		
	property_get(RELEASE_PRODUCT_TOKEN, product, "");
	META_LOG("[Meta][FT] Product Name = %s\n", product);
			
	ft_cnf.status = META_SUCCESS;
		
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

///////////////////////////////////////////////////////////////////////////////////////////////
//for chip version
#define CHIPINFO_CODE_FUNC          "/proc/chip/code_func"
#define CHIPINFO_CODE_PROJ          "/proc/chip/code_proj"
#define CHIPINFO_CODE_DATE          "/proc/chip/code_date"
#define CHIPINFO_CODE_FAB           "/proc/chip/code_fab"

FtModChipInfo::FtModChipInfo(void)
	:CmdTarget(FT_CHIP_INFO_REQ_ID)
{
}

FtModChipInfo::~FtModChipInfo(void)
{
}

void FtModChipInfo::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);
	META_LOG("[Meta][FT] FT_ChipInfo");
	FT_CHIP_INFO_CNF ft_cnf;
	FILE *fd = 0;
	char szInfoPath[][32] = { 
			CHIPINFO_CODE_FUNC,
			CHIPINFO_CODE_PROJ,
		    CHIPINFO_CODE_DATE,
			CHIPINFO_CODE_FAB};

	char chip_info[4][64];
	memset(&chip_info,0,sizeof(char)*4*64);
	
	memset(&ft_cnf, 0, sizeof(ft_cnf));
	
	//initail the value of ft header
	ft_cnf.header.id = pFrm->getCmdTarget()->getId()+1;
	ft_cnf.header.token = pFrm->getCmdTarget()->getToken();
	ft_cnf.status = META_FAILED;
	
	/* Get the chip info */
	int i = 0;
	for(i=0; i<4; i++)
	{
		if((fd = fopen(szInfoPath[i], "r")) != NULL)
		{
			if(fgets((char*)chip_info[i], 64, fd) != NULL)
			{
				META_LOG("[Meta][FT] %s = %s\n", szInfoPath[i], chip_info[i]);
			}
			else
			{
				META_LOG("[Meta][FT] failed to read <%s>\n", szInfoPath[i]);
			}

			fclose(fd);
		}
		else
		{
			META_LOG("[Meta][FT] Can't open file : %s\n", szInfoPath[i]);
			break;
		}
	}

	if(i == 4)
	{
		strncpy((char*)ft_cnf.code_func, chip_info[0], 63);
		strncpy((char*)ft_cnf.code_proj, chip_info[1], 63);
		strncpy((char*)ft_cnf.code_date, chip_info[2], 63);
		strncpy((char*)ft_cnf.code_fab, chip_info[3], 63);
		ft_cnf.status = META_SUCCESS;
	}
	
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}
///////////////////////////////////////////////////////////////////////////////////////////////
FtModPowerOff::FtModPowerOff(void)
	:CmdTarget(FT_POWER_OFF_REQ_ID)
{
}

FtModPowerOff::~FtModPowerOff(void)
{
}

void FtModPowerOff::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FT_PowerOff");

	
	FT_POWER_OFF_REQ *req = (FT_POWER_OFF_REQ *)pFrm->localBuf();

	META_LOG("[Meta][FT] req->dummy = %d", req->dummy);

	//sync();
    usleep(200*1000);

    int comPortType = getComType();
    if(comPortType == META_USB_COM && req->dummy != 3)
    {
		closeUSB();		
    }
	else if(comPortType == META_SOCKET)
	{
		if(NORMAL_BOOT == getBootMode())
		{
		    /*
		    META_LOG("[Meta][FT] Send broadcast to disconnect WIFI");
		    const char *strBroadcast = "am broadcast -a Exit_Meta_Info_Activity";
		    system(strBroadcast);
		    */
		    MSocket *mSocket = getSocket(SOCKET_ATM_COMM);
		    if(mSocket != NULL)
		    {
				mSocket->send_msg("ATM_EXIT_WIFI_APP");
		    }

		}
    }
    else
    {
        META_LOG("[Meta][FT] com port type is uart! ");
    }


	switch(req->dummy)
	{
		case 0:
			{
				usleep(1000 * 1000);
				property_set("sys.powerctl","shutdown");
			}
			break;
		case 2:
			{
				usleep(1000 * 1000);
				property_set("sys.powerctl","reboot");
			}
			break;
		case 3:
			{
				META_LOG("[Meta][FT] Disconnect ATM Meta ");
				destroySerPortThread();	
				MSocket *pSocket = getSocket(SOCKET_ATCI_CLIENT);
				if(pSocket != NULL)
				{
					pSocket->send_msg("calibration_stop");
				}
				else
				{
					META_LOG("[Meta][FT] pSocket is NULL");	
				}
			}
			break;
		case 4:
			{
				checkUSBOnline();
				property_set("sys.powerctl","shutdown");
			}
			break;
		case 5:
			{
				rebootToRecovery();
			}
			break;
	}
	
}

void FtModPowerOff::checkUSBOnline()
{	
	int status = 0;
	while((status=readSys_int(USBONLINE_STATUS_PATH)) != 0)
	{
		usleep(200*1000);
		META_LOG("[Meta][PowerOff] usb online status= %d", status);	
	}

	META_LOG("[Meta][PowerOff] usb online off and to power off ");	
}

#define MISC_PART_PATH "/dev/block/platform/bootdevice/by-name/para"
void FtModPowerOff::rebootToRecovery()
{
	META_LOG("[Meta][PowerOff] enter rebootToRecovery");
	const char* command = "boot-recovery";
	int fd = -1;
	int ret = 0;

	fd = open(MISC_PART_PATH, O_RDWR);
	if(fd < 0)
	{
		META_LOG("[Meta][PowerOff] open misc partition fail, errno = %d", errno);
		return;
	}

	META_LOG("[Meta][PowerOff] command is %s", command);

	ret = write(fd, command, 13);
	if(ret > 0)
	{
		META_LOG("[Meta][PowerOff] write %s to misc partition success", command);
	}
	else
	{
		META_LOG("[Meta][PowerOff] write %s to misc partition failed, errno = %d",command, errno);
	}
	close(fd);
	usleep(200*1000);
	property_set("sys.powerctl","reboot");
}

void FtModPowerOff::closeUSB()
{
	FILE *PUsbFile = NULL;
    PUsbFile = fopen("sys/devices/platform/mt_usb/cmode","w");
    if(PUsbFile == NULL)
	{
       	META_LOG("[Meta][FT] Could not open sys/devices/platform/mt_usb/cmode ");
	    PUsbFile = fopen("/sys/devices/platform/musb-mtu3d/musb-hdrc/cmode","w");
		if(PUsbFile == NULL)
		{
		    META_LOG("[Meta][FT] Could not open /sys/devices/platform/musb-mtu3d/musb-hdrc/cmode ");
		    PUsbFile = fopen("/sys/class/udc/musb-hdrc/device/cmode","w");
		    if(PUsbFile == NULL)
		    {
		        META_LOG("[Meta][FT] Could not open /sys/class/udc/musb-hdrc/device/cmode ");
		    }
		    else
		    {
		        fputc('0',PUsbFile);
		        fclose(PUsbFile);
		    }
		}
		else
		{
		    fputc('0',PUsbFile);
		    fclose(PUsbFile);
		}
    }
    else
    {
    	fputc('0',PUsbFile);
		fclose(PUsbFile);	
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
FtModMetaDisconnect::FtModMetaDisconnect(void)
	:CmdTarget(FT_DISCONNECT_REQ_ID)
{
}

FtModMetaDisconnect::~FtModMetaDisconnect(void)
{
}

void FtModMetaDisconnect::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FtModMetaDisconnect");
	
	FT_DISCONNECT_REQ *req = (FT_DISCONNECT_REQ *)pFrm->localBuf();
	FtModPowerOff ft_poweroff;

	META_LOG("[Meta][FT] req->type = %d", req->type);

    usleep(200*1000);

    int comPortType = getComType();
    if(comPortType == META_USB_COM && req->type != FT_CTRL_DISCONN_ATM)
    {
		ft_poweroff.closeUSB();		
    }
	else if(comPortType == META_SOCKET)
	{
		if(NORMAL_BOOT == getBootMode())
		{
			META_LOG("[Meta][FT] Send broadcast to disconnect WIFI");
			const char *strBroadcast = "am broadcast -a Exit_Meta_Info_Activity";
			system(strBroadcast);	
		}
    }
    else
    {
        META_LOG("[Meta][FT] com port type is uart! ");
    }

	switch(req->type)
	{
		case FT_CTRL_POWEROFF:
			{
				usleep(1000 * 1000);
				property_set("sys.powerctl","shutdown");
			}
			break;
		case FT_CTRL_REBOOT:
			{
				usleep(1000 * 1000);
				property_set("sys.powerctl","reboot");
			}
			break;
		case FT_CTRL_REBOOT_BYDELAY:
			{
				META_LOG("[Meta][FT] reboot by delay = %d", req->cmd.disconnect_req.delay);
				sleep(req->cmd.disconnect_req.delay);
				property_set("sys.powerctl","reboot");
			}
			break;
		case FT_CTRL_DISCONN_ATM:
			{
				META_LOG("[Meta][FT] Disconnect ATM Meta ");
				destroySerPortThread();	
				MSocket *pSocket = getSocket(SOCKET_ATCI_CLIENT);
				if(pSocket != NULL)
				{
					pSocket->send_msg("calibration_stop");
				}
				else
				{
					META_LOG("[Meta][FT] pSocket is NULL");	
				}
			}
			break;
		case FT_CTRL_CHECKUSB_POWEROFF:
			{
				ft_poweroff.checkUSBOnline();
				property_set("sys.powerctl","shutdown");
			}
			break;
		case FT_CTRL_REBOOT_RECOVERY:
			{
				ft_poweroff.rebootToRecovery();
			}
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModReboot::FtModReboot(void)
	:CmdTarget(FT_REBOOT_REQ_ID)
{
}

FtModReboot::~FtModReboot(void)
{
}

void FtModReboot::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FT_Reboot");

	
	FT_META_REBOOT_REQ *req = (FT_META_REBOOT_REQ *)pFrm->localBuf();

	sleep(req->delay);

    //Reboot target side after finishing the meta
	//sync();
	//reboot(RB_AUTOBOOT);
	property_set("sys.powerctl","reboot");
}

///////////////////////////////////////////////////////////////////////////////////////////////
FtModBuildProp::FtModBuildProp(void)
	:CmdTarget(FT_BUILD_PROP_REQ_ID)
{
}

FtModBuildProp::~FtModBuildProp(void)
{
}

void FtModBuildProp::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FT_BuildProp");

	FT_BUILD_PROP_CNF ft_cnf;
	FT_BUILD_PROP_REQ *req = (FT_BUILD_PROP_REQ *)pFrm->localBuf();

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
	
    property_get((const char*)req->tag, (char *)ft_cnf.content, "unknown");	

	META_LOG("[Meta][FT] %s = %s ",req->tag,ft_cnf.content);

	ft_cnf.status = META_SUCCESS;

	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);	
}

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_GSENSOR_FEATURE

FtModGSensor::FtModGSensor(void)
	:CmdTarget(FT_GSENSOR_REQ_ID)
{
}

FtModGSensor::~FtModGSensor(void)
{
}

void FtModGSensor::exec(Frame *pFrm)
{

	CmdTarget::exec(pFrm);

     //do the G-Sensor test by called the interface in meta G-Sensor lib
    if(getInitState())
	    Meta_GSensor_OP((GS_REQ *)pFrm->localBuf());

}

int FtModGSensor::init(Frame*pFrm)
{
	META_LOG("[Meta][FT] FT_GSensor");	
	
  	GS_CNF ft_cnf;
    static int bInitFlag_GS = false;
	GS_REQ *req = (GS_REQ *)pFrm->localBuf();


    memset(&ft_cnf, 0, sizeof(GS_CNF));

    if (false == bInitFlag_GS)
    {
        // initial the G-Sensor module when it is called first time
        if (!Meta_GSensor_Open())
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;
		    ft_cnf.op = req->op;
			 
            META_LOG("[Meta][FT] FT_GSENSOR_OP Meta_GSensor_Open Fail ");
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
			return false;
        }
        bInitFlag_GS = true;
    }
	return true;
}

#endif


///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_MSENSOR_FEATURE

FtModMSensor::FtModMSensor(void)
	:CmdTarget(FT_MSENSOR_REQ_ID)
{
   memset(&m_ft_cnf, 0, sizeof(FT_MSENSOR_CNF));
}

FtModMSensor::~FtModMSensor(void)
{
}

void FtModMSensor::exec(Frame *pFrm)
{

  META_LOG("[Meta][FT] FT_MSensor");	
    
	int res = -1;
  	memset(&m_ft_cnf, 0, sizeof(FT_MSENSOR_CNF));
	
	FT_MSENSOR_REQ *req = (FT_MSENSOR_REQ *)pFrm->localBuf();
	
	m_ft_cnf.header.id = req->header.id + 1;
	m_ft_cnf.header.token = req->header.token;
	m_ft_cnf.status = META_SUCCESS;

  	CmdTarget::exec(pFrm);

	if(getInitState())
	{
		res = Meta_MSensor_OP();
		if (0 == res)
		{
		    META_LOG("[Meta][FT] FT_MSENSOR_OP Meta_MSensor_OP success!");
		    m_ft_cnf.status = META_SUCCESS;
		}
		else
		{
		    META_LOG("[Meta][FT] FT_MSENSOR_OP Meta_MSensor_OP failed!");
		    m_ft_cnf.status = META_FAILED;
		}

	  WriteDataToPC(&m_ft_cnf, sizeof(m_ft_cnf),NULL, 0);
	}

}

int FtModMSensor::init(Frame*)
{
	static int bInitFlag_MS = false;
	

	if (false == bInitFlag_MS)
    {
        // initial the M-Sensor module when it is called first time
        if (!Meta_MSensor_Open())
        {
            META_LOG("[Meta][FT] FT_MSENSOR_OP Meta_MSensor_Open failed!");
			m_ft_cnf.status = META_FAILED;
            WriteDataToPC(&m_ft_cnf, sizeof(m_ft_cnf),NULL, 0);
			return false;
        }
        bInitFlag_MS = TRUE;
    }

	return true;
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FT_ALSPS_FEATURE

FtModALSPS::FtModALSPS(void)
	:CmdTarget(FT_ALSPS_REQ_ID)
{
    memset(&m_ft_cnf, 0, sizeof(FT_ALSPS_CNF));
}

FtModALSPS::~FtModALSPS(void)
{
}

void FtModALSPS::exec(Frame *pFrm)
{
	META_LOG("[Meta][FT] FT_ALSPS");	
	
	int res = -1;
  	memset(&m_ft_cnf, 0, sizeof(FT_ALSPS_CNF));
	
	FT_ALSPS_REQ *req = (FT_ALSPS_REQ *)pFrm->localBuf();
		
	m_ft_cnf.header.id = req->header.id + 1;
	m_ft_cnf.header.token = req->header.token;
	m_ft_cnf.status = META_SUCCESS;

	CmdTarget::exec(pFrm);

	if(getInitState())
	{		
	    res = Meta_ALSPS_OP();
	    if (0 == res)
	    {
	        META_LOG("[Meta][FT] FT_ALSPS_OP Meta_ALSPS_OP success!");
	        m_ft_cnf.status = META_SUCCESS;
	    }
		else
		{
		    META_LOG("[Meta][FT] FT_ALSPS_OP Meta_ALSPS_OP failed!");
		    m_ft_cnf.status = META_FAILED;
		}
	    WriteDataToPC(&m_ft_cnf, sizeof(m_ft_cnf),NULL, 0);
	}

}

int FtModALSPS::init(Frame*)
{
	static int bInitFlag_ALSPS = false;

	if (false == bInitFlag_ALSPS)
    {
        // initial the M-Sensor module when it is called first time
        if (!Meta_ALSPS_Open())
        {
            META_LOG("[Meta][FT] FT_ALSPS_OP Meta_ALSPS_Open failed!");
			m_ft_cnf.status = META_FAILED;
            WriteDataToPC(&m_ft_cnf, sizeof(m_ft_cnf),NULL, 0);
			return false;
        }
        bInitFlag_ALSPS = true;
    }
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_GYROSCOPE_FEATURE

FtModGyroSensor::FtModGyroSensor(void)
	:CmdTarget(FT_GYROSCOPE_REQ_ID)
{
}

FtModGyroSensor::~FtModGyroSensor(void)
{
}

void FtModGyroSensor::exec(Frame *pFrm)
{

	CmdTarget::exec(pFrm);

    //do the Gyroscope-Sensor test by called the interface in meta Gyroscope-Sensor lib  
	if(getInitState())
		Meta_Gyroscope_OP((GYRO_REQ *)pFrm->localBuf());

}

int FtModGyroSensor::init(Frame* pFrm)
{
 	META_LOG("[Meta][FT] FT_GyroSensor");
	
	GYRO_REQ *req = (GYRO_REQ *)pFrm->localBuf();
	
	
    static int bInitFlag_GYRO = false;
	GYRO_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(GYRO_CNF));
	
    if (false == bInitFlag_GYRO)
    {
        // initial the Gyroscope-Sensor module when it is called first time
        if (!Meta_Gyroscope_Open())
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;
	    	ft_cnf.op = req->op;

            META_LOG("[Meta][FT] FT_GYROSENSOR_OP Meta_GYROSensor_Open Fail ");
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
			return false;
        }
        bInitFlag_GYRO = true;
    }

	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#define MDDB_FILE_FOLDER "/data/vendor_de/meta/mddb/"
#define MDDB_FILE_FOLDER_EX "/data/vendor_de/meta/mddb"
#define MD1INFO_FILE_PATH "/data/vendor_de/meta/mddb/md1_file_map.log"
#define MD1INFO_FILE_MAP_KEYWORD "md1_file_map"
#define MD1DB_FILE_KEYWORD "md1_mddbmeta"
#define MD1OPENDB_FILE_KEYWORD "md1_mddbmetaodb"

#define MD3INFO_FILE_PATH "/data/vendor_de/meta/mddb/md3_file_map.log"
#define MD3INFO_FILE_MAP_KEYWORD "md3_file_map"
#define MD3DB_FILE_KEYWORD "md3_mddb_c2k_meta"



FtModModemInfo::FtModModemInfo(void)
	:CmdTarget(FT_MODEM_REQ_ID)
{
}

FtModModemInfo::~FtModModemInfo(void)
{
}

int FtModModemInfo::getModemCapability(MODEM_CAPABILITY_LIST_CNF* modem_capa)
{
	int modem_type = 0;	
	modem_type = getMdmType();

	if((modem_type & MD1_INDEX) == MD1_INDEX)
	{
	    if(getModemProtocol(0, (void*)modem_capa) == 0)
	    {
	        META_LOG("[Meta][FT] MD1 getModemProtocol fail");
	        return 0;
	    }
	}

	if((modem_type & MD2_INDEX) == MD2_INDEX)
	{
	    if(getModemProtocol(1, (void*)modem_capa) == 0)
	    {
	        META_LOG("[Meta][FT] MD2 getModemProtocol fail");
	        return 0;
	    }
	}

	if((modem_type & MD3_INDEX) == MD3_INDEX)
	{
	    if(getModemProtocol(2, (void*)modem_capa) == 0)
	    {
	        META_LOG("[Meta][FT] MD3 getModemProtocol fail");
	        return 0;
	    }
	}

	if((modem_type & MD5_INDEX) == MD5_INDEX)
	{	
	    if(getModemProtocol(4, (void*)modem_capa) == 0)
	    {
	        META_LOG("[Meta][FT] MD5 getModemProtocol fail");
	        return 0;
	    }
	}

	return 1;
}

void FtModModemInfo::rebootModem(FT_MODEM_REQ *req, FT_MODEM_CNF & ft_cnf, int fd)
{
	META_LOG("[Meta] Enter rebootModem");
	int result = 0;
	int modem_mode = req->cmd.reboot_modem_req.mode;
	int modem_index = req->cmd.reboot_modem_req.modem_index;
	META_LOG("[Meta] modem_mode = %d", modem_mode);
	META_LOG("[Meta] modem_index = %d", modem_index);

	if(getModemHwVersion(0) >= MODEM_6293) //for Gen 93 and subsequent modem
	{
		if(0 == ChangeModemMode(modem_mode))
		{
			META_LOG("[Meta] Switch modem mode to %d success", modem_mode);
			setMDMode(modem_mode);
			if(modem_mode == 1)
			{
				property_set("persist.vendor.atm.mdmode", "normal");
			}
			else if(modem_mode == 2)
			{
				property_set("persist.vendor.atm.mdmode", "meta");
				createAllModemThread();
			}
			ft_cnf.status = META_SUCCESS;
		}
		else
		{
			META_LOG("[Meta] Switch modem mode from %d to %d fail", getMDMode(), modem_mode);
			ft_cnf.status = META_FAILED;
		}
	}
	else  //for modem before 93
	{
		setActiveATModem(modem_index);

		META_LOG("[Meta] Destory modem thread and close modem handle");
		destroyAllModemThread();

		if (fd >= 0 && 0 == (result = ioctl(fd, CCCI_IOC_SET_MD_BOOT_MODE, &modem_mode)))
		{	
			META_LOG("[Meta][FT] ioctl CCCI_IOC_SET_MD_BOOT_MODE success modem_mode = %d", modem_mode);
			if (0 == (result = ioctl(fd, CCCI_IOC_MD_RESET)))
			{
				ft_cnf.status = META_SUCCESS;
				META_LOG("[Meta][FT] ioctl CCCI_IOC_MD_RESET success " );
			}
			else
			{
				ft_cnf.status = META_FAILED;
				META_LOG("[Meta][FT] ioctl CCCI_IOC_MD_RESET fail result = %d, errno = %d, fd = %d", result, errno, fd);	
			}
		}
		else
		{
			ft_cnf.status = META_FAILED;
			META_LOG("[Meta][FT] ioctl CCCI_IOC_SET_MD_BOOT_MODE fail modem_mode = %d, result = %d, errno = %d, fd = %d", modem_mode, result, errno, fd);
		}
	}
}
int FtModModemInfo::getModemMode(FT_MODEM_REQ *req, FT_MODEM_CNF & ft_cnf, int fd)
{
	int modem_state = 0;
	int modem_boot_mode = 0;
	int result = 0;

	META_LOG("[Meta][FT] getModemMode req->type = %d", req->type);
	if(fd < 0)
	{
		ft_cnf.status = META_FAILED;
		META_LOG("[Meta][FT] invalid ioctl dev port. fd = %d", fd );
		return -1;
	}
	
	while(modem_state != 2)
	{
		if(0 == (result=ioctl(fd, CCCI_IOC_GET_MD_STATE, &modem_state)))
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_STATE success modem_state = %d",modem_state );
		usleep(200*1000);
		}
		else
		{
			ft_cnf.status = META_FAILED;
			META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_STATE fail result = %d, errno = %d, fd = %d", result, errno, fd );
			return -1;
		}				
	}
		
	if (0 == (result=ioctl(fd, CCCI_IOC_GET_MD_BOOT_MODE,&modem_boot_mode)))
	{
		ft_cnf.status = META_SUCCESS;
		ft_cnf.result.get_modem_mode_cnf.mode = modem_boot_mode;
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_BOOT_MODE success modem_boot_mode = %d",modem_boot_mode );
		setMDMode(modem_boot_mode);//normal= 1 meta=2
		return 0;
		}
	else
	{
		ft_cnf.status = META_FAILED;
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_BOOT_MODE fail result = %d, errno = %d, fd = %d", result, errno, fd);	
		return -1;
	}
}

int FtModModemInfo::getModemState(int *modem_state, int fd)
{
	int result = 0;
	int retry_count = 0;

	//Get modem reboot status
	do
	{
		if(0 == ioctl(fd, CCCI_IOC_GET_MD_STATE, modem_state))
		{
			retry_count++;
		}
		usleep(500*1000);
		META_LOG("[Meta][FT][DEBUG] Query modem reboot result %d times", retry_count);
	}while((*modem_state != 2) && (retry_count<10));
	if(*modem_state == 2)
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_STATE success, modem_state = %d", *modem_state);
	}
	else
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_STATE fail, modem_state = %d", *modem_state);
		result = -1;
	}

	return result;
}

int FtModModemInfo::getModemType(int *modem_type, int fd)
{
	int result = 0;
	//Get modem type
	if (fd >= 0 && 0 == ioctl(fd, CCCI_IOC_GET_MD_TYPE, modem_type))
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_TYPE success, modem_type = %d", *modem_type);
	}
	else
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_TYPE fail, modem_type = %d", *modem_type);
	    result = -1;
	}
	return result;
}

int FtModModemInfo::setModemType(int modem_type, int fd)
{
	int result = 0;

	//Set modem type and reboot modem
	if (fd >= 0 && 0 == ioctl(fd, CCCI_IOC_RELOAD_MD_TYPE, &modem_type))
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_RELOAD_MD_TYPE success, modem_type = %d", modem_type);
		if (0 == ioctl(fd, CCCI_IOC_MD_RESET))
		{
			META_LOG("[Meta][FT] ioctl CCCI_IOC_MD_RESET success " );
		}
		else
		{
			META_LOG("[Meta][FT] ioctl CCCI_IOC_MD_RESET fail " );
		    result = -1;
		}
	}
	else
	{
		META_LOG("[Meta][FT] ioctl CCCI_IOC_RELOAD_MD_TYPE fail, modem_type = %d", modem_type);
		result = -1;
	}

	return result;
}

int FtModModemInfo::getModemIndex(FT_MODEM_REQ *req)
{
	int nModemIndex = 0;	
	
	if(req->type == FT_MODEM_OP_SET_MODEMTYPE)
    {
        nModemIndex = req->cmd.set_modem_type_req.modem_id;
    }
	else if(req->type == FT_MODEM_OP_GET_CURENTMODEMTYPE)
	{
	    nModemIndex = req->cmd.get_currentmodem_type_req.modem_id;
	}
	else if(req->type == FT_MODEM_OP_QUERY_MDIMGTYPE)
	{
	    nModemIndex = req->cmd.query_modem_imgtype_req.modem_id;
	}
	
	if(nModemIndex > 0 && nModemIndex < 6)
	{
	    nModemIndex = nModemIndex - 1;
	}
	else
	{
	    nModemIndex = 0;
	}	

	return nModemIndex;
	
}

#ifdef MTK_SINGLE_BIN_MODEM_SUPPORT
int FtModModemInfo::CopyMDDBFile(unsigned int nModemId)
{
    if (access(MDDB_FILE_FOLDER,F_OK) != 0 )
    {
        META_LOG("[Meta][FT] CopyMDDBFile %s folder doesn't exist error %s\n", MDDB_FILE_FOLDER, strerror(errno));
        return 0;
    }

    char mddb_path[256] = {0};
    char mdopendb_path[256] = {0};
    char info_name[128] = {0};
    char info_path[256] = {0};
    memcpy(mddb_path,MDDB_FILE_FOLDER, strlen(MDDB_FILE_FOLDER));
    memcpy(mdopendb_path,MDDB_FILE_FOLDER, strlen(MDDB_FILE_FOLDER));
    if(nModemId == 2)
    {
        memcpy(info_name,MD3INFO_FILE_MAP_KEYWORD, strlen(MD3INFO_FILE_MAP_KEYWORD));
        memcpy(info_path,MD3INFO_FILE_PATH, strlen(MD3INFO_FILE_PATH));
    }
    else
    {
        memcpy(info_name,MD1INFO_FILE_MAP_KEYWORD, strlen(MD1INFO_FILE_MAP_KEYWORD));
        memcpy(info_path,MD1INFO_FILE_PATH, strlen(MD1INFO_FILE_PATH));
    }

    int dbRet = -1;
    int opendbRet = -1;
    int info_ret = restore_image_from_pt(info_name, info_path);
    META_LOG("[Meta][FT] CopyMDDBFile copy info file retore_image_from_pt info_ret:%d", info_ret);
    if(info_ret <= 0)
    {
        META_LOG("[Meta][FT] CopyMDDBFile copy info file error %s\n", strerror(errno));
        return -1;
    }
    else
    {
        FILE* fileInfoFd = NULL;
        fileInfoFd = fopen(info_path,"r");
        if (fileInfoFd == NULL)
        {
            META_LOG("[Meta][FT] CopyMDDBFile open info file error %s\n", strerror(errno));
            return -1;
        }
        else
        {
            char *loc = NULL;
            char* tmp = NULL;
            char str[256] = {0};
            while(!feof(fileInfoFd))
            {
                if(fgets(str, 256, fileInfoFd)!=NULL)
                {
                    tmp = str;
                    loc = strsep(&tmp, "=");
                    if(nModemId == 2)
                    {
                        if(!strcmp(loc, MD3DB_FILE_KEYWORD))
                        {
                            if (tmp[strlen(tmp)-1] == '\n')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            else if (tmp[strlen(tmp)-1] == '\r')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            strncat(mddb_path, tmp, strlen(tmp));
                            META_LOG("[Meta][FT] CopyMDDBFile MDDB file path =%s, len =%d\n", mddb_path,strlen(mddb_path));
                        }
                    }
                    else   //default is 0
                    {
                        if(!strcmp(loc, MD1DB_FILE_KEYWORD))
                        {
                            if (tmp[strlen(tmp)-1] == '\n')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            else if (tmp[strlen(tmp)-1] == '\r')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            strncat(mddb_path, tmp, strlen(tmp));
                            META_LOG("[Meta][FT] CopyMDDBFile MDDB file path =%s, len =%d\n", mddb_path,strlen(mddb_path));
                        }
                        else if(!strcmp(loc, MD1OPENDB_FILE_KEYWORD))
                        {
                            if (tmp[strlen(tmp)-1] == '\n')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            else if (tmp[strlen(tmp)-1] == '\r')
                            {
                                tmp[strlen(tmp)-1] = '\0';
                            }
                            strncat(mdopendb_path, tmp, strlen(tmp));
                            META_LOG("[Meta][FT] CopyMDDBFile MD open DB file path =%s, len =%d\n", mdopendb_path,strlen(mdopendb_path));
                        }
                    }
                }
            }
            if (fileInfoFd != NULL)
            {
                fclose(fileInfoFd);
            }
        }
    }
    if(nModemId == 2)
    {
        dbRet = restore_image_from_pt(MD3DB_FILE_KEYWORD, mddb_path);
        return dbRet;
    }
    else
    {
        dbRet = restore_image_from_pt(MD1DB_FILE_KEYWORD, mddb_path);
        opendbRet = restore_image_from_pt(MD1OPENDB_FILE_KEYWORD, mdopendb_path);
        if (dbRet > 0)
        {
            return dbRet;
        }
        else
        {
            return opendbRet;
        }
    }
}
#endif

void FtModModemInfo::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] FT_ModemInfo");	

	FT_MODEM_CNF ft_cnf;
	int fd = -1;
	static int modemCreate = 0;
	int bDataDevice = FALSE;
	

	memset(&ft_cnf, 0, sizeof(FT_MODEM_CNF));
	ft_cnf.status = META_FAILED;

	
	FT_MODEM_REQ *req = (FT_MODEM_REQ *)pFrm->localBuf();
    META_LOG("[Meta][FT] FT_MODEM_INFO_OP, req type:%d ",req->type);

	if(req->type == FT_MODEM_OP_QUERY_INFO)
	{
		ft_cnf.result.query_modem_info_cnf.modem_number = getMdmNumber();
		ft_cnf.result.query_modem_info_cnf.modem_id = getActiveMdmId();
		ft_cnf.status = META_SUCCESS;
	}
	else if(req->type == FT_MODEM_OP_CAPABILITY_LIST)
	{
		int nRet = 0;
		MODEM_CAPABILITY_LIST_CNF modem_capa;
		memset(&modem_capa, 0, sizeof(MODEM_CAPABILITY_LIST_CNF));
		nRet = getModemCapability(&modem_capa);
		memcpy(&ft_cnf.result.query_modem_cap_cnf,&modem_capa,sizeof(MODEM_CAPABILITY_LIST_CNF));
		if (nRet == 1)
		{
		    ft_cnf.status = META_SUCCESS;
		}
		else
		{
		    ft_cnf.status = META_FAILED;
		}
	}
	else if(req->type == FT_MODEM_OP_QUERY_MDDOWNLOADSTATUS)
	{
		char temp[128]={0};
		int percentage = 0;
		int status_code = 0;
		//property_get("persist.sys.extmddlprogress",temp,NULL);
		META_LOG("[Meta][FT] persist.sys.extmddlprogress = %s",temp);
		sscanf(temp,"%03d_%04d",&percentage,&status_code);
		META_LOG("[Meta][FT] FT_MODEM_INFO_OP FT_MODEM_OP_QUERY_MDDOWNLOADSTATUS percentage = %d,status_code = %d",percentage,status_code);
		ft_cnf.result.query_modem_download_status_cnf.percentage = percentage;
		ft_cnf.result.query_modem_download_status_cnf.status_code = status_code;
		
		if(percentage == 110 && status_code ==0 && modemCreate == 0)
		{
			unsigned int modemType = getMdmType();
			if((modemType & MD5_INDEX) == MD5_INDEX)
			{
				createModemThread(4,1);
			}
			modemCreate = 1;
		}
		ft_cnf.status = META_SUCCESS;
	}
	else if(req->type == FT_MODEM_OP_QUERY_MDDBPATH)
	{
		char szMDDBPath[64] = {0};
#ifdef MTK_SINGLE_BIN_MODEM_SUPPORT		
        	if (CopyMDDBFile(req->cmd.query_mddbpath_req.modem_id) <= 0)
		{
		    META_LOG("[Meta][FILE OPERATION] FtModModemInfo Failed to copy MD %d DB from modem image, error %s\n",req->cmd.query_mddbpath_req.modem_id,strerror(errno));
		    ft_cnf.status = META_FAILED;
		}
		else
		{
		    ft_cnf.status = META_SUCCESS;
		}

		strncpy(szMDDBPath, MDDB_FILE_FOLDER_EX, strlen(MDDB_FILE_FOLDER_EX));
#else

                strncpy(szMDDBPath, MTK_MDDB_PATH, strlen(MTK_MDDB_PATH));
                ft_cnf.status = META_SUCCESS;
#endif
		memcpy(ft_cnf.result.query_mddbpath_cnf.mddb_path, szMDDBPath, strlen(szMDDBPath));
		META_LOG("[Meta][FT] FtModModemInfo mddb_path: %s",ft_cnf.result.query_mddbpath_cnf.mddb_path);
		ft_cnf.header.id = req->header.id +1;
		ft_cnf.header.token = req->header.token;
		ft_cnf.type = req->type;
		WriteDataToPC(&ft_cnf, sizeof(FT_MODEM_CNF),NULL, 0);
		return;
	}

	else if(req->type == FT_MODEM_OP_SET_MODEMTYPE)
    {

		unsigned int modem_type = req->cmd.set_modem_type_req.modem_type;
		fd = getIOCPort(getModemIndex(req),bDataDevice);
	
		if (fd >= 0 && 0 == ioctl(fd, CCCI_IOC_RELOAD_MD_TYPE, &modem_type))
		{
			if (0 == ioctl(fd, CCCI_IOC_MD_RESET))
			{
				ft_cnf.status = META_SUCCESS;		
			}
			else
			{
				ft_cnf.status = META_FAILED;
				META_LOG("[Meta][FT] ioctl CCCI_IOC_MD_RESET fail " );	
			}
		}
		else
		{
			ft_cnf.status = META_FAILED;
			META_LOG("[Meta][FT] ioctl CCCI_IOC_RELOAD_MD_TYPE fail modem_type = %d", modem_type);	
		}
	}
	else if(req->type == FT_MODEM_OP_GET_CURENTMODEMTYPE)
	{
		unsigned int modem_type=0;

		fd = getIOCPort(getModemIndex(req),bDataDevice);
		
		if (fd >= 0 && 0 == ioctl(fd, CCCI_IOC_GET_MD_TYPE, &modem_type))
		{
			ft_cnf.status = META_SUCCESS;
			ft_cnf.result.get_currentmodem_type_cnf.current_modem_type = modem_type;	
			META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_TYPE success modem_type = %d", modem_type);
		}
		else
		{
			ft_cnf.status = META_FAILED;
			META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_TYPE fail");	
		}	
		
	}
	else if(req->type == FT_MODEM_OP_QUERY_MDIMGTYPE)
	{
		unsigned int mdimg_type[16]={0};
		fd = getIOCPort(getModemIndex(req),bDataDevice);

		if (fd >= 0 && 0 == ioctl(fd, CCCI_IOC_GET_MD_IMG_EXIST, &mdimg_type))
		{
			ft_cnf.status = META_SUCCESS;
			memcpy(ft_cnf.result.query_modem_imgtype_cnf.mdimg_type,mdimg_type,16*sizeof(unsigned int));

			for(int i = 0;i<16;i++)
			{
				META_LOG("[Meta][FT] mdimg_type[%d] %d",i, mdimg_type[i]);	
			}
				
		}
		else
		{
			ft_cnf.status = META_FAILED;
			META_LOG("[Meta][FT] ioctl CCCI_IOC_GET_MD_IMG_EXIST fail");	
		}	
	}
	else if(req->type == FT_MODEM_OP_REBOOT_MODEM)
	{
		fd = getIOCPort(getModemIndex(req),bDataDevice);
		rebootModem(req,ft_cnf,fd);
	}
	else if(req->type == FT_MODEM_OP_GET_MODEMMODE)
	{
		fd = getIOCPort(getModemIndex(req),bDataDevice);
		int result = getModemMode(req,ft_cnf,fd);
		if(bDataDevice == FALSE)
		{
	     	if(fd != -1)
		 	{
		     	close(fd);
				META_LOG("[Meta][FT]Close fd");
				fd = -1;
	     	}
		}

		//create modem thread and open modem handle
		if(result == 0)
			createAllModemThread();		
		
	}
	else if(req->type == FT_MODEM_OP_SUPPORT_COMPRESS)
	{

#ifdef MTK_META_COMPRESS_SUPPORT
		META_LOG("[Meta][FT] set data compress status: %d", req->cmd.set_compress_req.action);
		SetDataCompressStatus(req->cmd.set_compress_req.action);
		ft_cnf.result.set_compress_cnf.result = 1;
		ft_cnf.status = META_SUCCESS;
#else
		META_LOG("[Meta][FT] not support data compress");
#endif

	
	}
	else
	{
		META_LOG("[Meta][FT] FT_MODEM_REQ have no this type %d",req->type );		
	}
	

	if(bDataDevice == FALSE)
	{
	     if(fd != -1)
		 {
		     close(fd);
			 META_LOG("[Meta][FT]Close fd");
			 fd = -1;
	     }
	}
	
	ft_cnf.header.id = req ->header.id +1;
    ft_cnf.header.token = req ->header.token;
	ft_cnf.type = req ->type;	


	META_LOG("[META] ft_cnf.status = %d",ft_cnf.status);
	
	WriteDataToPC(&ft_cnf, sizeof(FT_MODEM_CNF),NULL, 0);	
}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModSIMNum::FtModSIMNum(void)
	:CmdTarget(FT_SIM_NUM_REQ_ID)
{
}

FtModSIMNum::~FtModSIMNum(void)
{
}

void FtModSIMNum::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_SIMNum");	

	FT_GET_SIM_CNF ft_cnf;

	FT_GET_SIM_REQ *req = (FT_GET_SIM_REQ *)pFrm->localBuf();

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.type = req->type;
    ft_cnf.status = META_SUCCESS;

	char tempstr[128]={0};
	property_get("persist.vendor.radio.multisim.config",tempstr,"ss");

    ft_cnf.number = 2;
    META_LOG("[Meta][FT] The sim card number default is two");
	if((strcmp(tempstr,"dsds")==0) || (strcmp(tempstr,"dsda")==0))
	{
		ft_cnf.number = 2;
		META_LOG("[Meta][FT] The sim card number is two");
	}
	else if(strcmp(tempstr,"tsts")==0)
	{
		ft_cnf.number = 3;
		META_LOG("[Meta][FT] The sim card number is three");
	}
	else if(strcmp(tempstr,"qsqs")==0)
	{
		ft_cnf.number = 4;
		META_LOG("[Meta][FT] The sim card number is four");
    }
	else if(strcmp(tempstr,"ss")==0)
    {
		ft_cnf.number = 1;
    }
   	
   	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_SDCARD_FEATURE

FtModSDcard::FtModSDcard(void)
	:CmdTarget(FT_SDCARD_REQ_ID)
{
}

FtModSDcard::~FtModSDcard(void)
{
}

void FtModSDcard::exec(Frame *pFrm)
{

	CmdTarget::exec(pFrm);
	
	if(getInitState())   
	    Meta_SDcard_OP((SDCARD_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());

}

int FtModSDcard::init(Frame* pFrm)
{
 	META_LOG("[Meta][FT] Ft_SDcard");

    SDCARD_CNF ft_cnf;	
	memset(&ft_cnf, 0, sizeof(SDCARD_CNF));
    static int bInitFlag_SDcard = FALSE;
	
	SDCARD_REQ *req = (SDCARD_REQ *)pFrm->localBuf();

	META_LOG("[Meta][FT] FT_SDcard_OP META Test req: %zd , %zd ",
        sizeof(SDCARD_REQ), sizeof(SDCARD_CNF));
		
    if (FALSE == bInitFlag_SDcard)
    {
        // initial the DVB module when it is called first time
        if (!Meta_SDcard_Init(req))
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;

            META_LOG("[Meta][FT] FT_SDcard_OP Meta_SDcard_Init Fail ");
    		WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
			return false;
        }
        bInitFlag_SDcard = TRUE;
    }
	return true;
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_EMMC_FEATURE

FtModEMMC::FtModEMMC(void)
	:CmdTarget(FT_EMMC_REQ_ID)
{
}

FtModEMMC::~FtModEMMC(void)
{
}

void FtModEMMC::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_EMMC");

    META_CLR_EMMC_OP((FT_EMMC_REQ *)pFrm->localBuf());

}
#endif

#ifdef FT_NAND_FEATURE

FtModEMMC::FtModEMMC(void)
	:CmdTarget(FT_EMMC_REQ_ID)
{
}

FtModEMMC::~FtModEMMC(void)
{
}

void FtModEMMC::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_EMMC");

    META_CLR_EMMC_OP((FT_EMMC_REQ *)pFrm->localBuf());

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_CRYPTFS_FEATURE

FtModCRYPTFS::FtModCRYPTFS(void)
	:CmdTarget(FT_CRYPTFS_REQ_ID)
{
}

FtModCRYPTFS::~FtModCRYPTFS(void)
{
}

void FtModCRYPTFS::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_CRYPTFS");
	
    META_CRYPTFS_OP((FT_CRYPTFS_REQ *)pFrm->localBuf());    

}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_ADC_FEATURE

FtModADC::FtModADC(void)
	:CmdTarget(FT_ADC_REQ_ID)
{
}

FtModADC::~FtModADC(void)
{
}

void FtModADC::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_ADC");

	Meta_ADC_OP((ADC_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(),  pFrm->peerLen());  
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////

FtModCustomer::FtModCustomer(void)
	:CmdTarget(FT_CUSTOMER_REQ_ID)
{
}

FtModCustomer::~FtModCustomer(void)
{
}

void FtModCustomer::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_Customer");

    FT_CUSTOMER_CNF ft_cnf;
    memset(&ft_cnf, 0, sizeof(FT_CUSTOMER_CNF));
	FT_CUSTOMER_REQ *req = (FT_CUSTOMER_REQ *)pFrm->localBuf();
	
	int peer_buff_size = 1;
    char* peer_buf = NULL;
    int setResult = -1;
    
	// Implement custom API logic here. The following is a sample code for testing.
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
	ft_cnf.type = req->type;
    ft_cnf.status = META_SUCCESS;
	
	peer_buf = (char*)malloc(peer_buff_size);
	memset(peer_buf, 0, peer_buff_size);

	META_LOG("[Meta][FT] setNormalModeTestFlag");
	setResult = setNormalModeTestFlag(req->cmd.m_u1Dummy);
	if(0 == setResult)
	{
		ft_cnf.status = META_SUCCESS;
	}
	else
	{
	    ft_cnf.status = META_FAILED;
	}
	
	META_LOG("[Meta][FT] FT_CUSTOMER_OP successful, OP type is %d!", req->type);
	
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf), peer_buf, peer_buff_size);
	free(peer_buf);
}

///////////////////////////////////////////////////////////////////////////////////////////////

FtModSpecialTest::FtModSpecialTest(void)
	:CmdTarget(FT_SPECIALTEST_REQ_ID)
{
}

FtModSpecialTest::~FtModSpecialTest(void)
{
}

void FtModSpecialTest::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_SpecialTest");
	
	FT_SPECIALTEST_REQ *req = (FT_SPECIALTEST_REQ *)pFrm->localBuf();
	
    FT_SPECIALTEST_CNF SpecialTestCnf;
    memset(&SpecialTestCnf, 0, sizeof(FT_SPECIALTEST_CNF));
    SpecialTestCnf.header.id = req->header.id +1;
    SpecialTestCnf.header.token = req->header.token;
    SpecialTestCnf.type = req->type;
    SpecialTestCnf.status= META_FAILED;

    switch (req->type)
    {
          case FT_SPECIALTEST_OP_HUGEDATA:		//query the supported modules
                  META_LOG("[Meta][FT] pFTReq->type is FT_SPECIALTEST_OP_HUGEDATA ");                 
                  SpecialTestCnf.status= META_SUCCESS;
                  break;
          default :
                  break;
    }

    WriteDataToPC(&SpecialTestCnf, sizeof(FT_SPECIALTEST_CNF),(char *)pFrm->peerBuf(),  pFrm->peerLen());   
}


///////////////////////////////////////////////////////////////////////////////////////////////

#define CHIP_RID_PATH	"/proc/rid"
#define CHIP_RID_LEN 	16 


FtModChipID::FtModChipID(void)
	:CmdTarget(FT_GET_CHIPID_REQ_ID)
{
}

FtModChipID::~FtModChipID(void)
{
}

void FtModChipID::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_ChipID");

	FT_GET_CHIPID_CNF ft_cnf;
	int bytes_read = 0;
	int res = 0;
		
	memset(&ft_cnf, 0, sizeof(FT_GET_CHIPID_CNF));

	
	FT_GET_CHIPID_REQ *req = (FT_GET_CHIPID_REQ *)pFrm->localBuf();
		
	ft_cnf.header.id = req->header.id +1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.status = META_FAILED;
		
	int fd = open(CHIP_RID_PATH, O_RDONLY);
	if (fd != -1)
	{
		while (bytes_read < CHIP_RID_LEN)
		{
			res = read(fd, ft_cnf.chipId + bytes_read, CHIP_RID_LEN);
			if (res > 0)
				bytes_read += res;
			else
				break;
		}
		close(fd);
		ft_cnf.status = META_SUCCESS;
		META_LOG("[Meta][FT] Chip rid=%s", ft_cnf.chipId);
	}
	else
	{
		if (errno == ENOENT)
		{
			ft_cnf.status = META_FAILED;
		}
		META_LOG("[Meta][FT] Failed to open chip rid file %s, errno=%d", CHIP_RID_PATH, errno);
	}
		
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf), NULL, 0);

}

///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FT_TOUCH_FEATURE

FtModCTP::FtModCTP(void)
	:CmdTarget(FT_CTP_REQ_ID)
{
	memset(&m_ft_cnf, 0, sizeof(Touch_CNF));
}

FtModCTP::~FtModCTP(void)
{
}

void FtModCTP::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);
	
	memset(&m_ft_cnf, 0, sizeof(Touch_CNF));
	Touch_REQ *req = (Touch_REQ *)pFrm->localBuf();
	
	m_ft_cnf.header.id = req->header.id + 1;
	m_ft_cnf.header.token = req->header.token;
	m_ft_cnf.status = META_SUCCESS;
	m_ft_cnf.tpd_type = req->tpd_type;
	
	if(getInitState())
		Meta_Touch_OP(req,(char *)pFrm->peerBuf(),  pFrm->peerLen());	

}

int FtModCTP::init(Frame*)
{
	META_LOG("[Meta][FT] Ft_CTP");
	
	static int bInitFlag_CTP = false;
	if (false == bInitFlag_CTP)
	{
		// initial the touch panel module when it is called first time
		if (!Meta_Touch_Init())
		{
			META_LOG("[Meta][FT] FT_CTP_OP Meta_Touch_Init failed!");
			m_ft_cnf.status = META_FAILED;
			WriteDataToPC(&m_ft_cnf, sizeof(m_ft_cnf),NULL, 0);
			return false;
		}
		bInitFlag_CTP = TRUE;
	}

	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FT_GPIO_FEATURE

FtModGPIO::FtModGPIO(void)
	:CmdTarget(FT_GPIO_REQ_ID)
{
	
}

FtModGPIO::~FtModGPIO(void)
{
}

void FtModGPIO::exec(Frame *pFrm)
{
	GPIO_CNF ft_cnf;
	CmdTarget::exec(pFrm);

    META_LOG("[Meta][FT] Ft_GPIO");
	memset(&ft_cnf, 0, sizeof(GPIO_CNF));

	GPIO_REQ *req = (GPIO_REQ *)pFrm->localBuf();

	if(getInitState())
	{	//do the bat test by called the interface in meta bat lib
    	ft_cnf = Meta_GPIO_OP(*req,(unsigned char *)pFrm->peerBuf(),  pFrm->peerLen());
    	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
	}
}

int FtModGPIO::init(Frame*pFrm)
{
	GPIO_CNF ft_cnf;
    static int bInitFlag_GPIO = false;

    META_LOG("[Meta][FT] FT_GPIO_OP META Test ");
    memset(&ft_cnf, 0, sizeof(GPIO_CNF));
	GPIO_REQ *req = (GPIO_REQ *)pFrm->localBuf();

    if (FALSE == bInitFlag_GPIO)
    {
        // initial the bat module when it is called first time
        if (!Meta_GPIO_Init())
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;

            META_LOG("[Meta][FT] FT_GPIO_OP Meta_GPIO_Init Fail ");
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

			return false;
        }
        bInitFlag_GPIO = true;
    }

	return true;
}

#endif


///////////////////////////////////////////////////////////////////////////////////////////////
//for mdlogger ctrl
//#define MDLOG_SOCKET_NAME		     "com.mediatek.mdlogger.socket1"
//#define MBLOG_SOCKET_NAME          "mobilelogd"

FtModTargetloggerCtrl::FtModTargetloggerCtrl(void)
	:CmdTarget(FT_TARGETLOG_CTRL_REQ_ID)
{
}

FtModTargetloggerCtrl::~FtModTargetloggerCtrl(void)
{
}

void FtModTargetloggerCtrl::exec(Frame *pFrm)
{
	META_LOG("[Meta][TARGETLOG CTRL] FtModTargetloggerCtrl");	

	FT_TARGETLOG_CTRL_CNF  ft_cnf;	

	memset(&ft_cnf, 0, sizeof(FT_TARGETLOG_CTRL_CNF));
	ft_cnf.status = META_FAILED;

	
	FT_TARGETLOG_CTRL_REQ *req = (FT_TARGETLOG_CTRL_REQ *)pFrm->localBuf();

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.type= req->type;

	META_LOG("[Meta][TARGETLOG CTRL] FtModTargetloggerCtrl req->type = %d", req->type);
	switch(req->type)
	{
		case FT_MDLOGGER_OP_SWITCH_TYPE:
			{
				if(SwitchMdloggerMode(req))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MDLOGGER_OP_QUERY_STATUS:
			{
				if(QueryMdloggerStatus(ft_cnf))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MDLOGGER_OP_QUERY_NORMALLOG_PATH:
			{
				if(QueryMdNormalLogPath(ft_cnf))
	                ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MDLOGGER_OP_QUERY_EELOG_PATH:
			{
				if(QueryMdEELogPath(ft_cnf))
	                ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MOBILELOG_OP_SWITCH_TYPE:
			{
				if(SwitchMobilelogMode(req))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MOBILELOG_OP_QUERY_LOG_PATH:
			{
                if(QueryMBLogPath(ft_cnf))
	                ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_TARGETLOG_OP_PULL:
			{
				if(TargetLogPulling(req))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_TARGETLOG_OP_PULLING_STATUS:
			{
                if(GetTargetLogPullingStatus(req, ft_cnf))
	                ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_MDLOGGER_OP_SET_FILTER:
			{
				if(SetModemLogFilter(req))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		case FT_CONNSYSLOG_OP_SWITCH_TYPE:
			{
				if(SwitchConnsyslogMode(req))
					ft_cnf.status = META_SUCCESS;
			}
			break;
		default:
			break;
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_TARGETLOG_CTRL_CNF),NULL, 0);
}

unsigned int FtModTargetloggerCtrl::SwitchMdloggerMode(FT_TARGETLOG_CTRL_REQ *req)
{
	char msg[32] = {0};
	char rsp[256] = {0};
	int mode   = req->cmd.mdlogger_ctrl_req.mode;
	int action = req->cmd.mdlogger_ctrl_req.action;
	META_LOG("[Meta][TARGETLOG CTRL] mode = %d, action = %d", mode, action);

	if(action)
		sprintf(msg, MDLOG_START, mode);
	else
		sprintf(msg, MDLOG_STOP);

	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(msg, true);
		return 1;
	}
			
	return 0;			
}

unsigned int FtModTargetloggerCtrl::QueryMdloggerStatus(FT_TARGETLOG_CTRL_CNF &cnf)
{
	char rsp[256] = {0};
	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(MDLOG_QUERY_STATUS, false);
		pSocket->recv_rsp(rsp);

		cnf.result.mdlogger_status_cnf.status = atoi(rsp);
		META_LOG("[Meta][TARGETLOG CTRL] mdlogger staus = %d", cnf.result.mdlogger_status_cnf.status);
				
		return 1;
	}
	else
	{
		META_LOG("[Meta][TARGETLOG CTRL] failed to find psocket");
	}
	return 0;
}

unsigned int FtModTargetloggerCtrl::QueryMdNormalLogPath(FT_TARGETLOG_CTRL_CNF &cnf)
{
	char rsp[256] = {0};
	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(MDLOG_QUERY_NORMALLOG_PATH, false);
		pSocket->recv_rsp(rsp);

		if(rsp[0] != 0)
            strncpy((char*)cnf.result.mdlogger_logpath_cnf.path, rsp, strlen(rsp));			
		else
		    cnf.result.mdlogger_logpath_cnf.path[0] = 0;

		META_LOG("[Meta][TARGETLOG CTRL] modem normal log path = (%s)", cnf.result.mdlogger_logpath_cnf.path);
		return 1;
	}

	return 0;
}

unsigned int FtModTargetloggerCtrl::QueryMdEELogPath(FT_TARGETLOG_CTRL_CNF &cnf)
{
	char rsp[256] = {0};
	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(MDLOG_QUERY_EELOG_PATH, false);
		pSocket->recv_rsp(rsp);

		if(rsp[0] != 0)
			strncpy((char*)cnf.result.mdlogger_logpath_cnf.path, rsp, strlen(rsp));				
		else
			cnf.result.mdlogger_logpath_cnf.path[0] = 0; 			

		META_LOG("[Meta][TARGETLOG CTRL] modem EE log path = (%s)", cnf.result.mdlogger_logpath_cnf.path);	
		return 1;
	}

	return 0;
}

unsigned int FtModTargetloggerCtrl::QueryMBLogPath(FT_TARGETLOG_CTRL_CNF &cnf)
{
	char rsp[256] = {0};
	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MOBILELOG, MBLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(MBLOG_QUERY_NORMALLOG_PATH, false);
		pSocket->recv_rsp(rsp);

		if(rsp[0] != 0)
			strncpy((char*)cnf.result.mobilelog_logpath_cnf.path, rsp, strlen(rsp));				
		else
			cnf.result.mobilelog_logpath_cnf.path[0] = 0; 			
		
		META_LOG("[Meta][TARGETLOG CTRL] mobile log path = (%s)", cnf.result.mobilelog_logpath_cnf.path);	
		return 1;
	}

	return 0;
}
unsigned int FtModTargetloggerCtrl::SwitchMobilelogMode(FT_TARGETLOG_CTRL_REQ *req)
{
	char msg[32] = {0};
	char rsp[256] = {0};
	int mode   = req->cmd.mobilelog_ctrl_req.mode;
	int action = req->cmd.mobilelog_ctrl_req.action;
	META_LOG("[Meta][TARGETLOG CTRL] mode = %d, action = %d", mode, action);

	if(action)
		sprintf(msg, MBLOG_START);
	else
		sprintf(msg, MBLOG_STOP);

	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MOBILELOG, MBLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(msg, true);
		return 1;
	}
	
	return 0;			
}

unsigned int FtModTargetloggerCtrl::TargetLogPulling(FT_TARGETLOG_CTRL_REQ *req)
{
	char service[32] = {0};
	char msg[32]     = {0};
	char rsp[256]    = {0};
	int socket_type  = -1;
	
	int type   = req->cmd.targetlog_pull_req.type;
	int action = req->cmd.targetlog_pull_req.action;
	META_LOG("[Meta][TARGETLOG CTRL] type = %d, action = %d", type, action);

	switch(type)
	{
		case 0:
			{
				socket_type = SOCKET_MDLOGGER;
				strncpy(service, MDLOG_SOCKET_NAME, strlen(MDLOG_SOCKET_NAME));
		        if(action)
			        strncpy(msg, MDLOG_PULL_START, strlen(MDLOG_PULL_START));
		        else
			        strncpy(msg, MDLOG_PULL_STOP, strlen(MDLOG_PULL_STOP));
			}
			break;
		case 1:
			{
				socket_type = SOCKET_MOBILELOG;
				strncpy(service, MBLOG_SOCKET_NAME, strlen(MBLOG_SOCKET_NAME));
				if(action)
					strncpy(msg, MBLOG_PULL_START, strlen(MBLOG_PULL_START));
				else
					strncpy(msg, MBLOG_PULL_STOP, strlen(MBLOG_PULL_STOP));
			}
			break;
		case 2:
			{
				socket_type = SOCKET_CONNSYSLOG;
				strncpy(service,CONNSYSLOG_SOCKET_NAME, strlen(CONNSYSLOG_SOCKET_NAME));
				if(action)
					strncpy(msg, CONNLOG_PULL_START, strlen(CONNLOG_PULL_START));
				else
					strncpy(msg, CONNLOG_PULL_STOP, strlen(CONNLOG_PULL_STOP));
			}
			break;
		case 3:
			{
				socket_type = SOCKET_MDLOGGER;
				strncpy(service, MDLOG_SOCKET_NAME, strlen(MDLOG_SOCKET_NAME));
		        if(action)
			        strncpy(msg, MDDB_PULL_START, strlen(MDDB_PULL_START));
			}
			break;
		default:
			return 0;
	}

	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(socket_type, service);
	if(pSocket != NULL)
	{
		pSocket->setLogPullingStatus(socket_type, 0);
		pSocket->send_msg(msg, true);
		return 1;
	}

	return 0;
	
}

unsigned int FtModTargetloggerCtrl::GetTargetLogPullingStatus(FT_TARGETLOG_CTRL_REQ *req, FT_TARGETLOG_CTRL_CNF &cnf)
{
	unsigned int status = 0;
	unsigned int ret    = 0;
	char rsp[32]     = {0};
	unsigned int socket_type = (unsigned int)SOCKET_END;
	unsigned int type = req->cmd.targetlog_pulling_status_req.type;
	MLogSocket *pSocket = NULL;
		
	META_LOG("[Meta][TARGETLOG CTRL] type = %d", type);

	switch(type)
	{
		case 0:
			{
				pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
				if(pSocket != NULL)
				{
					cnf.result.targetlog_pulling_status_cnf.status = pSocket->getLogPullingStatus(type);
					META_LOG("[Meta][GetTargetLogPullingStatus] status = %d", cnf.result.targetlog_pulling_status_cnf.status);
					ret = 1;
				}
			}
			break;
		case 1:
			{
				pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MOBILELOG, MBLOG_SOCKET_NAME);
				if(pSocket != NULL)
				{
					cnf.result.targetlog_pulling_status_cnf.status = pSocket->getLogPullingStatus(type);
					META_LOG("[Meta][GetTargetLogPullingStatus] status = %d", cnf.result.targetlog_pulling_status_cnf.status);
					ret = 1;
				}
			}
			break;
		case 2:
			{
				pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
				if(pSocket != NULL)
				{
					pSocket->send_msg(MDLOG_EE_DONE_STATUS, false);
					pSocket->recv_rsp(rsp);
					cnf.result.targetlog_pulling_status_cnf.status = atoi(rsp);
					META_LOG("[Meta][GetTargetLogPullingStatus] status = %d", cnf.result.targetlog_pulling_status_cnf.status);
					ret = 1;
				}
			}
			break;
		case 3:
			{
				pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_CONNSYSLOG, CONNSYSLOG_SOCKET_NAME);
				if(pSocket != NULL)
				{
					cnf.result.targetlog_pulling_status_cnf.status = pSocket->getLogPullingStatus(type);
					META_LOG("[Meta][GetTargetLogPullingStatus] status = %d", cnf.result.targetlog_pulling_status_cnf.status);
					ret = 1;
				}
			}
			break;
		case 4:
			{
				pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
				if(pSocket != NULL)
				{
					cnf.result.targetlog_pulling_status_cnf.status = pSocket->getLogPullingStatus(type);
					META_LOG("[Meta][GetTargetLogPullingStatus] status = %d", cnf.result.targetlog_pulling_status_cnf.status);
					ret = 1;
				}
			}
			break;
		default:
			break;
	}
	
	return ret;
	
}

unsigned int FtModTargetloggerCtrl::GetLogPropValue(char *key)
{
	char val[128]={0};
    property_get(key, val, "0");

	return atoi(val);
}

unsigned int FtModTargetloggerCtrl::SetModemLogFilter(FT_TARGETLOG_CTRL_REQ *req)
{
	char msg[32] = {0};
	char rsp[32]     = {0};
	META_LOG("[Meta][TARGETLOG CTRL] Set modem log filter");
	
	sprintf(msg, MDLOG_SET_FILTER, req->cmd.mdlogger_setfilter_req.type);

	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_MDLOGGER, MDLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(msg, true);
		//pSocket->recv_rsp(rsp);
		return 1;
	}

	return 0;	
}

void* FtModTargetloggerCtrl::GetLoggerSocket(unsigned int type, const char * service)
{
	MSocket *pSocket = getSocket(type);
	if(pSocket == NULL)
	{
		pSocket = createSocket(type);
		if(pSocket != NULL)
		{
			int bInit = pSocket->initClient(service, ANDROID_SOCKET_NAMESPACE_ABSTRACT);
			if(bInit == 0)
			{
				delSocket(type);
				return NULL;
			}
		}
		else
			return NULL;
	}

	return (void*)pSocket;
}

unsigned int FtModTargetloggerCtrl::SwitchConnsyslogMode(FT_TARGETLOG_CTRL_REQ *req)
{
	char msg[32] = {0};
	char rsp[256] = {0};
	int mode   = req->cmd.mobilelog_ctrl_req.mode;
	int action = req->cmd.mobilelog_ctrl_req.action;
	META_LOG("[Meta][TARGETLOG CTRL] mode = %d, action = %d", mode, action);

	if(action)
		sprintf(msg, CONNLOG_START);
	else
		sprintf(msg, CONNLOG_STOP);

	MLogSocket *pSocket = (MLogSocket*)GetLoggerSocket(SOCKET_CONNSYSLOG, CONNSYSLOG_SOCKET_NAME);
	if(pSocket != NULL)
	{
		pSocket->send_msg(msg, true);
		return 1;
	}
	
	return 0;			
}


///////////////////////////////////////////////////////////////////////////////////////////////

FtModFileOperation::FtModFileOperation(void)
	:CmdTarget(FT_FILE_OPERATION_REQ_ID)
{
	m_nFileCount = 0;
}

FtModFileOperation::~FtModFileOperation(void)
{    
	ClearFileInfoList();
}

unsigned int FtModFileOperation::GetFileLen(char *pFilePath)
{
    struct stat st;
	if(stat((const char*)pFilePath, &st) < 0)
	{
	    META_LOG("[Meta][FT] GetFileLen pFilePath(%s) fail, errno=%d",pFilePath,errno);
	    return 0;
	}
	else
	{
	    return (unsigned int)st.st_size;
	}
}

void FtModFileOperation::ClearFileInfoList(void)
{
    mlist<FT_FILE_INFO*>::iterator it1 = m_fileInfoList.begin();	
	while (it1 != m_fileInfoList.end())
	{
		delete (*it1);
		++ it1;
	}
	m_fileInfoList.clear();
}


int FtModFileOperation::ListPath(unsigned char *pPath,unsigned char *pFileNameSubStr) 
{
	if(pPath == NULL)
	{
	    META_LOG("[Meta][FT] ListPath path is NULL");
		return 1;
	}

	if(pFileNameSubStr == NULL || pFileNameSubStr[0]  == '\0')
	{
	    META_LOG("[Meta][FT] ListPath file name substr is NULL or 0");
		return 1;
	}

	struct stat s;
	if(stat((const char*)pPath, &s) < 0)
	{
	    META_LOG("[Meta][FT] ListPath call stat fail, errno=%d",errno);
		return 1;
	}

	if(!S_ISDIR(s.st_mode))
	{
	    META_LOG("[Meta][FT] ListPath path(%s) is not a folder name",pPath);
		return 1;
	}

    char currfile[1024] = {0};
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    dir = opendir((const char*)pPath);
	if(dir == NULL)
    {
        META_LOG("[Meta][FT] ListPath opendir(%s) fail, errno=%d",pPath,errno);
        return 1;
    }
    
    m_nFileCount = 0;
	ClearFileInfoList();
    while((ptr = readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".") == 0
          || strcmp(ptr->d_name,"..") ==0)
            continue;
		if(strlen(ptr->d_name) > 256)
		{
		    META_LOG("[Meta][FT] ListPath file name(%s) length is too large,just skip this file!!!!",ptr->d_name);
		    continue;
		}

        if(strcmp((const char*)pFileNameSubStr,"*") == 0
			|| strstr(ptr->d_name,(const char*)pFileNameSubStr) != NULL)
        {
		     FT_FILE_INFO *pFileInfo = new FT_FILE_INFO;
	         if(pFileInfo==NULL)
	         {		    
		         META_LOG("[Meta][FT] ListPath new FT_FILE_INFO fail,errno=%d",errno);
			     closedir(dir);
		         return 1;
	         }
		     memset(pFileInfo,0,sizeof(FT_FILE_INFO));		
             memcpy(pFileInfo->file_name, ptr->d_name, strlen(ptr->d_name));
			 if(pPath[strlen((const char*)pPath)-1] != '/')
			 {
			     sprintf(currfile,"%s/%s",(char*)pPath,ptr->d_name);
			 }
			 else
		     {
		         sprintf(currfile,"%s%s",(char*)pPath,ptr->d_name);
			 }
		
             if(ptr->d_type == DT_REG)  //file
             {
                 pFileInfo->file_type = FT_FILE_TYPE_FILE;
			     pFileInfo->file_size = GetFileLen(currfile);
             }
             else if(ptr->d_type == DT_DIR) //directory
             {
                 pFileInfo->file_type = FT_FILE_TYPE_FOLDER;
			     pFileInfo->file_size = 0;			
             }
			 else
			 {
			     pFileInfo->file_type = FT_FILE_TYPE_INVALID;
			     pFileInfo->file_size = 0;
			 }
		     m_nFileCount++;
		     META_LOG("[Meta][FT] ListPath find one file or folder,file_name=%s,file_type:%d,file_size:%d,m_nFileCount:%d",pFileInfo->file_name,pFileInfo->file_type,pFileInfo->file_size,m_nFileCount);
		     m_fileInfoList.push_back(pFileInfo);
        }

    }

	closedir(dir);
	return 0;
}

FT_FILE_INFO* FtModFileOperation::GetFileInfo(unsigned int id)
{
    mlist<FT_FILE_INFO*>::iterator it = m_fileInfoList.begin();
	unsigned int i = 0;

	while (it != m_fileInfoList.end())
	{
		
		if (i == id)
		{
			return (*it);
		}
		i++;
		++ it;
	}
	return NULL;
}



int FtModFileOperation::SaveSendData(FILE_OPERATION_SENDFILE_REQ *req, char *peer_buff, unsigned short peer_len)
{
    int nRet = 1;
    if(req == NULL || peer_buff == NULL)
    {
        return nRet;
    }
    if(req->dest_file_name[0] == '\0')
    {
      	return nRet;
    }
	  
    int SendFileFd = -1;
    unsigned int fileLen;
	META_LOG("[Meta][FT] SaveSendData receive block stage %x, file size %d!",req->stream_block.stage,req->stream_block.file_size);
	if(req->stream_block.stage & BLK_CREATE)
	{
		SendFileFd = open((const char*)req->dest_file_name, O_RDWR | O_TRUNC | O_CREAT, 0777);
		chown((const char*)req->dest_file_name,2000,1000);
	}
	else
	{
		SendFileFd = open((const char*)req->dest_file_name, O_RDWR | O_APPEND);
	}
	
	if(SendFileFd >= 0)
	{
		META_LOG("[Meta][FT] SaveSendData create or open file OK!");
		unsigned short sWriten = 0;
		sWriten = write(SendFileFd, peer_buff, peer_len);

		if(sWriten)
		{
			META_LOG("[Meta][FT] SaveSendData  write %d data total data %d!",sWriten,peer_len);
			if(req->stream_block.stage & BLK_EOF)
			{
				fileLen = getFileSize(SendFileFd);
				if(req->stream_block.file_size == fileLen)
				{
					META_LOG("[Meta][FT] SaveSendData write file BLK_EOF success! ");
					close(SendFileFd);
					SendFileFd = -1;
					nRet = 0;
				}
				else
				{
					META_LOG("[Meta][FT] SaveSendData file size(%d) error! ",req->stream_block.file_size);
				}
			}
			else
			{
			    META_LOG("[Meta][FT] SaveSendData write file BLK_WRITE success! ");
			    nRet = 0;
			}
		}
		else
		{
			META_LOG("[Meta][FT] SaveSendData write file failed!");
		}
	
		if(SendFileFd != -1)
			close(SendFileFd);
	
	}
	else
	{
		META_LOG("[Meta][FT] SaveSendData create or open file failed!");
	}
	return nRet;
	
}

#define RECEIVE_PEER_MAX_LEN 20*1024

int FtModFileOperation::SetReceiveData(FILE_OPERATION_RECEIVEFILE_REQ *req, FT_FILE_OPERATION_CNF* pft_cnf)
{
    int nRet = 1;
	if(req == NULL || pft_cnf == NULL)
	{
		return nRet;
	}
	if(req->source_file_name[0]  == '\0')
	{
		return nRet;
	}

    unsigned int nReceiveDataSize = 0;
    int ReceiveFileFd = -1;
	int nPeerBuffSize = 0;
    char* pPeerBuf = NULL;
	
	ReceiveFileFd = open((const char*)req->source_file_name, O_RDONLY);
 
  
	if(ReceiveFileFd >= 0)
	{
            unsigned int nFileLen = getFileSize(ReceiveFileFd);
            META_LOG("[Meta][FT] SetReceiveData open file %s succeed, fileSize %d ! ",req->source_file_name,nFileLen);

		pPeerBuf = (char*)malloc(RECEIVE_PEER_MAX_LEN);
		memset(pPeerBuf, 0, RECEIVE_PEER_MAX_LEN);

		pft_cnf->result.receivefile_cnf.stream_block.stage = BLK_CREATE;

		while(!(pft_cnf->result.receivefile_cnf.stream_block.stage & BLK_EOF))
		{
			nPeerBuffSize = read(ReceiveFileFd, pPeerBuf, RECEIVE_PEER_MAX_LEN);

                        META_LOG("[Meta][FT] SetReceiveData nPeerBuffSize:%d,nReceiveDataSize:%d,",nPeerBuffSize,nReceiveDataSize);
			if(nPeerBuffSize != -1)
			{
				pft_cnf->status = META_SUCCESS;
				pft_cnf->result.receivefile_cnf.receive_result = 0;
				if(nPeerBuffSize == 0)
				{
					pft_cnf->result.receivefile_cnf.stream_block.stage |= BLK_EOF;
					pft_cnf->result.receivefile_cnf.stream_block.file_size = nReceiveDataSize;
					WriteDataToPC(pft_cnf, sizeof(FT_FILE_OPERATION_CNF),NULL, 0);
					META_LOG("[Meta][FT] SetReceiveData file end, set BLK_EOF! ");
					nRet = 0;
					break;
				}
				else
				{
					pft_cnf->result.receivefile_cnf.stream_block.stage |= BLK_WRITE;
					pft_cnf->result.receivefile_cnf.stream_block.file_size = nReceiveDataSize;
					META_LOG("[Meta][FT] SetReceiveData File set %d data ! ",nPeerBuffSize);
					WriteDataToPC(pft_cnf, sizeof(FT_FILE_OPERATION_CNF),pPeerBuf, nPeerBuffSize);
					memset(pPeerBuf,0,RECEIVE_PEER_MAX_LEN);
					pft_cnf->result.receivefile_cnf.stream_block.stage &= ~BLK_CREATE;
					META_LOG("[Meta][FT] SetReceiveData set BLK_WRITE! ");
				}
				nReceiveDataSize = nReceiveDataSize + nPeerBuffSize;

			}
			else
			{
				pft_cnf->result.receivefile_cnf.stream_block.stage |= BLK_EOF;
				META_LOG("[Meta][FT] SetReceiveData read file fail, set BLK_EOF! ");
			}

		}

		free(pPeerBuf);
		close(ReceiveFileFd);

	}
	else
	{
	    META_LOG("[Meta][FT] SetReceiveData open File %s failed, errno=%d",req->source_file_name,errno);
	}

	return nRet;
}
void FtModFileOperation::exec(Frame *pFrm)
{
	META_LOG("[Meta][FILE OPERATION] FtModFileOperation");	

	FT_FILE_OPERATION_CNF  ft_cnf;

	memset(&ft_cnf, 0, sizeof(FT_FILE_OPERATION_CNF));
	ft_cnf.status = META_FAILED;

	
	FT_FILE_OPERATION_REQ *req = (FT_FILE_OPERATION_REQ *)pFrm->localBuf();

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.type= req->type;

	META_LOG("[Meta][FILE OPERATION] FtModFileOperation req->type = %d", req->type);
	switch(req->type)
	{
		case FT_FILE_OP_PARSE:
			{
				if(ListPath(req->cmd.parse_req.path_name,req->cmd.parse_req.filename_substr) == 0)
				{
				    ft_cnf.result.parse_cnf.file_count = m_nFileCount;
				    ft_cnf.status = META_SUCCESS;
				    META_LOG("[Meta][FILE OPERATION] FtModFileOperation parse folder success!");
				}
				else
				{
				    META_LOG("[Meta][FILE OPERATION] FtModFileOperation parse folder fail!");
				}
			}
			break;
		case FT_FILE_OP_GETFILEINFO:
			{
				if(req->cmd.getfileinfo_req.index > m_nFileCount)
				{
				    META_LOG("[Meta][FILE OPERATION] FtModFileOperation invalid file index!");
				}
				else
				{
				    FT_FILE_INFO* pFileInfo = GetFileInfo(req->cmd.getfileinfo_req.index);
				    if(pFileInfo != NULL)
				    {				  
				        memcpy(&(ft_cnf.result.getfileinfo_cnf.file_info),pFileInfo,sizeof(FT_FILE_INFO));
						META_LOG("[Meta][FILE OPERATION] file_name=%s,file_type:%d,file_size:%d",pFileInfo->file_name,pFileInfo->file_type,pFileInfo->file_size);
				        ft_cnf.status = META_SUCCESS;
				    }
				}
			}
			break;
		case FT_FILE_OP_SENDFILE:
			{
				if(SaveSendData(&(req->cmd.sendfile_req),(char *)pFrm->peerBuf(), pFrm->peerLen()) == 0)
				{
				    ft_cnf.result.sendfile_cnf.send_result = 0;
				    ft_cnf.status = META_SUCCESS;					   
				}
			}
			break;
		case FT_FILE_OP_RECEIVEFILE:
			{
				if(SetReceiveData(&(req->cmd.receivefile_req),&ft_cnf) == 0)
		        {
			        META_LOG("[Meta][FILE OPERATION] FtModFileOperation success to set receive data!");
					return;
		        }
	            else
	            {
		            META_LOG("[Meta][FILE OPERATION] FtModFileOperation Failed to set receive data!");
	            }
			}
			break;
		default:
			break;
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_FILE_OPERATION_CNF),NULL, 0);
}

#ifdef FT_RAT_FEATURE

FtModRAT::FtModRAT(void)
	:CmdTarget(FT_RATCONFIG_REQ_ID)
{
}

FtModRAT::~FtModRAT(void)
{
}

int FtModRAT::init(Frame*pFrm)
{
	RAT_CNF ft_cnf;
    static int bInitFlag_RAT = false;

    memset(&ft_cnf, 0, sizeof(RAT_CNF));
	RAT_REQ *req = (RAT_REQ *)pFrm->localBuf();

    if (FALSE == bInitFlag_RAT)
    {
        // initial the bat module when it is called first time
        if (!META_RAT_init())
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;

            META_LOG("[Meta][FT] FT_RAT_OP META_RAT_init Fail ");
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

			return false;
        }
        bInitFlag_RAT = true;
    }

	return true;
}


void FtModRAT::exec(Frame *pFrm)
{
	META_LOG("[Meta][FT] FT_RAT_OP FtModRAT ");

	CmdTarget::exec(pFrm);
	if(getInitState())
		META_RAT_OP((RAT_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}

#endif


#ifdef FT_MSIM_FEATURE
FtModMSIM::FtModMSIM(void)
	:CmdTarget(FT_MSIM_REQ_ID)
{
}

FtModMSIM::~FtModMSIM(void)
{
}

int FtModMSIM::init(Frame*pFrm)
{
	MSIM_CNF ft_cnf;
    static int bInitFlag_MSIM = false;

    memset(&ft_cnf, 0, sizeof(MSIM_CNF));
	MSIM_REQ *req = (MSIM_REQ *)pFrm->localBuf();

    if (FALSE == bInitFlag_MSIM)
    {
        // initial the bat module when it is called first time
        if (!META_MSIM_init())
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;
            ft_cnf.status = META_FAILED;

            META_LOG("[Meta][FT] FT_MSIM_OP META_MSIM_init Fail ");
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

			return false;
        }
        bInitFlag_MSIM = true;
    }

	return true;
}


void FtModMSIM::exec(Frame *pFrm)
{
	META_LOG("[Meta][FT] FT_MSIM_OP FtModMSIM");

	CmdTarget::exec(pFrm);
	if(getInitState())
		META_MSIM_OP((MSIM_REQ *)pFrm->localBuf(), (char *)pFrm->peerBuf(), pFrm->peerLen());
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////

BOOL Meta_Mobile_Log()
{        
    int fd = 0;
	int len = 0;
	BOOL ret = FALSE;
	META_LOG("[Meta][FT] Meta_Mobile_Log ");

    //support end load and user load,send stop command to mobilelog 

	fd = socket_local_client("mobilelogd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
	if (fd < 0) 
	{
		META_LOG("[Meta][FT] socket fd <0 ");
		return FALSE;
	}
	META_LOG("[Meta][FT] socket ok\n");
	if((len = write(fd, "stop", sizeof("stop"))) < 0)
	{
		META_LOG("[Meta][FT] socket write error!");
		ret = FALSE;
	}
	else
	{
		META_LOG("[Meta][FT] write %d Bytes.", len);
		ret = TRUE;
	}
	close(fd);
	sleep(4);	      
	return ret;
	
}

void FT_UtilCheckIfFuncExist(FT_UTILITY_COMMAND_REQ  *req, FT_UTILITY_COMMAND_CNF  *cnf)
{

    unsigned int	query_ft_msg_id = req->cmd.CheckIfFuncExist.query_ft_msg_id;
    unsigned int	query_op_code = req->cmd.CheckIfFuncExist.query_op_code;
    META_LOG("[Meta][FT] FT_UtilCheckIfFuncExist META Test ");
    cnf->status = FT_CNF_FAIL;

	META_LOG("[Meta][FT] request id = %d op = %d",query_ft_msg_id,query_op_code);

	
    switch (query_ft_msg_id)
    {

#ifdef FT_FM_FEATURE 
    	case FT_FM_REQ_ID:
			if(query_op_code == 0)//FT_FM_OP_READ_CHIP_ID
			{
				cnf->status = FT_CNF_OK; 
			}        	
        	break;
#endif

#ifdef FT_CRYPTFS_FEATURE
		case FT_CRYPTFS_REQ_ID:
			if(query_op_code == 0)//FT_CRYPTFS_OP_QUERYSUPPORT
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == 1)//FT_CRYPTFS_OP_VERITIFY
			{
				cnf->status = FT_CNF_OK;
			}
        	break;
#endif
			
		case FT_MODEM_REQ_ID:
			if(query_op_code == FT_MODEM_OP_QUERY_INFO )
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_CAPABILITY_LIST)
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_SET_MODEMTYPE)
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_GET_CURENTMODEMTYPE)
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_QUERY_MDIMGTYPE )
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_QUERY_MDDBPATH)
			{
			    cnf->status = FT_CNF_OK;
			}
#ifdef MTK_META_COMPRESS_SUPPORT
			else if(query_op_code == FT_MODEM_OP_SUPPORT_COMPRESS)
			{
				cnf->status = FT_CNF_OK;
			}
#endif
			break;

		case FT_L4AUD_REQ_ID:
			if(query_op_code == 59)//FT_L4AUD_OP_SPEAKER_CALIBRATION_SUPPORT
			{
#ifdef MTK_SPEAKER_MONITOR_SUPPORT
				cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] MTK_SPEAKER_MONITOR_SUPPORT = yes");
#else
				cnf->status = FT_CNF_FAIL;
				META_LOG("[Meta][FT] MTK_SPEAKER_MONITOR_SUPPORT = no");
#endif
			}

			break;
		case FT_SIM_DETECT_REQ_ID:
			if(query_op_code == FT_SIM_DETECT_OP_EXTMOD)
			{
				cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_SIM_DETECT_OP_EXTMOD = yes");
			}
			break;
		case FT_TARGETLOG_CTRL_REQ_ID:
			if(query_op_code <= FT_CONNSYSLOG_OP_SWITCH_TYPE)
			{
				if(getDataChannelType() == 1) //93 modem ccb channel
				{
					cnf->status = FT_CNF_OK;
					META_LOG("[Meta][FT] FT_MDLOGGER_OP = yes");
				}
				else
				{
					META_LOG("[Meta][FT] FT_MDLOGGER_OP = no");
				}
			}
			break;			
		case FT_TARGETCLOCK_REQ_ID:
			if(query_op_code <= FT_CLOCK_GET)
			{
				cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_TARGETCLOCK_OP = yes");
			}
			break;					
		case FT_DISCONNECT_REQ_ID:
			if(query_op_code < FT_CTRL_DONOTHING)
			{
				cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_DISCONN_TARGET_OP = yes");
			}
			break;				
		case FT_UTILITY_COMMAND_REQ_ID:
			if(query_op_code == FT_UTILCMD_QUERY_WCNDRIVER_READY)
			{
			    cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_UTILCMD_QUERY_WCNDRIVER_READY = yes");
			}
			else if(query_op_code == FT_UTILCMD_SWITCH_WIFI_USB)
			{
			    cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_UTILCMD_SWITCH_WIFI_USB = yes");
			}
			else if(query_op_code == FT_UTILCMD_PRINTF_CUSLOG)
			{
			    cnf->status = FT_CNF_OK;
				META_LOG("[Meta][FT] FT_UTILCMD_PRINTF_CUSLOG = yes");
			}
			break;
		case FT_FILE_OPERATION_REQ_ID:
			if(query_op_code == FT_FILE_OP_PARSE )
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_FILE_OP_GETFILEINFO)
			{
			    cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_FILE_OP_SENDFILE)
			{
			    cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_FILE_OP_RECEIVEFILE)
			{
				cnf->status = FT_CNF_OK;
			}
            break;			
		case FT_RATCONFIG_REQ_ID:
#ifdef FT_RAT_FEATURE
			if(query_op_code == RAT_OP_READ_OPTR )
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == RAT_OP_READ_OPTRSEG)
			{
			    cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == RAT_OP_GET_CURRENT_RAT)
			{
			    cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == RAT_OP_SET_NEW_RAT)
			{
			    cnf->status = FT_CNF_OK;
			}
#else
            cnf->status = FT_CNF_FAIL;
#endif
            break;
		case FT_MSIM_REQ_ID:
#ifdef FT_MSIM_FEATURE
            if(query_op_code == MSIM_OP_GET_MSIM )
            {
				cnf->status = FT_CNF_OK;
            }
            else if(query_op_code == MSIM_OP_SET_MSIM)
            {
				cnf->status = FT_CNF_OK;
            }
#else
            cnf->status = FT_CNF_FAIL;
#endif
            break;
        case FT_APDB_REQ_ID:
#ifdef FT_NVRAM_FEATURE
            if(query_op_code == FT_APDB_OP_QUERYPATH )
            {
				cnf->status = FT_CNF_OK;
            }
#else
            cnf->status = FT_CNF_FAIL;
#endif
			break;
#if ((defined FT_EMMC_FEATURE) || (defined FT_NAND_FEATURE))
		case FT_EMMC_REQ_ID:
			if(query_op_code == 3/*FT_EMMC_OP_UNMOUNT*/ )
			{
				cnf->status = FT_CNF_OK;
            }
			break;
#endif
		case FT_POWER_OFF_REQ_ID:
			if(query_op_code == FT_SHUTDOWN_OP_WAITUSB )
			{
				META_LOG("[Meta][FT] FT_SHUTDOWN_OP_WAITUSB = yes");
				cnf->status = FT_CNF_OK;
            }
			break;
    	default:
			META_LOG("[Meta][FT] NOT FOUND THE PRIMITIVE_ID");
        	cnf->status = FT_CNF_FAIL;		
        	break;
    }
    

    // assign return structure
    cnf->result.CheckIfFuncExist.query_ft_msg_id = query_ft_msg_id;
    cnf->result.CheckIfFuncExist.query_op_code = query_op_code;
}

FtModUtility::FtModUtility(void)
	:CmdTarget(FT_UTILITY_COMMAND_REQ_ID)
{
}

FtModUtility::~FtModUtility(void)
{
}

void FtModUtility::covertArray2Vector(unsigned char* in, int len, std::vector<uint8_t>& out) {
    out.clear();
    for(int i = 0; i < len; i++) {
       out.push_back(in[i]);
    }
}
void FtModUtility::covertVector2Array(std::vector<uint8_t> in, char* out) {
	int size = in.size();
	for(int i = 0; i < size; i++) {
		out[i] = in.at(i);
	}
}

void FtModUtility::exec(Frame *pFrm)
{
	META_LOG("[Meta][FT] FT_Peripheral_OP META Test");
	 
    using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
    android::sp<INvram> client = INvram::getService();
	std::vector<uint8_t> nvramvec;
	CmdTarget::exec(pFrm);  

    if (client == NULL) {
      META_LOG("client is NULL");
    }

	FT_UTILITY_COMMAND_CNF UtilityCnf;
    //PROCESS_INFORMATION cleanBootProcInfo;
    static META_BOOL bLCDBKInitFlag_Peri = FALSE;
    static META_BOOL bLCDFtInitFlag_Peri = FALSE;
    static META_BOOL bVibratorInitFlag_Peri = FALSE;
	int nNVRAMFlag = 0;
	unsigned int level = 0;
	char tempstr[128]={0};

    //cleanBootProcInfo.hProcess = NULL;
    //cleanBootProcInfo.hThread = NULL;

    memset(&UtilityCnf, 0, sizeof(FT_UTILITY_COMMAND_CNF));
	FT_UTILITY_COMMAND_REQ *req = (FT_UTILITY_COMMAND_REQ *)pFrm->localBuf();
	
    META_LOG("[Meta][FT] FT_Peripheral_OP META Test, type = %d", req->type); 

    UtilityCnf.header.id = req->header.id +1;
    UtilityCnf.header.token = req->header.token;
    UtilityCnf.type = req->type;
    UtilityCnf.status= META_FAILED;

    //do the related test.
    switch (req->type)
    {
    case FT_UTILCMD_CHECK_IF_FUNC_EXIST:		//query the supported modules
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_CHECK_IF_FUNC_EXIST ");
        FT_UtilCheckIfFuncExist(req, &UtilityCnf);
        break;
    //delete since no use.
    //case FT_UTILCMD_QUERY_LOCAL_TIME:			//query RTC from meta cpu lib
    //    META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_QUERY_LOCAL_TIME ");
    //    UtilityCnf.result.m_WatchDogCnf= META_RTCRead_OP(req->cmd.m_WatchDogReq);
    //    UtilityCnf.status= META_SUCCESS;
    //    break;

#ifdef FT_LCDBK_FEATURE
    case FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL:	//test lcd backlight from meta lcd backlight lig
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL ");
	    if(bLCDBKInitFlag_Peri==FALSE)
     	{   		
			if (!Meta_LCDBK_Init())
			{
				META_LOG("[Meta][FT] FT_Peripheral_OP Meta_LCDBK_Init Fail ");
				goto Per_Exit;
			}   		
		    bLCDBKInitFlag_Peri = TRUE;
    	}        
        UtilityCnf.result.m_LCDCnf = Meta_LCDBK_OP(req->cmd.m_LCDReq);
        UtilityCnf.status= META_SUCCESS;
        break;
#endif

#ifdef FT_LCD_FEATURE
	case FT_UTILCMD_LCD_COLOR_TEST:
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_LCD_COLOR_TEST ");
	    if(bLCDFtInitFlag_Peri==FALSE)
     	{   					
			if (!Meta_LCDFt_Init())
			{
			    META_LOG("[Meta][FT]] FT_Peripheral_OP Meta_LCDFt_Init Fail ");
			    goto Per_Exit;
			}			
		    bLCDFtInitFlag_Peri = TRUE;
    	}        
        UtilityCnf.result.m_LCDColorTestCNF = Meta_LCDFt_OP(req->cmd.m_LCDColorTestReq);
        UtilityCnf.status= META_SUCCESS;
        break;
#endif

#ifdef FT_VIBRATOR_FEATURE
    case FT_UTILCMD_SIGNAL_INDICATOR_ONOFF:
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SIGNAL_INDICATOR_ONOFF ");
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(req->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;
#endif

		
#ifdef FT_VIBRATOR_FEATURE
    case FT_UTILCMD_VIBRATOR_ONOFF:				//test vibrate and indicator from meta nled lib
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_VIBRATOR_ONOFF ");
	    if(bVibratorInitFlag_Peri==FALSE)
     	{
    		if (!Meta_Vibrator_Init())
        	{
            	META_LOG("[Meta][FT] FT_Peripheral_OP Meta_Vibrator_Init Fail ");
            	goto Per_Exit;
        	}	
		    bVibratorInitFlag_Peri = TRUE;
    	}         
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(req->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;
#endif

#ifdef FT_VIBRATOR_FEATURE
    case FT_UTILCMD_KEYPAD_LED_ONOFF:	
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_KEYPAD_LED_ONOFF ");
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(req->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;
#endif

      case FT_UTILCMD_SET_CLEAN_BOOT_FLAG:
        nNVRAMFlag = req->cmd.m_SetCleanBootFlagReq.Notused;
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SET_CLEAN_BOOT_FLAG, nNVRAMFlag =%d",nNVRAMFlag);
        if ((req->cmd.m_SetCleanBootFlagReq.BackupTime)[0] != '\0')
        {
            META_LOG("[Meta][FT] FT_UTILCMD_SET_CLEAN_BOOT_FLAG, BackupTime =%s",req->cmd.m_SetCleanBootFlagReq.BackupTime);
			covertArray2Vector(req->cmd.m_SetCleanBootFlagReq.BackupTime,64,nvramvec);
            //UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = FileOp_BackupToBinRegion_All_Exx(req->cmd.m_SetCleanBootFlagReq.BackupTime);
            UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = client->BackupToBinRegion_All_Exx(nvramvec);
        }
        else
        {
            //For NVRAM to record write barcode(1) and IMEI(2) and both barcode and IMEI(3) history
            if ( nNVRAMFlag == 1 || nNVRAMFlag == 2 || nNVRAMFlag == 3 )
            {
                //UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = FileOp_BackupToBinRegion_All_Ex(nNVRAMFlag);
                UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = client->BackupToBinRegion_All_Ex(nNVRAMFlag);
            }
            else
            {
                //UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = FileOp_BackupToBinRegion_All();
                UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = client->BackupToBinRegion_All();
            }
        }
        UtilityCnf.status=META_SUCCESS;
        break;

    case FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH:	//query the single flash feature, we now just return.
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH ");
        UtilityCnf.status=META_SUCCESS;
        break;

    case FT_UTILCMD_SAVE_MOBILE_LOG:                           //save mobile log
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SAVE_MOBILE_LOG ");
        META_LOG("[Meta][FT] FT_UTILCMD_SAVE_MOBILE_LOG META Test %s,%d,%s",__FILE__,__LINE__,__FUNCTION__);
        UtilityCnf.result.m_SaveMobileLogCnf.drv_status = Meta_Mobile_Log();
        UtilityCnf.status = META_SUCCESS;
		break;
	case FT_UTILCMD_SET_LOG_LEVEL:	  
		level = req->cmd.m_SetLogLevelReq.level;
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SET_LOG_LEVEL ");
		META_LOG("[Meta][FT] FT_UTILCMD_SET_LOG_LEVEL META Test %s,%d,%s,level = %d",__FILE__,__LINE__,__FUNCTION__,level);
		setLogLevel(level);
		UtilityCnf.status = META_SUCCESS;
		break;
	case FT_UTILCMD_SDIO_AUTO_CALIBRATION:	
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SDIO_AUTO_CALIBRATION ");
		META_LOG("[Meta][FT] FT_UTILCMD_SDIO_AUTO_CALIBRATION META Test: no more supported");
		break;
	case FT_UTILCMD_QUERY_WCNDRIVER_READY:
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_QUERY_WCNDRIVER_READY ");
		property_get("vendor.connsys.driver.ready",tempstr,"no");
		if(strcmp(tempstr,"yes")==0)
		{
		    UtilityCnf.result.m_QueryWCNDriverReadyCnf.result = 1; //has ready
		    META_LOG("[Meta][FT] FT_UTILCMD_QUERY_WCNDRIVER_READY() wcn driver ready");
		}
		else
		{
		    UtilityCnf.result.m_QueryWCNDriverReadyCnf.result = 0;  //not ready
		    META_LOG("[Meta][FT] FT_UTILCMD_QUERY_WCNDRIVER_READY() wcn driver not ready");
		}
		UtilityCnf.status = META_SUCCESS;
		break;
	case FT_UTILCMD_SET_ATM_FLAG:
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SET_ATM_FLAG ");

		META_LOG("[Meta][FT] setNormalModeTestFlag");
	
		if(0 == setNormalModeTestFlag(req->cmd.m_SetATMFlagReq.flag))
		{
			UtilityCnf.status = META_SUCCESS;
		}
		else
		{
	    	UtilityCnf.status = META_FAILED;
		}
		break;
	case FT_UTILCMD_SET_PRODUCT_INFO:		
		{
			int offset = 0;
			META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SET_PRODUCT_INFO ");
		
			switch(req->cmd.m_SetProductInfo.type)
			{	
				case 0: //ATM flag
					offset = OFFSET_ATM;
					break;
				case 1: //meta log flag
					offset = OFFSET_METALOG;
					break;
				default:
					break;
			}
			
			META_LOG("[Meta][FT] setProductInfo, type = %d, offset = %d", req->cmd.m_SetProductInfo.type, offset);
			if(0 == setProductInfo(req->cmd.m_SetProductInfo.type, req->cmd.m_SetProductInfo.flag, offset))
			{
				UtilityCnf.status = META_SUCCESS;
			}
			else
			{
	    		UtilityCnf.status = META_FAILED;
			}
		}
		break;
	case FT_UTILCMD_SWITCH_WIFI_USB:
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SWITCH_WIFI_USB ");
		//send response to PC
		UtilityCnf.result.m_SwitchWiFiUSBCnf.result = 1;
		UtilityCnf.status = META_SUCCESS;
		WriteDataToPC(&UtilityCnf, sizeof(FT_UTILITY_COMMAND_CNF),NULL, 0);
		//destroy related resource
		destroyVirtualRxThread();
		//set property and then create resource
		if(req->cmd.m_SwitchWiFiUSBReq.flag == 0) //wifi to usb
		{
			property_set("persist.vendor.meta.connecttype","usb");
			META_LOG("[Meta][FT] persist.vendor.meta.connecttype = usb");
			setComType(META_USB_COM);
		}
		else if(req->cmd.m_SwitchWiFiUSBReq.flag == 1) //usb to wifi
		{
		    property_set("persist.vendor.meta.connecttype","wifi");
			META_LOG("[Meta][FT] persist.vendor.meta.connecttype = wifi");
			setComType(META_SOCKET);
		}
		else
		{
		    META_LOG("[Meta][FT] FT_UTILCMD_SWITCH_WIFI_USB flag = %d is valid",req->cmd.m_SwitchWiFiUSBReq.flag);
		}
		createVirtualRxThread();
		return;
	case FT_UTILCMD_PRINTF_CUSLOG:
		META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_PRINTF_CUSLOG "); 
		if(strlen((char*)req->cmd.m_PrintCusLogReq.log) > 0)
		{
			META_LOG("[Meta][Customization Log] %s", req->cmd.m_PrintCusLogReq.log);
		}
		UtilityCnf.status = META_SUCCESS;
		break;
    default:
        META_LOG("[Meta][FT] FT_Peripheral_OP pFTReq->type error ");
        UtilityCnf.status= META_FAILED;
        break;

    }

Per_Exit:
    WriteDataToPC(&UtilityCnf, sizeof(FT_UTILITY_COMMAND_CNF),NULL, 0);

}

/////////////////////////////////////////////////////////////////////////////////////////



