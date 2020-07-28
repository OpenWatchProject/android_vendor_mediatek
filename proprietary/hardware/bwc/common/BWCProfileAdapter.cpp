#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>

extern "C"
{
#include <stdio.h>
#include <stdarg.h>
}

#include "BWCProfileAdapter.h"
#include  "bandwidth_control_private.h"


BWCProfile::BWCProfile(int bwcProfile, const char * _name, int _smiProfile, const char* _emiProfileStr){
    this->bwcProfile = bwcProfile;
    this->smiProfile = _smiProfile;
    this->emiProfileStr = _emiProfileStr;
    this->name = _name;
}

int BWCProfile::getBwcProfile(void){
    return this->bwcProfile;
}

int BWCProfile::getSMIProfile(void){
    return this->smiProfile;
}
const char * BWCProfile::getEmiProfileStr(void){
    return this->emiProfileStr;

}

const char * BWCProfile::getName(){
    return this->name;
}

BWCProfileAdapter::BWCProfileAdapter(BWCProfile * _profiles, int total){
    this->profiles = _profiles;
    this->totalProfiles = total;
    this->defaultProfiles = NULL;
    this->totalDefaultProfiles = 0;
}

BWCProfile * BWCProfileAdapter::getProfile(int bwcProfile){
    for(int i=0; i< this->totalProfiles; i++){
        BWCProfile * temp = this->profiles+i;
        if(temp != NULL && temp->getBwcProfile() == bwcProfile){
            return temp;
        }
    }
    return NULL;
}

int BWCProfileAdapter::getSMIProfile(int bwcProfile){
    if(this->profiles == NULL){
        return -1;
    }
    BWCProfile * temp = this->getProfile(bwcProfile);
    if(temp == NULL){
        return -1;
    } else{
        return temp->getSMIProfile();
    }
}

const char * BWCProfileAdapter::getEmiProfileStr(int bwcProfile){
    if(this->profiles == NULL){
        return NULL;
    }
    BWCProfile * temp = this->getProfile(bwcProfile);
    if(temp == NULL){
        return NULL;
    } else{
        return temp->getEmiProfileStr();
    }
}

const char * BWCProfileAdapter::getName(int bwcProfile){
    if(this->profiles == NULL){
        return NULL;
    }
    BWCProfile * temp = this->getProfile(bwcProfile);
    if(temp == NULL){
        return NULL;
    } else{
        return temp->getName();
    }
}

void BWCProfileAdapter::dump(){
    const char * name = NULL;
    const char * emiProfileStr = NULL;

    for(int i = 0;i< this->totalProfiles;i++){
        BWCProfile * temp = this->profiles+i;
        name = temp->getName();
        if(name == NULL){
            name = "unknown profile";
        }
        emiProfileStr = temp->getEmiProfileStr();
        if (emiProfileStr == NULL){
            emiProfileStr = "NONE";
        }
        // cout<<temp->getName()<<":"<<temp->getSMIProfile()<<":"<<temp->getEmiProfileStr()<<endl;
        BWC_INFO("%s:%d,%s", name, temp->getSMIProfile(), emiProfileStr);
        
    }
}

BWCProfileHelper::BWCProfileHelper(BWCProfileAdapter * _default, BWCProfileAdapter * _extend){
    this->defaultAdapter = _default;
    this->extendAdapter = _extend;
}

int BWCProfileHelper::getSMIProfile(int bwcProfile){
    int temp = 0;

    if(this->defaultAdapter == NULL)
    return -1;

    if(this->extendAdapter != NULL){
        temp = this->extendAdapter->getSMIProfile(bwcProfile);
    }
    if(temp <0){
        return this->defaultAdapter->getSMIProfile(bwcProfile);
    } else{
        return temp;
    }
}

const char * BWCProfileHelper::getEmiProfileStr(int bwcProfile){
    const char * temp = NULL;
    if(this->defaultAdapter == NULL) {
    return NULL;
    }

    if(this->extendAdapter != NULL){
        temp = this->extendAdapter->getEmiProfileStr(bwcProfile);
    }
    if(temp == NULL){
        return this->defaultAdapter->getEmiProfileStr(bwcProfile);
    } else{
        return temp;
    }
}
const char * BWCProfileHelper::getName(int bwcProfile){
    const char * temp = NULL;
    if(this->defaultAdapter == NULL)
    return NULL;

    if(this->extendAdapter != NULL){
        temp = this->extendAdapter->getName(bwcProfile);
    }
    if(temp == NULL){
        return this->defaultAdapter->getName(bwcProfile);
    } else{
        return temp;
    }
}
