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
            #headers = { "Content-Type": "multipart/form-data"}
            data = {
                'image': open(filename, 'rb')
            }
            response = requests.post(self.server, files=data)
            if response.status_code == 200:
                logging.info("Requests successful")
                logging.info('Response: {}'.format(response))
            else:
                logging.error('Requests unsuccessful')
                logging.info('Response: {}'.format(response.status_code))
        except requests.ConnectionError as error:
            logging.error("Connection error: {}".format(error))
        except requests.Timeout as error:
            logging.error("Timeout on server: {}".format(error))
        except OSError:
            logging.error("File couldn't be removed")

if __name__ == "__main__":
    api = Api('http://192.168.1.238:5000/motion')
    api.publish_data('/home/simon/sync/cam_images/16:02:2023-19:35:14.png')
