import serial
from camera import close, take_picture, save_image
from detector import detect_mult
import subprocess

# setup serial communication between arduino and jetson with baud rate 9600
arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

frames = []

# wait for command
while True:
    message = arduino.readline().decode("utf-8").strip()
    if message == "SHUTDOWN":
        print("Shutdown requested by Arduino.", flush=True)
        subprocess.run(["sudo", "-n", "shutdown", "-h", "now"], check=True)
        break
    elif (message == "VISION"):
        frame = take_picture()
        #save_image(frame)
        frames.append(frame)

        arduino.write(b"ROTATE\n")
        print("vision")
    elif message == "COLOR":
        isBadScrew = detect_mult(frames)
        frames = []
        if isBadScrew:
            arduino.write(b"RED\n")
        else:
            arduino.write(b"GREEN\n")
    




