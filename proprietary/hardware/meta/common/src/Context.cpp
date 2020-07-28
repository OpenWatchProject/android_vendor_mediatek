#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <cutils/sockets.h>

#include "mlist.h"
#include "Modem.h"
#include "MdRxWatcher.h"
#include "UsbRxWatcher.h"
#include "MSocket.h"

#include "FtModule.h"
#include "Context.h"
#include "LogDefine.h"
#include "hardware/ccci_intf.h"
#include "PortInterface.h"

#include "libnvram.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "Custom_NvRam_LID.h"


#ifdef TST_C2K_SUPPORT
#ifndef MTK_ECCCI_C2K
#include "c2kutils.h"
#endif
#endif

#define CCCI_ONE_PATH	"/dev/ttyC1"
#define CCCI_TWO_PATH	"/dev/ccci2_tty1"
#define CCCI_FIVE_PATH	"/dev/eemcs_md_log"
#define USB_EXTERNAL_PATH "/dev/ttyACM0"

UsbRxWatcher hostSerPortRx;

class Context
{
private:
	Context(void);
public:
	~Context(void);

public:
	static Context *instance();

	Modem * createModem(const char *ccci, unsigned short id);
	Modem * createModem(unsigned short id);
	void destroyModem(Modem *p);
	Modem * getModem(unsigned short id);

	CmdTarget *getModule(unsigned short id);
	
	unsigned int dumpData(const unsigned char* con, int length);
	unsigned int dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow);
	
	unsigned int getFileSize(int fd);
	const char* makepath(unsigned char file_ID);

	unsigned getMdmInfo();

	unsigned int getMdmType();
	unsigned int getActiveMdmId();	
	unsigned int getMdmNumber();
	signed int getModemHandle(unsigned short id);
	void createModemThread(unsigned short modemIndex,int usbUsb);
	void createSerPortThread();
	void destroySerPortThread();
	void destroyModemThread(unsigned short modemIndex);	
	void createAllModemThread();
	void destroyAllModemThread();
	
	void setLogLevel(unsigned int level);
	unsigned int getLogLevel();
	int queryModemProtocol(unsigned short modemIndex);
	int getModemProtocol(unsigned short modemIndex, MODEM_CAPABILITY_LIST_CNF* modem_capa);
	FT_MODEM_CH_TYPE getMDChType(unsigned short modemIndex);
	unsigned int getPropValue(const char *key);
	unsigned int getMDMode(void);
	void setMDMode(unsigned int modem_boot_mode);//normal= 1 meta=2
	void setActiveATModem(unsigned int activeATModemId);
	unsigned int getActiveATModem();
	int getIOCPort(unsigned int nModemIndex, int & bDataDevice);
	void destroy();
	void setMDThreadFlag(int modemThreadFlag);
	int getMDThreadFlag();
	void destroyVirtualRxThread();
	void createVirtualRxThread();
	UsbRxWatcher * getVirtualRxWatcher() const;
	void setVirtualRxWatcher(UsbRxWatcher * virtualRxWatcher);
	int readSys_int(char const * path);
	int getBootMode();
	int getDataChannelType();
	MSocket * createSocket(SOCKET_TYPE type);
	MSocket * getSocket(SOCKET_TYPE type);
	void delSocket(SOCKET_TYPE type);
	
	int encrypt(int plainText, int key){return plainText^key;}
	int decrypt(int cipherText, int key){return cipherText^key;}
	void queryNormalModeTestFlag();
	int getNormalModeTestFlag();
	int setNormalModeTestFlag(int flag);

	int setProductInfo(int type, int flag, int offset);
	int getProductInfo(int type, int offset);
	int modifyProductInfo(int type, int flag, int offset);

    void queryWifiPara(int argc, char** argv);
    WIFI_PARA getWifiPara();
	
	int getLoadType();
	void queryModemHwVersion(unsigned short modemIndex);
	int getModemHwVersion(unsigned short modemIndex);
	
	void setATRespFlag(int atFlag);
	int getATRespFlag();

	void setCurrentMdMode(int mdMode);

	int notifyModemDoRFByATCI();
	int ChangeModemMode(int mode);

	void writeBootprof(char * str);

	int writePortIndex();
	
	int getModemModeSwitching();

	void setModemModeSwitching(int modemModeSwitching);

	void switchComType(META_COM_TYPE targetComType);
	unsigned int checkMdStatus();

    void SetDataCompressStatus(unsigned int enable);
	unsigned int GetDataCompressStatus();

	void HandleSocketCmd(char* socket_cmd);

private:
	void initModuleList();
	bool IsModemSupport(int idx);
	int notifyModemDoRF(int mdIdx);
	void waitMdResponse(int fd, const char *rsp);

private:
	mlist<Modem*>		m_mdmList;
	mlist<CmdTarget*>	m_modList;
	SerPort *			m_serPort;
	UsbRxWatcher *      m_virtualRxWatcher;
	MSocket *			m_socket[SOCKET_END];

	MODEM_CAPABILITY_LIST_CNF m_modem_cap_list;

	static Context *	m_myInst;

	unsigned int m_mdmNumber;
	unsigned int m_activeMdmId;
	unsigned int m_activeATModemId;
	unsigned int m_mdmType;
	unsigned int m_logLevel;
	unsigned int m_modem_boot_mode;
	unsigned int m_modemThreadFlag;
	unsigned int m_virtualRxThreadFlag;
	unsigned int m_mdDataChannel; //0: CCCI, 1:CCB

	Modem_Hw_Version m_modem_hw_version;
	char m_modemProtocol[16];

	int m_normalModeTestFlag;
	int m_bootMode;

	WIFI_PARA m_WifiPara;

	int m_atFlag;
	int m_currentMdMode;
	unsigned int m_dataCompressStatus;
};

Context *Context::m_myInst = NULL;


Context::Context(void)
	:m_serPort(NULL),m_virtualRxWatcher(NULL)
{
	initModuleList();
	META_LOG("[Meta] initModuleList");
	getMdmInfo();
	memset(&m_modem_cap_list,0,sizeof(m_modem_cap_list));
	m_logLevel = 0;
	m_modemThreadFlag = 0;
	m_modem_boot_mode = 0;
	m_virtualRxThreadFlag = 0;
	m_activeATModemId = 0;
	m_mdDataChannel = 0;
	m_normalModeTestFlag = 0;
	m_bootMode = UNKNOWN_BOOT;

	m_modem_hw_version = MODEM_END;
	memset(m_modemProtocol, 0, sizeof(m_modemProtocol));

	m_WifiPara.ip_addr = (char *)"0.0.0.0";
	m_WifiPara.port = 0;

	for(int i=0; i<SOCKET_END; i++)
	{
		m_socket[i] = NULL;
	}
	
	m_atFlag = 0;
	m_currentMdMode = 0;
        m_dataCompressStatus = 0;
}

Context::~Context(void)
{
	mlist<Modem*>::iterator it0 = m_mdmList.begin();

	while (it0 != m_mdmList.end())
	{
		delete (*it0);
		++ it0;
	}

	mlist<CmdTarget*>::iterator it1 = m_modList.begin();

	while (it1 != m_modList.end())
	{
		delete (*it1);
		++ it1;
	}

	if (m_serPort != NULL)
	{
		delete m_serPort;
	}
}

Context *Context::instance()
{
	return (m_myInst==NULL) ? ((m_myInst=new Context)) : m_myInst;
}

void Context::destroy()
{
	delete m_myInst;
	m_myInst = NULL;
}

bool Context::IsModemSupport(int idx)
{
	char szVal[128]       = {0};
	char szProperty[128]  = {0};
	int  ret              = 0;
	
	sprintf(szProperty, "ro.vendor.mtk_md%d_support", idx);
	property_get(szProperty, szVal, "0");	
	ret = atoi(szVal);

	if(ret > 0)
		return true;

	return false;
}

int Context::getLoadType()  //eng : 1 or user : 2
{
	char szVal[128] = {0};
	int  ret  = 0;
	property_get("ro.build.type", szVal, NULL);

	if(strcmp(szVal,"eng")==0)
	{
		ret = 1;	
	}
	else if(strcmp(szVal,"user")==0)
	{
		ret = 2;
	}
	else if(strcmp(szVal,"userdebug")==0)
	{
		ret = 3;
	}

	META_LOG("[Meta] ro.build.type = %s ret = %d.",szVal,ret);
	return ret;
	
}

void Context::destroyModem(Modem *p)
{	
	if(p!=NULL)
	{
		m_mdmList.destroy_node(p);
		delete p; //close handle
		META_LOG("[Meta] Delete modem success.");
	}
	else
	{
		META_LOG("[Meta] Delete modem fail.");
	}
}

Modem * Context::createModem(const char *ccci, unsigned short id)
{
	Modem *p = new Modem(ccci, id);

	if(p!=NULL)
	{
		m_mdmList.push_back(p);
		META_LOG("[Meta] Create modem%d success.",id+1);
	}
	else
	{
		META_LOG("[Meta] Create modem%d fail.",id+1);
	}
	return p;
}

Modem * Context::createModem(unsigned short id)
{
	Modem *p = new Modem(id);

	if(p == NULL)
	{
		return NULL;
	}

	if(p->getDevHandle()> 0)
	{
		m_mdmList.push_back(p);
		META_LOG("[Meta] Create modem%d success.",id+1);	
	}
	else
	{
		delete(p);
		META_LOG("[Meta] Create modem%d failed.",id+1);
		return NULL;	
	}
	return p;
}


CmdTarget * Context::getModule(unsigned short id)
{
	mlist<CmdTarget*>::iterator it = m_modList.begin();

	while (it != m_modList.end())
	{
		//META_LOG("[Meta] it->id = %d",(*it)->getId());
		if ((*it)->getId() == id)
		{
			return (*it);
		}
		++ it;
	}
	return NULL;
}

int Context::readSys_int(char const * path)
{
    int fd;
    
    if (path == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        char buffer[20];
        int amt = read(fd, buffer, sizeof(int));          
        close(fd);
        return amt == -1 ? -errno : atoi(buffer);
    }
    META_LOG("[Meta] write_int failed to open %s\n", path);
    return -errno;    
}

int Context::getBootMode()
{
	if(UNKNOWN_BOOT == m_bootMode)
	{
		m_bootMode = readSys_int(BOOTMODE_PATH);

		if(NORMAL_BOOT== m_bootMode)
			META_LOG("[Meta] Normal mode boot!");
		else if(META_BOOT== m_bootMode)
			META_LOG("[Meta] Meta mode boot!");
		else {
			META_LOG("[Meta] Not Support boot mode! BootMode=%d",m_bootMode);
			m_bootMode = UNKNOWN_BOOT;
		}
	}
    return m_bootMode;
}

Modem * Context::getModem(unsigned short id)
{
	mlist<Modem*>::iterator it = m_mdmList.begin();

	while (it != m_mdmList.end())
	{
		META_LOG("[Meta] modem it->id = %d",(*it)->getId());
		if ((*it)->getId() == id)
		{
			return (*it);
		}
		++ it;
	}
	return NULL;
}

void Context::initModuleList()
{
		META_LOG("[Meta] Enter initModuleList");

#ifdef FT_WIFI_FEATURE
	m_modList.push_back(new FtModWifi);
#endif

#ifdef FT_GPS_FEATURE
	m_modList.push_back(new FtModGPS);
#endif

#ifdef FT_NFC_FEATURE
	m_modList.push_back(new FtModNFC);
#endif

#ifdef FT_BT_FEATURE
	m_modList.push_back(new FtModBT);
#endif

#ifdef FT_FM_FEATURE
	m_modList.push_back(new FtModFM);
#endif

#ifdef FT_AUDIO_FEATURE
	m_modList.push_back(new FtModAudio);
#endif

#ifdef FT_CCAP_FEATURE
	m_modList.push_back(new FtModCCAP);
#endif

#ifdef FT_DRM_KEY_MNG_FEATURE
	m_modList.push_back(new FtModDRM);
#endif

#ifdef FT_GAMMA_FEATURE
    m_modList.push_back(new FtModGAMMA);
#endif

#ifdef FT_ATTESTATION_KEY_FEATURE
	m_modList.push_back(new FtModAttestationKey);
#endif

#ifdef FT_NVRAM_FEATURE
	m_modList.push_back(new FtModNvramBackup);
	m_modList.push_back(new FtModNvramRestore);
	m_modList.push_back(new FtModNvramReset);
	m_modList.push_back(new FtModNvramRead);
	m_modList.push_back(new FtModNvramWrite);
    m_modList.push_back(new FtModAPDB);
#endif

#ifdef FT_GSENSOR_FEATURE  
	m_modList.push_back(new FtModGSensor);
#endif

#ifdef FT_MSENSOR_FEATURE
	m_modList.push_back(new FtModMSensor);
#endif

#ifdef FT_ALSPS_FEATURE 
	m_modList.push_back(new FtModALSPS);
#endif

#ifdef FT_GYROSCOPE_FEATURE   
	m_modList.push_back(new FtModGyroSensor);
#endif

#ifdef FT_SDCARD_FEATURE	
	m_modList.push_back(new FtModSDcard);
#endif

#ifdef FT_EMMC_FEATURE
	m_modList.push_back(new FtModEMMC);
#endif

#ifdef FT_NAND_FEATURE
	m_modList.push_back(new FtModEMMC);
#endif

#ifdef FT_CRYPTFS_FEATURE
	m_modList.push_back(new FtModCRYPTFS);
#endif

#ifdef FT_ADC_FEATURE
	m_modList.push_back(new FtModADC);
#endif

#ifdef FT_TOUCH_FEATURE
	m_modList.push_back(new FtModCTP);
#endif

#ifdef FT_GPIO_FEATURE
	m_modList.push_back(new FtModGPIO);
#endif

#ifdef FT_RAT_FEATURE
	m_modList.push_back(new FtModRAT);
#endif

#ifdef FT_MSIM_FEATURE
	m_modList.push_back(new FtModMSIM);
#endif

	m_modList.push_back(new FtModCustomer);
	m_modList.push_back(new FtModChipID);
	m_modList.push_back(new FtModTestAlive);
	m_modList.push_back(new FtModVersionInfo);
	m_modList.push_back(new FtModVersionInfo2);
	m_modList.push_back(new FtModPowerOff);
	m_modList.push_back(new FtModReboot);
	m_modList.push_back(new FtModBuildProp);
	m_modList.push_back(new FtModModemInfo);
	m_modList.push_back(new FtModSIMNum);
	m_modList.push_back(new FtModUtility);
	m_modList.push_back(new FtModSpecialTest);
	m_modList.push_back(new FtModChipInfo);
	m_modList.push_back(new FtModFileOperation);
	m_modList.push_back(new FtModTargetloggerCtrl);
	m_modList.push_back(new FtModTargetClock);
	m_modList.push_back(new FtModMetaDisconnect);
	
#ifdef MTK_META_SYSENV_SUPPORT  
	m_modList.push_back(new FtModSysEnv);
#endif

	
}

unsigned int Context::getMdmInfo()
{
	unsigned int modem_number =0;
	unsigned int active_modem_id = 0;
	unsigned int modem_type = 0;
	bool isactive = false;

    if(IsModemSupport(3))
	{
	    modem_type |= MD3_INDEX;	
		modem_number++;
		META_LOG("[Meta] modem[3] is enable");
	}
	
    if(IsModemSupport(1))
	{
		modem_type |= MD1_INDEX;	
		modem_number++;
		if(!isactive)
		{
		    active_modem_id = 1;
			isactive = true;
		}
		META_LOG("[Meta] modem[1] is enable");
	}

    if(IsModemSupport(2))
	{
		modem_type |= MD2_INDEX;
		modem_number++;
		if(!isactive)
		{
			active_modem_id = 2;
			isactive = true;
		}
		META_LOG("[Meta] modem[2] is enable");
	}

    if(IsModemSupport(5))
	{
		modem_type |= MD5_INDEX;	
		modem_number++;
		if(!isactive)
		{
			active_modem_id = 5;
			isactive = true;
		}
		META_LOG("[Meta] modem[5] is enable");
	}

	META_LOG("[Meta] modem_type = %d, modem_number = %d, active_modem_id = %d", modem_type, modem_number, active_modem_id);

	m_mdmType = modem_type;
	m_mdmNumber = modem_number;
	m_activeMdmId = active_modem_id;
	
	return modem_number;		
}


void Context::setLogLevel(unsigned int level)
{
	m_logLevel = level;
}
unsigned int Context::getLogLevel()
{
	return m_logLevel;
}

unsigned int Context::getMdmType()
{
	return m_mdmType;
}


unsigned int Context::getActiveMdmId()
{
	return m_activeMdmId;
}


unsigned int Context::getMdmNumber()
{
	return m_mdmNumber;		
}

signed int Context::getModemHandle(unsigned short id) 
{
	Modem *md = getModem(id);
	if(md != NULL)
		return md->getDevHandle();

	return -1;
}

int Context::getDataChannelType()
{
	return m_mdDataChannel;
}

unsigned int Context::dumpData(const unsigned char* con, int length)
{
	META_LOG("[Meta] Dump data is:  ");
	int i = 0;
	for(i = 0; i < length; i++)
		printf(" (%02x) ",con[i]);
	META_LOG("[Meta] Dump finished!");
	return 0;


}

unsigned int Context::dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow)
{

	if(getLogLevel() || getPropValue("persist.vendor.meta.dumpdata") == 1)
	{
		int i = 0;
		unsigned int j = 0;
		unsigned int rowLength = 3 * bytesPerRow + 1;
		unsigned char hex[rowLength];
		unsigned char high;
		unsigned char low;
		META_LOG("[Meta] Dump begin!");
		for(i = 0; i < length; i++)
		{
			high = (con[i] >> 4);
			low = (con[i] & 0x0f);
		
			if(high < 0x0a)
         	   high += 0x30;
        	else
         	   high += 0x37;
        
        	if(low < 0x0a)
        	    low += 0x30;
        	else
         	   low += 0x37;
        
        	hex[j++] = high;
        	hex[j++] = low;
        	hex[j++] = ' ';

			if (j == rowLength - 1 || i == length - 1)
			{
				hex[j] = '\0';
				j = 0;
				META_LOG("%s", hex);
			}
		}

		META_LOG("[Meta] Dump finished!");	
	}
	
	return 0;
}


unsigned int Context::getFileSize(int fd)
{
	struct stat file_stat;
	if(fstat(fd, &file_stat) < 0)
	{
		return 0;
	}
	else
	{
		return (unsigned int)file_stat.st_size;
	}
}


const char* Context::makepath(unsigned char file_ID)
{
	if(access("/data/nvram/AllMap",F_OK)==0)
	{
	    META_LOG("[Meta] /data/nvram/AllMap exist");
		if(file_ID == 0)
		{
			return "/data/nvram/AllMap";
		}
		else if(file_ID == 1)
		{
			return "/data/nvram/AllFile";	
		}
		else
		{
			META_LOG("[Meta] makepath error: invalid file_ID %d! ", file_ID);
			return "";
		}
	}
	else
	{
		META_LOG("[Meta] /data/nvram/AllMap not exist");
		if(file_ID == 0)
		{
			return "/mnt/vendor/nvdata/AllMap";
		}
		else if(file_ID == 1)
		{
			return "/mnt/vendor/nvdata/AllFile";	
		}
		else
		{
			META_LOG("[Meta] makepath error: invalid file_ID %d! ", file_ID);
			return "";
		}
	}
}

void Context::destroyAllModemThread()
{
	if(getMDThreadFlag()==1)
	{
		setMDThreadFlag(0);
	}
	else
	{
		META_LOG("[Meta] No MD thread!");
		return;
	}
	unsigned int modemType = getMdmType();
	
	if((modemType & MD1_INDEX) == MD1_INDEX)
	{
		META_LOG("[Meta] DestroyModemThread 0");
		destroyModemThread(0);
	}

	if((modemType & MD2_INDEX) == MD2_INDEX)
	{
		META_LOG("[Meta] DestroyModemThread 1");
		destroyModemThread(1);
	}

#ifdef TST_C2K_SUPPORT
	if((modemType & MD3_INDEX) == MD3_INDEX)
	{
		META_LOG("[Meta] DestroyModemThread 2");
		destroyModemThread(2);
	}
#endif

	if((modemType & MD5_INDEX) == MD5_INDEX)
	{
		META_LOG("[Meta] DestroyModemThread 4");
		destroyModemThread(4);
	}
	
}
void Context::createSerPortThread()
{
	SerPort *pPort = NULL;
	
	pPort = createSerPort();
	
	if (pPort != NULL)
	{
		pPort->pumpAsync(&hostSerPortRx);
	}
	else
	{
		META_LOG("[Meta] Enter meta_tst normal mode init fail");
	}

}
void Context::destroySerPortThread()
{
	SerPort *pPort = NULL;
	pPort = getSerPort();
	pPort->setExitFlag(1);
	destroySerPort();
}
void Context::createAllModemThread()
{
	if(getMDThreadFlag()==0)
	{
		setMDThreadFlag(1);
	}
	else
	{
		META_LOG("[Meta] Alread created MD thread");
		return;
	}
	unsigned int modemType = getMdmType();
	
	if((modemType & MD1_INDEX) == MD1_INDEX)
	{
		META_LOG("[Meta] CreateAllModemThread 0");
		createModemThread(0,0);
		Modem *p = getModem(0);
		if(p!=NULL)
		{
			META_LOG("[Meta] P is not NULL");	
		}
		else
		{
			META_LOG("[Meta] P is NULL");	
		}
	}

	if((modemType & MD2_INDEX) == MD2_INDEX)
	{
		META_LOG("[Meta] CreateAllModemThread 1");
		createModemThread(1,0);
	}

#ifdef TST_C2K_SUPPORT
	if((modemType & MD3_INDEX) == MD3_INDEX)
	{
		META_LOG("[Meta] CreateAllModemThread 3");
		createModemThread(2,0);
	}
#endif

	if((modemType & MD5_INDEX) == MD5_INDEX)
	{
		META_LOG("[Meta] CreateAllModemThread 4");
		createModemThread(4,0);
	}
}

void Context::destroyModemThread(unsigned short modemIndex)
{
	Modem *p = getModem(modemIndex);
	
	if(p!=NULL)
	{
		p->popUpAsync();
		destroyModem(p);
		META_LOG("[Meta] DestroyModemThread success modemIndex = %d" ,modemIndex);		
	}
	else
	{
		META_LOG("[Meta] DestroyModemThread fail");		
	}
	
		
}

void Context::createModemThread(unsigned short modemIndex, int usbUsb)
{
	Modem *pMdHandle = NULL;	
	MdRxWatcher *pRxWatcher = NULL;
	char dev_node[32] = {0};
	pRxWatcher = new MdRxWatcher(modemIndex);
	if(usbUsb == 1)
	{
		snprintf(dev_node, 32, "%s",USB_EXTERNAL_PATH);
	}
	else 
	{
		if(getMDMode()==1)
		{
			META_LOG("[Meta] Modem normal mode ");
			#ifdef TST_C2K_SUPPORT
			if(modemIndex == 2)
			{
				snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_C2K_AT,(CCCI_MD)modemIndex));
			}
			else
			{
				snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA,(CCCI_MD)modemIndex));	
			}
			#else
			       snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA,(CCCI_MD)modemIndex));
			#endif
			
		}
		else
		{
			#ifdef TST_C2K_SUPPORT	
			if(modemIndex == 2)
			{
		    	META_LOG("[Meta] To get c2k modem path!");
				#ifndef MTK_ECCCI_C2K
					META_LOG("[Meta] Modem meta mode TST_C2K_SUPPORT = yes MTK_ECCCI_C2K = yes");
					snprintf(dev_node, 32, "%s", viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_ETS));
				#else
					META_LOG("[Meta] Modem meta mode TST_C2K_SUPPORT = yes MTK_ECCCI_C2K = no");
			    	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_META_DATA,(CCCI_MD)modemIndex));
				#endif
	    	}
			else
			#endif
			{
				snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_META_DATA,(CCCI_MD)modemIndex));
			}
	    }
	}
	
	META_LOG("[Meta][CreateModem] dev_node = %s ", dev_node);
	getModemProtocol(modemIndex, &m_modem_cap_list);
	if((m_mdDataChannel == 1) && (modemIndex == 0))
	{
		if(META_BOOT == getBootMode())
		{
		    notifyModemDoRF(modemIndex);
		}
#ifndef ATM_PCSENDAT_SUPPORT
		else if(NORMAL_BOOT == getBootMode())
	    {
		    //notifyModemDoRFByATCI();
	    }
#endif
		pMdHandle = createModem(modemIndex);
	}
	else
	{
	    pMdHandle = createModem(dev_node, modemIndex);
    }
	if(pMdHandle != NULL)
	{
	    pMdHandle->pumpAsync(pRxWatcher);
	}
	
}

void Context::setActiveATModem(unsigned int activeATModemId)
{
	m_activeATModemId = activeATModemId;			
}

unsigned int Context::getActiveATModem()
{
	return m_activeATModemId;
}

int Context::getModemHwVersion(unsigned short modemIndex)
{
	if(MODEM_END == m_modem_hw_version)
	{
		if(-1 != queryModemProtocol(modemIndex))
		{
			META_LOG("[Meta][Protocol] get MD%d protocol, modem_protocol:%s",(modemIndex+1),m_modemProtocol);
			if(strcmp(m_modemProtocol,"AP_TST") == 0)
			{
				m_modem_hw_version = MODEM_AP_TST;
			}
			else if(strcmp(m_modemProtocol,"DHL") == 0)
			{
				m_modem_hw_version = MODEM_DHL;
			}
			else if(strcmp(m_modemProtocol,"6292") == 0)
			{
				m_modem_hw_version = MODEM_6292;	
			}
			else if(strcmp(m_modemProtocol,"6293") == 0)
			{
				m_modem_hw_version = MODEM_6293;
			}
			else if(strcmp(m_modemProtocol,"6295") == 0)
			{
				m_modem_hw_version = MODEM_6295;
			}
			else if(strcmp(m_modemProtocol,"6297") == 0)
			{
				m_modem_hw_version = MODEM_6297;
			}
			else
			{
				m_modem_hw_version = MODEM_6293;
				META_LOG("[Meta][Protocol] Set m_modem_hw_version as default value MODEM_6293"); 	
			}
		}
	}

	return m_modem_hw_version;
}
	
int Context::getModemProtocol(unsigned short modemIndex, MODEM_CAPABILITY_LIST_CNF* modem_capa)
{
	int nRet = 0;

	if(modem_capa == NULL)
	{
		META_LOG("[Meta][Protocol] parameter error ");
		return nRet;
	}

	if(modemIndex == 2)//C2K
	{
	 	modem_capa->modem_cap[modemIndex].md_service = FT_MODEM_SRV_ETS;
	#ifdef TST_C2K_SUPPORT	
		modem_capa->modem_cap[modemIndex].ch_type = FT_MODEM_CH_TUNNELING;
	#else
		modem_capa->modem_cap[modemIndex].ch_type    = FT_MODEM_CH_NATIVE_ETS;
	#endif
		META_LOG("[Meta][Protocol] modem_cap[%d]%d,%d",modemIndex,modem_capa->modem_cap[modemIndex].md_service,modem_capa->modem_cap[modemIndex].ch_type);
		nRet = 1;
	}
	else
	{
		if(-1 != queryModemProtocol(modemIndex))
		{
			META_LOG("[Meta][Protocol] get MD%d protocol, modem_protocol:%s",(modemIndex+1),m_modemProtocol);
			if(strcmp(m_modemProtocol,"AP_TST") == 0)
			{
				modem_capa->modem_cap[modemIndex].md_service = FT_MODEM_SRV_TST;
				modem_capa->modem_cap[modemIndex].ch_type = FT_MODEM_CH_NATIVE_TST;
				nRet = 1;
			}
			else if((strcmp(m_modemProtocol,"DHL") == 0)||(strcmp(m_modemProtocol,"6292") == 0))
			{
				modem_capa->modem_cap[modemIndex].md_service = FT_MODEM_SRV_DHL;
				modem_capa->modem_cap[modemIndex].ch_type = FT_MODEM_CH_TUNNELING;
				nRet = 1;
			}
			else if((strcmp(m_modemProtocol,"6293") == 0) || (strcmp(m_modemProtocol,"6295") == 0) || (strcmp(m_modemProtocol,"6297") ==0))
			{
				modem_capa->modem_cap[modemIndex].md_service = FT_MODEM_SRV_DHL;
				modem_capa->modem_cap[modemIndex].ch_type = FT_MODEM_CH_TUNNELING;
				m_mdDataChannel = 1;
				nRet = 1;
			}
			else
			{
				modem_capa->modem_cap[modemIndex].md_service = FT_MODEM_SRV_DHL;
				modem_capa->modem_cap[modemIndex].ch_type = FT_MODEM_CH_TUNNELING;
				m_mdDataChannel = 1;
				nRet = 1;
				META_LOG("[Meta][Protocol] Set modem_cap as default value");	
			}
			META_LOG("[Meta][Protocol] modem_cap[%d]%d,%d",modemIndex,modem_capa->modem_cap[modemIndex].md_service,modem_capa->modem_cap[modemIndex].ch_type);
		}
		else
		{
			META_LOG("[Meta][Protocol] MD%d ioctl CCCI_IOC_GET_MD_PROTOCOL_TYPE fail, errno=%d",(modemIndex+1),errno);
		}
	}
	return nRet;
}

FT_MODEM_CH_TYPE Context::getMDChType(unsigned short modemIndex)
{
	return m_modem_cap_list.modem_cap[modemIndex].ch_type;
}

unsigned int Context::getPropValue(const char *key)
{
    char tempstr[128]={0};
    property_get(key,tempstr,"0");
	if(tempstr[0] == '1')
	    return 1;
	else
		return 0;
}

unsigned int Context::getMDMode(void)
{
	return m_modem_boot_mode;
}
void Context::setMDMode(unsigned int modem_boot_mode)
{
	m_modem_boot_mode = modem_boot_mode; 
}

int Context::getIOCPort(unsigned int nModemIndex,int & bDataDevice)
{
	int fd = -1;
	char dev_node[32] = {0};
	
	if((nModemIndex == 0) || (nModemIndex == 1) || (nModemIndex == 4 && ccci_get_version() == EDSDA))
	{
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_META_IOCTL,(CCCI_MD)nModemIndex));
	    fd = open(dev_node, O_RDWR|O_NOCTTY|O_NDELAY );
		bDataDevice = 0;
	}
	else
	{
	    unsigned short id = getActiveMdmId() - 1; 
		fd= getModemHandle(id);
		bDataDevice = 1;
	}

	if(fd< 0)
	{
		META_LOG("[Meta]Open MD%d device note %s fail errno = %d",(nModemIndex+1),dev_node,errno);	
	}

	return fd;
}

int Context::queryModemProtocol(unsigned short modemIndex)
{
	int fd = -1;
	int nRet = -1;
	META_LOG("[DEBUG] m_modemProtocol[0] = %c",m_modemProtocol[0]);
	if(0 == m_modemProtocol[0])
	{
		int bDataDevice = 0;

		fd = getIOCPort(modemIndex,bDataDevice);

		if(fd >= 0 && 0 == ioctl(fd, CCCI_IOC_GET_MD_PROTOCOL_TYPE, m_modemProtocol))
		{
			META_LOG("[Meta][Protocol] get MD%d protocol, modem_protocol:%s",(modemIndex+1),m_modemProtocol);
			nRet = 0;
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
	}
	else
	{
		nRet = 0;
	}
	return nRet;
}

void Context::setMDThreadFlag(int modemThreadFlag)
{
	m_modemThreadFlag = modemThreadFlag;
	META_LOG("[Meta] Set MD Thread Flag = %d",m_modemThreadFlag);
}

int Context::getMDThreadFlag()
{
	return m_modemThreadFlag;
}

unsigned int Context::checkMdStatus()
{
	char status[128]={0};
    int ret = 0;
    property_get("vendor.mtk.md1.status",status, "0");

	META_LOG("[Meta] modem status = %s", status);
	if(0 ==	strncmp(status, "ready", 5)) //ccb owner tell us to check this property.
		ret = 1;
	else if(0 == strncmp(status, "exception", 9))
		ret = 2;

	return ret;
}

void Context::setATRespFlag(int atFlag)
{
	m_atFlag = atFlag;	
}

int Context::getATRespFlag()
{
	return m_atFlag;
}

void Context::setCurrentMdMode(int mdMode)
{
	m_currentMdMode = mdMode;
}

int Context::notifyModemDoRFByATCI()
{
	META_LOG("[Meta] notifyModemDoRFByATCI");
	if(0 == ChangeModemMode(2))
	{
	    setMDMode(2);//normal= 1 meta=2
	}
	return 0;
}

int Context::ChangeModemMode(int mode)  // 1:modem to normal  2:modem to meta
{
	unsigned int nRetry = 0;
	int nMdStatus = 0;
	int nCurrentMdmode = 0;

	META_LOG("[Meta] Enter ChangeModemMode");
	META_LOG("[Meta] To wait modem ready");
	do
	{
		nMdStatus = checkMdStatus();
		if( 2 == nMdStatus) //Modem exception, return -1 immediatly
		{
			return -1;
		}
		usleep(100*1000);
		nRetry++;
		if(nRetry > 100)  //return if it takes more than 10 seconds
		    return -1;
	}while(0 == nMdStatus);

	MSocket *pSocket = getSocket(SOCKET_ATCI_SERVER);
	if(pSocket == NULL)
	{
		pSocket = createSocket(SOCKET_ATCI_SERVER);
		if(pSocket != NULL)
		{
			int bInit = pSocket->initClient("adb_atci_socket", ANDROID_SOCKET_NAMESPACE_RESERVED);
			if(bInit == 0)
			{
				delSocket(SOCKET_ATCI_SERVER);
				return -1;
			}
		}
		else
			return -1;
	}

	META_LOG("[META] Check Modem ready by send AT");

	setATRespFlag(1);
	pSocket->send_msg("AT\r");
	nRetry = 0;
	while(getATRespFlag()!=0)
	{
		usleep(100*1000);
		if(getATRespFlag()==-1)
			pSocket->send_msg("AT\r");		
        nRetry++;
		if(nRetry > 100)    //return if it takes more than 10 seconds
		    return -1;
	}

	META_LOG("[META] Check sim status");
	char strSimStatus[128] = {0};
	property_get("persist.vendor.radio.simswitch",strSimStatus,"unknown");
	nRetry = 0;
	if((strcmp(strSimStatus,"1") == 0) || (strcmp(strSimStatus,"unknown") == 0))
	{
		META_LOG("[META] Sim1 is the main slot, need to send AT+ESUO=4 first");
		setATRespFlag(1);
		pSocket->send_msg("AT+ESUO=4\r");	

		while(getATRespFlag()!=0)
		{
			usleep(100*1000);
			nRetry++;
			if(nRetry > 100)    //return if it takes more than 10 seconds
				return -1;
		}
	}
	else if(strcmp(strSimStatus,"2") == 0)
	{
		META_LOG("[META] Sim2 is the main slot, need to send AT+ESUO=5 first");
		setATRespFlag(1);
		pSocket->send_msg("AT+ESUO=5\r");	

		while(getATRespFlag()!=0)
		{
			usleep(100*1000);
			nRetry++;
			if(nRetry > 100)    //return if it takes more than 10 seconds
				return -1;
		}
	}

   META_LOG("[Meta] Query current modem mode");
	setCurrentMdMode(0);
	pSocket->send_msg("AT+EMETACFG?\r");
	nRetry = 0;
	while(1)
	{
		if(1 == m_currentMdMode || 2 == m_currentMdMode) 
			break;
		usleep(100*1000);
		nRetry++;
		if(nRetry > 100)    //return if it takes more than 10 seconds
			return -1;
	}
	META_LOG("[Meta] Current modem mode = %d", m_currentMdMode);
	if(m_currentMdMode == mode)  //No need to switch modem mode, return 0 directly
	{
		META_LOG("[Meta] No need to switch modem mode");
		return 0;
	}

	META_LOG("[META] Switch modem mode to %d", mode);
	setATRespFlag(1);
	if(mode == 1)
	{
		pSocket->send_msg("AT+EMETACFG=0\r");
		META_LOG("[Meta] Send AT+EMETACFG=0");
	}
	else if(mode == 2)
	{
		pSocket->send_msg("AT+EMETACFG=1\r");
		META_LOG("[Meta] Send AT+EMETACFG=1");
	}
	else
	{
		META_LOG("[Meta]Invalid mode = %d",mode);
		return -1;
	}
	nRetry = 0;
	while(getATRespFlag()!=0)
	{
		usleep(100*1000);
		nRetry++;
		if(nRetry > 100)    //return if it takes more than 10 seconds
			return -1;
	}

	META_LOG("[META]AT+EMETACFG return OK, switch modem success");
	return 0;
}

void Context::writeBootprof(char * str)
{
	FILE *pBootProfFile = NULL;
    pBootProfFile = fopen("proc/bootprof","w");
	if(pBootProfFile!=NULL)
	{
		 fputs(str,pBootProfFile);
		 fclose(pBootProfFile);
		 pBootProfFile = NULL;
		 META_LOG("[META] write proc/bootprof success");
	}
	else
	{
		META_LOG("[META] open proc/bootprof fail!");
	}
}

int Context::notifyModemDoRF(int mdIdx)
{
	META_LOG("[Meta] To wait modem ready");
	char dev_node[32] = {0};
	const char *cmd = "AT+EMETACFG=1\r";
	const char *urc = "+EIND: 128";
	const char *rsp = "OK";

	while(0 == checkMdStatus())
	{
		META_LOG("[Meta] To check modem status before open USR_MUXD_DATA");
		usleep(100*1000);
	}

	//write bootprof
	writeBootprof((char *)"[META] checkMdStatus mtk.md1.status=ready.");

	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA,(CCCI_MD)mdIdx));
    signed int fd = open(dev_node, O_RDWR|O_NOCTTY);
	if(fd < 0)
	{
		META_LOG("[Meta] Can't open CCCI MUXD channel: %s", dev_node);
		return 0;	
	}
  
	META_LOG("[Meta] open CCCI MUXD channel: %s", dev_node);
	META_LOG("[Meta] To read modem URC");
	waitMdResponse(fd, urc);

	writeBootprof((char *)"[META] Wait Modem urc +EIND: 128");
	
	//Send AT command to modem
	int len = write(fd, cmd, strlen(cmd));
	META_LOG("[Meta] Send AT command - len = %d, cmd = %s", len, cmd);
	META_LOG("[Meta] To read AT response");
	waitMdResponse(fd, rsp);

	writeBootprof((char *)"[META] AT+EMETACFG=1 return OK.");

	close(fd);
    META_LOG("[Meta] Close CCCI MUXD channel. fd = %d", fd);

	return 0;
	
}

void Context::waitMdResponse(int fd, const char *rsp)
{
	char szbuf[1024] = {0};
	char data[33] = {0};
	int  len = 0;
	int  totalLen = 0;

	while(1)
	{
		len = ::read(fd, data, 32);
		if(len > 0)
		{	
			META_LOG("[Meta] read data from AT channel: len= %d,  %s", len, data);
			dumpDataInHexString((const unsigned char*)data,len, 16); 
			strncpy(szbuf+totalLen, data, len);
			totalLen += len;
			if(totalLen >= 1024)
			{
				totalLen = 0;
				META_LOG("[Meta] Modem response too long");
			}

			if( NULL != strstr(szbuf, rsp))
				break;
		}
		memset(data, 0, 33);
	}
	META_LOG("[Meta] Read AT response sucess");
}

MSocket* Context::createSocket(SOCKET_TYPE type)
{
	for(int i=0; i<SOCKET_END; i++)
	{
		if(m_socket[i] == NULL)
		{
			switch(type)
			{
				case SOCKET_MDLOGGER:
				case SOCKET_MOBILELOG:
				case SOCKET_CONNSYSLOG:
					m_socket[i]  = (MSocket*)new MLogSocket(type);	
					break;
				case SOCKET_ATCI_CLIENT:
					m_socket[i]  = (MSocket*)new MATCIClientSocket(type);	
					break;
				case SOCKET_ATCI_SERVER:
					m_socket[i]  = (MSocket*)new MATCIServerSocket(type);	
					break;
				case SOCKET_ATM_COMM:
					m_socket[i]  = (MSocket*)new MATMSocket(type);	
					break;
				default:
					return NULL;
			}
			
			if(m_socket[i] != NULL)
	        {
				//META_LOG("[Meta][Socket] Create socket success. idx=%d, pSocket=0x%08x, type=%d", i, m_socket[i], m_socket[i]->m_type);
				return m_socket[i];
	        }
	        else
	        {
		        META_LOG("[Meta][Socket] Create socket fail.");
				return NULL;
			}
		}
	}
	
	META_LOG("[Meta][Socket] no empty socket object!");
	return NULL;

}

MSocket* Context::getSocket(SOCKET_TYPE type)
{
	
	META_LOG("[Meta][Socket] To get socket object!");
	for(int i=0; i<SOCKET_END; i++)
	{
		if(m_socket[i] != NULL)
		{
			//META_LOG("[Meta][Socket] Get socket,  idx=%d, pSocket=0x%08x, type=%d", i, m_socket[i], m_socket[i]->m_type);		
			if(m_socket[i]->m_type == type)
				return m_socket[i];
		}
	}

	META_LOG("[Meta][Socket] Can not find socket object!");
	return NULL;	
}

void Context::delSocket(SOCKET_TYPE type)
{
	META_LOG("[Meta][Socket] To delete socket object!");
	for(int i=0; i<SOCKET_END; i++)
	{
		if(m_socket[i] != NULL)
		{
			//META_LOG("[Meta][Socket] Get socket,  idx=%d, pSocket=0x%08x, type=%d", i, m_socket[i], m_socket[i]->m_type);		
			if(m_socket[i]->m_type == type)
			{
				free(m_socket[i]);
				m_socket[i] = NULL;
			}
		}
	}

	return;
}
void Context::destroyVirtualRxThread()
{
	SerPort *pPort = getSerPort();
	if(pPort!=NULL)
	{
        pPort->setExitFlag(1);
        pPort->setSerPortExitFlag();  //Stop socket connect while loop
        pPort->waitForThreadExit();   //Sync virtual rx thread with main thread before destroy
		//destroyPortHandle();
		destroySerPort();
		META_LOG("[Meta] destroyVirtualRxThread success" );
	}
	else
	{
		META_LOG("[Meta] destroyVirtualRxThread fail");
	}
}

//For USB,UART,SOCKET, etc.
void Context::createVirtualRxThread()
{
    SerPort *pPort = createSerPort();
	UsbRxWatcher *pVirtualRxWatcher = getVirtualRxWatcher();

	if (pPort != NULL && pVirtualRxWatcher != NULL)
	{
	    META_LOG("[Meta] createVirtualRxThread success");
		pPort->pumpAsync(pVirtualRxWatcher);
	}
	else
	{
		META_LOG("[Meta] createVirtualRxThread fail");
	}
}

UsbRxWatcher * Context::getVirtualRxWatcher() const
{
	return m_virtualRxWatcher;
}

void Context::setVirtualRxWatcher(UsbRxWatcher * virtualRxWatcher)
{
	m_virtualRxWatcher = virtualRxWatcher;
}

void Context::queryNormalModeTestFlag()
{
	m_normalModeTestFlag = getProductInfo(0, OFFSET_ATM);
}

int Context::getNormalModeTestFlag()
{
	return m_normalModeTestFlag;
}

int Context::setNormalModeTestFlag(int flag)
{
   return modifyProductInfo(0, flag, OFFSET_ATM);
}

int Context::modifyProductInfo(int type, int flag, int offset)
{
	int fd = -1;
    int result = 0;
    int cipherText = 0;
    char write_buf[1024] = {0};
	char log_flag = 0;

	META_LOG("[Meta][FT] setProductInfo flag = %d", flag);

	if(type == 0) //ATM flag
    {
		cipherText = encrypt((KEY1+flag), KEY2);
		META_LOG("[Meta][FT] setProductInfo after encrypt, flag = %d", cipherText);
		sprintf(write_buf, "%d", cipherText);
	}
	
	fd = open(FLAG_PATH, O_RDWR);
	if(fd < 0)
	{
		META_LOG("[Meta][FT] setProductInfo open /proinfo fail, errno = %d", errno);
		return -1;
	} 
	else
	{
		lseek(fd, offset, SEEK_SET);

		if(type == 0) //ATM flag
			result = write(fd, write_buf, ENCRYPT_LENTH); 
		else if(type == 1) //meta log
		{
			log_flag = (char)flag;
			result = write(fd, &log_flag, 1);
		}

		if(result > 0)
		{
			META_LOG("[Meta][FT] setProductInfo write /proinfo success");
			close(fd);
			fd = NULL_FILE_DESCRIPTOR;
			return 0;
		}
		else
		{
			META_LOG("[Meta][FT] setProductInfo write /proinfo fail, errno = %d", errno);
			close(fd);
			fd = NULL_FILE_DESCRIPTOR;
			return -1;
		}
	}
}

int Context::setProductInfo(int type, int flag, int offset)
{
	return modifyProductInfo(type, flag, offset);
}

int Context::getProductInfo(int type, int offset)
{
    int fd = -1;
    char read_buf[1024] = {0};
    int result = 0;
    int val = -1;
    char log_val = -1;
    int ret = 0;
  
    fd = open(FLAG_PATH,O_RDWR);
    if(fd < 0)
    {
       META_LOG("[Meta][FT] queryProductInfo open /proinfo fail, errno = %d", errno);
       return val;
    }
    ret = lseek(fd, offset, SEEK_SET);
    if(ret == -1)
    {
        close(fd);
        return -1;
    }

    if(type == 0)
    {
    	result = read(fd, read_buf, ENCRYPT_LENTH);
	META_LOG("[Meta][FT] queryProductInfo read /proinfo success");
	META_LOG("[Meta][FT] queryProductInfo before decrypt, flag = %d", atoi(read_buf));
	if(result > 0)
	    val = decrypt(atoi(read_buf), KEY2) - KEY1;
    }
    else if(type == 1)
    {
	result = read(fd, &log_val, 1);
	val = log_val;
    }
	
    if(result > 0)
    {

        META_LOG("[Meta][FT] queryProductInfo flag = %d", val);
	}
	else
	{
		META_LOG("[Meta][FT] queryProductInfo read /proinfo fail, errno = %d", errno);
	}

	close(fd);
	fd = NULL_FILE_DESCRIPTOR;

	return val;
}

void Context::queryWifiPara(int argc, char** argv)
{	
    META_LOG("[Meta] Enter queryWifiPara");
    const char* wifi_opt = "a:p:";
    int opt = 0;

    while( (opt = getopt(argc, argv, wifi_opt)) != -1)
    {
        META_LOG("[Meta] queryWifiPara opt = %d", opt);
        switch(opt)
        {			
            case 'a':
                m_WifiPara.ip_addr = optarg;
                META_LOG("[Meta] queryWifiPara get ip address: %s", m_WifiPara.ip_addr);
                break;
            case 'p':
                m_WifiPara.port = atoi(optarg);
                META_LOG("[Meta] queryWifiPara get port: %d", m_WifiPara.port);
                break;
            default:
                META_LOG("[Meta] queryWifiPara invalid option!");
                break;
        }
    }

    META_LOG("[Meta] Exit queryWifiPara");
}

WIFI_PARA Context::getWifiPara()
{
    char strIpAddr[128] = {0};
    property_get("persist.vendor.atm.ipaddress", strIpAddr, "0,0,0,0");
    m_WifiPara.ip_addr = strIpAddr;
    m_WifiPara.port = 9000;
    return m_WifiPara;
}

int Context::writePortIndex() 
{
    int res = 0;
    int fd = ::open("/sys/class/android_usb/android0/f_acm/port_index", O_WRONLY);
    if (fd != -1)
    {
        res = ::write(fd,"1,4",4);
		if(res>0)
        {
            META_LOG("[Meta] writePortIndex /sys/class/android_usb/android0/f_acm/port_index 1,4 Success");
			close(fd);
			fd = NULL_FILE_DESCRIPTOR;
		    return 0;
		}
		else
		{
			META_LOG("[Meta] writePortIndex /sys/class/android_usb/android0/f_acm/port_index 1,4 Failed");
			close(fd);
			fd = NULL_FILE_DESCRIPTOR;
			return -1;
		}
    }
    else
    {
        META_LOG("[Meta] Failed to open:/sys/class/android_usb/android0/f_acm/port_index");
		return -1;
    }
         	
}

void Context::switchComType(META_COM_TYPE targetComType)
{
	if(getComType() != targetComType)
	{
		usbMutexLock(true);
		destroyVirtualRxThread();
		usleep(100*1000); //sleep 100 ms
		setComType(targetComType);
#ifndef MTK_ATM_METAWIFIONLY
		createVirtualRxThread();
#else
		if(META_SOCKET == targetComType)
		{
			createVirtualRxThread();
		}
#endif
		META_LOG("[Meta] Change connect type to %d", (int)targetComType);
		usbMutexLock(false);
	}
	else
	{
		META_LOG("[Meta] Connect type is already %d, no need to switch", (int)targetComType);
	}
}

void Context::SetDataCompressStatus(unsigned int enable)
{
	m_dataCompressStatus = enable;
}
unsigned int Context::GetDataCompressStatus()
{
	return m_dataCompressStatus;
}

void Context::HandleSocketCmd(char* socket_cmd)
{
	if(sizeof(socket_cmd) > 32) {
		META_LOG("[Meta] HandleSocketCmd: Invalid socket command: %s", socket_cmd);
		return;
	}
	META_LOG("[Meta] HandleSocketCmd: command: %s", socket_cmd);

	if(strstr(socket_cmd, "ATM_WIFI_INFO_IP_ADDRESS") != NULL)
	{
		string strcmd = string(socket_cmd);
		string strIP = strcmd.erase(0, strcmd.find_last_of(':')+1);
		property_set("persist.vendor.atm.ipaddress", strIP.c_str());
		META_LOG("[Meta] HandleSocketCmd: ipaddr: %s", strIP.c_str());
	}	
	else
	{
	if(0 == strcmp(socket_cmd, "ATM_SWITCH_META_TO_WIFI")) {
		switchComType(META_SOCKET);
			property_set("persist.vendor.meta.connecttype", "wifi");
	} else if(0 == strcmp(socket_cmd, "ATM_SWITCH_META_TO_USB")) {
		switchComType(META_USB_COM);
			property_set("persist.vendor.meta.connecttype", "usb");
	} else if(0 == strcmp(socket_cmd, "ATM_SWITCH_MODEM_TO_META")) {
		if(0 == ChangeModemMode(2))
		{
			META_LOG("[Meta] HandleSocketCmd: switch modem to META mode success");
			setMDMode(2);
			property_set("persist.vendor.atm.mdmode", "meta");
		}
		else
		{
			META_LOG("[Meta] HandleSocketCmd: switch modem to META mode fail");
		}
	} else if(0 == strcmp(socket_cmd, "ATM_SWITCH_MODEM_TO_NORMAL")) {
		if(0 == ChangeModemMode(1))
		{
			META_LOG("[Meta] HandleSocketCmd: switch modem to normal mode success");
			setMDMode(1);
			property_set("persist.vendor.atm.mdmode", "normal");
		}
		else
		{
			META_LOG("[Meta] HandleSocketCmd: switch modem to normal mode fail");
		}
	} else if(0 == strcmp(socket_cmd, "ATM_DESTORY_WIFI_SOCKET")) {
		destroyVirtualRxThread();
		setComType(META_UNKNOWN_COM);
	} else if(0 == strcmp(socket_cmd, "ATM_NEW_WIFI_SOCKET")) {
		setComType(META_SOCKET);
		createVirtualRxThread();
	} else if(0 == strcmp(socket_cmd, "ATM_REBOOT_MODEM")) {
		int bDataDevice = 0;
		int fd = getIOCPort(0, bDataDevice);
		if(fd >= 0)
		{
			if(0 == ioctl(fd, CCCI_IOC_MD_RESET))
			{
				setMDMode(1);
				property_set("persist.vendor.atm.mdmode", "normal");
				META_LOG("[Meta] HandleSocketCmd: reboot modem to normal mode success");
			}
			else
			{
				META_LOG("[Meta] HandleSocketCmd: reboot modem to normal mode fail");
			}
			if(FALSE == bDataDevice)
			{
				close(fd);
				META_LOG("[Meta] HandleSocketCmd: close fd: %d", fd);
				fd = -1;
			}
		}
		}

	}
}
//////////////////////////////////////////////////////////////////////////

void destroyModem(Modem *p)
{
	return Context::instance()->destroyModem(p);	
}

Modem * createModem(const char *ccci, unsigned short id)
{
	return Context::instance()->createModem(ccci, id);
}

Modem * createModem(unsigned short id)
{
	return Context::instance()->createModem(id);
}

CmdTarget * getModule(unsigned short id)
{
	return Context::instance()->getModule(id);
}

Modem * getModem(unsigned short id)
{
	return Context::instance()->getModem(id);
}

unsigned int dumpData(const unsigned char* con, int length)
{
	return Context::instance()->dumpData(con,length);
}

unsigned int dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow)
{
	return Context::instance()->dumpDataInHexString(con,length,bytesPerRow);
}

unsigned int getFileSize(int fd)
{
	return Context::instance()->getFileSize(fd);
}

const char* makepath(unsigned char file_ID)
{
	return Context::instance()->makepath(file_ID);
}

void destroyContext()
{
	return Context::instance()->destroy();
}

unsigned int getMdmType()
{
	return Context::instance()->getMdmType();
}

unsigned int getActiveMdmId()
{
	return Context::instance()->getActiveMdmId();
}

unsigned int getMdmNumber()
{
	return Context::instance()->getMdmNumber();
}

signed int getModemHandle(unsigned short id)
{
	return Context::instance()->getModemHandle(id);
}

void setLogLevel(unsigned int level)
{
	return Context::instance()->setLogLevel(level);	
}
unsigned int getLogLevel()
{
	return Context::instance()->getLogLevel();	
}

void destroyModemThread(unsigned short modemIndex)
{
	return Context::instance()->destroyModemThread(modemIndex);
}

void destroyAllModemThread()
{
	return Context::instance()->destroyAllModemThread();
}

void createAllModemThread()
{
	return Context::instance()->createAllModemThread();		
}


void createModemThread(unsigned short modemIndex,int usbUsb)
{
	return Context::instance()->createModemThread(modemIndex,usbUsb);	
}

void createSerPortThread()
{
	return Context::instance()->createSerPortThread();
}

void destroySerPortThread()
{
	return Context::instance()->destroySerPortThread();	
}


int getModemProtocol(unsigned short modemIndex, void* modem_capa)
{
    return Context::instance()->getModemProtocol(modemIndex, (MODEM_CAPABILITY_LIST_CNF*)modem_capa);
}

int getMDChType(unsigned short modemIndex)
{
    return Context::instance()->getMDChType(modemIndex);
}

unsigned int getMDMode(void)
{
	return Context::instance()->getMDMode();;
}

void setMDMode(unsigned int modem_boot_mode)
{
	return Context::instance()->setMDMode(modem_boot_mode);
}

void setActiveATModem(unsigned int activeATModemId)
{
	return Context::instance()->setActiveATModem(activeATModemId);			
}

unsigned int getActiveATModem()
{
	return Context::instance()->getActiveATModem();	
}

int getIOCPort(unsigned int nModemIndex,int & bDataDevice)
{
	return Context::instance()->getIOCPort(nModemIndex,bDataDevice);
}

int getBootMode()
{
	return Context::instance()->getBootMode();
}

void queryNormalModeTestFlag()
{
	return Context::instance()->queryNormalModeTestFlag();
}

int getNormalModeTestFlag()
{
	return Context::instance()->getNormalModeTestFlag();
}

int setNormalModeTestFlag(int flag)
{
	return Context::instance()->setNormalModeTestFlag(flag);
}

int setProductInfo(int type, int flag, int offset)
{
	return Context::instance()->setProductInfo(type, flag, offset);
}

int getProductInfo(int type, int offset)
{
	return Context::instance()->getProductInfo(type, offset);
}

int getDataChannelType()
{
	return Context::instance()->getDataChannelType();
}

int getPropValue(const char *key)
{
	return Context::instance()->getPropValue(key);
}

MSocket * createSocket(unsigned int  type)
{
	return Context::instance()->createSocket((SOCKET_TYPE)type);
}

MSocket * getSocket(unsigned int  type)
{
	return Context::instance()->getSocket((SOCKET_TYPE)type);
} 

void delSocket(unsigned int  type)
{
	return Context::instance()->delSocket((SOCKET_TYPE)type);
}
void destroyVirtualRxThread()
{
	return Context::instance()->destroyVirtualRxThread();
}

void createVirtualRxThread()
{
	return Context::instance()->createVirtualRxThread();
}

void setVirtualRxWatcher(UsbRxWatcher * virtualRxWatcher)
{
	return Context::instance()->setVirtualRxWatcher(virtualRxWatcher);
}

int getLoadType()
{
	return Context::instance()->getLoadType();
}

int getModemHwVersion(unsigned short modemIndex)
{
	return Context::instance()->getModemHwVersion(modemIndex);
}

void queryWifiPara(int argc, char** argv)
{
    return Context::instance()->queryWifiPara(argc, argv);
}

WIFI_PARA getWifiPara()
{
    return Context::instance()->getWifiPara();
}

void setATRespFlag(int atFlag)
{
	return Context::instance()->setATRespFlag(atFlag);
}

int getATRespFlag()
{
	return Context::instance()->getATRespFlag();
}

int notifyModemDoRFByATCI()
{
	return Context::instance()->notifyModemDoRFByATCI();
}

int ChangeModemMode(int mode)
{
	return Context::instance()->ChangeModemMode(mode);
}

int writePortIndex()
{
	return Context::instance()->writePortIndex();
}

void writeBootprof(char * str)
{
	return Context::instance()->writeBootprof(str);
}


void setCurrentMdMode(int mdMode)
{
	return Context::instance()->setCurrentMdMode(mdMode);
}

void HandleSocketCmd(char* socket_cmd)
{
	return Context::instance()->HandleSocketCmd(socket_cmd);
}

int readSys_int(char const * path)
{
	return Context::instance()->readSys_int(path);
}

unsigned int checkMdStatus()
{
	return Context::instance()->checkMdStatus();

}

void SetDataCompressStatus(unsigned int enable)
{
	return Context::instance()->SetDataCompressStatus(enable);
}
unsigned int GetDataCompressStatus()
{
	return Context::instance()->GetDataCompressStatus();
}


