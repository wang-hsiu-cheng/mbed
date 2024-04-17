#include "mbed.h"

using namespace std::chrono;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread thread1;
Thread thread2;
Thread thread3;
Thread thread4;

Timer timer1;

void print()
{
    auto ms = chrono::duration_cast<chrono::milliseconds>(timer1.elapsed_time()).count();
    if (ms > 10000)
        return;
    printf("%llu\n", ms);
}
void counter1()
{
    queue.call_every(40ms, print);
    queue.dispatch();
}
void counter2()
{
    ThisThread::sleep_for(10ms);
    queue.call_every(40ms, print);
    queue.dispatch();
}
void counter3()
{
    ThisThread::sleep_for(20ms);
    queue.call_every(40ms, print);
    queue.dispatch();
}
void counter4()
{
    ThisThread::sleep_for(30ms);
    queue.call_every(40ms, print);
    queue.dispatch();
}

int main()
{
    timer1.start();
    thread1.start(counter1);
    thread2.start(counter2);
    thread3.start(counter3);
    thread4.start(counter4);

    // queue.dispatch();

    // events are executed by the dispatch method
}