from MicrophoneHandler import MicrophoneHandler
from WebSocketHandler import WebSocketHandler
from SerialResponse import SerialResponse
import utilities

import threading

# Replace with actual websocket URL
ws_url = 'ws://smart-intersection.go.ro:80/ws/pi_comms'

serial_parser = SerialResponse(serial_port="/dev/ttyACM0", baud_rate=9600)

api_handler = WebSocketHandler(ws_url=ws_url, serial_parser=serial_parser)
microphone_handler = MicrophoneHandler(serial_parser=serial_parser, device_name="hw:0,0")

video_stream_command = "libcamera-vid --framerate 15 -t 0 --inline --listen -o tcp://0.0.0.0:8888"

video_stream_command_thread = threading.Thread(target=utilities.run_command, args=(video_stream_command,))
api_handler_thread = threading.Thread(target=api_handler.run)
microphone_handler_thread = threading.Thread(target=microphone_handler.run)

video_stream_command_thread.start()
api_handler_thread.start()
microphone_handler_thread.start()

video_stream_command_thread.join()
api_handler_thread.join()
microphone_handler_thread.join()
