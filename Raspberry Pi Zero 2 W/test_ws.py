import logging
import threading
from WebSocketHandler import WebSocketHandler
import config

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

# Initialize thread-safe lock if not already present
config.write_lock = threading.Lock()

def run_client():
    handler = WebSocketHandler(
        ws_url="ws://smart-intersection.go.ro:80/ws/pi_comms", # Replace with actual ip address
        serial_device_path="/dev/ttyACM0"
    )
    handler.run()

if __name__ == "__main__":
    run_client()
