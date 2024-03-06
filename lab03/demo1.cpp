#include "mbed.h"

// Initialize a pins to perform analog and digital output functions
// Adjust analog output pin name to your board spec.
AnalogOut aout(PA_4);
DigitalOut dout(LED1);

int main(void)
{
    while (1)
    {
        // change the voltage on the digital output pin by 0.1 * VCC
        //  and print what the measured voltage should be (assuming VCC = 3.3v)
        for (float i = 0; i < 10; i++)
        {
            aout.write_u16(i * 0.1);
            printf("aout = %f volts\n", aout.read() * 3.3f);
            // turn on the led if the voltage is greater than 0.5f * VCC
            dout = (aout.read() > 0.5f) ? 1 : 0;
            ThisThread::sleep_for(1s);
        }
    }
}