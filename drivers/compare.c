#include "compare.h"

int compare_ct_time(volatile uint8_t *a1, volatile uint8_t *a2, int len)
{
    volatile uint8_t result = 0x0;

    for (int i=0; i<len; i++)
    {
	result |= a1[i] ^ a2[i];
    }
    
    return (result == 0x0);
}
