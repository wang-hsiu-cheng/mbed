#include "mbed.h"

// Blinking rate in milliseconds
#define RATE 100ms

// Initialise the digital pin LED1 as an output
DigitalOut led(LED3);

int main()
{
    // declare total time
    double lightTime = 5;
    // open the led and close it when time comes to zero
    led.write(1);  // set LED1 pin to high
    while (lightTime > 0)
    {
        printf("%1.1f\n", (5 - lightTime));
        lightTime -= 0.1;
        ThisThread::sleep_for(RATE);
    }
    led = 0;

    return 0;
}
