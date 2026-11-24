import serial
from camera import close, take_picture, save_image

# setup serial communication between arduino and jetson with baud rate 9600
arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

# wait for command
while True:
    message = arduino.readline().decode("utf-8").strip()
    if (message == "VISION"):
        frame = take_picture()
        save_image(frame)
        arduino.write(b"ROTATE\n")
        print("vision")
    elif message == "COLOR":
        arduino.write(b"GREEN\n")
        print("turning green")



