from MicrophoneHandler import MicrophoneHandler
from WebSocketHandler import WebSocketHandler
from SerialResponse import SerialResponse
import utilities

import threading
import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(threadName)s] %(levelname)s: %(message)s",
)

#TODO PUT EVERYTHING IN TRY CATCH AND ON KEYBOARD EXCEPTION CALL stop_tunnel.sh

try:
    # Replace with actual websocket URL
    ws_url = 'ws://smart-intersection.go.ro:80/ws/pi_comms'

    serial_parser = SerialResponse(serial_port="/dev/ttyACM0", baud_rate=9600)

    api_handler = WebSocketHandler(ws_url=ws_url, serial_parser=serial_parser)
    microphone_handler = MicrophoneHandler(serial_parser=serial_parser, device_name="hw:0,0")

    # utilities.run_command("./start_tunnel.sh")
    video_stream_command = "libcamera-vid -v 0 -q 100 --mode 2592:1944:12:P --awb indoor --contrast 1.5 --saturation 1.5 --sharpness 1.5 --denoise cdn_hq  --framerate 15 -t 0 --inline --listen -o tcp://0.0.0.0:8888"

    video_stream_command_thread = threading.Thread(target=utilities.run_command, args=(video_stream_command,))
    api_handler_thread = threading.Thread(target=api_handler.run)
    microphone_handler_thread = threading.Thread(target=microphone_handler.run)

    video_stream_command_thread.start()
    api_handler_thread.start()
    microphone_handler_thread.start()

    video_stream_command_thread.join()
    api_handler_thread.join()
    microphone_handler_thread.join()

except KeyboardInterrupt:
    logging.info("Keyboard interrupt detected. Stopping all processes...")
    # utilities.run_command("./stop_tunnel.sh")
