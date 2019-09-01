int v[] = {1,2,3,4,5,6,7,8,9,10};
int sum;

#define TXFE 0x80
#define TXFF 0x20
#define RXFE 0x10
#define RXFF 0x40
#define BUSY 0x08

#define UDR 0x00
#define UFR 0x18

typedef volatile struct uart {
    char *base;
    int n;
}UART;

UART uart[4];

int uart_init()
{
    int i;
    UART *up;
    for(i=0; i<4; i++){
        up = &uart[i];
        up->base = (char *)(0x101F1000 + i*0x1000);
        up->n = i;
    }
    uart[3].base = (char *)(0x10009000);
}


char ugetc(UART *up)
{
    while(*(up->base + UFR) & RXFE);
    return *(up->base + UDR);
}

void uputc(UART *up, char c)
{
    while(*(up->base + UFR) & TXFF);
    *(up->base + UDR) = c;
}

void upgets(UART *up, char *s)
{
    while((*s = ugetc(up))!='\r'){
        uputc(up, *s);
        s++;
    }
    *s = 0;
}

void uprints(UART *up, char *s)
{
    while(*s)
    {
        uputc(up, *s++);
    }
}

int strcmp(char *s1, char *s2)
{
    while((*s1++==*s2++)&&(*s1!=0)&&(*s2!=0));
    if(*s1==0 && *s2==0)
    {
        return 0;
    }
    return 1;
}


int main()
{
    int i;
    char string[64];
    UART *up;
    uart_init();
    up = &uart[0];
    uprints(up, "\n\rEnter lines from serial terminal 0\n\r");

    while(1){
        upgets(up, string);
        uprints(up, "   ");
        uprints(up, string);
        uprints(up, "\n\r");
        if(strcmp(string, "end")==0){
            break;
        }        
    }
    uprints(up, "Compute sum of array:\n\r");
    sum = 0;
    for(i=0; i<10; i++){
        sum += v[i];
    }

    uprints(up, "sum = ");
    uputc(up, (sum/10)+'0');
    uputc(up, (sum%10)+'0');
    uprints(up, "\n\rEND OF RUN\n\r");
}