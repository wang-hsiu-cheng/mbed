#ifndef BBCAR_H
#define BBCAR_H
#include "parallax_servo.h"
#include "parallax_ping.h"
#include "parallax_laserping.h"
#include "parallax_qti.h"

class BBCar{
	public:
        BBCar( PwmOut &pinc_servo0, PwmIn &pinf_servo0, PwmOut &pinc_servo1, 
                PwmIn &pinf_servo1, Ticker &servo_control_ticker, Ticker &servo_feedback_ticker );
		parallax_servo servo0;
		parallax_servo servo1;

		void controlWheel();
		void stop();
		void goStraight( double speed );

		// turn left/right with a factor of speed
		void turn( double speed, double factor );
        void turnAround(double speed, bool direction);

		// limit the value by max and min
		float clamp( float value, float max, float min );
		int turn2speed( float turn );
        
        //feedbackservo
        double Dest();
        void feedbackWheel();
        void initPathDist();
        void goCertainDistance(float distance);
		int checkDistance(float errorDistance_Range);
        double path = 0;
    private:
        double initAngle0;
        double initAngle1;
};

#endif
