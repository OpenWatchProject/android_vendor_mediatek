#ifndef _FT_MODULE_H_
#define _FT_MODULE_H_
#include "DriverInterface.h"
#include "CmdTarget.h"
#include "mlist.h"
#include <vector>


class FtModWifi : public CmdTarget
{
public:
	FtModWifi(void);
	virtual ~FtModWifi(void);

public:
	virtual void exec(Frame*);
};

class FtModGPS : public CmdTarget
{
public:
	FtModGPS(void);
	virtual ~FtModGPS(void);

public:
	virtual void exec(Frame*);
};


class FtModNFC : public CmdTarget
{
public:
	FtModNFC(void);
	virtual ~FtModNFC(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};


class FtModBT : public CmdTarget
{
public:
	FtModBT(void);
	virtual ~FtModBT(void);

public:
	virtual void exec(Frame*);
};

class FtModFM : public CmdTarget
{
public:
	FtModFM(void);
	virtual ~FtModFM(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};

class FtModAudio : public CmdTarget
{
public:
	FtModAudio(void);
	virtual ~FtModAudio(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};

class FtModCCAP : public CmdTarget
{
public:
	FtModCCAP(void);
	virtual ~FtModCCAP(void);

public:
	virtual void exec(Frame*);
};


class FtModDRM : public CmdTarget
{
public:
	FtModDRM(void);
	virtual ~FtModDRM(void);

public:
	virtual void exec(Frame*);
};

class FtModGAMMA : public CmdTarget
{
public:
	FtModGAMMA(void);
	virtual ~FtModGAMMA(void);
public:
	virtual void exec(Frame*);
};

class FtModAttestationKey : public CmdTarget
{
public:
	FtModAttestationKey(void);
	virtual ~FtModAttestationKey(void);

public:
	virtual void exec(Frame*);
};

class FtModNvramBackup : public CmdTarget
{
public:
	FtModNvramBackup(void);
	virtual ~FtModNvramBackup(void);
	void covertArray2Vector(const char* in, int len, std::vector<uint8_t>& out);
	void covertVector2Array(std::vector<uint8_t> in, char* out);

private:	
	bool SendNVRAMFile(unsigned char file_ID, FT_NVRAM_BACKUP_CNF* pft_cnf);
	
public:
	virtual void exec(Frame*);
};

class FtModNvramRestore : public CmdTarget
{
public:
	FtModNvramRestore(void);
	virtual ~FtModNvramRestore(void);

public:
	virtual void exec(Frame*);

};

class FtModNvramReset : public CmdTarget
{
public:
	FtModNvramReset(void);
	virtual ~FtModNvramReset(void);

public:
	virtual void exec(Frame*);
};

class FtModNvramRead : public CmdTarget
{
public:
	FtModNvramRead(void);
	virtual ~FtModNvramRead(void);

public:
	virtual void exec(Frame*);
};


class FtModNvramWrite : public CmdTarget
{
public:
	FtModNvramWrite(void);
	virtual ~FtModNvramWrite(void);

public:
	virtual void exec(Frame*);
};


class FtModTestAlive : public CmdTarget
{
public:
	FtModTestAlive(void);
	virtual ~FtModTestAlive(void);

public:
	virtual void exec(Frame*);
};

class FtModVersionInfo : public CmdTarget
{
public:
	FtModVersionInfo(void);
	virtual ~FtModVersionInfo(void);

public:
	virtual void exec(Frame*);
};



class FtModVersionInfo2 : public CmdTarget
{
public:
	FtModVersionInfo2(void);
	virtual ~FtModVersionInfo2(void);

public:
	virtual void exec(Frame*);
};


class FtModChipInfo : public CmdTarget
{
public:
	FtModChipInfo(void);
	virtual ~FtModChipInfo(void);

public:
	virtual void exec(Frame*);
};


class FtModPowerOff : public CmdTarget
{
public:
	FtModPowerOff(void);
	virtual ~FtModPowerOff(void);

public:
	virtual void exec(Frame*);
	void checkUSBOnline();
	void rebootToRecovery();
	void closeUSB();

};


class FtModReboot : public CmdTarget
{
public:
	FtModReboot(void);
	virtual ~FtModReboot(void);

public:
	virtual void exec(Frame*);
};

class FtModMetaDisconnect : public CmdTarget
{
public:
	FtModMetaDisconnect(void);
	virtual ~FtModMetaDisconnect(void);

public:
	virtual void exec(Frame*);
};


class FtModBuildProp : public CmdTarget
{
public:
	FtModBuildProp(void);
	virtual ~FtModBuildProp(void);

public:
	virtual void exec(Frame*);
};



class FtModGSensor : public CmdTarget
{
public:
	FtModGSensor(void);
	virtual ~FtModGSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};

class FtModMSensor : public CmdTarget
{
public:
	FtModMSensor(void);
	virtual ~FtModMSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	FT_MSENSOR_CNF m_ft_cnf;
};

class FtModALSPS : public CmdTarget
{
public:
	FtModALSPS(void);
	virtual ~FtModALSPS(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	FT_ALSPS_CNF m_ft_cnf;
};

class FtModGyroSensor : public CmdTarget
{
public:
	FtModGyroSensor(void);
	virtual ~FtModGyroSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};


class FtModModemInfo : public CmdTarget
{
public:
	FtModModemInfo(void);
	virtual ~FtModModemInfo(void);
	int getModemCapability(MODEM_CAPABILITY_LIST_CNF* modem_capa);
	void rebootModem(FT_MODEM_REQ *req, FT_MODEM_CNF & ft_cnf, int fd);
	int getModemMode(FT_MODEM_REQ *req, FT_MODEM_CNF & ft_cnf, int fd);
	int getModemIndex(FT_MODEM_REQ *req);
	int getModemState(int *modem_state, int fd);
	int getModemType(int *modem_type, int fd);
	int setModemType(int modem_type, int fd);
#ifdef MTK_SINGLE_BIN_MODEM_SUPPORT
        int CopyMDDBFile(unsigned int nModemId);
#endif

public:
	virtual void exec(Frame*);

};


class FtModSIMNum : public CmdTarget
{
public:
	FtModSIMNum(void);
	virtual ~FtModSIMNum(void);

public:
	virtual void exec(Frame*);
};

class FtModSDcard : public CmdTarget
{
public:
	FtModSDcard(void);
	virtual ~FtModSDcard(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};

class FtModEMMC : public CmdTarget
{
public:
	FtModEMMC(void);
	virtual ~FtModEMMC(void);

public:
	virtual void exec(Frame*);
};

class FtModCRYPTFS : public CmdTarget
{
public:
	FtModCRYPTFS(void);
	virtual ~FtModCRYPTFS(void);

public:
	virtual void exec(Frame*);
};

class FtModADC : public CmdTarget
{
public:
	FtModADC(void);
	virtual ~FtModADC(void);

public:
	virtual void exec(Frame*);
};
#if 0
class FtModAUXADC : public CmdTarget
{
public:
	FtModAUXADC(void);
	virtual ~FtModAUXADC(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};



class FtModCPURegR : public CmdTarget
{
public:
	FtModCPURegR(void);
	virtual ~FtModCPURegR(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};

class FtModCPURegW : public CmdTarget
{
public:
	FtModCPURegW(void);
	virtual ~FtModCPURegW(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};

#endif


class FtModChipID : public CmdTarget
{
public:
	FtModChipID(void);
	virtual ~FtModChipID(void);

public:
	virtual void exec(Frame*);

};

class FtModCTP : public CmdTarget
{
public:
	FtModCTP(void);
	virtual ~FtModCTP(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	Touch_CNF m_ft_cnf;
};

class FtModGPIO : public CmdTarget
{
public:
	FtModGPIO(void);
	virtual ~FtModGPIO(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);
};

class FtModFileOperation : public CmdTarget
{
public:
	FtModFileOperation(void);
	virtual ~FtModFileOperation(void);

public:
	virtual void exec(Frame*);

private:
    mlist<FT_FILE_INFO*>		m_fileInfoList;
	unsigned m_nFileCount;
	unsigned int GetFileLen(char *pFilePath);
	int ListPath(unsigned char *pPath,unsigned char *pFileNameSubStr);
	void ClearFileInfoList(void);
	FT_FILE_INFO* GetFileInfo(unsigned int id);
	int SaveSendData(FILE_OPERATION_SENDFILE_REQ *req, char *peer_buff, unsigned short peer_len);
	int SetReceiveData(FILE_OPERATION_RECEIVEFILE_REQ *req, FT_FILE_OPERATION_CNF* pft_cnf);
};

class FtModRAT : public CmdTarget
{
public:
	FtModRAT(void);
	virtual ~FtModRAT(void);

public:
	void exec(Frame*);
protected:
	int init(Frame*);

};

class FtModMSIM : public CmdTarget
{
public:
	FtModMSIM(void);
	virtual ~FtModMSIM(void);

public:
	void exec(Frame*);
protected:
	int init(Frame*);

};


class FtModUtility : public CmdTarget
{
public:
	FtModUtility(void);
	virtual ~FtModUtility(void);
	void covertArray2Vector(unsigned char* in, int len, std::vector<uint8_t>& out);
	void covertVector2Array(std::vector<uint8_t> in, char* out);

public:
	void exec(Frame*);
	
};


class FtModCustomer : public CmdTarget
{
public:
	FtModCustomer(void);
	virtual ~FtModCustomer(void);

public:
	virtual void exec(Frame*);
};

class FtModSpecialTest : public CmdTarget
{
public:
	FtModSpecialTest(void);
	virtual ~FtModSpecialTest(void);

public:
	virtual void exec(Frame*);
};

class FtModTargetloggerCtrl : public CmdTarget
{
public:
	FtModTargetloggerCtrl(void);
	virtual ~FtModTargetloggerCtrl(void);

public:
	virtual void exec(Frame*);

private:
	unsigned int SwitchMdloggerMode(FT_TARGETLOG_CTRL_REQ *req);
	unsigned int	SwitchMobilelogMode(FT_TARGETLOG_CTRL_REQ *req);
	unsigned int SwitchConnsyslogMode(FT_TARGETLOG_CTRL_REQ *req);
	unsigned int TargetLogPulling(FT_TARGETLOG_CTRL_REQ *req);
	unsigned int GetTargetLogPullingStatus(FT_TARGETLOG_CTRL_REQ *req, FT_TARGETLOG_CTRL_CNF &cnf);
	unsigned int QueryMdloggerStatus(FT_TARGETLOG_CTRL_CNF &cnf);
	unsigned int GetLogPropValue(char *key);
	unsigned int SetModemLogFilter(FT_TARGETLOG_CTRL_REQ *req);
	void* GetLoggerSocket(unsigned int type, const char * service);
	unsigned int	QueryMdNormalLogPath(FT_TARGETLOG_CTRL_CNF &cnf);
	unsigned int QueryMdEELogPath(FT_TARGETLOG_CTRL_CNF &cnf);
    unsigned int	QueryMBLogPath(FT_TARGETLOG_CTRL_CNF &cnf);

};

class FtModAPDB : public CmdTarget
{
public:
    FtModAPDB(void);
    virtual ~FtModAPDB(void);

public:
    virtual void exec(Frame*);
};

class FtModSysEnv : public CmdTarget
{
public:
    FtModSysEnv(void);
    virtual ~FtModSysEnv(void);

public:
    virtual void exec(Frame*);
};

class FtModTargetClock : public CmdTarget
{
public:
    FtModTargetClock(void);
    virtual ~FtModTargetClock(void);

public:
    virtual void exec(Frame*);

private:
	unsigned int SetSysClock(SET_TARGET_CLOCK_REQ *req);
	unsigned int IsValidDate(SET_TARGET_CLOCK_REQ *req);

};


#endif	// _FT_MODULE_H_
