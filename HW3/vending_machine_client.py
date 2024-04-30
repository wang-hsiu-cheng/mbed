# led_test_client.py
# Test client for erpc led server example
# Author: becksteing/Jing-Jia Liou
# Date: 02/13/2022
# Blinks LEDs on a connected Mbed-enabled board running the erpc LED server example

from time import sleep
import erpc
from vending_machine import *
import sys

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage: python vending_machine_client.py <serial port to use>")
        exit()

    # Initialize all erpc infrastructure
    xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
    client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
    client = client.VendingMachineServiceClient(client_mgr)

    # Blink LEDs on the connected erpc server
    turning_on = True
    input_flow = 0
    money = 0
    cola_number = 5
    sevenup_number = 5
    while True:
        input_flow = client.check_item_state()
        money = input_flow / 100
        cola_number = (input_flow % 100 - input_flow % 10) / 10
        sevenup_number = input_flow % 10
        if (cola_number == 0):
            print("add cola")
            client.add_item_amount(0)
        if (sevenup_number == 0):
            print("add 7up")
            client.add_item_amount(1)
        sleep(0.5)