import asyncio
import json
import logging
import websockets
import time
from SerialResponse import SerialResponse
import config

class WebSocketAPIHandler:
    def __init__(self, ws_url, serial_device_path="/dev/ttyACM0"):
        self.ws_url = ws_url
        self.serial_parser = SerialResponse(serial_port=serial_device_path)
        self.reconnect_delay = 5  # seconds
        logging.info(f"[WebSocketAPIHandler] Initialized for {ws_url}")

    async def send_and_receive(self, websocket):
        last_tick = time.monotonic()
        interval = 1.0  # run every second

        while True:
            now = time.monotonic()
            if now - last_tick >= interval:
                try:
                    state = self.serial_parser.read_and_parse_state()
                    if state:
                        # Send current intersection state to backend
                        message = json.dumps(state)
                        await websocket.send(message)
                        logging.info(f"[WS] Sent state: {message}")

                        # Await backend response
                        response = await websocket.recv()
                        logging.info(f"[WS] Received response: {response}")
                        command_data = json.loads(response)
                        command = command_data.get("action")

                        # Forward valid command to Arduino
                        if command in ("Increase", "Decrease"):
                            with config.write_lock:
                                self.serial_parser.write_command(command)
                        elif command:
                            logging.warning(f"[WS] Ignoring unrecognized action: {command}")
                except websockets.ConnectionClosed as e:
                    logging.warning(f"[WS] Connection closed: {e}")
                    raise
                except Exception as e:
                    logging.error(f"[WS] Processing error: {e}")
                last_tick = now
            await asyncio.sleep(0.01)  # let event loop breathe

    async def run_forever(self):
        while True:
            try:
                logging.info(f"[WS] Attempting to connect to {self.ws_url}")
                async with websockets.connect(self.ws_url) as websocket:
                    logging.info("[WS] Connected to backend.")
                    await self.send_and_receive(websocket)
            except (websockets.ConnectionClosed, ConnectionRefusedError) as e:
                logging.warning(f"[WS] Reconnecting in {self.reconnect_delay}s: {e}")
                await asyncio.sleep(self.reconnect_delay)
            except Exception as e:
                logging.error(f"[WS] Unexpected error: {e}")
                await asyncio.sleep(self.reconnect_delay)

    def run(self):
        logging.info("[WebSocketAPIHandler] Event loop started.")
        asyncio.run(self.run_forever())
