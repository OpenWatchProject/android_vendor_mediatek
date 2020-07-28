/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "[VT]VcodecCap"

#include <expat.h>

#include <unistd.h>
#include "IVcodecCap.h"
#include "VcodecCap.h"
#include <cutils/properties.h>
#include "pthread.h"

static const int MAX_SIM = 2;
static void setFormat(MediaProfileList_t *pMediaProfileList_t, video_format_t format)
{
    bool bFound = false;
    for (Vector<video_format_t>::iterator it = pMediaProfileList_t->mVideoFormatList.begin() ; it != pMediaProfileList_t->mVideoFormatList.end(); ++it)
    {
        if (*it == format)
        {
            bFound = true;
        }
    }

    if (bFound == false)
    {
        pMediaProfileList_t->mVideoFormatList.push_back(format);
        ALOGI("setFormat(%d)", format);
    }
}

static void startElementHandler(void *userData, const char *name, const char **atts)
{
    MediaProfileList_t *pMediaProfileList_t = (MediaProfileList_t *)userData;

    if (strcmp("VideoProfile", name) == 0)
    {
        video_media_profile_t *pmedia_profile = new video_media_profile_t;
        if (strcmp("h264", atts[1]) == 0)
        {
            pmedia_profile->format = VIDEO_H264;
        }
        else
        {
            pmedia_profile->format = VIDEO_HEVC;
        }

        pmedia_profile->profile = (video_profile_t)atoi(atts[3]);
        pmedia_profile->level = (video_level_t)atoi(atts[5]);
        pmedia_profile->width = atoi(atts[7]);
        pmedia_profile->height = atoi(atts[9]);
        pmedia_profile->fps = atoi(atts[11]);
        pmedia_profile->Iinterval = atoi(atts[13]);
        pmedia_profile->minbitrate = atoi(atts[15]);
        pmedia_profile->bitrate = atoi(atts[17]);
        pMediaProfileList_t->mMediaProfile[pmedia_profile->format].push_back(pmedia_profile);
    }
    else if (strcmp("VideoQuality", name) == 0)
    {
        QualityInfo_t      qualityInfo;
        qualityInfo.quality = VIDEO_QUALITY_LOW;

        if (strcmp("fine", atts[1]) == 0)
        {
            qualityInfo.quality = VIDEO_QUALITY_FINE;
        }
        else if (strcmp("high", atts[1]) == 0)
        {
            qualityInfo.quality = VIDEO_QUALITY_HIGH;
        }
        else if (strcmp("medium", atts[1]) == 0)
        {
            qualityInfo.quality = VIDEO_QUALITY_MEDIUM;
        }
        else if (strcmp("low", atts[1]) == 0)
        {
            qualityInfo.quality = VIDEO_QUALITY_LOW;
        }

        if (strcmp("h264", atts[3]) == 0)
        {
            qualityInfo.format = VIDEO_H264;
        }
        else
        {
            qualityInfo.format = VIDEO_HEVC;
        }

        qualityInfo.profile = (video_profile_t)atoi(atts[5]);
        qualityInfo.level = (video_level_t)atoi(atts[7]);

        pMediaProfileList_t->mQualityList[qualityInfo.format].push_back(qualityInfo);
        setFormat(pMediaProfileList_t, qualityInfo.format);
    }
    else if (strcmp("DefaultVideoQuality", name) == 0)
    {
        if (strcmp("fine", atts[1]) == 0)
        {
            pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_FINE;
        }
        else if (strcmp("high", atts[1]) == 0)
        {
            pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_HIGH;
        }
        else if (strcmp("medium", atts[1]) == 0)
        {
            pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_MEDIUM;
        }
        else if (strcmp("low", atts[1]) == 0)
        {
            pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_LOW;
        }
        ALOGI("DefaultVideoQuality: %s", atts[1]);
    }
}

video_media_profile_t *getMediaProfileEntry(MediaProfileList_t *pMediaProfileList_t, video_format_t format, video_profile_t profile, video_level_t level)
{
    // Search mediaprofile entry
    Vector<video_media_profile_t *> *pMediaProfile = &pMediaProfileList_t->mMediaProfile[format];
    for (Vector<video_media_profile_t *>::iterator it = pMediaProfile->begin() ; it != pMediaProfile->end(); ++it)
    {
        video_media_profile_t *pMPEntry = (*it);
        if (pMPEntry->profile == profile && pMPEntry->level == level)
        {
            video_property_t* pvideo_property_t = getVideoProperty();
            pMPEntry->bitrate = pvideo_property_t->bitrate != 0?pvideo_property_t->bitrate:pMPEntry->bitrate;
            pMPEntry->fps = pvideo_property_t->fps != 0?pvideo_property_t->fps:pMPEntry->fps;
            pMPEntry->Iinterval = pvideo_property_t->Iinterval != 0?pvideo_property_t->Iinterval:pMPEntry->Iinterval;

            ALOGI("[getMediaProfileEntry][%d] out:(%s, %s, %dx%d, %d fps, %d kbps, i period: %d)",
                  pMPEntry->format,
                  toString(pMPEntry->profile),
                  toString(pMPEntry->level),
                  pMPEntry->width, pMPEntry->height,
                  pMPEntry->fps, pMPEntry->bitrate / 1000, pMPEntry->Iinterval);
            return pMPEntry;
        }
    }
    ALOGI("[getMediaProfileEntry] Can't get mediaprofle entry(%d, %s, %s)", format, toString(profile), toString(level));
    return NULL;
}

static void createH264DefaultMediaProfile(int opid, Vector<video_media_profile_t *> *prMP)
{
    video_media_profile_t *pvideo_media_profile_t;
    int count = 0;

    getDefaultH264MediaProfileByOperator(opid, &pvideo_media_profile_t, &count);

    prMP->clear();
    for (int index = 0; index < count; index++)
    {
        prMP->push_back(&pvideo_media_profile_t[index]);
    }
    ALOGI("createH264DefaultMediaProfile() default profile table(op%d), count = %d", opid, count);
}

static void createH264DefaultMediaProfileXml(int opid)
{
    video_media_profile_t *pvideo_media_profile_t;
    int count = 0;
    int i = 0;
    const char *defaultSDXmlFile = "/sdcard/viLTE_media_profiles.xml";
    FILE *fp;
    char xmlString[4096];
    video_media_profile_t *profiles;
    QualityInfo_t qualityInfo[16];
    QualityInfo_t *pqualityInfo;
    char value[PROPERTY_VALUE_MAX];

    property_get("persist.vendor.vt.ezconfig", value, "0");
    if (atoi(value) == 0) {
        return;
    }

    getDefaultH264MediaProfileByOperator(opid, &pvideo_media_profile_t, &count);

    createH264QualityMediaProfileByOperator(opid,qualityInfo);


    fp = fopen(defaultSDXmlFile, "r"); // try default xml
    if (fp == NULL)
    {
        fp = fopen(defaultSDXmlFile, "w"); // try default xml
        ALOGI("createH264DefaultMediaProfileXml(): %s enable eazy config mode", defaultSDXmlFile);


        ALOGI("createH264DefaultMediaProfileXml  count %d",count);


        fprintf(fp,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        fprintf(fp,"<MediaSettings>\n");
        fprintf(fp,"<Operator name=\"xxxx\"/>\n");
        for (i=0;i<count;i++)
        {
            profiles = (pvideo_media_profile_t + i);
            ALOGI("count %d  format %d profile %d  level %d\n",i,profiles->format,profiles->profile,profiles->level);
            fprintf(fp," <VideoProfile name=\"h264\" profile=\"%d\" level=\"%d\" width=\"%d\" height=\"%d\" framerate=\"%d\" Iinterval=\"%d\" minBitRate=\"%d\" maxBitRate=\"%d\" />\n",
                    profiles->profile,
                    profiles->level,
                    profiles->width,
                    profiles->height,
                    profiles->fps,
                    profiles->Iinterval,
                    profiles->minbitrate,
                    profiles->bitrate);

        }

        //pqualityInfo = qualityInfo;
        for (i=0;i<4;i++)
        {
            //if (pqualityInfo != NULL)
            {
                pqualityInfo = (qualityInfo + i);

                if (pqualityInfo->quality == VIDEO_QUALITY_FINE) {
                    ALOGI("createH264DefaultMediaProfileXml quality index is fine\n");
                    fprintf(fp," <VideoQuality name=\"fine\" format=\"h264\" profile=\"%d\" level=\"%d\"/>\n",
                            pqualityInfo->profile,
                            pqualityInfo->level);

                }
                else if (pqualityInfo->quality == VIDEO_QUALITY_HIGH) {
                    ALOGI("createH264DefaultMediaProfileXml quality index is high\n");
                    fprintf(fp," <VideoQuality name=\"high\" format=\"h264\" profile=\"%d\" level=\"%d\"/>\n",
                            pqualityInfo->profile,
                            pqualityInfo->level);
                }
                else if (pqualityInfo->quality == VIDEO_QUALITY_MEDIUM) {
                    ALOGI("createH264DefaultMediaProfileXml quality index is medium\n");
                    fprintf(fp," <VideoQuality name=\"medium\" format=\"h264\" profile=\"%d\" level=\"%d\"/>\n",
                            pqualityInfo->profile,
                            pqualityInfo->level);
                }
                else if (pqualityInfo->quality == VIDEO_QUALITY_LOW) {
                    ALOGI("createH264DefaultMediaProfileXml quality index is low\n");
                    fprintf(fp," <VideoQuality name=\"low\" format=\"h264\" profile=\"%d\" level=\"%d\"/>\n",
                            pqualityInfo->profile,
                            pqualityInfo->level);
                }
                else {
                    ALOGE("createH264DefaultMediaProfileXml unsupported quality index %d",pqualityInfo->quality);
                }


            }

        }

        fprintf(fp,"  <DefaultVideoQuality name=\"high\" />\n");
        fprintf(fp,"</MediaSettings>\n");

        fclose(fp);


    }
    else
    {
        fclose(fp);
        ALOGI("createH264DefaultMediaProfileXml(): %s already exists", defaultSDXmlFile);
    }


    ALOGI("createH264DefaultMediaProfileXML() default profile table(op%d), count = %d", opid, count);
}

static void createHEVCDefaultMediaProfile(int opid, Vector<video_media_profile_t *> *prMP)
{
    video_media_profile_t *pvideo_media_profile_t;
    int count = 0;

    getDefaultHEVCMediaProfileByOperator(opid, &pvideo_media_profile_t, &count);

    prMP->clear();
    for (int index = 0; index < count; index++)
    {
        prMP->push_back(&pvideo_media_profile_t[index]);
    }

    ALOGI("createHEVCDefaultMediaProfile() default profile table(op%d)", opid);
}

static void createMediaProfile(uint32_t opID, MediaProfileList_t *pMediaProfileList_t)
{
    const char *defaultXmlFile = "/system/vendor/etc/vilte/viLTE_media_profiles.xml";
    const char *defaultSDXmlFile = "/sdcard/viLTE_media_profiles.xml";
    char opXmlFile[64];
    int opid = opID;

    pMediaProfileList_t->mOPID = opid;
    pMediaProfileList_t->mMediaProfile[VIDEO_H264].clear();
    pMediaProfileList_t->mMediaProfile[VIDEO_HEVC].clear();

    sprintf(opXmlFile, "/system/vendor/etc/vilte/viLTE_media_profiles_op%d.xml", opid);
    ALOGI("createMediaProfile(): %s", opXmlFile);

    FILE *fp = fopen(opXmlFile, "r"); // try operator xml
    if (fp == NULL)
    {
        fp = fopen(defaultXmlFile, "r"); // try default xml
        if (fp == NULL)
        {
            fp = fopen(defaultSDXmlFile, "r"); // try default xml
            if (fp != NULL)
            {
                ALOGI("createMediaProfile(): %s enable eazy config mode", defaultSDXmlFile);
            }
        }
    }

    if (fp)
    {
        do
        {
            XML_Parser parser = ::XML_ParserCreate(NULL);
            if (parser == NULL)
                break;

            ::XML_SetUserData(parser, pMediaProfileList_t);
            ::XML_SetElementHandler(parser, startElementHandler, NULL);

            const int BUFF_SIZE = 512;
            for (;;) {
                void *buff = ::XML_GetBuffer(parser, BUFF_SIZE);
                if (buff == NULL) {
                    ALOGE("failed to in call to XML_GetBuffer()");
                    break;
                }

                int bytes_read = ::fread(buff, 1, BUFF_SIZE, fp);
                if (bytes_read < 0) {
                    ALOGE("failed in call to read");
                    break;
                }

                ::XML_ParseBuffer(parser, bytes_read, bytes_read == 0);

                if (bytes_read == 0) break;  // done parsing the xml file
            }
            ::XML_ParserFree(parser);
        }while(0);
        ::fclose(fp);
    }
    else    // create from default table
    {
        createH264DefaultMediaProfileXml(opid);
        createH264DefaultMediaProfile(opid, &pMediaProfileList_t->mMediaProfile[VIDEO_H264]);
        createHEVCDefaultMediaProfile(opid, &pMediaProfileList_t->mMediaProfile[VIDEO_HEVC]);
    }
}

static void createQualityMediaProfile(int format, MediaProfileList_t *pMediaProfileList_t)
{
    // Cerate mediaprofile entry for quality
    video_media_profile_t *pMPEntry = NULL;
    Vector<QualityInfo_t> &QualityList = pMediaProfileList_t->mQualityList[format];
    for (Vector<QualityInfo_t>::iterator it = QualityList.begin() ; it != QualityList.end(); ++it)
    {
        pMPEntry = getMediaProfileEntry(pMediaProfileList_t, (*it).format, (*it).profile, (*it).level);
        if (pMPEntry) pMediaProfileList_t->mQualityMediaProfile[format].add((*it).quality, *pMPEntry);
    }
}


static void createH264QualtiyMediaProfile(uint32_t opID, MediaProfileList_t *pMediaProfileList_t)
{
    //vdec_cap_t rvdec_cap_t;
    //venc_cap_t rvenc_cap_t;
    //video_default_cap_t rdefault_cap_t;

    // Get Max level
    //getVDecCapability(VIDEO_H264, &rvdec_cap_t);
    //getVEncCapability(VIDEO_H264, &rvenc_cap_t);
    //getDefaultCapability(&rvdec_cap_t, &rvenc_cap_t, &rdefault_cap_t);
    int nQualityCount = pMediaProfileList_t->mQualityList[VIDEO_H264].size();
    KeyedVector<video_quality_t, video_media_profile_t> *prQMP = &(pMediaProfileList_t->mQualityMediaProfile[VIDEO_H264]);
    prQMP->clear();

    if (nQualityCount > 0)
    {
        createQualityMediaProfile(VIDEO_H264, pMediaProfileList_t);
    }
    else
    {
        ALOGI("createH264QualtiyMediaProfile: default setting");
        QualityInfo_t qualityInfo[4];
        createH264QualityMediaProfileByOperator(opID, qualityInfo);

        for(int i = 0; i < 4; i++)
        {
            prQMP->add(qualityInfo[i].quality, *getMediaProfileEntry(pMediaProfileList_t, qualityInfo[i].format, qualityInfo[i].profile, qualityInfo[i].level));
        }
        setFormat(pMediaProfileList_t, VIDEO_H264);
        pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_HIGH;
    }
}

static void createHEVCQualtiyMediaProfile(uint32_t opID, MediaProfileList_t *pMediaProfileList_t)
{
    int nQualityCount = pMediaProfileList_t->mQualityList[VIDEO_HEVC].size();
    KeyedVector<video_quality_t, video_media_profile_t> *prQMP = &(pMediaProfileList_t->mQualityMediaProfile[VIDEO_HEVC]);
    prQMP->clear();

    if (nQualityCount > 0)
    {
        createQualityMediaProfile(VIDEO_HEVC, pMediaProfileList_t);
    }
    else
    {
        ALOGI("createHEVCQualtiyMediaProfile: default setting(op%d)", opID);
         QualityInfo_t qualityInfo[4];
        createHEVCQualityMediaProfileByOperator(opID, qualityInfo);

        for(int i = 0; i < 4; i++)
        {
            prQMP->add(qualityInfo[i].quality, *getMediaProfileEntry(pMediaProfileList_t, qualityInfo[i].format, qualityInfo[i].profile, qualityInfo[i].level));
        }
        setFormat(pMediaProfileList_t, VIDEO_HEVC);
        pMediaProfileList_t->mDefault_quality = VIDEO_QUALITY_HIGH;
        //rVTMP.add(VIDEO_QUALITY_FINE,   *getMediaProfileEntry(VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3));
        //rVTMP.add(VIDEO_QUALITY_HIGH,   *getMediaProfileEntry(VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2_1));
        //rVTMP.add(VIDEO_QUALITY_MEDIUM, *getMediaProfileEntry(VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2));
        //rVTMP.add(VIDEO_QUALITY_LOW,    *getMediaProfileEntry(VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_1));
        //setFormat(VIDEO_HEVC);
    }
}

MediaProfileList_t *getMediaProfileListInst(uint32_t opID)
{
    static bool MediaProfileListLockInit;
    static pthread_mutex_t MediaProfileListLock;
    static MediaProfileList_t *pMediaProfileList_t;
    video_property_t* pvideo_property_t = getVideoProperty();

    if (MediaProfileListLockInit == false)
    {
        pthread_mutex_init(&MediaProfileListLock, NULL);
        ALOGI("pthread_mutex_init MediaProfileListLock");
        MediaProfileListLockInit = true;
    }

    pthread_mutex_lock(&MediaProfileListLock);

    if (pMediaProfileList_t != NULL)
    {
        if (pMediaProfileList_t->mOPID != opID)
        {
            // operator is changed
            delete pMediaProfileList_t;
            pMediaProfileList_t = NULL;
        }
    }

    if (pMediaProfileList_t == NULL)
    {
        pMediaProfileList_t = new MediaProfileList_t;
        // Create media profile table
        createMediaProfile(opID, pMediaProfileList_t);
        //createMediaProfile(1, &pMediaProfileList_t[1]); // SIM1

        // Quality setting

        if (pvideo_property_t->format == VIDEO_HEVC)
        {
            createHEVCQualtiyMediaProfile(opID, pMediaProfileList_t);
        }

        {
            createH264QualtiyMediaProfile(opID, pMediaProfileList_t);
        }

       if (pvideo_property_t->format != VIDEO_HEVC && pvideo_property_t->format != VIDEO_H264)
        {
            ALOGE("getMediaProfileListInst: Media profile not available(op%d), format: %d", opID, pvideo_property_t->format);
        }
    }
    pthread_mutex_unlock(&MediaProfileListLock);
    return pMediaProfileList_t;
}

