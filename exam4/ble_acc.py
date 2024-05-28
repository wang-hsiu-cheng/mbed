import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import threading
import logging
from time import sleep
import erpc
from exam4 import *
import sys
import asyncio
import time
from bleak import BleakScanner, BleakClient

uuid_gatt_service = '0000a390-0000-1000-8000-00805f9b34fb'
uuid_one_byte_characteristic = '0000a391-0000-1000-8000-00805f9b34fb'
uuid_hour_characteristic = '0000b391-0000-1000-8000-00805f9b34fb'
uuid_min_characteristic = '0000b392-0000-1000-8000-00805f9b34fb'
uuid_second_characteristic = '0000b393-0000-1000-8000-00805f9b34fb'

x_limit = 100
i = 0
acce_x = [0] * x_limit
acce_y = [0] * x_limit
acce_z = [0] * x_limit
gyro_x = [0] * x_limit
gyro_y = [0] * x_limit
gyro_z = [0] * x_limit

def animate(i):
    """Animates the accelerometer and gyroscope data plots."""
    ax[0].clear()
    ax[0].plot(acce_x, label="acceleration x")
    ax[0].plot(acce_y, label="acceleration y")
    ax[0].plot(acce_z, label="acceleration z")
    ax[0].set_title("Accelerometer")
    ax[0].legend()
    ax[0].set_xlim(0, x_limit)

async def main():
    devices = await BleakScanner.discover()
    device_dict = {}
    for d in devices:
        if d.name != "Unknown" and d.name is not None:
            if(d.name=="GattServer39"):
                my_device=d
            device_dict[d.name] = d
    print(device_dict)

    address=my_device.address
    async with BleakClient(address) as client:
        svcs = await client.get_services()
        order = int(input('input order: '))
        if(order == 1):
            tr = input('input tr: ')
            print("Call led_on ", tr)
            battery_level = await client.write_gatt_char(uuid_one_byte_characteristic, tr, response=False)
            i = 0
        elif(order == 2):
            print("Call led_off ", i)
            battery_level = await client.write_gatt_char(uuid_one_byte_characteristic, 0, response=False)
            i = 0
        elif(order == 3):
            hour_byte = await client.read_gatt_char(uuid_hour_characteristic, response=False)
            min_byte = await client.read_gatt_char(uuid_min_characteristic, response=False)
            second_byte = await client.read_gatt_char(uuid_second_characteristic, response=False)
            hour=int.from_bytes(hour_byte)
            minute=int.from_bytes(min_byte)
            second=int.from_bytes(second_byte)
            hour = acce_x[i]
            minute = acce_y[i]
            second = acce_z[i]
            i = (i + 1) % x_limit
            print(f"hour:min:second: {hour}:{minute}:{second}")
        # charset=[b'0', b'1']
        # for c in charset:
        # battery_level = await client.write_gatt_char(uuid_one_byte_characteristic, tr, response=False)
            # time.sleep(1)
            #print(int.from_bytes(battery_level,byteorder='big'))

asyncio.run(main())

fig, ax = plt.subplots(2)
threading.Thread(target=read_serial_history).start()
ani = animation.FuncAnimation(fig, animate, interval=1, cache_frame_data=False)
plt.show()
