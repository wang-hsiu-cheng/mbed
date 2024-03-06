#include "mbed.h"

DigitalIn mypin(BUTTON1);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
void changeState(int &state)
{
    if (mypin.read() == 1)
    {
        ThisThread::sleep_for(20ms);
        if (mypin.read() == 1)
            state = !state;
        ThisThread::sleep_for(980s);
    }
    else
    {
        ThisThread::sleep_for(1s);
    }
}

int main()
{
    bool state = false;
    int currentNumber = 0;
    mypin.mode(PullNone);
    while (1)
    {
        while (!state)
        {
            if (currentNumber > 9)
                currentNumber = -1;
            currentNumber++;
            display = table[currentNumber];
            changeState(state);
        }
        while (state)
        {
            if (currentNumber <= 0)
                currentNumber = 10;
            currentNumber--;
            display = table[currentNumber];
            changeState(state);
        }
    }
}