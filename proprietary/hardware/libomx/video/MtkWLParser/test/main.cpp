#include "stdlib.h"
#include "stdio.h"
#include "malloc.h"
#include <libexpat/expat.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include "MtkWLParser.h"

#define PARSER_LIB_FULL_NAME "/vendor/lib/libwlparser.so"
#define _LIB_FULL_NAME "/vendor/lib/libvcodecdrv.so"


typedef void *(*inti_wl_parser)(char *);
typedef bool (*wl_get_app_valid_name)(void *, char *);
typedef bool (*wl_deinit_wl_parser)(void *);

int main(int argc, char *argv[])
{
    P_WLPARSER_T pwlParser = NULL;
    void *pWLParserLibrary, *func;

    void *(*pWLInitWLParser)(char *) = NULL;
    bool (*pWLGetAppValidName)(void *, char *) = NULL;
    bool (*pWLDeinitWLParser)(void *) = NULL;

    if (argc != 3) {
        printf("[Usage] wl_test [FEATURE] [APPLICATION] %d\n", argc);
        return -1;
    }
    pWLParserLibrary = dlopen(PARSER_LIB_FULL_NAME, RTLD_NOW);
    if (pWLParserLibrary == NULL) {
        printf("[ERROR] dlopen White List Parser failed, %s %s\n", PARSER_LIB_FULL_NAME, dlerror());
    }
    else {
        func = dlsym(pWLParserLibrary, "WLInitWLParser");
        pWLInitWLParser = (inti_wl_parser)(func);
        func = dlsym(pWLParserLibrary, "WLGetAppValidName");
        pWLGetAppValidName = (wl_get_app_valid_name)(func);
        func = dlsym(pWLParserLibrary, "WLDeinitWLParser");
        pWLDeinitWLParser = (wl_deinit_wl_parser)(func);
        if (pWLInitWLParser == NULL || pWLGetAppValidName == NULL || pWLDeinitWLParser == NULL) {
            printf("load function fail ?\n");
            return -1;
        }
        else {
            pwlParser = (P_WLPARSER_T) pWLInitWLParser(argv[1]);
            if (pwlParser == NULL) {
                printf("Init white list parser fail\n");
                return -1;
            }
            else {
                printf("WL parser 0x%08x\n", pwlParser);
            }
            bool bSuccessful = pWLGetAppValidName(pwlParser, argv[2]);
            P_Option pOptionTemp = NULL;
            printf("start to dump option (%d)\n", bSuccessful);
            if (bSuccessful == true && pwlParser->mMagicCode == WLPARSER_MAGIC) {
                for (int i = 0; i < pwlParser->mOptionCount; i++) {
                    pOptionTemp = pwlParser->mOptionList[i];
                    if (pOptionTemp != NULL) {
                        printf("%d, %s : %s\n", i, pOptionTemp->_name, pOptionTemp->_value);
                    }
                    else {
                        printf("pOptionTemp is NULL (%d)", i);
                    }
                }
            }
            pWLDeinitWLParser(pwlParser);
        }
        dlclose(pWLParserLibrary);
    }

    return 0;
}
