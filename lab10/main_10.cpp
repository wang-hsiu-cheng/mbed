#include "mbed.h"
#include "drivers/DigitalOut.h"

#include "erpc_simple_server.hpp"
#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "blink_led_server.h"

mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1);
mbed::DigitalOut* leds[] = { &led1, &led2, &led3 };

/****** erpc declarations *******/

void led_on(uint8_t led) 
{
    if(0 < led && led <= 3) 
    {
        *leds[led - 1] = 0;
        printf("LED %d is On.\n", led);
    }
    return;
}

void led_off(uint8_t led) 
{
    if(0 < led && led <= 3) 
    {
        *leds[led - 1] = 1;
        printf("LED %d is Off.\n", led);
    }
    return;
}
uint8_t led_state()
{
    return ((*leds[0]+1) * 100 + (*leds[1]+1) * 10 + (*leds[0]+1));
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

/** LED service */
LEDBlinkService_service led_service;

int main(void) {

  // Initialize the rpc server
  uart_transport.setCrc16(&crc16);

  printf("Initializing server.\n");
  rpc_server.setTransport(&uart_transport);
  rpc_server.setCodecFactory(&basic_cf);
  rpc_server.setMessageBufferFactory(&dynamic_mbf);

  // Add the led service to the server
  printf("Adding LED server.\n");
  rpc_server.addService(&led_service);

  // Run the server. This should never exit
  printf("Running server.\n");
  rpc_server.run();
}