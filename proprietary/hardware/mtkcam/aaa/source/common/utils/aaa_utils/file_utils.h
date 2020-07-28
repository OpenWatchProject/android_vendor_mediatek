#pragma once

int getFileCount(const char *fname, int *fcnt, int defValue);
int setFileCount(const char *fname, int fcnt);
int createDir(const char *dir);
int dumpBufToFile(char* fileName, char* folderName, void* buf, int size);

