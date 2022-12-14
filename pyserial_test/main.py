import serial
import time

PORT = "/dev/cu.usbserial-14240" # Arduino的埠號
BAUD = 115200 #設定baud rate 

arduino = serial.Serial(PORT, BAUD, timeout=0.1)

# def write_read(x):
#     arduino.write(bytes(x, 'utf-8'))
#     time.sleep(0.05)
#     data = arduino.readline().decode('utf-8').rstrip()
#     return data

# while True:
#     num = input("Enter a number: ") # Taking input from user
#     value = write_read(num)
#     print(value) # printing the value

while(1):
    data = arduino.readline().decode('utf-8').rstrip()
    if (data):
        print(data)