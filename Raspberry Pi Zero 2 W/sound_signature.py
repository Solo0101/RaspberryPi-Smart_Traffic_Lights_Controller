import librosa
import numpy as np
import matplotlib.pyplot as plt
import logging
from scipy.signal import find_peaks

# === CONFIG ===
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(threadName)s] %(levelname)s: %(message)s",
)

AUDIO_PATH = "Police_Siren_Sound_Effect.mp3"
TOP_N = 20                # Number of dominant frequencies to extract
MIN_PEAK_HEIGHT = 0.1     # Relative height (0 to 1) for peak filtering

# === Load and preprocess ===
y, sr = librosa.load(AUDIO_PATH, sr=16000, mono=True)
logging.info(f"Audio loaded: {AUDIO_PATH}, duration: {len(y)/sr:.2f}s, sample rate: {sr}")

# === Compute average magnitude spectrum ===
S = np.abs(librosa.stft(y, n_fft=2048, hop_length=512))  # shape: (freq_bins, frames)
avg_spectrum = np.mean(S, axis=1)

# === Convert bin indices to frequencies ===
frequencies = librosa.fft_frequencies(sr=sr, n_fft=2048)

# === Normalize and detect peaks ===
norm_spectrum = avg_spectrum / np.max(avg_spectrum)
peaks, _ = find_peaks(norm_spectrum, height=MIN_PEAK_HEIGHT)

# === Sort peaks by magnitude ===
peak_freqs = frequencies[peaks]
peak_mags = avg_spectrum[peaks]
sorted_indices = np.argsort(peak_mags)[::-1]

top_freqs = peak_freqs[sorted_indices][:TOP_N]

logging.info(f"\nTop {TOP_N} dominant frequencies (Hz):")
logging.info(np.round(top_freqs, 2))

# === Optional: plot ===
plt.figure(figsize=(10, 5))
plt.plot(frequencies, norm_spectrum, label="Normalized Spectrum")
plt.plot(peak_freqs, norm_spectrum[peaks], "x", label="Peaks")
plt.title("Average Spectrum and Detected Peaks")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Normalized Magnitude")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
