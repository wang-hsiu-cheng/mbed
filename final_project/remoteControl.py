import asyncio
from bleak import BleakScanner, BleakClient
import time

uuid_car_return_service = '0000b390-0000-1000-8000-00805f9b34fb'
uuid_distaince_cm = '0000b391-0000-1000-8000-00805f9b34fb'
uuid_qti_pattern = '0000b392-0000-1000-8000-00805f9b34fb'
uuid_qti_hint = '0000b393-0000-1000-8000-00805f9b34fb'
uuid_accelerometer_0 = '0000b394-0000-1000-8000-00805f9b34fb'
uuid_accelerometer_1 = '0000b395-0000-1000-8000-00805f9b34fb'
uuid_accelerometer_2 = '0000b396-0000-1000-8000-00805f9b34fb'

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
        while True:
            distance_byte = await client.read_gatt_char(uuid_distaince_cm, response=False)
            pattern_byte = await client.read_gatt_char(uuid_qti_pattern, response=False)
            hint_byte = await client.read_gatt_char(uuid_qti_hint, response=False)
            acc_0_byte = await client.read_gatt_char(uuid_accelerometer_0, response=False)
            acc_1_byte = await client.read_gatt_char(uuid_accelerometer_1, response=False)
            acc_2_byte = await client.read_gatt_char(uuid_accelerometer_2, response=False)
            distance = float.from_bytes(distance_byte)
            pattern = int.from_bytes(pattern_byte)
            hint = int.from_bytes(hint_byte)
            acc_0 = float.from_bytes(acc_0_byte)
            acc_1 = float.from_bytes(acc_1_byte)
            acc_2 = float.from_bytes(acc_2_byte)
            print(f"distance:pattern:hint: {distance}:{pattern}:{hint}:{acc_0}:{acc_1}:{acc_2}")
            time.sleep(1)

asyncio.run(main())