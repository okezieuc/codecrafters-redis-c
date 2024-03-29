
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/**
 * parse_number receives a pointer to the start of a chain of
 * characters in C representing a number that is terminated with
 * \\r\\n and returns the number in that portion of the string as
 * an integer.
 *
 * this is useful when parsing arrays and bulk strings, as you need
 * to know the number of items in the array and the number of
 * characters in the string.
 *
 *
 * Parameters:
 * - digit_ptr: a pointer a pointer to the start of the string
 * - adjust_pointer: set this to a non-zero digit if you want to move
 *      digit_ptr to the last character in the string after getting the
 *      digit. set to 0 otherwise.
 *
 * Returns:
 * - an integer representing the number
 *
 *
 * Sample usage:
 * char digits[9] = "*24\r\n..."
 * int num = parse_number(&(digits + 1), 1); // returns 24
 */
int parse_number(char **digit_ptr, int adjust_pointer)
{
    char *ptr = *digit_ptr;
    int num;

    while (*ptr != '\r')
    {
        ptr += 1;
    }

    // temporarily change the char at *ptr to a null terminator
    *ptr = '\0';

    num = atoi(*digit_ptr);

    // revert the chat at ptr to \r
    *ptr = '\r';

    if (adjust_pointer)
    {
        *digit_ptr = ptr - 1;
    }

    return num;
}

/**
 * Returns the current time in milliseconds.
 */
long int get_current_time()
{
    struct timespec now;
    long int time_in_ms;

    clock_gettime(CLOCK_REALTIME, &now);
    time_in_ms = now.tv_sec * 1000 + (now.tv_nsec) / 1000000;

    return time_in_ms;
}

/**
 * The implementations of bin2hex and hex2bin were gotten from
 * https://nachtimwald.com/2017/09/24/hex-encode-and-decode-in-c/
 */

char *bin2hex(const unsigned char *bin, size_t len)
{
    char *out;
    size_t i;

    if (bin == NULL || len == 0)
        return NULL;

    out = malloc(len * 2 + 1);
    for (i = 0; i < len; i++)
    {
        out[i * 2] = "0123456789ABCDEF"[bin[i] >> 4];
        out[i * 2 + 1] = "0123456789ABCDEF"[bin[i] & 0x0F];
    }
    out[len * 2] = '\0';

    return out;
}

int hexchr2bin(const char hex, char *out)
{
    if (out == NULL)
        return 0;

    if (hex >= '0' && hex <= '9')
    {
        *out = hex - '0';
    }
    else if (hex >= 'A' && hex <= 'F')
    {
        *out = hex - 'A' + 10;
    }
    else if (hex >= 'a' && hex <= 'f')
    {
        *out = hex - 'a' + 10;
    }
    else
    {
        return 0;
    }

    return 1;
}

size_t hexs2bin(const char *hex, unsigned char **out)
{
    size_t len;
    char b1;
    char b2;
    size_t i;

    if (hex == NULL || *hex == '\0' || out == NULL)
        return 0;

    len = strlen(hex);
    if (len % 2 != 0)
        return 0;
    len /= 2;

    *out = malloc(len);
    memset(*out, 'A', len);
    for (i = 0; i < len; i++)
    {
        if (!hexchr2bin(hex[i * 2], &b1) || !hexchr2bin(hex[i * 2 + 1], &b2))
        {
            return 0;
        }
        (*out)[i] = (b1 << 4) | b2;
    }
    return len;
}