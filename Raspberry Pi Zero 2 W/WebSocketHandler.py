import asyncio
import json
import logging
import websockets
import time
from SerialResponse import SerialResponse
import config
from constants import ARDUINO_COMMANDS
import base64

class WebSocketHandler:
    def __init__(self, ws_url: str, serial_parser: SerialResponse):
        self.ws_url = ws_url
        self.serial_parser = serial_parser
        self.read_interval = 1.0 # Read serial state every 1s
        self.reconnect_interval = 5.0 # Reconnect to websocket every 5s
        self.last_sent_state = None
        self.username = "dani"
        self.password = "vdani"
        credentials = f"{self.username}:{self.password}"
        encoded_credentials = base64.b64encode(credentials.encode()).decode()

        self.headers = {
        "Authorization": f"Basic {encoded_credentials}"
        }
        logging.info(f"[WebSocketHandler] Initialized for {ws_url}")

    async def reader_task(self, websocket):
        self.last_sent_state = None

        while True:
            try:
                state = self.serial_parser.read_and_parse_state()
                if state:
                    state_str = json.dumps(state, sort_keys=True)

                    if state_str != self.last_sent_state:
                        await websocket.send(state_str)
                        self.last_sent_state = state_str
                        logging.info(f"[WS] Sent updated state: {state_str}")
                    else:
                        logging.debug("[WS] Skipped sending unchanged state.")
            except Exception as e:
                logging.warning(f"[Reader] Serial read/send error: {e}")

            await asyncio.sleep(0.1)  # check frequently but only send if needed

    async def responder_task(self, websocket):
        while True:
            try:
                response = await websocket.recv()
                logging.info(f"[WS] Received: {response}")
                command_data = json.loads(response)
                command = command_data.get("action")

                if command in ARDUINO_COMMANDS:
                    with config.emergency_lock:
                        if config.emergency_active:
                            logging.info(f"[WS] Skipped backend command '{command}' due to emergency.")
                            continue
                    
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
                async with websockets.connect(self.ws_url, additional_headers=self.headers) as websocket:
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
