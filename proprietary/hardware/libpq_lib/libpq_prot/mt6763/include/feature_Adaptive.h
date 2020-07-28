#ifndef FEATURE_ADAPTIVE_H
#define FEATURE_ADAPTIVE_H

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define OutputDummy 14

int DumpRegBufferToBuffer(const char ** CmparamList, int ListNum, void* RegBuffer, char* DumpBuffer, int& Offset, int MaxBufferSize, int RegBufferNum = 0);
int DumpRegToBuffer(const char *CmparamName, int RegValue, char* DumpBuffer, int& Offset, int MaxBufferSize);
int Dump2DArrayToBuffer(const char *CmparamName, int* RegValue, char* DumpBuffer, int& Offset, int Limity, int Limitx, int MaxBufferSize);
int Dump3DArrayToBuffer(const char *CmparamName, int* RegValue, char* DumpBuffer, int& Offset, int Limity, int Limitx, int Limitc, int MaxBufferSize);

#endif