#include "mbed.h"

AnalogOut Aout(D7);
AnalogIn Ain(A0);

int sample = 128;
int i;

int ADCdata[128];

int main(){
  for (i = 0; i < sample; i++){
    Aout = Ain;
    ADCdata[i] = Ain.read_u16();
    ThisThread::sleep_for(2ms/sample);
  }
  for (i = 0; i < sample; i++){
    printf("%d\r\n", ADCdata[i]);
    // ThisThread::sleep_for(1ms);
  }
}