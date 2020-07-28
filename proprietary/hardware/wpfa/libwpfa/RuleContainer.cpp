#include "RuleContainer.h"
#include <mtk_log.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>



//only one thread will add/remove/get filters at the same time,
//so there's no need to have a mutex lock.
int RuleContainer::addFilter(int fid, WPFA_filter_reg_t filter) {
    iter = filterMap.find(fid);
    if (iter != filterMap.end()) {
       mtkLogD(RC_LOG_TAG, "addFilter: error! The fid: %d, already in the map!", fid);
       return -1;
    }
    copyFilter(fid, filter);
    mtkLogD(RC_LOG_TAG, "addFilter: Adding %d into the map", fid);
    return 1;
}

//only one thread will add/remove/get filters at the same time,
//so there's no need to have a mutex lock.
int RuleContainer::removeFilter(int fid) {
    iter = filterMap.find(fid);
    if (iter != filterMap.end()) {
        mtkLogD(RC_LOG_TAG, "removeFilter: The fid %d, is in the map, removing %d",
            fid, iter->first);
        filterMap.erase(iter);
        return 1;
    }
    else {
        mtkLogD(RC_LOG_TAG, "removeFilter: error! The fid: %d, not found in the map!", fid);
    }
    return -1;
}

//only one thread will add/remove/get filters at the same time,
//so there's no need to have a mutex lock.
WPFA_filter_reg_t* RuleContainer::getFilterFromMap(int fid) {
    iter = filterMap.find(fid);

    if (iter != filterMap.end()) {
        mtkLogD(RC_LOG_TAG, "getFilterFromMap: The fid %d, is in the map, return it", fid);
        return &iter->second;
    }
    else {
        mtkLogD(RC_LOG_TAG, "getFilterFromMap: error: The fid: %d, not found in the map!", fid);
    }

    /*//Use dummyFilter to indicate Error
    WPFA_filter_reg_t dummyFilter;
    dummyFilter.ip_ver = ERROR_IP_VER;*/
    return NULL;
}

//only one thread will add/remove/get filters at the same time,
//so there's no need to have a mutex lock.
int RuleContainer::findFilterById(int fid) {
    iter = filterMap.find(fid);

    if (iter != filterMap.end()) {
        //cout<<"findFilterById: The fid:"<<fid<<" is in the map"<<endl;
        return 1;
    }
    else {
        //cout<<"findFilterById: The fid:"<<fid<<" not found in the map!" <<endl;
        return 0;
    }
}

int RuleContainer::getFilterMapSize() {
    return filterMap.size();
}

void RuleContainer::dumpAllFilterId() {
    stringstream oss("");
    //traversal
    for(iter = filterMap.begin(); iter != filterMap.end(); iter++) {
        oss << iter->first <<", ";
    }
    string str(oss.str());
    mtkLogD(RC_LOG_TAG, "dumpAllFilterId: %s", str.c_str());
}

void RuleContainer::copyFilter(int fid, WPFA_filter_reg_t filter) {
    filterMap[fid].filter_config = filter.filter_config;
    filterMap[fid].ip_ver = filter.ip_ver;
    filterMap[fid].protocol = filter.protocol;
    filterMap[fid].icmp_type = filter.icmp_type;
    filterMap[fid].icmp_code = filter.icmp_code;

    memcpy(filterMap[fid].ip_src, filter.ip_src, 16);
    memcpy(filterMap[fid].ip_dest, filter.ip_dest, 16);

    filterMap[fid].src_port = filter.src_port;
    filterMap[fid].src_port_mask = filter.src_port_mask;

    filterMap[fid].dst_port = filter.dst_port;
    filterMap[fid].dst_port_mask = filter.dst_port_mask;

    filterMap[fid].esp_spi = filter.esp_spi;
    filterMap[fid].esp_spi_mask = filter.esp_spi_mask;
}

RuleContainer::RuleContainer() {
    filterMap.clear();
    //cout << "RuleContainer-new()" << endl;
    mtkLogD(RC_LOG_TAG, "RuleContainer-new()");
}

RuleContainer::~RuleContainer() {
    filterMap.clear();
    //cout << "RuleContainer-del()" << endl;
    mtkLogD(RC_LOG_TAG, "RuleContainer-del()");
}
