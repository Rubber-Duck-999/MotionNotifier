'''
Created on 2020
@author: Rubber-Duck-999
'''

#!/usr/bin/env python
import pika, sys, time


print("## CM Integrator Start up")
credentials = pika.PlainCredentials('guest', 'password')
connection = pika.BlockingConnection(pika.ConnectionParameters('localhost', 5672, '/', credentials))
channel = connection.channel()
#
channel.exchange_declare(exchange='topics', exchange_type='topic', durable=True)
#
result = channel.queue_declare(queue='', exclusive=False, durable=True)
queue_name = result.method.queue
#

## Topics
motion_response   = 'Motion.Response'
failure_camera    = 'Failure.Camera'
failure_component = 'Failure.Component'

channel.queue_bind(exchange='topics', queue=queue_name, routing_key=motion_response)
channel.queue_bind(exchange='topics', queue=queue_name, routing_key=failure_camera)
channel.queue_bind(exchange='topics', queue=queue_name, routing_key=failure_component)

print("Beginning Subscribe")
print("Waiting for notifications")

def callback(ch, method, properties, body):
    print("Received: " + method.routing_key)
    string = body.decode()
    print("CMIntegrator: I think we received a message: " + string)
    

channel.basic_consume(queue=queue_name, on_message_callback=callback, auto_ack=True)
channel.start_consuming()