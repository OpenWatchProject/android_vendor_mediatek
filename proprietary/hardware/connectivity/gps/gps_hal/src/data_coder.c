#include <string.h>

#ifndef MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

//===============================================================
// get
char get_byte(char* buff, int* offset, int src_len) {
    char ret = 0;
    if (*offset >= 0 && *offset < src_len) {
        ret = buff[*offset];
        *offset += 1;
    }
    return ret;
}

short get_short(char* buff, int* offset, int src_len) {
    short ret = 0;
    ret |= get_byte(buff, offset, src_len) & 0xff;
    ret |= (get_byte(buff, offset, src_len) << 8);
    return ret;
}

int get_int(char* buff, int* offset, int src_len) {
    int ret = 0;
    ret |= get_short(buff, offset, src_len) & 0xffff;
    ret |= (get_short(buff, offset, src_len) << 16);
    return ret;
}

long long get_long(char* buff, int* offset, int src_len) {
    long long ret = 0;
    ret |= get_int(buff, offset, src_len) & 0xffffffffL;
    ret |= ((long long)get_int(buff, offset, src_len) << 32);
    return ret;
}

float get_float(char* buff, int* offset, int src_len) {
    float ret;
    int tmp = get_int(buff, offset, src_len);
    ret = *((float*)&tmp);
    return ret;
}

double get_double(char* buff, int* offset, int src_len) {
    double ret;
    long long tmp = get_long(buff, offset, src_len);
    ret = *((double*)&tmp);
    return ret;
}

char* get_string(char* buff, int* offset, int src_len) {
    char ret = get_byte(buff, offset, src_len);
    if (ret == 0) {
        return NULL;
    } else {
        char* p = NULL;
        int len = get_int(buff, offset, src_len);
        if (*offset < 0 || *offset >= src_len) {
            *offset = 0;
        }
        if ((len > src_len - *offset) || len < 0) {
            len = src_len - *offset;
        }
        if (len <= 0) {
            return NULL;
        }
        p = &buff[*offset];
        p[len-1] = 0;
        *offset += len;
        return p;
    }
}

char* get_string2(char* buff, int* offset, int src_len) {
    char* output = get_string(buff, offset, src_len);
    if (output == NULL) {
        return "";
    } else {
        return output;
    }
}

int get_binary(char* buff, int* offset, char* output, int src_len, int des_len) {
    int len = 0;
    if (*offset >= 0 && *offset <= src_len) {
        len = get_int(buff, offset, src_len);
        if (len > MIN((src_len-(*offset)), des_len)) {
            len = MIN((src_len-(*offset)), des_len);
        }
        if (len > 0) {
            memcpy(output, &buff[*offset], len);
            *offset += len;
        }
    }
    return len;
}

//===============================================================
// put
void put_byte(char* buff, int* offset, const char input) {
    *((char*)&buff[*offset]) = input;
    *offset += 1;
}

void put_short(char* buff, int* offset, const short input) {
    put_byte(buff, offset, input & 0xff);
    put_byte(buff, offset, (input >> 8) & 0xff);
}

void put_int(char* buff, int* offset, const int input) {
    put_short(buff, offset, input & 0xffff);
    put_short(buff, offset, (input >> 16) & 0xffff);
}

void put_long(char* buff, int* offset, const long long input) {
    put_int(buff, offset, input & 0xffffffffL);
    put_int(buff, offset, ((input >> 32L) & 0xffffffffL));
}

void put_float(char* buff, int* offset, const float input) {
    int* data = (int*)&input;
    put_int(buff, offset, *data);
}

void put_double(char* buff, int* offset, const double input) {
    long long* data = (long long*)&input;
    put_long(buff, offset, *data);
}

void put_string(char* buff, int* offset, const char* input) {
    if (input == NULL) {
        put_byte(buff, offset, 0);
    } else {
        int len = strlen(input) + 1;
        put_byte(buff, offset, 1);
        put_int(buff, offset, len);
        memcpy(&buff[*offset], input, len);
        *offset += len;
    }
}

void put_binary(char* buff, int* offset, const char* input, const int len) {
    put_int(buff, offset, len);
    if (len > 0) {
        memcpy(&buff[*offset], input, len);
        *offset += len;
    }
}

