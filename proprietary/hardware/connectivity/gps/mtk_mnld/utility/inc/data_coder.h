#ifndef __DATA_CODER_H__
#define __DATA_CODER_H__

#ifdef __cplusplus
extern "C" {
#endif

char        get_byte(char* buff, int* offset, int src_len);
short       get_short(char* buff, int* offset, int src_len);
int         get_int(char* buff, int* offset, int src_len);
long long   get_long(char* buff, int* offset, int src_len);
float       get_float(char* buff, int* offset, int src_len);
double      get_double(char* buff, int* offset, int src_len);
char*       get_string(char* buff, int* offset, int src_len);
char*       get_string2(char* buff, int* offset, int src_len);
int         get_binary(char* buff, int* offset, char* output, int src_len, int des_len);

void put_byte(char* buff, int* offset, const char input);
void put_short(char* buff, int* offset, const short input);
void put_int(char* buff, int* offset, const int input);
void put_long(char* buff, int* offset, const long long input);
void put_float(char* buff, int* offset, const float input);
void put_double(char* buff, int* offset, const double input);
void put_string(char* buff, int* offset, const char* input);
void put_binary(char* buff, int* offset, const char* input, int len);

#ifdef __cplusplus
}
#endif

#endif

