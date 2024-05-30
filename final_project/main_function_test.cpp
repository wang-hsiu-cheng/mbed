#include "mbed.h"
#include "bbcar.h"
#include <iostream>
Ticker servo_ticker;
Ticker servo_feedback_ticker;

DigitalOut led1(LED1);
PwmIn servo0_f(D13), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);
InterruptIn button(BUTTON1);
DigitalInOut pin8(D8);
BusInOut qti_pin(D4,D5,D6,D7);
bool level = true;

void toggle()
{
    level = !level;
}
int main() {
    // parallax_ping  ping1(pin8);
    parallax_laserping  ping1(pin8);
    // car.goStraight(50);
    // while(1) {
    //     if((float)ping1 > 10) {
    //         printf("runnung\n");
    //         led1 = 1;
    //     }
    //     else {
    //         led1 = 0;
    //         printf("stop\n");
    //         car.stop();
    //         break;
    //     }
    //     ThisThread::sleep_for(10ms);
    // }

    // if (level)
    // { 
    //     car.goCertainDistance(21.05);   //6.7*3.14*3

    //     while(car.checkDistance(0.5)){
    //         ThisThread::sleep_for(500ms);
    //     }
    //     car.stop();

    //     ThisThread::sleep_for(3s);
    //     printf("error distance = %f\n", (car.servo0.targetAngle + car.servo0.angle)*6.7*3.1416/360);
    // }

    parallax_qti qti1(qti_pin);
    int pattern;

    car.goStraight(50);
    while(1) {
        int hint;
        pattern = qti1;
        // printf("%u\n",pattern);
        // std::cout << pattern << std::endl;

        switch (pattern) {
            case 0b1000: car.turn(40, 0.05); printf("1000\n"); break;
            case 0b1100: car.turn(55, 0.35); printf("1100\n"); break;
            case 0b0100: car.turn(60, 0.55); printf("0100\n"); break;
            case 0b0110: car.turn(65, 0.75); printf("0110\n"); break;
            case 0b0010: car.turn(60, -0.55); printf("0010\n"); break;
            case 0b0011: car.turn(55, -0.35); printf("0011\n"); break;
            case 0b0001: car.turn(40, -0.05); printf("0001\n"); break;
            case 0b0111: hint = 0b0111; printf("0111\n"); break;
            case 0b1110: hint = 0b0111; printf("0111\n"); break;
            case 0b1111: 
            {
                if (hint == 0b0111)
                    car.turn(50, 0.05);
                else if (hint == 0b1110)
                    car.turn(50, -0.05);
                else 
                    car.stop(); printf("1111\n"); break;
            }
            default: car.stop();
        }
        ThisThread::sleep_for(10ms);
    }
    button.rise(&toggle);
}