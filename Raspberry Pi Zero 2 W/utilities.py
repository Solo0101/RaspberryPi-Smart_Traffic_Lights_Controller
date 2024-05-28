import serial
import subprocess

def initialize_serial(serial_port, baud_rate):
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    ser.flush()
    return ser

def run_command(command):
    try:
        result = subprocess.run(command, shell=True)
        print(f"Command output: {result.stdout}")
        print(f"Command errors: {result.stderr}")
    except Exception as e:
        print(f"An error occurred: {e}")