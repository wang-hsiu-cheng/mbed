#include "mbed.h"
#include "LCD.h"
using namespace std::chrono;

DigitalOut led1(LED1);
DigitalOut led2(LED2);
InterruptIn button(BUTTON1);
EventQueue queue1(32 * EVENTS_EVENT_SIZE);
Timer timer1;
Ticker ticker1;

bool isTicking = false;
char arabicNumber[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

void display(int min, int sec)
{
    LCD_init();
    ThisThread::sleep_for(10ms);
    display_to_LCD(arabicNumber[min / 10]);
    display_to_LCD(arabicNumber[min % 10]);
    display_to_LCD(0x3a);
    display_to_LCD(arabicNumber[sec / 10]);
    display_to_LCD(arabicNumber[sec % 10]);
}
void update()
{
    auto time = chrono::duration_cast<chrono::seconds>(timer1.elapsed_time()).count();
    int second = time % 60;
    int minute = time / 60;
    display(minute, second);
    // printf("%llu:%llu\n", minute, second);
}
void setupTicker()
{
    if (!isTicking)
        ticker1.attach(queue1.event(&update), 1000ms);
    else
        ticker1.detach();
    isTicking = !isTicking;
}

int main()
{
    timer1.start();
    button.rise(&setupTicker);
    queue1.dispatch_forever();
}