import logging
import subprocess

def run_command(command):
    """
        Executes a shell command and prints its output.

        Parameters:
            command (str): Shell command to execute.
    """
    try:
        result = subprocess.run(command, shell=True)
        logging.info(f"Command output: {result.stdout}")
        logging.error(f"Command errors: {result.stderr}")
    except Exception as e:
        logging.error(f"An error occurred: {e}")
