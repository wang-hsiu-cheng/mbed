#include "mbed.h"
/*
0x20, 0x30, 0x22, 0x26, 0x24, 0x32, 0x36, 0x34, 0x12, 0x16,
0x28, 0x38, 0x2a, 0x2e, 0x2c, 0x3a, 0x3e, 0x3c, 0x1a, 0x1e,
0xa8, 0xb8, 0x96, 0xaa, 0xae, 0xac, 0x00, 0x94, 0x10, 0x8e
*/
DigitalIn mypin(BUTTON1);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char arabicNumber[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
char brailleCode[10] = {0x16, 0x20, 0x30, 0x22, 0x26, 0x24, 0x32, 0x36, 0x34, 0x12};
char alphabetE = 0x79;
char alphabetO = 0x5c;

int main()
{
    int state = 3;
    int randNum = 0;
    int currentNumber = 0;
    bool isRandGen = false;
    mypin.mode(PullNone);
    while (1)
    {
        if (state == 1)
        {
            if (!isRandGen)
            {
                srand(time(NULL));
                randNum = rand() % 10;
                isRandGen = true;
            }
            display = brailleCode[randNum];
        }
        else if (state == 2)
        {
            if (currentNumber < 9)
            {
                display = arabicNumber[currentNumber];
                currentNumber++;
            }
            else if (currentNumber == 9)
            {
                display = arabicNumber[currentNumber];
                currentNumber = 0;
            }
        }
        else if (state == 3)
        {
            if ((currentNumber - 1) == randNum)
                display = alphabetO;
            else if ((currentNumber - 1) != randNum)
                display = alphabetE;
            if (currentNumber == 0 && randNum == 9)
                display = alphabetO;
        }
        // ThisThread::sleep_for(1s);
        for (int i = 0; i < 5; i++)
        {
            if (mypin.read() == 0)
                state++;
            if (state == 3)
                isRandGen = false;
            if (state == 4)
            {
                state = 1;
                currentNumber = 0;
            }
            ThisThread::sleep_for(200ms);
        }
    }
}