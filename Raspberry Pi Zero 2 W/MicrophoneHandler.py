import numpy as np
import pyaudio
from scipy.fft import fft
import serial
import time

import config

class MicrophoneHandler:
    SAMPLE_RATE = 16000
    SAMPLES = 128
    BUFFER_SIZE = 128
    THRESHOLD = 20000

    def __init__(self, serial_device, device_index):
        self.device_index = device_index
        self.ser = serial_device
        self.p = self.initialize_audio()
        self.stream = self.open_audio_stream()

    def initialize_audio(self):
        p = pyaudio.PyAudio()
        return p

    def open_audio_stream(self):
        stream = self.p.open(format=pyaudio.paInt16,
                             channels=1,
                             rate=self.SAMPLE_RATE,
                             input=True,
                             frames_per_buffer=self.BUFFER_SIZE,
                             input_device_index=self.device_index)
        return stream

    def read_current_state(self):
        if self.ser.in_waiting > 0:
            with config.read_lock:
                state = self.ser.readline().decode('utf-8').strip()
                return state
        return None

    def detect_siren(self, data):
        vReal = np.array(data, dtype=np.float32)
        fft_data = fft(vReal)
        magnitudes = np.abs(fft_data)

        freqs = np.fft.fftfreq(self.SAMPLES, 1/self.SAMPLE_RATE)
        siren_detected = False

        for i in range(self.SAMPLES // 2):
            frequency = freqs[i]
            if 600 <= frequency <= 2000 and magnitudes[i] > self.THRESHOLD:
                siren_detected = True
                break

        return siren_detected

    def run(self):
        try:
            while True:
                try:
                    data = np.frombuffer(self.stream.read(self.SAMPLES, exception_on_overflow=False), dtype=np.int16)
                    current_state = self.read_current_state()
                    if current_state:
                        print("Current State from siren detection:", current_state)

                    if self.detect_siren(data):
                        with config.write_lock:
                            print("Siren detected")
                            self.ser.write(b"AllRed\n")
                            time.sleep(3)
                except IOError as e:
                    print(f"Error recording: {e}")
        except KeyboardInterrupt:
            pass
        finally:
            self.stream.stop_stream()
            self.stream.close()
            self.p.terminate()