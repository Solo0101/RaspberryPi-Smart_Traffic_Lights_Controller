import logging
import serial

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(threadName)s] %(levelname)s: %(message)s",
)

class SerialResponse:
    """
        Manages serial communication with the Arduino device.

        Parameters:
            serial_port (str): Path to the serial port.
            baud_rate (int): Baud rate for serial communication.
    """

    def __init__(self, serial_port: str = "/dev/ttyACM0", baud_rate: int = 9600):
        """
            Initializes the serial connection with the specified port and baud rate.

            Parameters:
                serial_port (str): Serial port identifier.
                baud_rate (int): Communication speed.
        """

        try:
            self.ser = serial.Serial(
                port=serial_port,
                baudrate=baud_rate
            )
            logging.info(f"[Serial] Connected to {serial_port} at {baud_rate} baud.")
        except serial.SerialException as e:
            logging.critical(f"[Serial] Failed to open serial port: {e}")
            raise

    def read_and_parse_state(self) -> dict | None:
        """
            Reads serial data and parses traffic light state if available.

            Returns:
                dict or None: Parsed state dictionary or None on failure.
        """

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

    def _parse_line(self, line: str) -> dict:
        """
            Internal method to parse the 'STATE:' line format.

            Parameters:
                line (str): Raw input line.

            Returns:
                dict: Parsed traffic light state.
        """

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
        """
            Sends a command string over the serial connection.

            Parameters:
                command (str): Command to send to the Arduino.
        """

        try:
            self.ser.write((command + "\n").encode())
            logging.info(f"[Serial] Sent command: {command}")
        except serial.SerialException as e:
            logging.error(f"[Serial] Write failed: {e}")
            raise
