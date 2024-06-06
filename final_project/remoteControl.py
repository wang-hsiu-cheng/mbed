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

car_width = 10.3  # distance between two wheels
global_x = 0
global_y = 0
car_direction = -np.pi / 2
communication_frequency = 0.1
n = 79  # total steps
x_position = []
y_position = []

# set graph style
plt.ion()
plt.figure(figsize=(6, 6))
plt.xlim([-10, 10])
plt.ylim([-10, 10])


def draw_map():
    car_velocity = (left_velocity + right_velocity) / 2
    car_angular_vel = (right_velocity - left_velocity) / car_width
    car_direction += -car_angular_vel * communication_frequency  # update local theta
    # print(left_velocity, right_velocity)
    # get local position
    car_x = car_velocity * np.cos(car_angular_vel) * 0.1
    car_y = car_velocity * np.sin(car_angular_vel) * 0.1
    # transform to local position
    global_x += np.cos(car_direction) * car_x + np.sin(car_direction) * car_y
    global_y += -np.sin(car_direction) * car_x + np.cos(car_direction) * car_y
    x_position.append(global_x)
    y_position.append(global_y)

    # plot point as time goes on
    plt.scatter(x_position, y_position, s=5, c='r',
                label="Car Path", marker='o')
    plt.ioff()
    plt.pause(0.005)


async def main():
    devices = await BleakScanner.discover()
    device_dict = {}
    for d in devices:
        if d.name != "Unknown" and d.name is not None:
            if (d.name == "GattServer39"):
                my_device = d
            device_dict[d.name] = d
    print(device_dict)

    address = my_device.address
    async with BleakClient(address) as client:
        svcs = await client.get_services()
        while True:
            distance_byte = await client.read_gatt_char(uuid_distaince_cm, response=False)
            pattern_byte = await client.read_gatt_char(uuid_qti_pattern, response=False)
            length_byte = await client.read_gatt_char(uuid_qti_hint, response=False)
            left_velocity_byte = await client.read_gatt_char(uuid_accelerometer_0, response=False)
            right_velocity_byte = await client.read_gatt_char(uuid_accelerometer_1, response=False)
            bagNumber = int.from_bytes(distance_byte)
            pattern = int.from_bytes(pattern_byte)
            length = int.from_bytes(length_byte)
            left_velocity = int.from_bytes(left_velocity_byte) / 10
            right_velocity = int.from_bytes(right_velocity_byte) / 10
            draw_map()
            print(f"distance:pattern: {bagNumber*255+length}:{pattern}")
            time.sleep(1)

asyncio.run(main())
plt.show()
