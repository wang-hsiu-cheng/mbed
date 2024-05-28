import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import threading
import logging
from time import sleep
import erpc
from exam4 import *
import sys

# Serial device and baud rate configuration
# serdev = "COM3"
# baudrate = 9600

# Limit for data points and initializing arrays for accelerometer and gyroscope data
x_limit = 100
i = 0
acce_x = [0] * x_limit
acce_y = [0] * x_limit
acce_z = [0] * x_limit
gyro_x = [0] * x_limit
gyro_y = [0] * x_limit
gyro_z = [0] * x_limit

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage: python led_test_client.py <serial port to use>")
        exit()

    # Initialize all erpc infrastructure
    xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
    client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
    client = client.exam4ServiceClient(client_mgr)

    # Blink LEDs on the connected erpc server
    turning_on = True
    input_flow = 0
    while True:
        order = input('input order: ')
        if(order == '1'):
            tr = int(input('input tr: '))
            print("Call led_on ", tr)
            client.StartSampling(tr)
            i = 0
        elif(order == '2'):
            print("Call led_off ", i)
            client.StopSampling(i)
            i = 0
        elif(order == '3'):
            input_flow = client.SensorData()
            input_flow // 1000000
            (input_flow % 100000 - input_flow % 1000) // 1000
            input_flow % 1000
            print()
            print(client.SensorData())
            print(client.SensorData())
            acce_x[i] = x
            acce_y[i] = y
            acce_z[i] = z
            i = (i + 1) % x_limit
        sleep(1)

def animate(i):
    """Animates the accelerometer and gyroscope data plots."""
    ax[0].clear()
    ax[0].plot(acce_x, label="acceleration x")
    ax[0].plot(acce_y, label="acceleration y")
    ax[0].plot(acce_z, label="acceleration z")
    ax[0].set_title("Accelerometer")
    ax[0].legend()
    ax[0].set_xlim(0, x_limit)

    # ax[1].clear()
    # ax[1].plot(gyro_x, label="gyro x")
    # ax[1].plot(gyro_y, label="gyro y")
    # ax[1].plot(gyro_z, label="gyro z")
    # ax[1].set_title("Gyroscope")
    # ax[1].legend()
    # ax[1].set_xlim(0, x_limit)


# Setup plot and start reading serial data in a separate thread
fig, ax = plt.subplots(2)
threading.Thread(target=read_serial_history).start()
ani = animation.FuncAnimation(fig, animate, interval=1, cache_frame_data=False)
plt.show()