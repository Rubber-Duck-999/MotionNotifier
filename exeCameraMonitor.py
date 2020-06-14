#!/usr/bin/env python3
# USAGE
# python exeCameraMonitor.py -c conf.json

# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import argparse, warnings
from PIL import Image
from datetime import datetime
import imutils, json, time, cv2, logging
import pika, sys, os

# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-c", "--conf", required=True,
	help="JSON configuration file")
args = vars(ap.parse_args())

try:
	os.remove('logs/cameramonitor.log')
except:
	print("The log did not exist")

logging.basicConfig(filename='logs/cameramonitor.log',filemode='w', format='%(levelname)s - %(message)s', level=logging.INFO)
# filter warnings, load the configuration
warnings.filterwarnings("ignore")
try:
	conf = json.load(open(args["conf"]))
except IOError:
	logging.error("File was not able to be loaded")
	exit(1)

# Setup rabbitmq connection
logging.info("[INFO] Initialise pika connection")
try:
	credentials = pika.PlainCredentials('guest', conf["password"])
	connection = pika.BlockingConnection(pika.ConnectionParameters('localhost', 5672, '/', credentials))
except pika.exceptions.ProbableAuthenticationError:
	logging.error("The rabbitmq credentials are incorrect")
	exit(1)

try:
	logging.info("Starting up channel")
	channel = connection.channel()
	channel.exchange_declare(exchange='topics', exchange_type='topic', durable=True)
except pika.exceptions.ChannelError:
	logging.error("Channel failed to connect, exiting")
	exit(1)

## Topics
motion_response   = 'Motion.Response'
failure_camera    = 'Failure.Camera'
failure_component = 'Failure.Component'
ping = 'Camera.Ping'
result = channel.queue_declare(queue='', exclusive=False, durable=True)
queue_name = result.method.queue

# initialize the camera and grab a reference to the raw camera capture
try:
	logging.info("Starting up camera")
	camera = PiCamera()
	camera.resolution = tuple(conf["resolution"])
	camera.framerate = conf["fps"]
	rawCapture = PiRGBArray(camera, size=tuple(conf["resolution"]))
except:
	logging.error("Camera failure - publish topic")
	today = datetime.now()
	x = { 	
		"time": today.strftime("%d:%m:%Y %H:%M:%S"),
		"severity": 4
	}
	try:
		motion = json.dumps(x)
		channel.basic_publish(exchange='topics', routing_key=motion_response, body=motion)
	except:
		logging.error("Failed to publish")

# allow the camera to warmup, then initialize the average frame, last
# uploaded timestamp, and frame motion counter
time.sleep(conf["camera_warmup_time"])

# capture frames from the camera
def motion():
	avg = None
	lastUploaded = datetime.now()
	motionCounter = 0
	for f in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
		# grab the raw NumPy array representing the image and initialize
		# the timestamp and occupied/unoccupied text
		frame = f.array
		timestamp = datetime.now()
		text = "Unoccupied"

		# resize the frame, convert it to grayscale, and blur it
		frame = imutils.resize(frame, width=500)
		gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
		gray = cv2.GaussianBlur(gray, (21, 21), 0)

		# if the average frame is None, initialize it
		if avg is None:
			logging.info("Starting background model...")
			avg = gray.copy().astype("float")
			rawCapture.truncate(0)
			continue

		# accumulate the weighted average between the current frame and
		# previous frames, then compute the difference between the current
		# frame and running average
		cv2.accumulateWeighted(gray, avg, 0.5)
		frameDelta = cv2.absdiff(gray, cv2.convertScaleAbs(avg))

		# threshold the delta image, dilate the thresholded image to fill
		# in holes, then find contours on thresholded image
		thresh = cv2.threshold(frameDelta, conf["delta_thresh"], 255,
			cv2.THRESH_BINARY)[1]
		thresh = cv2.dilate(thresh, None, iterations=2)
		cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
			cv2.CHAIN_APPROX_SIMPLE)
		cnts = imutils.grab_contours(cnts)

		# loop over the contours
		for c in cnts:
			# if the contour is too small, ignore it
			if cv2.contourArea(c) < conf["min_area"]:
				continue

			# compute the bounding box for the contour, draw it on the frame,
			# and update the text
			(x, y, w, h) = cv2.boundingRect(c)
			cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
			text = "Occupied"

		# check to see if the room is occupied
		if text == "Occupied":
			# check to see if enough time has passed between uploads
			if (timestamp - lastUploaded).seconds >= conf["min_image_seconds"]:
				# increment the motion counter
				motionCounter += 1

				# check to see if the number of frames with consistent motion is
				# high enough
				if motionCounter >= conf["min_motion_frames"]:
					# check to see if we should take pictures
					image = None
					logging.warning("Motion detected")
					today = datetime.now()
					if conf["use_images"]:
						image = today.strftime("%d:%m:%Y-%H:%M:%S") + ".jpg"
						logging.warning("Creating file: " + image)
						cv2.imwrite(image, frame)
						colorImage  = Image.open(image)
						transposed  = colorImage.rotate(180)
						transposed.save(image)
					logging.warning("Image created")
					# Publish to Rabbitmq
					x = {
						"file": image,
						"time": today.strftime("%d:%m:%Y-%H:%M:%S"),
						"severity": 4
					}
					motion = json.dumps(x)
					channel.basic_publish(exchange='topics', routing_key=motion_response, body=motion)
					time.sleep(20)

					# update the last uploaded timestamp and reset the motion
					# counter
					lastUploaded = timestamp
					motionCounter = 0

		# otherwise, the room is not occupied
		else:
			motionCounter = 0

		# clear the stream in preparation for the next frame
		rawCapture.truncate(0)

		if datetime.now().second == 0:
			x = 'Alive'
			channel.basic_publish(exchange='topics', routing_key=ping, body=x)

motion()
