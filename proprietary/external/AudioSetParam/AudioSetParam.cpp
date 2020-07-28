#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <media/AudioSystem.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSetParam"


using namespace android;

static char key_aurisys[] = "AURISYS";

static void usage(const char *name);

static int getCommands();
static int getOptCommands(int argc, char *argv[]);
static int dispatchCommand(char *cmd);

static int runSetParam(const char *cmd);
static int runGetParam(const char *cmd);



int main(int argc, char *argv[]) {
    int retval = 0;

    if (argc == 1) { // interaction mode
        retval = getCommands();
    } else if (argc == 2) { // single mode
        retval = dispatchCommand(argv[1]);
    } else { // arg mode
        retval = getOptCommands(argc, argv);
    }

    return retval;
}

static int getCommands() {
    int retval = 0;

    char cmd[1024];

    while (true) {
        fprintf(stderr, "\nplease enter command, ex: 'GET_XXX_ENABLE', 'SET_XXX_ENABLE=0', 'SET_XXX_ENABLE=1', and '0' for exit\n\n");
        if (fgets(cmd, 1024, stdin) == NULL) {
            retval = -1;
            break;
        }

        if (cmd[0] == '0') { // exit
            retval = 0;
            break;
        }
        retval = dispatchCommand(cmd);
    }

    return retval;
}


static void usage(const char *name) {
    fprintf(stderr, "Usage: %s [-s parameter=value] [-g parameter]\n", name);
    fprintf(stderr, "    -s    set parameter\n");
    fprintf(stderr, "    -g    get parameter\n");
    fprintf(stderr, "If no options, it will run in the command line mode.'\n");
}


static int getOptCommands(int argc, char *argv[]) {
    int retval = 0;

    const char *const progname = argv[0];
    int opt = 0;

    while ((opt = getopt(argc, argv, "s:g:")) != -1) {
        switch (opt) {
        case 's':
            if (strchr(optarg, '=') == NULL) { // no '=', it's not set function
                usage(progname);
                retval = -1;
                goto EXIT;
            }
            retval = runSetParam(optarg);
            break;
        case 'g':
            if (strrchr(optarg, '=') != NULL) { // has '=', it's set function
                usage(progname);
                retval = -1;
                goto EXIT;
            }
            retval = runGetParam(optarg);
            break;
        default:
            usage(progname);
            retval = -1;
            goto EXIT;
        }
    }

EXIT:
    return retval;
}


static int dispatchCommand(char *cmd) {
    int retval = 0;

    // remove '\n'
    char *ptr_newline = strchr(cmd, '\n');
    if (ptr_newline != NULL) {
        *ptr_newline = '\0';
    }

    if (strchr(cmd, '=') != NULL) { // has '=', it's set function
        retval = runSetParam(cmd);
    } else {
        retval = runGetParam(cmd);
    }

    return retval;
}


static int runSetParam(const char *cmd) {
    status_t ret = NO_ERROR;
    int retval = 0;

    try {
        ret = AudioSystem::setParameters(0, String8(cmd));
    } catch (...) {
        fprintf(stderr, "cmd \"%s\" setParameters fail!! ret = %d\n", cmd, ret);
        return -1;
    }

    if (strncmp(cmd, key_aurisys, sizeof(key_aurisys) - 1) == 0) {
        retval = runGetParam("AURISYS_SET_PARAM_RESULT");
        if (retval != 0) {
            fprintf(stderr, "SET_FAIL\n");
        }
    }

    return retval;
}


static int runGetParam(const char *cmd) {
    const char *get_string = NULL;
    int retval = 0;

    try {
        get_string = AudioSystem::getParameters(0, String8(cmd)).string();
    } catch (...) {
        fprintf(stderr, "cmd \"%s\" getParameters fail!!\n", cmd);
        return -1;
    }

    if (strncmp(cmd, key_aurisys, sizeof(key_aurisys) - 1) == 0) {
        const char *value = strstr(get_string, "=");
        if (value == NULL) {
            retval = -1;
            fprintf(stderr, "\"%s\" without '='!!\n", get_string);
        } else if (strlen(value) == 1) {
            retval = -1;
            fprintf(stderr, "\"%s\" without value after '='!!\n", get_string);
        } else {
            retval = 0;
            fprintf(stderr, "%s\n", value + 1);
        }
    } else {
        retval = 0;
        fprintf(stderr, "%s\n", get_string);
    }

    return retval;
}



