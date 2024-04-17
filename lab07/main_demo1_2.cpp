#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 6

static DigitalOut led1(LED1); // led1 = PA_5
static DigitalOut led2(LED2); // led2 = PB_14

Thread thread1;
Thread thread2;

static BufferedSerial device1(D10, D9); // tx, rx  D10:tx  D9:rx
static BufferedSerial device2(D1, D0);  // tx, rx   D1:tx   D0:rx
static BufferedSerial serial_port(USBTX, USBRX);

void master_thread() {
    while(1)
    {
        if (device2.readable()) 
        {
            int receive;
            device2.read(&receive, 1);
            printf("%d\n", receive);
        }
    }
}

void slave_thread() {
    int number = 0;
    while (number < 100) {
        device1.write(&number, 1);
        number ++;
        ThisThread::sleep_for(100ms);
    }
}

int main() {
    // Set desired properties (9600-8-N-1).
    device1.set_baud(9600);
    device1.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1);

    // Set desired properties (9600-8-N-1).
    device2.set_baud(9600);
    device2.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1);

    thread1.start(master_thread);
    thread2.start(slave_thread);
}