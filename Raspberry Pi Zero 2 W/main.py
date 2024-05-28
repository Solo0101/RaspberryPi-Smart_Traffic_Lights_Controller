from MicrophoneHandler import MicrophoneHandler
from APIHandler import APIHandler
import utilities

import threading

get_url = 'http://192.168.223.190:8000/traffic_light/get'
post_url = 'http://192.168.223.190:8000/traffic_light/post'

ser = utilities.initialize_serial(serial_port='/dev/ttyACM0', baud_rate=9600)

api_handler = APIHandler(get_url=get_url, post_url=post_url, serial_device=ser)
microphone_handler = MicrophoneHandler(serial_device=ser, device_index=1)

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