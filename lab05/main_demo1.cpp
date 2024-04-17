#include "mbed.h"

using namespace std::chrono;

Timer timer;

int main()
{
    int N = 10, M = 50000;
    float output = 0;
    timer.start();
    for (int i = 0; i < M; i++)
    {
        output = exp(N);
    }
    timer.stop();
    auto us = timer.elapsed_time().count();
    printf("N = %d M = %d\nspend %llu us\noutput: %.3f\n", N, M, us, output);
    timer.reset();
}