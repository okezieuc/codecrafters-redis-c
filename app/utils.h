#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
 * - digit_ptr: a pointer to the start of the string
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
 * int num = parse_number(digits + 1); // returns 24
 */
int parse_number(char *digit_ptr, int adjust_pointer)
{
    char *ptr = digit_ptr;
    int num;

    while (*ptr != '\r')
    {
        ptr += 1;
    }

    // temporarily change the char at ptr to a null terminator
    *ptr = '\0';

    num = atoi(digit_ptr);

    // revert the chat at ptr to \r
    *ptr = '\r';

    if (adjust_pointer) {
        digit_ptr = ptr - 1;
    }

    return num;
}
