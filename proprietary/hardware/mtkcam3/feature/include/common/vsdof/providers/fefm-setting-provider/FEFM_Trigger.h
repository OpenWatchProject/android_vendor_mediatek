#ifndef __FEFM_TRIGGER_H__
#define __FEFM_TRIGGER_H__

#define REQULAR_TRIGGER_COUNT_DEFAULT   (6)

#include <stdio.h>
#include <stdlib.h>
#include <utils/Mutex.h>

typedef enum
{
    FEFM_TRIGGER_FOCUS_FF = 0,
    FEFM_TRIGGER_FOCUS_AF
}
FEFM_TRIGGER_FOCUS_TYPE_ENUM ;

typedef enum
{
    FEFM_TRIGGER_SCENARIO_PV = 0,
    FEFM_TRIGGER_SCENARIO_IC
}
FEFM_TRIGGER_SCENARIO_TYPE_ENUM ;

typedef enum
{
    FEFM_TRIGGER_NO = 0,
    FEFM_TRIGGER_YES
}
FEFM_TRIGGER_TYPE_ENUM ;

struct CheckQueueElement
{
    int magicNumber[2] = {0};
    int isAFValid[2]   = {1};
};

struct CheckQueueElement_FF
{
    int magicNumber = 0;
    bool isTrigger = false;
};

#define CHECK_QUEUE_SIZE 64

class MTKFEFMTrigger
{
public:
    static MTKFEFMTrigger* getInstance();

    static void destroyInstance();

    MTKFEFMTrigger();
    virtual ~MTKFEFMTrigger();

    void FEFMTrigger_init( bool isMainAF, bool isAuxiAF );

    void FEFMTrigger_set_reqular_trigger( int count );

    int FEFMTrigger_query( int main1MagicNumber, int main2MagicNumber, int af_valid_main, int af_valid_auxi );

    int FEFMTrigger_get_debug_info();

private:
    int __queryAF( int main1MagicNumber, int main2MagicNumber, int af_valid_main, int af_valid_auxi );
    int __queryFF( int main1MagicNumber );

private:
    int focus_type_main = FEFM_TRIGGER_FOCUS_AF;
    int focus_type_auxi = FEFM_TRIGGER_FOCUS_AF;

    int para_reqular_trigger_count = REQULAR_TRIGGER_COUNT_DEFAULT;

    int af_valid_main_now = -1;
    int af_valid_main_pre = -1;

    int af_valid_auxi_now = -1;
    int af_valid_auxi_pre = -1;

    int reqular_trigger_count = 0;

    //Circular queue
    CheckQueueElement       __afQueue[CHECK_QUEUE_SIZE];
    CheckQueueElement_FF    __ffQueue[CHECK_QUEUE_SIZE];
    int __curIndex = 0;

    static android::Mutex __instanceLock;
    static android::Mutex __runLock;

    static MTKFEFMTrigger* __FEFMTrigger;
} ;


#endif