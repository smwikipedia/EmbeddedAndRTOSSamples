#include "types.h"
u32 strcmp(u8 *s1, u8 *s2)
{
    while ((*s1++ == *s2++) && (*s1 != 0) && (*s2 != 0))
        ;
    if (*s1 == 0 && *s2 == 0)
    {
        return 0;
    }
    return 1;
}

u8* strcpy(u8 *s1, u8 *s2)
{
    while(*s2 != 0)
    {
        *s1++ = *s2++;
    }
    *s1 = '\0';
    return s1;
}

u32 strlen(const u8 *s)
{
    u32 i = 0;
    while (*s++ != '\0') i++;
    return i;
}
