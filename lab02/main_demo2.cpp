#include "mbed.h"

DigitalIn mypin(BUTTON1);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
void changeState(int &state)
{
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
            for (int i = 0; i < 10; i++)
            {
                if (mypin.read() == 0 && state == false)
                    state = !state;
                ThisThread::sleep_for(100ms);
            }
        }
        while (state)
        {
            if (currentNumber <= 0)
                currentNumber = 10;
            currentNumber--;
            display = table[currentNumber];
            for (int i = 0; i < 10; i++)
            {
                if (mypin.read() == 0 && state == true)
                    state = !state;
                ThisThread::sleep_for(100ms);
            }
        }
    }
}