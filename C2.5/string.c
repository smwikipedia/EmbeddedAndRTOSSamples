int strcmp(char *s1, char *s2)
{
    while((*s1++==*s2++)&&(*s1!=0)&&(*s2!=0));
    if(*s1==0 && *s2==0)
    {
        return 0;
    }
    return 1;
}