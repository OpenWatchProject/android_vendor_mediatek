#include <stdio.h>
#include <iostream>
#include <string>

#include <mtk_log.h>
// #include "netcap/net_cap.h"
#include "net_cap.h"
#include "RuleContainer.h"
#include "WpfaRingBuffer.h"
#include "WpfaCppUtils.h"
#include "WpfaDriver.h"
extern "C" {
#include "FilterParsingUtilis.h"
}

//This is a wrapper cpp for WPFA c files.

RuleContainer *mRuleContainer = NULL;
WpfaRingBuffer *mWpfaRingBuffer = NULL;
WpfaDriver *mWpfaDriver = NULL;

//Call back functions for wpfa driver
int m2aVersionNotify(void * data);
int m2aExecuteFilterReg(void * data);
int m2aExecuteFilterDeReg(void * data);

int dump_hex(unsigned char *data, int len);

extern "C" void initialRuleContainer() {
    if(mRuleContainer == NULL) {
        mRuleContainer = new RuleContainer();
        mtkLogD(CPP_UTIL_TAG, "initialRuleContainer!");
    }
    else {
        mtkLogD(CPP_UTIL_TAG, "RuleContainer Already initialed!");
    }
}

extern "C" int addFilter(int fid, WPFA_filter_reg_t filter) {
    if(mRuleContainer == NULL) {
        return -1;
    }
    return mRuleContainer->addFilter(fid, filter);
}
extern "C" int removeFilter(int fid) {
    if (mRuleContainer == NULL) {
        return -1;
    }
    return mRuleContainer->removeFilter(fid);
}
extern "C" WPFA_filter_reg_t* getFilterFromMap(int fid) {
    if (mRuleContainer != NULL) {
        return mRuleContainer->getFilterFromMap(fid);
    }

    // ERROR!
    mtkLogD(CPP_UTIL_TAG, "getFilterFromMap: mRuleContainer not found");
    return NULL;
}
extern "C" int getFilterMapSize() {
    if (mRuleContainer == NULL) {
        return -1;
    }
    return mRuleContainer->getFilterMapSize();
}
extern "C" int findFilterById(int fid) {
    if (mRuleContainer == NULL) {
        return -1;
    }
    return mRuleContainer->findFilterById(fid);
}
extern "C" void dumpAllFilterId() {
    if (mRuleContainer == NULL) {
        return;
    }
    mRuleContainer->dumpAllFilterId();
}

extern "C" void deinitialRuleContainer() {
    mRuleContainer = NULL;
}

extern "C" void initialA2MRingBuffer() {
    if (mWpfaRingBuffer == NULL) {
        mWpfaRingBuffer = new WpfaRingBuffer();
        mWpfaRingBuffer->initRingBuffer();
        mtkLogD(CPP_UTIL_TAG, "initialA2MRingBuffer!");
    }
    else {
        mtkLogD(CPP_UTIL_TAG, "A2MRingBuffer Already initialed!");
    }
}

extern "C" void readRingBuffer() {
    unsigned char tmpStr[500];
    int ret = 0;

    mtkLogD(CPP_UTIL_TAG, "enter readRingBuffer()\n");
    if (mWpfaRingBuffer == NULL) {
        //cout<<"readRingBuffer Error! not initialed" <<endl;
        mtkLogD(CPP_UTIL_TAG, "readRingBuffer Error! not initialed");
    }
    else {
        mtkLogD(CPP_UTIL_TAG, "enter readRingBuffer's while loop\n");
        while(1) {
            mWpfaRingBuffer->lock("Consumer");
            mWpfaRingBuffer->waitCanRead("Consumer");
            //mWpfaRingBuffer->getRegionInfoForReader(mRegion);
            mWpfaRingBuffer->setState(RING_BUFFER_STATE_READING);
            mWpfaRingBuffer->unlock("Consumer");
            /*************** blocking call ***************/
            mtkLogD(CPP_UTIL_TAG, "sending DataPackage To Modem");
            ret = mWpfaDriver->sendDataPackageToModem(mWpfaRingBuffer);
            //mWpfaRingBuffer->readDataFromRingBuffer(tmpStr, mRegion->read_idx,mRegion->data_size);
            /*************** blocking call ***************/

            if (ret < 0) {
                mtkLogD(CPP_UTIL_TAG, "send DataPackage To Modem fail!");
            } else {
                //cout << "Consumer ==> read size:" << ret << endl;
                mtkLogD(CPP_UTIL_TAG, "send DataPackage To Modem OK");
                mWpfaRingBuffer->lock("Consumer");
                mWpfaRingBuffer->readDone();
                mWpfaRingBuffer->signalCanWrite("Consumer");
                mWpfaRingBuffer->unlock("Consumer");
            }
        }
    }
}

extern "C" void writeRingBuffer(unsigned char *data ,int len) {
    int ret = 0;
    cout << "writeRingBuffer()" << endl;

    if (mWpfaRingBuffer == NULL) {
        cout<<"readRingBuffer Error! not initialed" <<endl;
    }
    else{
        mtkLogD(CPP_UTIL_TAG, "writeRingBuffer: enter writeRingBuffer()\n");
        mWpfaRingBuffer->lock("Producer");
        mWpfaRingBuffer->waitCanWrite("Producer", len);
        ret = mWpfaRingBuffer->writeDataToRingBuffer(data, len);
        if (ret <= 0) {
            mtkLogD(CPP_UTIL_TAG, "writeRingBuffer: runReaderLoop() write fail!!!");
        } else {
            mtkLogD(CPP_UTIL_TAG, "writeRingBuffer: ==> write size: %d", ret);
        }
        mWpfaRingBuffer->signalCanRead("Producer");
        mWpfaRingBuffer->unlock("Producer");
    }
}

extern "C" void wpfaDriverInit() {
    mWpfaDriver = WpfaDriver::getInstance();
    wpfaDriverCbRegister();
}

extern "C" void wpfaDriverCbRegister() {
    if(mWpfaDriver != NULL) {
        mWpfaDriver->registerCallback(EVENT_M2A_WPFA_VERSION, m2aVersionNotify);
        mWpfaDriver->registerCallback(EVENT_M2A_REG_DL_FILTER, m2aExecuteFilterReg);
        mWpfaDriver->registerCallback(EVENT_M2A_DEREG_DL_FILTER, m2aExecuteFilterDeReg);
    } else {
        mtkLogD(CPP_UTIL_TAG, "wpfaDriverCbRegister: mWpfaDriver NULL");
    }
}

extern "C" int a2mWpfaInitNotify() {
    if(mWpfaDriver != NULL) {
        mWpfaDriver->notifyWpfaInit();
    } else {
        mtkLogD(CPP_UTIL_TAG, "a2mWpfaInitNotify: mWpfaDriver NULL");
    }
    return 1;
}

extern "C" int a2mWpfaVersionNotify() {
    if(mWpfaDriver != NULL) {
        // the tid(first para), will be removed from this api.
        mWpfaDriver->notifyWpfaVersion(CURRENT_AP_FILTER_VERSION, 0);
    } else {
        mtkLogD(CPP_UTIL_TAG, "a2mWpfaVersionNotify: mWpfaDriver NULL");
    }
    return 1;
}

//Call back function for DRV
int m2aVersionNotify(void * data) {
    wifiproxy_ap_md_filter_ver_t *msg =
            (wifiproxy_ap_md_filter_ver_t*)data;
    mtkLogD(CPP_UTIL_TAG, "m2aVersionNotify: md ver:%d",
            msg->md_filter_ver);
    if(msg->md_filter_ver != CURRENT_AP_FILTER_VERSION){
        mtkLogD(CPP_UTIL_TAG, "m2aVersionNotify: error md ver:%d",
            msg->md_filter_ver);
        return -1;
    }
    setUsingFilterVersion(msg->md_filter_ver);
    return 1;
}

//Call back function for DRV
int m2aExecuteFilterReg(void * data) {
    wifiproxy_m2a_reg_dl_filter_t *filterPtr =
            (wifiproxy_m2a_reg_dl_filter_t*)data;
    return executeFilterReg(*filterPtr);
}

//Call back function for DRV
int m2aExecuteFilterDeReg(void * data) {
    uint32_t *fid = (uint32_t*) data;
    return executeFilterDeReg(*fid);
}

int dump_hex(unsigned char *data, int len) {
    int i, counter, rest;
    char * dumpbuffer;
    char printbuf[1024];

    dumpbuffer = (char*)malloc(16*1024);
    if (!dumpbuffer) {
        mtkLogD(CPP_UTIL_TAG, "DUMP_HEX ALLOC memory fail \n");
        return -1;;
    }

    if (len >8*1024 ){
        mtkLogD(CPP_UTIL_TAG, "trac the packet \n");
        len = 8*1024;
    }

    //memset((void *)dumpbuffer,0,16*1024);
    memset(dumpbuffer, 0, 16*1024);

    for (i = 0 ; i < len ; i++) {
       sprintf(&dumpbuffer[i*2],"%02x",data[i]);
    }
    dumpbuffer[i*2] = '\0' ;

    // android log buffer =1024bytes, need to splite the log
    counter = len/300 ;
    rest = len - counter*300 ;

    mtkLogD(CPP_UTIL_TAG, " Data Length = %d ,counter =%d ,rest =%d", len ,counter,rest);

    mtkLogD(CPP_UTIL_TAG, " NFQUEU Data: ");
    for (i = 0 ; i < counter ; i++) {
        memset(printbuf, 0, sizeof(printbuf));
        memcpy(printbuf ,dumpbuffer+i*600 , 300*2) ;
        printbuf[600]='\0';
        mtkLogD(CPP_UTIL_TAG, "data:%s",printbuf);
    }

    //for rest data
    memset(printbuf, 0, sizeof(printbuf));
    memcpy(printbuf, dumpbuffer+counter*600, rest*2) ;
    printbuf[rest*2] = '\0';
    mtkLogD(CPP_UTIL_TAG, "%s", printbuf);

    free(dumpbuffer);
    return 1;
}



