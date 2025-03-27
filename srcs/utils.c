#include "../incs/ft_traceroute.h"

int is_num(const char* str) {
    if (*str == '\0') {
        return(0);
    }
    if (*str == '-') {
        str++;
    }
    while (*str) {
        if (!isdigit(*str)) {
            return(0);
        }
        str++;
    }
    return(1);
}