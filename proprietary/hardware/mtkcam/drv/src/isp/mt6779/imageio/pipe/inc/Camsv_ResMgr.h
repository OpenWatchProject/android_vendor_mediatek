#ifndef _CAMSV_RESMGR_H_
#define _CAMSV_RESMGR_H_

#include "ResMgr.h"

class Central_Camsv{
public:
    Central_Camsv(){
        m_available = CAMSV_MAX - CAMSV_START;
        for(MUINT32 i = 0 ; i < CAMSV_MAX - CAMSV_START; i++){
            m_module[i].occupied = MFALSE;
            *m_module[i].User = '\0';
        }
    }

    MBOOL           get_availNum(vector<ISP_HW_MODULE>& v_available, char Name[32]);
    MBOOL           Register(ISP_HW_MODULE module, char Name[32], MUINT32 logLevel = 1);
    MBOOL           Register(ISP_HW_MODULE module, char Name[32], MUINT32 logLevel = 1, MUINT32 mMipiPixrate = 0);
    MBOOL           Register(ISP_HW_MODULE module, char Name[32], MUINT32 logLevel, MUINT32 mMipiPixrate, vector<E_INPUT>& InOutPut);
    MBOOL           Release(ISP_HW_MODULE module);
    ISP_HW_MODULE   Register_search(ISP_HW_MODULE module, char Name[32]);
    ISP_HW_MODULE   Register_search(MUINT32 mMipiPixrate, char Name[32]);
    ISP_HW_MODULE   Register_search(MUINT32 mMipiPixrate, char Name[32], vector<E_INPUT>& InOutPut);
private:
            struct{
                char    User[32];
                MBOOL   occupied;
            }m_module[CAMSV_MAX - CAMSV_START];
            MUINT32 m_available;
    mutable Mutex   mLock;
};

class Camsv_ResMgr : public ResMgr{
friend class Central_Camsv;   //inorder to access internalName

private:
    Camsv_ResMgr(ISP_HW_MODULE master, char Name[32]);
public:
    //booking master camsv resource & get working obj
    static Camsv_ResMgr*    Res_Attach(ISP_HW_MODULE master, char Name[32]);
    static Camsv_ResMgr*    Res_Attach(MUINT32 mMipiPixrate, char Name[32]);
    static Camsv_ResMgr*    Res_Attach(MUINT32 mMipiPixrate, char Name[32], vector<E_INPUT>& InOutPut);
    //release master camsv's resource, including resources which r occupied by master cam
    virtual MBOOL           Res_Detach(void);

public:
    virtual MBOOL           Res_Meter(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam){return MFALSE;};
    virtual MINT32          Res_Recursive(V_CAM_THRU_PUT* pcfgIn){return -1;};

private:
    ISP_HW_MODULE           Res_GetCurrentModule(void);

private:
    static Mutex            mLock;
    static Central_Camsv    m_central;

    char                    m_User[32];
    char                    m_internalUser[24];
    static char             m_staticName[16];
};

#endif
