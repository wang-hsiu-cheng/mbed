#include "mbed.h"

DigitalIn mypin(BUTTON1);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[11] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0xbf};

int main()
{
    int state = 3;
    int currentNumber = 0;
    bool flashState = false;
    mypin.mode(PullNone);
    while (1)
    {
        if (state == 1)
        {
            srand(time(NULL));
            currentNumber = rand() % 10;
            display = table[currentNumber];
        }
        else if (state == 2)
        {
            if (currentNumber > 0)
            {
                display = table[currentNumber];
                currentNumber--;
            }
            else if (currentNumber == 0 && flashState)
            {
                display = table[10];
            }
            else if (currentNumber == 0 && !flashState)
            {
                display = table[0];
            }
        }
        else if (state == 3)
        {
            display = table[currentNumber];
        }
        // ThisThread::sleep_for(1s);
        flashState = !flashState;

        for (int i = 0; i < 5; i++)
        {
            if (mypin.read() == 0)
                state++;
            if (state == 4)
                state = 1;
            ThisThread::sleep_for(200ms);
        }
    }
}