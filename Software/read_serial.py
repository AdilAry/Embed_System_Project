import serial

ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=115200,
)

while(True):
    print(ser.read())

ser.close()