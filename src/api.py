import os
import requests
import logging

class Api:
    
    def __init__(self, server):
        '''Constructor for API'''
        self.server = server

    def publish_data(self, filename):
        '''Send data to server if asked'''
        try:
            data = {
                'image': filename
            }
            response = requests.post(self.server, json=data, timeout=5)
            if response.status_code == 200:
                logging.info("Requests successful")
                logging.info('Response: {}'.format(response))
                os.remove(filename)
            else:
                logging.error('Requests unsuccessful')
                logging.info('Response: {}'.format(response))
        except requests.ConnectionError as error:
            logging.error("Connection error: {}".format(error))
        except requests.Timeout as error:
            logging.error("Timeout on server: {}".format(error))
        except OSError:
            logging.error("File couldn't be removed")