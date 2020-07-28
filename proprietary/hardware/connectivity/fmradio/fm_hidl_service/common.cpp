/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/properties.h>
#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FMLIB_COM"
#define UNUSED(x) (void)(x)

int COM_get_seek_space()
{
    // FM radio seek space, 5:50KHZ; 1:100KHZ; 2:200KHZ
    if (property_get_int32("ro.vendor.mtk_fm_50khz_support", 0) == 1)
        return FM_SPACE_50K;
    return FM_SPACE_DEFAULT;
}

int COM_open_dev(const char *pname, int *fd)
{
    int ret = 0;
    int tmp = -1;

    FMR_ASSERT(pname);
    FMR_ASSERT(fd);

    LOGI("COM_open_dev start\n");
    tmp = open(pname, O_RDWR);
    if (tmp < 0) {
        LOGE("Open %s failed, %s\n", pname, strerror(errno));
        ret = -ERR_INVALID_FD;
    }
    *fd = tmp;
    LOGI("%s, [fd=%d] [ret=%d]\n", __func__, *fd, ret);
    return ret;
}

int COM_close_dev(int fd)
{
    int ret = 0;

    LOGI("COM_close_dev start\n");
    ret = close(fd);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_pwr_up(int fd, int band, int freq, int deemphasis)
{
    int ret = 0;
    struct fm_tune_parm parm;

    LOGI("%s, [freq=%d]\n", __func__, freq);
    bzero(&parm, sizeof(struct fm_tune_parm));

    parm.band = band;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.deemphasis = deemphasis;
    parm.space = FM_SEEK_SPACE;

    ret = ioctl(fd, FM_IOCTL_POWERUP, &parm);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_pwr_down(int fd, int type)
{
    int ret = 0;
    LOGI("%s, [type=%d]\n", __func__, type);
    ret = ioctl(fd, FM_IOCTL_POWERDOWN, &type);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_get_chip_id(int fd, int *chipid)
{
    int ret = 0;
    uint16_t tmp = 0;

    FMR_ASSERT(chipid);

    ret = ioctl(fd, FM_IOCTL_GETCHIPID, &tmp);
    *chipid = (int)tmp;
    if (ret){
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [chipid=%x] [ret=%d]\n", __func__, fd, *chipid, ret);
    return ret;
}

int COM_get_rssi(int fd, int *rssi)
{
    int ret = 0;

    FMR_ASSERT(rssi);

    ret = ioctl(fd, FM_IOCTL_GETRSSI, rssi);
    if(ret){
        LOGE("%s, failed, [ret=%d]\n", __func__, ret);
    }
    LOGI("%s, [rssi=%d] [ret=%d]\n", __func__, *rssi, ret);
    return ret;
}
/*0x20: space, 0x7E:~*/
#define ISVALID(c)((c)>=0x20 && (c)<=0x7E)
/*change any char which out of [0x20,0x7E]to space(0x20)*/
void COM_change_string(uint8_t *str, int len)
{
    int i = 0;
    for (i=0; i<len; i++) {
        if (false == ISVALID(str[i])) {
            str[i]= 0x20;
        }
    }
}

int COM_get_ps(int fd, RDSData_Struct *rds, uint8_t **ps, int *ps_len)
{
    int ret = 0;
    char tmp_ps[9] = {0};

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(ps);
    FMR_ASSERT(ps_len);

    if (rds->event_status&RDS_EVENT_PROGRAMNAME) {
        LOGD("%s, Success,[event_status=%d]\n", __func__, rds->event_status);
        *ps = &rds->PS_Data.PS[3][0];
        *ps_len = sizeof(rds->PS_Data.PS[3]);

        COM_change_string(*ps, *ps_len);
        memcpy(tmp_ps, *ps, 8);
        LOGI("PS=%s\n", tmp_ps);
    } else {
        LOGE("%s, Failed,[event_status=%d]\n", __func__, rds->event_status);
        *ps = NULL;
        *ps_len = 0;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int COM_get_rt(int fd, RDSData_Struct *rds, uint8_t **rt, int *rt_len)
{
    int ret = 0;
    char tmp_rt[65] = { 0 };

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(rt);
    FMR_ASSERT(rt_len);

    if (rds->event_status&RDS_EVENT_LAST_RADIOTEXT) {
        LOGD("%s, Success,[event_status=%d]\n", __func__, rds->event_status);
        *rt = &rds->RT_Data.TextData[3][0];
        *rt_len = rds->RT_Data.TextLength;

        COM_change_string(*rt, *rt_len);
        memcpy(tmp_rt, *rt, 64);
        LOGI("RT=%s\n", tmp_rt);
    } else {
        LOGE("%s, Failed,[event_status=%d]\n", __func__, rds->event_status);
        *rt = NULL;
        *rt_len = 0;
        ret = -ERR_RDS_NO_DATA;
    }
    return ret;
}

int COM_get_pi(int fd, RDSData_Struct *rds, uint16_t *pi)
{
    int ret = 0;

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(pi);

    if (rds->event_status & RDS_EVENT_PI_CODE) {
        LOGD("%s, Success,[event_status=%d] [PI=%d]\n", __func__, rds->event_status, rds->PI);
        *pi = rds->PI;
    } else {
        LOGI("%s, Failed, there's no pi,[event_status=%d]\n", __func__, rds->event_status);
        *pi = -1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int COM_get_ecc(int fd, RDSData_Struct *rds, uint8_t *ecc)
{
    int ret = 0;

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(ecc);

    if (rds->event_status & RDS_EVENT_ECC_CODE) {
        LOGD("%s, Success,[event_status=%d] [ECC=%d]\n", __func__,
             rds->event_status, rds->Extend_Country_Code);
        *ecc = rds->Extend_Country_Code;
    } else {
        LOGI("%s, Failed, there's no ECC,[event_status=%d]\n", __func__, rds->event_status);
        *ecc = -1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int COM_get_pty(int fd, RDSData_Struct *rds, uint8_t *pty)
{
    int ret = 0;

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(pty);

    if(rds->event_status&RDS_EVENT_PTY_CODE){
        LOGD("%s, Success,[event_status=%d] [PTY=%d]\n", __func__, rds->event_status, rds->PTY);
        *pty = rds->PTY;
    }else{
        LOGI("%s, Success, there's no pty,[event_status=%d]\n", __func__, rds->event_status);
        *pty = -1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int COM_tune(int fd, int freq, int band)
{
    int ret = 0;

    struct fm_tune_parm parm;

    bzero(&parm, sizeof(struct fm_tune_parm));

    parm.band = band;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SEEK_SPACE;

    ret = ioctl(fd, FM_IOCTL_TUNE, &parm);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [freq=%d] [ret=%d]\n", __func__, fd, freq, ret);
    return ret;
}

int COM_seek(int fd, int *freq, int band, int dir, int lev)
{
    int ret = 0;
    struct fm_seek_parm parm;

    bzero(&parm, sizeof(struct fm_tune_parm));

    parm.band = band;
    parm.freq = *freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SEEK_SPACE;
    if (dir == 1) {
        parm.seekdir = FM_SEEK_UP;
    } else if (dir == 0) {
        parm.seekdir = FM_SEEK_DOWN;
    }
    parm.seekth = lev;

    ret = ioctl(fd, FM_IOCTL_SEEK, &parm);
    if (ret == 0) {
        *freq = parm.freq;
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_set_mute(int fd, int mute)
{
    int ret = 0;
    int tmp = mute;

    LOGD("%s, start \n", __func__);
    ret = ioctl(fd, FM_IOCTL_MUTE, &tmp);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_is_fm_pwrup(int fd, int *pwrup)
{
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_IS_FM_POWERED_UP, pwrup);
    if (ret) {
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

/******************************************
 * Inquiry if RDS is support in driver.
 * Parameter:
 *      None
 *supt Value:
 *      1: support
 *      0: NOT support
 *      -1: error
 ******************************************/
int COM_is_rdsrx_support(int fd, int *supt)
{
    int ret = 0;
    int support = -1;

    if (fd < 0) {
        LOGE("FM isRDSsupport fail, g_fm_fd = %d\n", fd);
        *supt = -1;
        ret = -ERR_INVALID_FD;
        return ret;
    }

    ret = ioctl(fd, FM_IOCTL_RDS_SUPPORT, &support);
    if (ret) {
        LOGE("FM FM_IOCTL_RDS_SUPPORT fail, errno = %d\n", errno);
        //don't support
        *supt = 0;
        return ret;
    }
    LOGI("isRDSsupport Success,[support=%d]\n", support);
    *supt = support;
    return ret;
}

int COM_pre_search(int fd)
{
    fm_s32 ret = 0;
    ret = ioctl(fd, FM_IOCTL_PRE_SEARCH, 0);
    LOGD("COM_pre_search:%d\n",ret);
    return ret;
}

int COM_restore_search(int fd)
{
    fm_s32 ret = 0;
    ret = ioctl(fd, FM_IOCTL_RESTORE_SEARCH, 0);
    LOGD("COM_restore_search:%d\n",ret);
    return ret;
}

/*soft mute tune function, usually for sw scan implement or CQI log tool*/
int COM_Soft_Mute_Tune(int fd, fm_softmute_tune_t *para)
{
    fm_s32 ret = 0;
    //fm_s32 RSSI = 0, PAMD = 0,MR = 0, ATDC = 0;
    //fm_u32 PRX = 0;
    //fm_u16 softmuteGainLvl = 0;
    fm_softmute_tune_t value;

    value.freq = para->freq;
    ret = ioctl(fd, FM_IOCTL_SOFT_MUTE_TUNE, &value);
    if (ret) {
        LOGE("FM soft mute tune faild:%d\n",ret);
        return ret;
    }
#if 0
    LOGD("Raw data of soft mute tune[%d]: RSSI:[%x]PAMD:[%x]MR:[%x]ATDC:[%x]PRX:[%x]SMG:[%x]",
        para->freq,value.RSSI,value.PAMD,value.MR,value.ATDC,value.PRX,value.SMG);
    RSSI = ((value.RSSI & 0x03FF) >= 512) ? ((value.RSSI & 0x03FF) - 1024) : (value.RSSI & 0x03FF);
    PAMD = ((value.PAMD & 0xFF) >= 128) ? ((value.PAMD & 0x00FF) - 256) : (value.PAMD & 0x00FF);
    MR = ((value.MR & 0x01FF) >= 256) ? ((value.MR & 0x01FF) - 512) : (value.MR & 0x01FF);
    ATDC =((value.ATDC & 0x0FFF) >= 2048) ? ((value.ATDC & 0x0FFF) - 4096) : (value.ATDC & 0x0FFF);
    if (ATDC < 0) {
        ATDC = (~(ATDC)) - 1;//Get abs value of ATDC
    }
    PRX = (value.PRX & 0x00FF);
    softmuteGainLvl = value.SMG;
    //check if the channel is valid according to each CQIs
    if ((RSSI >= RSSI_TH)
     && (PAMD <= PAMD_TH)
     && (ATDC <= ATDC_TH)
     && (MR >= MR_TH)
     && (PRX >= PRX_TH)
     && (softmuteGainLvl <= softMuteGainTH)) {
        para->valid = fm_true;
    } else {
        para->valid = fm_false;
    }
#endif
    para->valid = value.valid;
    para->rssi = value.rssi;
    //LOGI("soft mute tune[%d] valid[%d]: RSSI:[%d]PAMD:[%d]MR:[%d]ATDC:[%d]PRX:[%d]SMG:[%d]",
    //    para->freq,para->valid,RSSI,PAMD,MR,ATDC,PRX,softmuteGainLvl);
    return 0;
}

int COM_get_cqi(int fd, int num, char *buf, int buf_len)
{
    int ret;
    struct fm_cqi_req cqi_req;

    //check buf
    num = (num > CQI_CH_NUM_MAX) ? CQI_CH_NUM_MAX : num;
    num = (num < CQI_CH_NUM_MIN) ? CQI_CH_NUM_MIN : num;
    cqi_req.ch_num = (uint16_t)num;
    cqi_req.buf_size = cqi_req.ch_num * sizeof(struct fm_cqi);
    if (!buf || (buf_len < cqi_req.buf_size)) {
        LOGE("get cqi, invalid buf\n");
        return -1;
    }
    cqi_req.cqi_buf = buf;

    //get cqi from driver
    ret = ioctl(fd, FM_IOCTL_CQI_GET, &cqi_req);
    if (ret < 0) {
        LOGE("get cqi, failed %d\n", ret);
        return -1;
    }

    return 0;
}

int COM_turn_on_off_rds(int fd, int onoff)
{
    int ret = 0;
    uint16_t rds_on = -1;

    LOGD("Rdsset start\n");
    if (onoff == FMR_RDS_ON) {
        rds_on = 1;
        ret = ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        if (ret) {
            LOGE("FM_IOCTL_RDS_ON failed\n");
            return ret;
        }
        LOGD("Rdsset Success,[rds_on=%d]\n", rds_on);
    } else {
        rds_on = 0;
        ret = ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        if (ret) {
            LOGE("FM_IOCTL_RDS_OFF failed\n");
            return ret;
        }
        LOGD("Rdsset Success,[rds_on=%d]\n", rds_on);
    }
    return ret;
}

int COM_read_rds_data(int fd, RDSData_Struct *rds, uint16_t *rds_status)
{
    int ret = 0;
    uint16_t event_status;
    //char tmp_ps[9] = {0};
    //char tmp_rt[65] = { 0 };

    FMR_ASSERT(rds);
    FMR_ASSERT(rds_status);

    if (read(fd, rds, sizeof(RDSData_Struct)) == sizeof(RDSData_Struct)) {
        event_status = rds->event_status;
        //memcpy(tmp_ps, &rds->PS_Data.PS[3][0], 8);
        //memcpy(tmp_rt, &rds->RT_Data.TextData[3][0], 64);
        LOGI("event_status = 0x%x\n", event_status);
        //memset(tmp_ps, 0, 9);
        //memset(tmp_rt, 0, 65);
        *rds_status = event_status;
        return ret;
    } else {
        //LOGE("readrds get no event\n");
        ret = -ERR_RDS_NO_DATA;
    }
    return ret;
}

static int COM_get_af_pi(int fd, uint16_t *pi)
{
    int ret;
    struct rds_raw_data rrd;
    uint16_t pi1, pi2;

    FMR_ASSERT(pi);

    memset(&rrd, 0, sizeof(rrd));
    ret = ioctl(fd, FM_IOCTL_RDS_GET_LOG, &rrd);
    if (ret) {
        LOGE("COM_get_af_pi fail(%d)\n", ret);
        *pi = 0;
        return ret;
    }
    if (rrd.len == 0) {
        LOGE("COM_get_af_pi fail, RDS log empty!\n");
        *pi = 0;
        return -1;
    }
    pi1 = rrd.data[4];
    pi1 |= (rrd.data[5] << 8);
    LOGI("data[4]=%02x,data[5]=%02x,pi1=%04x\n", rrd.data[4], rrd.data[5], pi1);
    pi2 = rrd.data[16];
    pi2 |= (rrd.data[17] << 8);
    LOGI("data[16]=%02x,data[17]=%02x,pi2=%04x\n", rrd.data[16], rrd.data[17], pi2);

    if (pi1 == pi2) {
        LOGI("got af pi!!!\n");
        *pi = pi1;
    } else {
        LOGE("af pi check fail\n");
        *pi = 0;
        return -1;
    }

    return 0;
}

int COM_get_af_list(int fd, RDSData_Struct *rds, int16_t **af_list, int *len)
{
    int ret = 0;
    int i = 0;
    int tmp_list[50] = {0};
    AF_Info af_list_temp;

    UNUSED(fd);
    FMR_ASSERT(rds);
    FMR_ASSERT(af_list);

    if (!(rds->event_status & RDS_EVENT_AF)) {
        LOGE("Get AF list failed.\n");
        ret = -ERR_RDS_NO_DATA;
        return ret;
    }
    memset(&af_list_temp, 0, sizeof(af_list_temp));
    memcpy(&af_list_temp, &rds->AF_Data, sizeof(AF_Info));

    af_list_temp.AF_Num = af_list_temp.AF_Num > 25 ? 25 : af_list_temp.AF_Num;
    LOGI("AF list length: %d\n", af_list_temp.AF_Num);
    *len = af_list_temp.AF_Num;
    *af_list = &af_list_temp.AF[1][0];
    return ret;
}

int COM_active_af(int fd, RDSData_Struct *rds, CUST_cfg_ds *cfg_data,
    uint16_t orig_pi, uint16_t cur_freq, uint16_t *ret_freq)
{
    int ret = 0;
    int i = 0, j = 0;
    struct fm_tune_parm parm;
    struct fm_softmute_tune_t smt_parm;
    uint16_t set_freq = 0, sw_freq = 0, org_freq = 0;
    uint16_t PAMD_Value = 0, AF_PAMD_LBound = 0, AF_PAMD_HBound = 0;
    uint16_t PAMD_Level[25];
    uint16_t PI[25];
    uint16_t PAMD_DB_TBL[5] = {// 5dB, 10dB, 15dB, 20dB, 25dB,
                               //  13, 17, 21, 25, 29};
                                8, 12, 15, 18, 20};
    AF_Info af_list_backup;
    AF_Info af_list;
    FMR_ASSERT(rds);
    FMR_ASSERT(cfg_data);
    sw_freq = cur_freq; //current freq
    org_freq = cur_freq;
    parm.band = cfg_data->band;
    parm.freq = sw_freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_DEFAULT;

    if (!(rds->event_status & RDS_EVENT_AF)) {
        LOGE("activeAF failed\n");
        *ret_freq = 0;
        ret = -ERR_RDS_NO_DATA;
        return ret;
    }
    memset(&af_list_backup, 0, sizeof(af_list_backup));
    memcpy(&af_list_backup, &rds->AF_Data, sizeof(AF_Info));
    memset(&af_list, 0, sizeof(af_list));

    AF_PAMD_LBound = PAMD_DB_TBL[0]; //5dB
    AF_PAMD_HBound = PAMD_DB_TBL[1]; //15dB
    ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
    for (i = 0; i < 3 && (PAMD_Value < AF_PAMD_LBound); i++) {
        usleep(10 * 1000);
        ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
        LOGI("check PAMD %d time(s), PAMD =%d", i+1, PAMD_Value);
    }
    LOGI("current_freq=%d, PAMD_Value=%d, orig_pi=%d\n", cur_freq, PAMD_Value, orig_pi);

    /* Start to detect AF channels when orignal channel turn weak */
    if (PAMD_Value < AF_PAMD_LBound) {

         /* Make sure rds->AF_Data.AF_Num is valid */
        af_list_backup.AF_Num = af_list_backup.AF_Num > 25 ? 25 : af_list_backup.AF_Num;

        /* Precheck af list*/
        for (i = 0, j = 0; i < af_list_backup.AF_Num; i++) {
            set_freq = af_list_backup.AF[1][i];
            if(set_freq < cfg_data->low_band || set_freq > cfg_data->high_band) {
                /*band check fail*/
                LOGI("AF[1][%d]:freq %d out of bandwidth[%d,%d], skip!\n",
                    i, af_list_backup.AF[1][i], cfg_data->low_band, cfg_data->high_band);
                continue;
            }

            /* Using Com_Soft_Mute_Tune to query valid channel*/
            memset(&smt_parm, 0, sizeof(fm_softmute_tune_t));
            smt_parm.freq = set_freq;
            COM_Soft_Mute_Tune(fd, &smt_parm);
            LOGE("af list pre-check:freq %d, valid:%d\n", smt_parm.freq, smt_parm.valid);
            if(smt_parm.valid == 1) {
                /* Update valid AF channel to af_list*/
                af_list.AF[1][j] = af_list_backup.AF[1][i];
                j++;
                af_list.AF_Num++;
            }

        }

        /*AF switch process*/
        for (i = 0; i < af_list.AF_Num; i++) {
            set_freq = af_list.AF[1][i];
            LOGI("set_freq[%d] = %d, org_freq = %d\n", i, set_freq, org_freq);

            if (set_freq != org_freq) {
                // Set mute to check every af channels
                COM_set_mute(fd, 1);
                parm.freq = set_freq;
                ioctl(fd, FM_IOCTL_TUNE, &parm);
                usleep(20 * 1000);
                ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);

                /* If signal is not good enough, skip */
                if (PAMD_Level[i] < AF_PAMD_HBound) {
                    LOGI("PAMD_Level[%d] =%d < AF_PAMD_HBound, continue", i, PAMD_Level[i]);
                    continue;
                }

                for (j = 0 ; j < 5; j++ ) {
                    usleep(200 * 1000);
                    /* Query pi to 5 times */
                    if (COM_get_af_pi(fd, &PI[i])) {
                        if (j == 4)
                            LOGE("get af pi fail\n");
                        continue;
                    } else
                        break;
                }

                if (orig_pi != PI[i]) {
                    LOGI("pi not match, current pi(%04x), orig pi(%04x)\n", PI[i], orig_pi);
                    continue;
                }
                LOGI("next_freq=%d, PAMD_Level[%d]=%d\n", parm.freq, i, PAMD_Level[i]);
                /* To get largest PAMD */
                /* This is a trade-off*/
                /* If want to get a better quality AF channel, do as follows, it will spend longer to do AF switch*/
                /* If want to mute shorter, just use one fixed PAMD_VALUE, don't need to compare */
                if (PAMD_Level[i] > AF_PAMD_HBound) {
                    LOGI("PAMD_Level[%d] =%d > AF_PAMD_HBound, af switch", i, PAMD_Level[i]);
                    sw_freq = set_freq;
                    PAMD_Value = PAMD_Level[i];
                    break;
                }
            }
        }
        LOGI("AF deside tune to freq: %d, PAMD_Level: %d\n", sw_freq, PAMD_Value);
        if ((PAMD_Value > AF_PAMD_HBound)&&(sw_freq != 0)) {  /* Tune to AF channel */
            parm.freq = sw_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        } else {  /* Tune to orignal channel */
            parm.freq = org_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        }
        /* Unmute when finish AF switch */
        COM_set_mute(fd, 0);
    } else {
        LOGD("RDS_EVENT_AF old freq:%d\n", org_freq);
    }
    *ret_freq = cur_freq;

    return ret;
}

int COM_active_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq)
{
    int ret = 0;

    FMR_ASSERT(rds);
    FMR_ASSERT(backup_freq);
    FMR_ASSERT(ret_freq);

    if(rds->event_status&RDS_EVENT_TAON){
        uint16_t rds_on = 0;
        struct fm_tune_parm parm;
        uint16_t PAMD_Level[25];
        uint16_t PAMD_DB_TBL[5] = {13, 17, 21, 25, 29};
        uint16_t set_freq, sw_freq, org_freq, PAMD_Value, TA_PAMD_Threshold;
        int i = 0;

        rds_on = 0;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        TA_PAMD_Threshold = PAMD_DB_TBL[2]; //15dB
        sw_freq = cur_freq;
        org_freq = cur_freq;
        *backup_freq = org_freq;
        parm.band = band;
        parm.freq = sw_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = COM_get_seek_space();

        ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
        //make sure rds->AF_Data.AF_Num is valid
        rds->AFON_Data.AF_Num = (rds->AFON_Data.AF_Num > 25)? 25 : rds->AFON_Data.AF_Num;
        for(i=0; i< rds->AFON_Data.AF_Num; i++){
            set_freq = rds->AFON_Data.AF[1][i];
            LOGI("set_freq=0x%02x,org_freq=0x%02x\n", set_freq, org_freq);
            if(set_freq != org_freq){
                parm.freq = sw_freq;
                ioctl(fd, FM_IOCTL_TUNE, &parm);
                ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);
                if(PAMD_Level[i] > PAMD_Value){
                    PAMD_Value = PAMD_Level[i];
                    sw_freq = set_freq;
                }
            }
        }

        if((PAMD_Value > TA_PAMD_Threshold)&&(sw_freq != 0)){
            rds->Switch_TP= 1;
            parm.freq = sw_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        }else{
            parm.freq = org_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        }
        rds_on = 1;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
    }

    *ret_freq = cur_freq;
    return ret;
}

int COM_deactive_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq)
{
    int ret = 0;

    UNUSED(band);
    FMR_ASSERT(rds);
    FMR_ASSERT(backup_freq);
    FMR_ASSERT(ret_freq);

    if(rds->event_status&RDS_EVENT_TAON_OFF){
        uint16_t rds_on = 0;
        struct fm_tune_parm parm;
        parm.band = FM_RAIDO_BAND;
        parm.freq = *backup_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = COM_get_seek_space();

        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);

        ioctl(fd, FM_IOCTL_TUNE, &parm);
        cur_freq = parm.freq;
        rds_on = 1;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
    }

    *ret_freq = cur_freq;
    return ret;
}

int COM_ana_switch(int fd, int antenna)
{
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_ANA_SWITCH, &antenna);
    if (ret < 0) {
        LOGE("%s: fail, ret = %d\n", __func__, ret);
    }

    LOGD("%s: [ret = %d]\n", __func__, ret);
    return ret;
}

int COM_get_badratio(int fd, int *badratio)
{
    int ret = 0;
    uint16_t tmp = 0;

    ret = ioctl(fd, FM_IOCTL_GETBLERRATIO, &tmp);
    *badratio = (int)tmp;
    if (ret){
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}


int COM_get_stereomono(int fd, int *stemono)
{
    int ret = 0;
    uint16_t tmp = 0;

    ret = ioctl(fd, FM_IOCTL_GETMONOSTERO, &tmp);
    *stemono = (int)tmp;
    if (ret){
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_set_stereomono(int fd, int stemono)
{
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_SETMONOSTERO, &stemono);
    if (ret){
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_get_caparray(int fd, int *caparray)
{
    int ret = 0;
    int tmp = 0;

    LOGD("%s, [fd=%d]\n", __func__, fd);
    ret = ioctl(fd, FM_IOCTL_GETCAPARRAY, &tmp);
    if (ret){
        LOGE("%s, failed\n", __func__);
    }
    *caparray = tmp;
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}

int COM_get_hw_info(int fd, struct fm_hw_info *info)
{
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GET_HW_INFO, info);
    if(ret){
        LOGE("%s, failed\n", __func__);
    }
    LOGD("%s, [fd=%d] [ret=%d]\n", __func__, fd, ret);
    return ret;
}


/*  COM_is_dese_chan -- check if gived channel is a de-sense channel or not
  *  @fd - fd of "dev/fm"
  *  @freq - gived channel
  *  return value: 0, not a dese chan; 1, a dese chan; else error NO.
  */
int COM_is_dese_chan(int fd, int freq)
{
    int ret = 0;
    int tmp = freq;

    ret = ioctl(fd, FM_IOCTL_IS_DESE_CHAN, &freq);
    if (ret < 0) {
        LOGE("%s, failed,ret=%d\n", __func__,ret);
        return ret;
    } else {
        LOGD("[fd=%d] %d --> dese=%d\n", fd, tmp, freq);
        return freq;
    }
}

/*  COM_desense_check -- check if gived channel is a de-sense channel or not
  *  @fd - fd of "dev/fm"
  *  @freq - gived channel
  *  @rssi-freq's rssi
  *  return value: 0, is desense channel and rssi is less than threshold; 1, not desense channel or it is but rssi is more than threshold.
  */
int COM_desense_check(int fd, int freq, int rssi)
{
    int ret = 0;
    fm_desense_check_t parm;

    parm.freq = freq;
    parm.rssi = rssi;
    ret = ioctl(fd, FM_IOCTL_DESENSE_CHECK, &parm);
    if (ret < 0) {
        LOGE("%s, failed,ret=%d\n", __func__,ret);
        return ret;
    } else {
        LOGD("[fd=%d] %d --> dese=%d\n", fd,freq,ret);
        return ret;
    }
}
/*
th_idx:
	threshold type: 0, RSSI. 1,desense RSSI. 2,SMG.
th_val: threshold value*/
int COM_set_search_threshold(int fd, int th_idx,int th_val)
{
    int ret = 0;
    fm_search_threshold_t th_parm;
    th_parm.th_type = th_idx;
    th_parm.th_val = th_val;
    ret = ioctl(fd, FM_IOCTL_SET_SEARCH_THRESHOLD, &th_parm);
    if (ret < 0)
    {
        LOGE("%s, failed,ret=%d\n", __func__,ret);
    }
    return ret;
}
int COM_full_cqi_logger(int fd, fm_full_cqi_log_t *log_parm)
{
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_FULL_CQI_LOG, log_parm);
    if (ret < 0)
    {
        LOGE("%s, failed,ret=%d\n", __func__,ret);
    }
    return ret;
}
void FM_interface_init(struct fm_cbk_tbl *cbk_tbl)
{
    //Basic functions.
    cbk_tbl->open_dev = COM_open_dev;
    cbk_tbl->close_dev = COM_close_dev;
    cbk_tbl->pwr_up = COM_pwr_up;
    cbk_tbl->pwr_down = COM_pwr_down;
    cbk_tbl->tune = COM_tune;
    cbk_tbl->set_mute = COM_set_mute;
    cbk_tbl->is_rdsrx_support = COM_is_rdsrx_support;
    cbk_tbl->turn_on_off_rds = COM_turn_on_off_rds;
    cbk_tbl->get_chip_id = COM_get_chip_id;
    //For RDS RX.
    cbk_tbl->read_rds_data = COM_read_rds_data;
    cbk_tbl->get_pi = COM_get_pi;
    cbk_tbl->get_ps = COM_get_ps;
    cbk_tbl->get_ecc = COM_get_ecc;
    cbk_tbl->get_pty = COM_get_pty;
    cbk_tbl->get_rssi = COM_get_rssi;
    cbk_tbl->get_rt = COM_get_rt;
    cbk_tbl->get_af_list = COM_get_af_list;
    cbk_tbl->active_af = COM_active_af;
    cbk_tbl->active_ta = COM_active_ta;
    cbk_tbl->deactive_ta = COM_deactive_ta;
    //FM short antenna
    cbk_tbl->ana_switch = COM_ana_switch;
    cbk_tbl->desense_check = COM_desense_check;
    //RX EM mode use
    cbk_tbl->get_badratio = COM_get_badratio;
    cbk_tbl->get_stereomono = COM_get_stereomono;
    cbk_tbl->set_stereomono = COM_set_stereomono;
    cbk_tbl->get_caparray = COM_get_caparray;
    cbk_tbl->get_cqi = COM_get_cqi;
    cbk_tbl->is_dese_chan = COM_is_dese_chan;
    cbk_tbl->desense_check = COM_desense_check;
    cbk_tbl->get_hw_info = COM_get_hw_info;
    //soft mute tune
    cbk_tbl->soft_mute_tune = COM_Soft_Mute_Tune;
    cbk_tbl->pre_search = COM_pre_search;
    cbk_tbl->restore_search = COM_restore_search;
    //EM
    cbk_tbl->set_search_threshold = COM_set_search_threshold;
    cbk_tbl->full_cqi_logger = COM_full_cqi_logger;
    return;
}

