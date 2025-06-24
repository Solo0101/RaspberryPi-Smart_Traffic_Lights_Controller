import logging
import threading
from WebSocketHandler import WebSocketHandler
from SerialResponse import SerialResponse
import config

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# Initialize thread-safe lock if not already present
config.write_lock = threading.Lock()

def run_client():
    serial_handler = SerialResponse(serial_port="/dev/ttyACM0", baud_rate=9600)
    handler = WebSocketHandler(
        ws_url="ws://smart-intersection.go.ro:80/ws/pi_comms", # Replace with actual ip address
        serial_parser=serial_handler
    )
    handler.run()

if __name__ == "__main__":
    run_client()
