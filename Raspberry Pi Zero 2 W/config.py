import threading

read_lock = threading.Lock()
write_lock = threading.Lock()
emergency_lock = threading.Lock()
emergency_active = False
