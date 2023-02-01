#!/usr/bin/env python3
'''Motion script'''
import os
import logging
import json
import utilities
from camera import Camera

filename = '/home/{}/sync/MotionNotifier.log'

try:
    name = utilities.get_user()
    filename = filename.format(name)
    os.remove(filename)
except OSError as error:
    pass

# Add the log message handler to the logger
logging.basicConfig(filename=filename,
                    format='%(asctime)s - %(levelname)s - %(message)s',
                    level=logging.INFO)

logging.info("Starting program")

class FileNotFound(Exception):
    '''Exception class for file checking'''

class Motion():
    '''Motion class for finding'''
    def __init__(self):
        '''Constructor'''
        self.server_address = ''

    def get_settings(self):
        '''Get config env var'''
        logging.info('get_settings()')
        config_name = '/home/{}/sync/config.json'.format(utilities.get_user())
        try:
            if not os.path.isfile(config_name):
                raise FileNotFound('File is missing')
            with open(config_name) as file:
                data = json.load(file)
            self.server_address = '{}/motion'.format(data["manager_address"])
            logging.info(self.server_address)
        except KeyError:
            logging.error("Variables not set")
        except FileNotFound:
            logging.error("File is missing")

    def loop(self):
        '''Loop and wait for event'''
        logging.info('loop()')
        self.get_settings()
        try:
            camera = Camera(self.server_address)
            camera.run_capture()
        except KeyboardInterrupt:
            logging.info('Quit')

if __name__ == "__main__":
    motion = Motion()
    motion.loop()
