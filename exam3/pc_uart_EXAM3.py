# from serial import Serial
import numpy as np
import serial
import time
import json

# some JSON:
with open('books.json', 'r') as file:
    content = file.read()

# parse x:
y = json.loads(content)

# the result is a Python dictionary:

# send the waveform table to mbed
serdev = 'COM3'
s = serial.Serial(serdev)
print("Sending ...")

# Blink LEDs on the connected erpc server
turning_on = True
input_flow = 0
money = 0
cola_number = 5
sevenup_number = 5
mcu_last_feedback = ' '

def Input_ID():
    student_ID = input('input ID: ')
    s.write(b'1\n')
    s.write(student_ID.to_bytes(4, 'little'))

Input_ID()
# s.write(b'5\n')

for book in y:
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)
    value_of_b = book["title"]
    s.write(value_of_b.encode('utf-8'))
    print(value_of_b)

while True:
    # order = input('input order: ')
    # if (order == '0'):
    #     s.write(b'0\n')
    #     print("add cola")
    # elif (order == '1'):
    #     s.write(b'1\n')
    #     print("add 7up")

    time.sleep(0.01)

    while s.in_waiting:
        mcu_feedback = s.readline().decode()  # receive and decode
        # if (mcu_feedback != mcu_last_feedback):
        print('feedback: ', mcu_feedback)
        mcu_last_feedback = mcu_feedback


