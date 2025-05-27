import numpy as np
import alsaaudio
import time
import logging

from SerialResponse import SerialResponse
import config
from constants import SIREN_SIGNATURE_FREQS, FREQ_TOLERANCE, MAG_THRESHOLD, ARDUINO_ALL_RED_TIMEOUT

class MicrophoneHandler:
    """
        Handles audio sampling and police siren detection using FFT.
    
        Parameters:
            serial_parser (SerialResponse): Parser for sending serial commands.
            device_name (str): ALSA audio device identifier.
    """
    SAMPLE_RATE = 44100
    SAMPLES = 2048
    CHANNELS = 1
    FORMAT = alsaaudio.PCM_FORMAT_S16_LE
    PERIOD_SIZE = 1024

    def __init__(self, serial_parser: SerialResponse, device_name: str = "default"):
        """
            Initializes audio configuration and internal buffer.

            Parameters:
                serial_parser (SerialResponse): Serial communication handler.
                device_name (str): ALSA audio device name.
        """

        self.serial_parser = serial_parser
        self.device_name = device_name
        self.pcm = self.initialize_audio()
        self.audio_buffer = np.array([], dtype=np.int16)

    def initialize_audio(self) -> alsaaudio.PCM:
        """
            Sets up ALSA audio capture in non-blocking mode.

            Returns:
                alsaaudio.PCM: Configured ALSA PCM capture object.
        """

        pcm = alsaaudio.PCM(type=alsaaudio.PCM_CAPTURE,
                            mode=alsaaudio.PCM_NONBLOCK,
                            channels=self.CHANNELS,
                            rate=self.SAMPLE_RATE,
                            format=self.FORMAT,
                            periodsize=self.PERIOD_SIZE,
                            device=self.device_name)
        return pcm

    def detect_siren(self, data: np.ndarray) -> bool:
        """
            Runs FFT on audio buffer and checks for police siren frequency pattern.

            Parameters:
                data (np.ndarray): Audio data for FFT analysis.

            Returns:
                bool: True if siren signature is detected, False otherwise.
        """

        vReal = np.array(data, dtype=np.float32)
        vReal *= np.hamming(len(vReal))

        fft_data = np.abs(np.fft.rfft(vReal))
        freqs = np.fft.rfftfreq(len(vReal), 1 / self.SAMPLE_RATE)

        matched_peaks = 0
        for target_freq in SIREN_SIGNATURE_FREQS:
            band = (freqs >= target_freq - FREQ_TOLERANCE) & (freqs <= target_freq + FREQ_TOLERANCE)
            peak_found = np.any(fft_data[band] > MAG_THRESHOLD)
            if peak_found:
                matched_peaks += 1

        return matched_peaks >= len(SIREN_SIGNATURE_FREQS) // 2

    def run(self):
        """
            Continuously captures and processes audio. Sends emergency command if siren is detected.
        """

        emergency_start_time = None
        allred_sent = False

        try:
            while True:
                current_time = time.monotonic()

                # Read a frame from ALSA (non-blocking)
                length, data = self.pcm.read()
                if length > 0:
                    chunk = np.frombuffer(data, dtype=np.int16)
                    self.audio_buffer = np.concatenate((self.audio_buffer, chunk))

                    # Only process once we have a full window
                    if len(self.audio_buffer) >= self.SAMPLES:
                        buffer_to_process = self.audio_buffer[:self.SAMPLES]
                        self.audio_buffer = self.audio_buffer[self.SAMPLES:]  # keep remainder

                        with config.emergency_lock:
                            if config.emergency_active:
                                if emergency_start_time and current_time - emergency_start_time > ARDUINO_ALL_RED_TIMEOUT:
                                    config.emergency_active = False
                                    logging.info("[MIC] Emergency cleared.")
                                    emergency_start_time = None
                                    allred_sent = False
                                continue

                        if self.detect_siren(buffer_to_process):
                            with config.emergency_lock:
                                config.emergency_active = True
                                emergency_start_time = current_time

                            if not allred_sent:
                                with config.write_lock:
                                    logging.info("[MIC] Siren detected. Sending AllRed.")
                                    self.serial_parser.write_command("AllRed")
                                    allred_sent = True

                time.sleep(0.1)  # small sleep to yield CPU

        except KeyboardInterrupt:
            pass
