#ifndef CC_REDIS_UTILS
#define CC_REDIS_UTILS
#include <stdlib.h>

int parse_number(char **digit_ptr, int adjust_pointer);
long int get_current_time();
char *bin2hex(const unsigned char *bin, size_t len);
int hexchr2bin(const char hex, char *out);
size_t hexs2bin(const char *hex, unsigned char **out);

#endif