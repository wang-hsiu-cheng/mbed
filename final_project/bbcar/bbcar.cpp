#include "bbcar.h"

BBCar::BBCar(PwmOut &pinc_servo0, PwmIn &pinf_servo0, PwmOut &pinc_servo1, PwmIn &pinf_servo1, Ticker &servo_control_ticker, Ticker &servo_feedback_ticker) : servo0(pinc_servo0, pinf_servo0), servo1(pinc_servo1, pinf_servo1)
{
    servo0.set_speed(0);
    servo1.set_speed(0);
    servo_control_ticker.attach(callback(this, &BBCar::controlWheel), 20ms);
    servo_feedback_ticker.attach(callback(this, &BBCar::feedbackWheel), 5ms);
}

void BBCar::controlWheel()
{
    servo0.control();
    servo1.control();
}

void BBCar::stop()
{
    servo0.set_factor(1);
    servo1.set_factor(1);
    servo0.set_speed(0);
    servo1.set_speed(0);
}

void BBCar::goStraight(double speed)
{
    servo0.set_factor(1);
    servo1.set_factor(1);
    servo0.set_speed(speed);
    servo1.set_speed(-speed);
}

/*	speed : speed value of servo
    factor: control the speed value with 0~1
            control left/right turn with +/-
*/
void BBCar::turn(double speed, double factor)
{
    if (factor > 0)
    {
        servo0.set_factor(factor);
        servo1.set_factor(1);
    }
    else if (factor < 0)
    {
        servo0.set_factor(1);
        servo1.set_factor(-factor);
    }
    servo0.set_speed(speed);
    servo1.set_speed(-speed);
}
void BBCar::turnAround(double speed, bool direction)
{
    if (direction)
    {
        servo0.set_factor(1);
        servo1.set_factor(-1);
    }
    else
    {
        servo0.set_factor(-1);
        servo1.set_factor(1);
    }
    servo0.set_speed(speed);
    servo1.set_speed(-speed);
}

float BBCar::clamp(float value, float max, float min)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    else
        return value;
}

int BBCar::turn2speed(float turn)
{
    return 25 + abs(25 * turn);
}

void BBCar::initPathDist()
{
    initAngle0 = servo0.angle;
    initAngle1 = servo1.angle;
}
double BBCar::Dest()
{
    return (-(servo0.angle - initAngle0) / 360 * (6.7 * 3.1416) + (servo1.angle - initAngle1) / 360 * (6.7 * 3.1416)) / 2;
}
void BBCar::feedbackWheel()
{
    servo0.feedback360();
    servo1.feedback360();
}

void BBCar::goCertainDistance(float distance)
{
    servo0.targetAngle = (int)(distance * 360 / (6.7 * 3.1416)) + servo0.angle;
    servo1.targetAngle = (int)(-distance * 360 / (6.7 * 3.1416)) + servo1.angle;
}

int BBCar::checkDistance(float errorDistance_Range)
{
    int speed, offset = 0; // Control system variables
    float errorDistance, factor = 1;

    errorDistance = (servo0.targetAngle + servo0.angle) * 6.7 * 3.1416 / 360; // Calculate error
    printf("error dist: %f", errorDistance);

    speed = int(errorDistance);

    if (errorDistance > 0) // Add offset
        offset = 30;
    else if (errorDistance < 0)
        offset = -30;
    else
        offset = 0;

    servo0.set_factor(factor);
    servo1.set_factor(factor);
    servo0.set_speed(speed + offset);
    servo1.set_speed(-speed - offset);

    if (abs(errorDistance) > errorDistance_Range)
    {
        printf("R\n");
        return 1;
    }
    else
    {
        printf("S\n");
        return 0;
    }
}
