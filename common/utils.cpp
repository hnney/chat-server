#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

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

