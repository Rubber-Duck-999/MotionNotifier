from picamera.array import PiRGBArray
from picamera import PiCamera
from PIL import Image
import cv2
import imutils
import logging
import datetime
from api import Api
import time


class ImageTaken(Exception):
    '''Exception to get out of capture'''

class Camera:
    def __init__(self, server):
        self.raw_capture = None
        self.camera = None
        self.motion_counter = 0
        self.motion = False
        self.timestamp = None
        self.last_uploaded = None
        self.api = Api(server)

    def get_base_image(self):
        # initialize the camera and grab a reference to the raw camera capture
        logging.info('get_base_image()')
        try:
            logging.info("Starting up camera")
            self.camera = PiCamera()
            self.camera.resolution = (640, 480)
            self.camera.framerate = 6
            self.raw_capture = PiRGBArray(self.camera, size=(640, 480))
            time.sleep(10)
            self.last_uploaded = datetime.datetime.now()
        except:
            logging.error("Camera failure")

    def check_motion(self, frame):
        '''Calculates whether a motion has occurred'''
        # check to see if the room is occupied
        if self.motion:
            # check to see if enough time has passed between uploads
            if (self.timestamp - self.last_uploaded).total_seconds() >= 10.0:
                # increment the motion counter
                self.motion_counter = self.motion_counter + 1
                # check to see if the number of frames with consistent motion is
                # high enough
                if self.motion_counter >= 5:
                    # check to see if we should take pictures
                    logging.info("Motion detected")
                    current = self.timestamp.strftime("%d:%m:%Y-%H:%M:%S")
                    filename = "{}/{}.jpg".format('/home/pi/sync/cam_images', current)
                    logging.info("Creating file: {}".format(filename))
                    cv2.imwrite(filename, frame)
                    colorImage  = Image.open(filename)
                    transposed  = colorImage.rotate(180)
                    transposed.save(filename)
                    logging.info("Image created")
                    # Call other object to send image
                    self.api.publish_data(current)
                    self.last_uploaded = self.timestamp
                    self.motion_counter = 0
        # otherwise, the room is not occupied
        else:
            self.motion_counter = 0

    def check_motion_capture(self):
        '''Calculates whether a human or colour shade has drastically
        changed the raw image'''
        logging.info('check_motion_capture()')
        average_frame = None
        for capture in self.camera.capture_continuous(self.raw_capture, format="bgr", use_video_port=True):
            # resize the frame, convert it to grayscale, and blur it
            self.timestamp = datetime.datetime.now()
            frame = capture.array
            frame = imutils.resize(frame, width=500)
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            gray = cv2.GaussianBlur(gray, (21, 21), 0)
            self.motion = False
            
            # if the average frame is None, initialize it
            if average_frame is None:
                logging.info("Starting background model")
                average_frame = gray.copy().astype("float")
                self.raw_capture.truncate(0)
                continue
            
            # accumulate the weighted average between the current frame and
            # previous frames, then compute the difference between the current
            # frame and running average
            cv2.accumulateWeighted(gray, average_frame, 0.5)
            frameDelta = cv2.absdiff(gray, cv2.convertScaleAbs(average_frame))

            # threshold the delta image, dilate the thresholded image to fill
            # in holes, then find contours on thresholded image
            thresh = cv2.threshold(frameDelta, 5, 255, cv2.THRESH_BINARY)[1]
            thresh = cv2.dilate(thresh, None, iterations=2)
            contours = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            contours = imutils.grab_contours(contours)
            
            # loop over the contours
            for c in contours:
                # if the contour is too small, return to start
                if cv2.contourArea(c) < 5000:
                    continue
                
                # compute the bounding box for the contour, draw it on the frame,
                # and update the text
                (x, y, w, h) = cv2.boundingRect(c)
                cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
                self.motion = True
            self.check_motion(frame)
            self.raw_capture.truncate(0)

    def run_capture(self):
        '''Setup for running capture'''
        logging.info('run_capture()')
        while True:
            try:
                self.get_base_image()
                self.check_motion_capture()
            except Exception as error:
                logging.error('Error found on camera capture: {}'.format(error))
                self.camera.close()
