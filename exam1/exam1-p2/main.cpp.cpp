#include "mbed.h"

const double pi = 3.141592653589793238462;

// Adjust analog output pin name to your board spec.
AnalogOut aout(PA_4);

int main()
{
    double rads = 0.0;
    uint16_t sample = 0;

    while (1)
    {
        // sinewave output
        for (int i = 0; i < 360; i = i + 3)
        {
            rads = (pi * i) / 180.0f;

            if (i < 180)
                sample = (3 * exp(rads) / exp(pi)) / 3.3f * 0xFFFF;
            else
                sample = (3 * exp(2 * pi - rads) / exp(pi)) / 3.3f * 0xFFFF;
            if (sample <= 0 / 3.3f * 0xFFFF || sample > 3 / 3.3f * 0xFFFF)
                sample = 0 / 3.3f * 0xFFFF;
            aout.write_u16(sample);
            printf("rads = %f, sample = %d, aout = %f\n", rads, sample, aout.read());
            wait_us(55);
        }
    }
}