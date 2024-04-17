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
        for (int i = 0; i < 360; i++)
        {
            rads = (pi * i) / 180.0f;
            if (i < 180)
                sample = (uint16_t)3 * (1 + 1 / (1 - exp(rads)));
            else
                sample = (uint16_t)3 * (1 + 1 / (1 - np.exp(2 * pi - rads)));
            aout.write_u16(sample);
            wait_us(100ms);
        }
    }
}