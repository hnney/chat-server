#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>

long long hl_timestamp(void)
{   
    struct timeval tv1;
    gettimeofday(&tv1, NULL);
    return tv1.tv_sec * 1000000LL + tv1.tv_usec;
}
    
struct timeval *hl_ts2timeval(struct timeval *tv, long long timestamp)
{   
    tv->tv_sec = timestamp / 1000000;
    tv->tv_usec = timestamp % 1000000;
    return tv;
}

string hl_ltostring(long num) 
{
    //128 bytes will be enough
    char buf[128];
    if (snprintf(buf, 128, "%ld", num) < 128) {
        return string(buf);
    }   
    return "NaN"; //not a number
}   
    
string hl_dtostring(double num)
{   
    //128 bytes will be enough
    char buf[128];
    if (snprintf(buf, 128, "%f", num) < 128) {
        return string(buf);
    }
    return "NaN"; //not a number
}

long hl_atol(const char *str, int *is_succ)
{
    if (str == NULL) {
        if (is_succ) {
            *is_succ = 0;
        }
        return 0;
    }
    char *endp = NULL;
    errno = 0;
    long ret = strtol(str, &endp, 10);
    if (is_succ) {
        *is_succ = 1;
        if (errno != 0 || endp == str || *endp != '\0') {
            *is_succ = 0;
        }
    }
    return ret;
}

double hl_atod(const char *str, int *is_succ)
{
    if (str == NULL) {
        if (is_succ) {
            *is_succ = 0;
        }
        return 0;
    }
    char *endp = NULL;
    errno = 0;
    double ret = strtod(str, &endp);
    if (is_succ) {
        *is_succ = 1;
        if (errno != 0 || endp == str || *endp != '\0') {
            *is_succ = 0;
        }
    }
    return ret;
}

//base64
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

void base64_encode(const char *in, const int in_len, char *out, int out_len)
{
    if (in_len == 0) {
        return ;
    }
    int base64_len = 4 * ((in_len+2)/3);
    assert(out_len >= base64_len);
    char *p = out;
    int times = in_len / 3;
    for(int i=0; i<times; ++i)
    {
        *p++ = base64_chars[(in[0] >> 2) & 0x3f];
        *p++ = base64_chars[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
        *p++ = base64_chars[((in[1] << 2) & 0x3c) + ((in[2] >> 6) & 0x3)];
        *p++ = base64_chars[in[2] & 0x3f];
        in += 3;
    }
    // pad .. 
    if(times * 3 + 1 == in_len) {
        *p++ = base64_chars[(in[0] >> 2) & 0x3f];
        *p++ = base64_chars[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
        *p++ = '=';
        *p++ = '=';
    }
    if(times * 3 + 2 == in_len) {
        *p++ = base64_chars[(in[0] >> 2) & 0x3f];
        *p++ = base64_chars[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
        *p++ = base64_chars[((in[1] << 2) & 0x3c)];
        *p++ = '=';
    }
    *p = 0;
}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i < 4; i ++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; (i < 3); i++) {
                ret += char_array_3[i];
            } 
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j <4; j++) {
            char_array_4[j] = 0;
        }
        for (j = 0; j <4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (j = 0; (j < i - 1); j++) {
            ret += char_array_3[j];
        }
    }
    return ret;
}
