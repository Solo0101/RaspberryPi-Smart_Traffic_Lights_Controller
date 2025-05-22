import time
import logging
from SerialResponse import SerialResponse
from constants import ARDUINO_COMMANDS

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

def main():
    try:
        serial_handler = SerialResponse(serial_port="/dev/ttyACM0", baud_rate=9600)
    except Exception as e:
        logging.critical(f"Failed to connect to Arduino: {e}")
        return

    print("\n>>> Serial test started.")
    print(">>> Press Ctrl+C to exit.")
    print(">>> Every 3s, Raspberry sends 'Increase' to Arduino.")

    last_sent = time.monotonic()

    try:
        while True:
            # Read and display state updates from Arduino
            state = serial_handler.read_and_parse_state()
            if state:
                logging.info(f"Arduino state: {state}")

            # Every 3 seconds, send a test command to Arduino
            if time.monotonic() - last_sent >= 3:
                serial_handler.write_command(ARDUINO_COMMANDS[0])
                logging.info("Sent 'Increase' command to Arduino.")
                last_sent = time.monotonic()

            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n>>> Serial test interrupted by user.")
    except Exception as e:
        logging.exception(f"Unexpected error: {e}")

if __name__ == "__main__":
    main()
