#ifndef _CAM_RESMGR_H_
#define _CAM_RESMGR_H_

#include "ResMgr.h"

class Central{
public:
    Central(){
        m_available = PHY_CAM;
        for(MUINT32 i=0;i<PHY_CAM;i++){
            m_module[i].occupied = MFALSE;
            *m_module[i].User = '\0';
        }
    }

    MBOOL           get_availNum(vector<ISP_HW_MODULE>& v_available,char Name[32]);
    MBOOL           Register(ISP_HW_MODULE module,char Name[32],MUINT32 logLevel = 1);
    MBOOL           Release(ISP_HW_MODULE module);
    ISP_HW_MODULE   Register_search(MUINT32 width,char Name[32]);
private:
            struct{
                char    User[32];
                MBOOL   occupied;
            }m_module[PHY_CAM];
            MUINT32 m_available;
    mutable Mutex   mLock;
};

struct T_ResMgr_SEN
{
public:
    MUINT32             tg_fps;         // fps . unit:fps x 10, 10 for 1fps,  1 for 0.1fps.
    MUINT32             MIPI_pixrate;   //unit: mhz.
    E_CAM_CTL_TG_FMT    SrcImgFmt;
    E_CamPattern        pattern;        //input data pattern
    MUINT32             curClklv;
    vector<MUINT32>     vClk;           //avail clk rate
    //
    T_ResMgr_SEN()
    : tg_fps(10)
    , MIPI_pixrate(1000000)
    , SrcImgFmt(TG_FMT_JPG)
    , pattern(eCAM_NORMAL)
    , curClklv(0)
    {
        vClk.clear();
    }

    inline MVOID assign(MUINT32 fps,MUINT32 mipirate,E_CAM_CTL_TG_FMT imgfmt,\
        E_CamPattern _pattern,MUINT32 _curClklv,vector<MUINT32>* _vClk){
        tg_fps = fps;
        MIPI_pixrate = mipirate;
        SrcImgFmt = imgfmt;
        pattern = _pattern;
        curClklv = _curClklv;
        vClk.clear();
        for(MUINT32 i=0;i<_vClk->size();i++)
            vClk.push_back(_vClk->at(i));
    }
};

class Cam_ResMgr : public ResMgr{
friend class Central;   //inorder to access internalName

private:
    Cam_ResMgr(ISP_HW_MODULE master,char Name[32]);
public:
    //request a best-matched cam under asymmetric pipeline
     static Cam_ResMgr*     Res_Attach(MUINT32 width,char Name[32]);
    //booking master cam resource & get working obj
     static Cam_ResMgr*     Res_Attach(ISP_HW_MODULE master,char Name[32]);
    //release master cam's resource, including resources which r occupied by master cam
    virtual MBOOL           Res_Detach(void);
    //based on current throughput from input source , output pipeline's throuput configuration, like twin/bin/isp_clk ...etc
    virtual MBOOL           Res_Meter(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam);
    virtual MINT32          Res_Recursive(V_CAM_THRU_PUT* pcfgIn);
private:
    ISP_HW_MODULE       Res_GetCurrentModule(void);
    MBOOL               Res_Meter_Bayer(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam,const void *ptr,MUINT32 cur_clkL);
    MBOOL               Res_Meter_YUV(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam);
    MBOOL               Res_Meter_Release(void);


    inline MBOOL        Res_Meter_kernel(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,MBOOL& bBin_useful,MUINT32& bin_pix_mode,vector<ISP_HW_MODULE>& availCam);
    inline MVOID        K_CLK_CASE(Res_Meter_OUT &cfgOutParam);
    inline MVOID        K_BIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,MBOOL& bBin_useful,MUINT32& bin_pix_mode);
    inline MVOID        K_TWIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,vector<ISP_HW_MODULE>& availCam);
private:
    static Mutex            mLock;
    static Central          m_central;

    vector<Cam_ResMgr*>     m_vSlave;
    char                    m_User[32];
    char                    m_internalUser[24];
    static char             m_staticName[16];
    enum{
        REG_Module = 0,     //register
        REL_Module = 1,     //release
    }m_switchLatency;       //latency to ctrl the fetch-timing of hw pipline.clear occupiedmodule until hw is released.
    vector<ISP_HW_MODULE>   m_slave_module_release;

    T_ResMgr_SEN            m_SenInfo;
};


#endif
