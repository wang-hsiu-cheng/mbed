#include "LCD.h"
char arabicNumber[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
int main()
{
    while(1)
    {
        LCD_init();                     // call the initialise function
        ThisThread::sleep_for(10ms);
        display_to_LCD(arabicNumber[1]);
        display_to_LCD(arabicNumber[1]);
        display_to_LCD(arabicNumber[1]);  
        display_to_LCD(arabicNumber[0]);
        display_to_LCD(arabicNumber[6]);
        display_to_LCD(arabicNumber[1]);
        display_to_LCD(arabicNumber[2]);
        display_to_LCD(arabicNumber[3]);
        display_to_LCD(arabicNumber[7]);
        ThisThread::sleep_for(2s);
        LCD_init();                     // call the initialise function
        ThisThread::sleep_for(10ms);
        display_to_LCD(0x48);           // ‘H’
        display_to_LCD(0x45);           // ‘E’
        display_to_LCD(0x4C);           // ‘L’
        display_to_LCD(0x4C);           // ‘L’
        display_to_LCD(0x4F);           // ‘O’
        ThisThread::sleep_for(2s);
    }
    // LCD_init();                     // call the initialise function
    // ThisThread::sleep_for(10ms);
    //   for (int i = 0; i <= 50; i+=5)
    //   {
    //       display_to_LCD(arabicNumber[i/10]);
    //       display_to_LCD(arabicNumber[i%10]);
    //   }
}