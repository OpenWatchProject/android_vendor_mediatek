#include <string.h>
#include <ctype.h>

int isInt(const char *str)
{
    int len;
    int i;

    /* verify argument */
    if (!str)
        return 0;

    /* check string */
    len = strlen(str);
    for (i = 0; i < len; i++)
        if (!isdigit(str[i]) && str[i] != '-')
            return 0;

    return 1;
}

