#include "mbed.h"
#include "UTouch.h"
#include "lcd.h"
//LCD           VCC GND  CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO  LED
//mbed                   D10,  D9,  D8,    D11,     D13, D12,      3V3
//Touch             tclk,  tcs,          tdin,  dout,  irq
//mbed (L4S5I)PMOD. PD_1,  PD_2 (Buttom) PD_3,  PD_4,  PD_5

UTouch  myTouch(PD_1, PD_2, PD_3, PD_4, PD_5);

//CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO
LCD lcd(D10, D9, D8, D11, D13, D12);

int main() {
    printf("Initializing LCD\n");
    lcd.InitLCD();
    lcd.LCD_Clear(0x0);
    myTouch.InitTouch();
    myTouch.SetPrecision(PREC_HI);
    uint16_t colors[]={0xf800, 0xf800, 0xf800, 0xf800,
                        0xf800, 0xf800, 0xf800, 0xf800,
                        0xf800, 0xf800, 0xf800, 0xf800,
                        0xf800, 0xf800, 0xf800, 0xf800
                        };
    int16_t x, y, x_last, y_last;
    while(true)
        {
        if (myTouch.DataAvailable())
        {
            if(myTouch.Read())
            {
                //    lcd.LCD_Clear(0);
                x = myTouch.GetX();
                y = myTouch.GetY();
                //lcd.FillRectangle(x, y, 4, 4, 0xf800);
                //  lcd.FillArea(x, y, x+3, y+3, colors);
                lcd.Rect(x_last, y_last, 10, 10, 0);
                lcd.Rect(x, y, 10, 10, 100);
                printf("Raw values X=%d, Y=%d\n", myTouch.TP_X, myTouch.TP_Y);
                printf("Actual point values X=%d, Y=%d\n", x, y);
                x_last = x;
                y_last = y;
            }
        }
    }
}