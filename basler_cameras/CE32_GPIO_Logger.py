import serial
import serial.tools.list_ports
import time
from datetime import datetime

def list_comports():
    ports = serial.tools.list_ports.comports()
    for i, port in enumerate(ports):
        print(f"{i + 1}. {port}")
    selected_port = int(input("Please select a port: ")) - 1
    return ports[selected_port].device

def listen_to_comport(port):
    ser = serial.Serial(port, 119200, timeout=0)
    # Create a unique filename using the current datetime
    start_time = datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f"comport_data_{start_time}.txt"

    while True:
        ser.read_until(b'\x3c')  # read until 0x3c
        while ser.inWaiting() < 4:
            time.sleep(0.01)
        data = ser.read(4)
        if data[3] == ord(b'\x3e'):
            if data[1]== ord(b'\x83'):
                with open(filename, 'a') as f:
                    current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
                    f.write(f"Time: {current_time}, Data: {data[2]}\n")
            print(f"Received Signal: {data[2]} " + current_time)
if __name__ == "__main__":
    comport = list_comports()
    listen_to_comport(comport)
