# for BLE
import asyncio
from bleak import BleakScanner, BleakClient
import time
# for graph
import numpy as np
import openpyxl
import matplotlib.pyplot as plt

uuid_car_return_service = '0000b390-0000-1000-8000-00805f9b34fb'
uuid_distaince_cm = '0000b391-0000-1000-8000-00805f9b34fb'
uuid_qti_pattern = '0000b392-0000-1000-8000-00805f9b34fb'
uuid_qti_hint = '0000b393-0000-1000-8000-00805f9b34fb'
uuid_accelerometer_0 = '0000b394-0000-1000-8000-00805f9b34fb'
uuid_accelerometer_1 = '0000b395-0000-1000-8000-00805f9b34fb'
# uuid_accelerometer_2 = '0000b396-0000-1000-8000-00805f9b34fb'

left_velocity = 0
right_velocity = 0

x_position = []
y_position = []

def draw_map(check):
    if (check == False):
        # set graph style for one time
        plt.ion()
        plt.figure(figsize=(6, 6))
        plt.xlim([-10, 10])
        plt.ylim([-10, 10])
    # plot point as time goes on
    plt.scatter(x_position, y_position, s=5, c='r', label="Car Path", marker='o')
    plt.ioff()
    plt.pause(0.005)


async def ble():
    check = False
    # define params
    car_width = 10.3  # distance between two wheels
    global_x = 0
    global_y = 0
    car_direction = -np.pi / 2
    communication_frequency = 0.1
    # connect to remote device
    devices = await BleakScanner.discover()
    device_dict = {}
    for d in devices:
        if d.name != "Unknown" and d.name is not None:
            if (d.name == "GattServer39"):
                my_device = d
            device_dict[d.name] = d
    print(device_dict)
    # receive ble message
    address = my_device.address
    async with BleakClient(address) as client:
        svcs = await client.get_services()
        while True:
            bagNumber_byte = await client.read_gatt_char(uuid_distaince_cm, response=False)
            pattern_byte = await client.read_gatt_char(uuid_qti_pattern, response=False)
            length_byte = await client.read_gatt_char(uuid_qti_hint, response=False)
            left_velocity_byte = await client.read_gatt_char(uuid_accelerometer_0, response=False)
            right_velocity_byte = await client.read_gatt_char(uuid_accelerometer_1, response=False)
            bagNumber = int.from_bytes(bagNumber_byte)
            pattern = int.from_bytes(pattern_byte)
            length = int.from_bytes(length_byte)
            # get 2 wheels' velocity
            left_velocity = int.from_bytes(left_velocity_byte) / 10
            right_velocity = int.from_bytes(right_velocity_byte) / 10
            print(f"distance:pattern: {bagNumber*255+length}:{pattern}")
            # caculate car's position
            car_velocity = (left_velocity + right_velocity) / 2
            car_angular_vel = (right_velocity - left_velocity) / car_width
            car_direction += -car_angular_vel * communication_frequency  # update local theta
                # get local position
            car_x = car_velocity * np.cos(car_angular_vel) * 0.1
            car_y = car_velocity * np.sin(car_angular_vel) * 0.1
                # transform to local position
            global_x += np.cos(car_direction) * car_x + np.sin(car_direction) * car_y
            global_y += -np.sin(car_direction) * car_x + np.cos(car_direction) * car_y
            x_position.append(global_x)
            y_position.append(global_y)
            draw_map(check)
            time.sleep(0.005)

async def main():
    async_task = asyncio.create_task(ble())
    plt.show()
    await async_task

asyncio.run(main())
