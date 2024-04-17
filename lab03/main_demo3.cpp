#include "mbed.h"

// Adjust pin name to your board specification.
// You can use LED1/LED2/LED3/LED4 if any is connected to PWM capable pin,
// or use any PWM capable pin, and see generated signal on logical analyzer.
PwmOut led(LED1);

int main()
{
    // specify period first
    // led = 0.5f;                // shorthand for led.write()
    while (1)
    {
        led.period_ms(4000); // 4 second period
        led = 1.0f;          // shorthand for led.write()
        // led.write(0.50f);        // 50% duty cycle, relative to period
        led.pulsewidth_ms(2000); // alternative to led.write, set duty cycle time in milliseconds
    }
}