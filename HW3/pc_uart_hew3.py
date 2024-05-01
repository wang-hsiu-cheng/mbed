# from serial import Serial
import numpy as np
import serial
import time

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

while True:
    order = input('input order: ')
    if (order == '0'):
        s.write(b'0\n')
        print("add cola")
    elif (order == '1'):
        s.write(b'1\n')
        print("add 7up")

    time.sleep(0.01)

    # while s.in_waiting:
    #     mcu_feedback = s.readline().decode()  # receive and decode
    #     # if (mcu_feedback != mcu_last_feedback):
    #     print('feedback: ', mcu_feedback)
    #     mcu_last_feedback = mcu_feedback

# i=0
# for data in waveformTable:
#     s.write(bytes(formatter(data), 'UTF-8'))
#     i=i+1
#     print("Loading " + str(i) + "th data.")
#     time.sleep(0.001)
# print("Waveform sended.")

# read the wavefrom from mbed
# for i in range(15):
while(s.readable()):
    line=s.readline()
    print(line.decode("utf-8"))
    line=line.rstrip(b'\r\n')
    if(line==b'end'):
      break

s.close()