# Constants for the traffic light control system
ARDUINO_ACTIONS = ("Increase", "Decrease", "Jump", "AllRed", "Resume")
ARDUINO_DIRECTIONS = ("NS", "EW", "NSG", "NSY", "EWG", "EWY")
ARDUINO_COMMANDS = ("IncreaseNS", "DecreaseNS", "IncreaseEW", "DecreaseEW", "JumpNSG", "JumpNSY", "JumpEWG", "JumpEWY", "AllRed", "Resume")
ARDUINO_ALL_RED_TIMEOUT = 10

# Constants for audio processing
SIREN_SIGNATURE_FREQS = [1171.88, 1109.38, 1046.88, 1070.31, 984.38, 1007.81, 945.31, 875., 812.5, 921.88, 750., 679.69, 648.44, 617.19, 1875., 1937.5, 2000., 1835.94, 2109.38, 2304.69]
FREQ_TOLERANCE = 30
MAG_THRESHOLD = 80000
