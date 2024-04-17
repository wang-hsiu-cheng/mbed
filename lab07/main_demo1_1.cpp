#include "mbed.h"

Thread thread_master;
Thread thread_slave;

//master

SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

SPISlave device(PD_4, PD_3, PD_1, PD_0); //mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int master()
{
    device.format(8, 3);
    device.frequency(1000000);
    //device.reply(0x00); // Prime SPI with first reply
    while (1)
    {
        if (device.receive())
        {
            int receive = device.read(); // Read byte from master
            printf("%d\n", receive);
        }
    }
}

void slave()
{
   int number = 0;

   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(8, 3);
   spi.frequency(1000000);

   while (number < 100)
   {
      // Chip must be deselected
      cs = 1;
      // Select the device by seting chip select low
      cs = 0;

      spi.write(number); //Send number to master
      ThisThread::sleep_for(100ms); //Wait
      number += 1;
   }
}

int main()
{
   thread_slave.start(slave);
   thread_master.start(master);
}