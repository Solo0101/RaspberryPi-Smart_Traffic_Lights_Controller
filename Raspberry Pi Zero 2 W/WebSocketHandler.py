import asyncio
import json
import logging
import websockets
import time
from SerialResponse import SerialResponse
import config
from constants import ARDUINO_COMMANDS

class WebSocketHandler:
    def __init__(self, ws_url, serial_device_path="/dev/ttyACM0"):
        self.ws_url = ws_url
        self.serial_parser = SerialResponse(serial_port=serial_device_path)
        self.send_queue = asyncio.Queue()
        self.read_interval = 1.0 # Read serial state every 1s
        self.reconnect_interval = 5.0 # Reconnect to websocket every 5s
        logging.info(f"[WebSocketHandler] Initialized for {ws_url}")

    async def reader_task(self, websocket):
        while True:
            try:
                state = self.serial_parser.read_and_parse_state()
                if state:
                    message = json.dumps(state)
                    await websocket.send(message)
                    logging.info(f"[WS] Sent state: {message}")
            except Exception as e:
                logging.warning(f"[Reader] Serial or send error: {e}")
            await asyncio.sleep(self.read_interval)

    async def responder_task(self, websocket):
        while True:
            try:
                response = await websocket.recv()
                logging.info(f"[WS] Received: {response}")
                command_data = json.loads(response)
                command = command_data.get("action")

                if command in ARDUINO_COMMANDS:
                    with config.write_lock:
                        self.serial_parser.write_command(command)
                elif command:
                    logging.warning(f"[Responder] Invalid command: {command}")
            except Exception as e:
                logging.warning(f"[Responder] Receive or serial write error: {e}")

    async def run_forever(self):
        while True:
            try:
                logging.info(f"[WS] Connecting to {self.ws_url}...")
                async with websockets.connect(self.ws_url) as websocket:
                    logging.info("[WS] Connected to backend.")
                    await asyncio.gather(
                        self.reader_task(websocket),
                        self.responder_task(websocket),
                    )
            except (websockets.ConnectionClosed, ConnectionRefusedError) as e:
                logging.warning(f"[WS] Disconnected: {e}. Reconnecting in {self.reconnect_interval}s.")
                await asyncio.sleep(self.reconnect_interval)
            except Exception as e:
                logging.error(f"[WS] Unexpected error: {e}")
                await asyncio.sleep(self.reconnect_interval)

    def run(self):
        logging.info("[WebSocketHandler] Starting event loop...")
        asyncio.run(self.run_forever())
        logging.info("[WebSocketHandler] Event loop terminated.")
