#include "mbed.h"
using namespace std::chrono;

Timer timer1;
Timer timer2;
InterruptIn signal(D7); // Interrupt on digital push button input SW2
EventQueue queue(32 * EVENTS_EVENT_SIZE);
auto duration2_ms;
auto duration1_ms;

void counter1()
{
    queue.call(printf, "period: %llu ms\n", duration1_ms + duration2_ms);
    timer1.start();
    timer2.stop();
    duration2_ms = duration_cast<milliseconds>(timer2.elapsed_time()).count();
    queue.call(printf, "timer2: %llu ms\n", duration2_ms);
}
void counter2()
{
    timer2.start();
    timer1.stop();
    duration1_ms = duration_cast<milliseconds>(timer1.elapsed_time()).count();
    queue.call(printf, "timer1: %llu ms\n", duration1_ms);
}
int main()
{
    signal.rise(&counter1); // attach the address of the toggle
    signal.fall(&counter2);
    queue.dispatch_forever();
}