import requests
import time
import serial

import config

# Define the URLs for the API endpoints
#get_url = 'http://192.168.223.190:8000/traffic_light/get'  # Replace with your actual URL
#post_url = 'http://192.168.223.190:8000/traffic_light/post'  # Replace with your actual URL

class APIHandler:
    def __init__(self, get_url, post_url, serial_device):
        self.get_url = get_url
        self.post_url = post_url
        self.ser = serial_device

    def read_current_state(self):
        if self.ser.in_waiting > 0:
                with config.read_lock:
                    state = self.ser.readline().decode('utf-8').strip()
                    return state
        return None

    def check_current_state_with_response(self, response):
        current_state = self.read_current_state()
        if current_state == 'NorthSouth' and response == [True, False, True, False]:
            return True
        elif current_state == 'EastWest' and response == [False, True, False, True]:
            return True
        return False

    def get_traffic_light_control_response(self):
        try:
            response = requests.get(self.get_url)
            if response.status_code == 200:
                return response.json()
            else:
                print(f"Failed to get data. Status code: {response.status_code}")
                return None
        except Exception as e:
            print(f"An error occurred: {e}")
            return None

    def post_traffic_light_control_status(self, status):
        try:
            data = {'trafficLightControlStatus': status}
            response = requests.post(self.post_url, json=data)
            if response.status_code == 202:
                print("Status posted successfully")
            else:
                print(f"Failed to post data. Status code: {response.status_code}")
        except Exception as e:
            print(f"An error occurred: {e}")

    def write_to_serial(self, response_list):
        if response_list == [True, False, True, False]:
            with config.write_lock:
                self.ser.write(b"NorthSouth\n")
        elif response_list == [False, True, False, True]:
            with config.write_lock:
                self.ser.write(b"EastWest\n")

    def toggle_traffic_lights(self, response_list):
        if not self.read_current_state() == 'AllRed':
           self.write_to_serial(response_list)

        while not self.check_current_state_with_response(response_list):
           pass

        new_status = 1
        time.sleep(2)
        self.post_traffic_light_control_status(new_status)

    def check_changed_state(self, response_list):
        current_state = self.read_current_state()
        if current_state == 'NorthSouth' and response_list == [False, True, False, True]:
            return True
        elif current_state == 'EastWest' and response_list == [True, False, True, False]:
            return True
        return False

    def run(self):
        while True:
            response_list = self.get_traffic_light_control_response()
            if response_list is not None:
                print("Received traffic light control response list:", response_list)
                print("Current State from API Handler: ", self.read_current_state())
                if self.check_changed_state(response_list):
                    self.toggle_traffic_lights(response_list)