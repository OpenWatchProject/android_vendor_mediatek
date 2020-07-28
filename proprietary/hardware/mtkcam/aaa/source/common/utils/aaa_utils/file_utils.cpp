#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "string_utils.h"

#define CANONICAL_FAIL_LENGTH 500
void _get_canonical_filepath(const char *path, char *buf, size_t bufsiz)
{
    if( NULL == path || NULL == buf || 0 >= bufsiz )
    {
        if(buf)
        {
            buf[0] = 0;
        }
        return;
    }
    if( (-1) == readlink(path, buf, bufsiz) )
    {
        if( NULL == strncpy(buf, path, bufsiz) )
        {
            buf[0] = 0;
            return;
        }
    }
}

int getFileCount(const char *fname, int *fcnt, int defValue)
{
    FILE *fp;
    char s[16];
    int ret;

    /* verify arguments */
    if (!fname || !fcnt)
        return -1;

    *fcnt = defValue;
    char in_filename[CANONICAL_FAIL_LENGTH];
    _get_canonical_filepath(fname, in_filename, CANONICAL_FAIL_LENGTH);
    /* file operations */

    fp = fopen(in_filename, "rb");
    if (!fp)
        return -1;
    ret = fscanf(fp, "%15s", s);
    fclose(fp);

    /* verify value, should be positive value */
    if (ret != 1 || (strlen(s) < 1) || !isInt(s))
        return -1;

    ret = atoi(s);
    /* verify value, should be less than INT_MAX */
    if (ret < 0)
        return -1;
    *fcnt = ret;

    return 0;
}

int setFileCount(const char *fname, int fcnt)
{
    FILE *fp;
    char in_filename[CANONICAL_FAIL_LENGTH];

    /* verify arguments */
    if (!fname)
        return -1;

    _get_canonical_filepath(fname, in_filename, CANONICAL_FAIL_LENGTH);

    /* file operations */
    fp = fopen(in_filename, "wt");
    if (!fp)
        return -1;
    fprintf(fp, "%d", fcnt);
    fclose(fp);

    return 0;
}

int createDir(const char *dir)
{
    /* verify arguments */
    if (!dir)
        return -1;

    return mkdir(dir, S_IRWXU | S_IRWXG);
}

int dumpBufToFile(char* fileName, char* folderName, void* buf, int size)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        mkdir(folderName, S_IRWXU | S_IRWXG);
        return -1;
    } else
    {
        fwrite(buf, 1, size, fp);
        fclose(fp);
        return 0;
    }
}

