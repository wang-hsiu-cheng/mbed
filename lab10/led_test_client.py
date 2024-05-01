# led_test_client.py
# Test client for erpc led server example
# Author: becksteing/Jing-Jia Liou
# Date: 02/13/2022
# Blinks LEDs on a connected Mbed-enabled board running the erpc LED server example

from time import sleep
import erpc
from blink_led import *
import sys

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage: python led_test_client.py <serial port to use>")
        exit()

    # Initialize all erpc infrastructure
    xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
    client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
    client = client.LEDBlinkServiceClient(client_mgr)

    # Blink LEDs on the connected erpc server
    turning_on = True
    input_flow = 0
    while True:
        for i in range(1, 4):
            if(turning_on):
                print("Call led_on ", i)
                client.led_on(i)
            else:
                print("Call led_off ", i)
                client.led_off(i)
            sleep(1)
        input_flow = client.led_state()
        led1_status = led2_status = led3_status = "off"
        if (input_flow // 100 == 1):
            led1_status = "on"
        if ((input_flow % 100 - input_flow % 10) // 10 == 1):
            led2_status = "on"
        if (input_flow % 10 == 1):
            led3_status = "on"
        print("led1: ", led1_status, "led2: ", led2_status, "led3: ", led3_status)

        turning_on = not turning_on