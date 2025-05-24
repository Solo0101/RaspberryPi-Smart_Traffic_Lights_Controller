import numpy as np
import pyaudio
from scipy.fft import fft
import time
import logging

from SerialResponse import SerialResponse
import config

class MicrophoneHandler:
    SAMPLE_RATE = 16000
    SAMPLES = 128
    BUFFER_SIZE = 128
    THRESHOLD = 20000

    def __init__(self, serial_parser: SerialResponse, device_index: int):
        self.serial_parser = serial_parser
        self.device_index = device_index
        self.p = self.initialize_audio()
        self.stream = self.open_audio_stream()

    def initialize_audio(self):
        return pyaudio.PyAudio()

    def open_audio_stream(self):
        return self.p.open(format=pyaudio.paInt16,
                           channels=1,
                           rate=self.SAMPLE_RATE,
                           input=True,
                           frames_per_buffer=self.BUFFER_SIZE,
                           input_device_index=self.device_index)

    def detect_siren(self, data):
        vReal = np.array(data, dtype=np.float32)
        fft_data = fft(vReal)
        magnitudes = np.abs(fft_data)

        freqs = np.fft.fftfreq(self.SAMPLES, 1 / self.SAMPLE_RATE)
        for i in range(self.SAMPLES // 2):
            frequency = freqs[i]
            if 600 <= frequency <= 2000 and magnitudes[i] > self.THRESHOLD:
                return True
        return False

    def run(self):
        emergency_start_time = None
        allred_sent = False

        try:
            while True:
                current_time = time.monotonic()

                try:
                    data = np.frombuffer(self.stream.read(self.SAMPLES, exception_on_overflow=False), dtype=np.int16)

                    # Check for emergency mode timeout
                    with config.emergency_lock:
                        if config.emergency_active:
                            if emergency_start_time and current_time - emergency_start_time > 10:
                                config.emergency_active = False
                                logging.info("[MIC] Emergency cleared.")
                                emergency_start_time = None
                                allred_sent = False
                            continue  # Skip detection during emergency

                    if self.detect_siren(data):
                        with config.emergency_lock:
                            config.emergency_active = True
                            emergency_start_time = current_time

                        if not allred_sent:
                            with config.write_lock:
                                logging.info("[MIC] Siren detected. Sending AllRed.")
                                self.serial_parser.write_command("AllRed")
                                allred_sent = True

                except IOError as e:
                    logging.error(f"[MIC] Audio error: {e}")
        except KeyboardInterrupt:
            pass
        finally:
            self.stream.stop_stream()
            self.stream.close()
            self.p.terminate()
