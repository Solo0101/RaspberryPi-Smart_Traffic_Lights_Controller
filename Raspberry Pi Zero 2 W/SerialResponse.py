import logging
import serial

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(threadName)s] %(levelname)s: %(message)s",
)

class SerialResponse:
    def __init__(self, serial_port="/dev/ttyACM0", baud_rate=9600):
        try:
            self.ser = serial.Serial(
                port=serial_port,
                baudrate=baud_rate
            )
            logging.info(f"[Serial] Connected to {serial_port} at {baud_rate} baud.")
        except serial.SerialException as e:
            logging.critical(f"[Serial] Failed to open serial port: {e}")
            raise

    def read_and_parse_state(self):
        try:
            if self.ser.in_waiting > 0:
                line = self.ser.readline().decode().strip()
                logging.info(f"[Serial] Raw input: {line}")
                if line.startswith("STATE:"):
                    parsed = self._parse_line(line)
                    logging.info(f"[Serial] Parsed state: {parsed}")
                    return parsed
        except serial.SerialException as e:
            logging.error(f"[Serial] Disconnected or error: {e}")
            raise
        except Exception as e:
            logging.warning(f"[Serial] Parsing failure or bad input: {e}")
        return None

    def _parse_line(self, line):
        parts = line.split(",")
        state_info = {k: v for k, v in (item.split(":") for item in parts)}
        return {
            "STATE": state_info.get("STATE", ""),
            "NSG": int(state_info.get("NSG", 0)),
            "NSY": int(state_info.get("NSY", 0)),
            "EWG": int(state_info.get("EWG", 0)),
            "EWY": int(state_info.get("EWY", 0)),
        }

    def write_command(self, command):
        try:
            self.ser.write((command + "\n").encode())
            logging.info(f"[Serial] Sent command: {command}")
        except serial.SerialException as e:
            logging.error(f"[Serial] Write failed: {e}")
            raise
