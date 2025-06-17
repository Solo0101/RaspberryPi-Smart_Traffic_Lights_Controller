import asyncio
import json
import logging
import websockets
import time
from SerialResponse import SerialResponse
import config
from constants import ARDUINO_COMMANDS, ARDUINO_ACTIONS, ARDUINO_DIRECTIONS
import base64

class WebSocketHandler:
    """
        Handles bidirectional communication over a WebSocket with the backend server.

        Parameters:
            ws_url (str): WebSocket URL of the backend server.
            serial_parser (SerialResponse): Parser object for serial communication with Arduino.
    """

    def __init__(self, ws_url: str, serial_parser: SerialResponse):
        """
            Initializes the WebSocketHandler and prepares authentication headers.

            Parameters:
                ws_url (str): WebSocket endpoint.
                serial_parser (SerialResponse): Instance to handle serial comms.
        """

        self.ws_url = ws_url
        self.serial_parser = serial_parser
        self.read_interval = 1.0 # Read serial state every 1s
        self.reconnect_interval = 5.0 # Reconnect to websocket every 5s
        self.last_sent_state = None
        self.username = "dani"
        self.password = "vdani"
        self.hazard_mode = False
        credentials = f"{self.username}:{self.password}"
        encoded_credentials = base64.b64encode(credentials.encode()).decode()

        self.headers = {
        "Authorization": f"Basic {encoded_credentials}"
        }
        logging.info(f"[WebSocketHandler] Initialized for {ws_url}")

    async def reader_task(self, websocket):
        """
            Periodically reads from Arduino and sends updates to the WebSocket server if changed.

            Parameters:
                websocket: Active WebSocket connection.
        """

        self.last_sent_state = None

        while True:
            try:
                state = self.serial_parser.read_and_parse_state()
                if state:
                    # Add hazard mode if active
                    if self.hazard_mode:
                        state["STATE"] = "HAZARD_MODE"

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
        """
            Handles incoming commands from the WebSocket server and sends them to the Arduino.

            Parameters:
                websocket: Active WebSocket connection.
        """
        while True:
            try:
                response = await websocket.recv()
                logging.info(f"[WS] Received: {response}")
                command_data = json.loads(response)

                action = command_data.get("action")
                direction = command_data.get("direction")

                if action is None:
                    action = ""
                    logging.warning("[WS] 'action' field is missing in message.")

                if direction is None:
                    direction = ""
                    logging.warning("[WS] 'direction' field is missing in message.")

                if action or direction:
                    if action in ARDUINO_ACTIONS and direction in ARDUINO_DIRECTIONS:
                        full_command = f"{action}{direction}"
                        if full_command in ARDUINO_COMMANDS:
                            
                            if full_command == "HazardMode":
                                logging.info("[WS] Hazard mode activated.")
                                self.hazard_mode = True
                            else:
                                self.hazard_mode = False

                            with config.emergency_lock:
                                if config.emergency_active:
                                    logging.info(f"[WS] Skipped command '{full_command}' due to active emergency.")
                                    continue

                            with config.write_lock:
                                self.serial_parser.write_command(full_command)
                                logging.info(f"[WS] Sent command to Arduino: {full_command}")
                    else:
                        logging.warning(f"[WS] Invalid action or direction: {action} {direction}")
                else:
                    logging.warning("[WS] Missing 'action' and 'direction' fields in message.")

            except Exception as e:
                logging.warning(f"[Responder] Error: {e}")

    async def run_forever(self):
        """
            Maintains a persistent WebSocket connection and starts communication tasks.
        """

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
        """
            Starts the asyncio event loop for WebSocket communication.
        """

        logging.info("[WebSocketHandler] Starting event loop...")
        asyncio.run(self.run_forever())
        logging.info("[WebSocketHandler] Event loop terminated.")
