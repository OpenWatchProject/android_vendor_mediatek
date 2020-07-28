#ifndef __BWC_PROFILE_ADAPTER_H__
#define __BWC_PROFILE_ADAPTER_H__

class BWCProfile{
private:
    int bwcProfile;
    int smiProfile;
    const char * emiProfileStr;
    const char * name;
public:
    BWCProfile(int bwcProfile, const char * name, int smiProfile, const char* emiProfileStr);
    int getBwcProfile(void);
    int getSMIProfile(void);
    const char * getEmiProfileStr();
    const char * getName();
};

class BWCProfileAdapter{
private:
    BWCProfile * profiles;
    int totalProfiles;
    BWCProfile * defaultProfiles;
    int totalDefaultProfiles;

public:
    BWCProfileAdapter(BWCProfile * profiles, int totalProfiles);
    int getSMIProfile(int bwcProfile);
    const char * getEmiProfileStr(int bwcProfile);
    const char * getName(int bwcProfile);
    BWCProfile * getProfile(int bwcProfile);
    void dump();
};

class BWCProfileHelper{
private:
    BWCProfileAdapter * defaultAdapter;
    BWCProfileAdapter * extendAdapter;
public:
    BWCProfileHelper(BWCProfileAdapter * defaultAdapter, BWCProfileAdapter* extendAdapter);
    int getSMIProfile(int bwcProfile);
    const char * getEmiProfileStr(int bwcProfile);
    const char * getName(int bwcProfile);

};

void testAdapter( void );

#endif 
