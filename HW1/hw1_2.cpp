#include "mbed.h"

BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[11] = {0x20, 0x30, 0x22, 0x26, 0x24, 0x32, 0x36, 0x34, 0x12, 0x16, 0x8E};
int words[10] = {7, 8, 6, 7, 10, 8, 10, 7, 10, 6};

int main()
{
    while (1)
    {
        for (int i = 0; i < 10; i = ++)
        {
            display = table[words[i]];
            ThisThread::sleep_for(1s);
        }
    }
}