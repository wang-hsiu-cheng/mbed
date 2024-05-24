#include "mbed.h"
#include "bbcar.h"
#include "drivers/DigitalOut.h"

#include "BLEcommunicate.cpp"
#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"

DigitalOut led1(LED1);
Ticker servo_ticker;
Ticker servo_feedback_ticker;
PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
DigitalInOut pin8(D8);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);
BusInOut qti_pin(D4, D5, D6, D7);

void GoCertainDistance(float distance)
{
    car.goCertainDistance(distance); // 6.5*3.14*3

    while (car.checkDistance(1))
    {
        if (LaserPingNavigation())
            break;
        ThisThread::sleep_for(500ms);
    }
    car.stop();
    ThisThread::sleep_for(3s);
    printf("error distance = %f\n", (car.servo0.targetAngle - car.servo0.angle) * 6.5 * 3.14 / 360);
}
int LaserPingNavigation()
{
    parallax_laserping ping1(pin8);
    if ((float)ping1 > 10)
    {
        led1 = 1;
        return 0;
    }
    else
    {
        led1 = 0;
        car.stop();
        return 1;
    }
}
void QTInavigation()
{
    parallax_qti qti1(qti_pin);
    int pattern = 0b0110;
    car.goStraight(50);

    while (pattern != 0b0000)
    {
        pattern = (int)qti1;
        printf("%d\n", pattern);

        switch (pattern)
        {
        case 0b1000:
            car.turn(50, 0.1);
            break;
        case 0b1100:
            car.turn(50, 0.5);
            break;
        case 0b0100:
            car.turn(50, 0.7);
            break;
        case 0b0110:
            car.goStraight(50);
            break;
        case 0b0010:
            car.turn(50, -0.7);
            break;
        case 0b0011:
            car.turn(50, -0.5);
            break;
        case 0b0001:
            car.turn(50, -0.1);
            break;
        case 0b1111:
            car.stop();
            break;
        default:
            car.goStraight(50);
        }
        ThisThread::sleep_for(10ms);
    }
    return;
}

int main()
{
    // simple test
    car.goStraight(200);
    ThisThread::sleep_for(5s);
    car.stop();
    ThisThread::sleep_for(5s);

    // go certain distance
    GoCertainDistance(61.2);

    // QTI Line Following Kit
    QTInavigation();
}