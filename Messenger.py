import queue, pika, threading

password = 'password'
exchange_name = 'topics'

class PikaMassenger():

    exchange_name = 'topics'

    def __init__(self, *args, **kwargs):
        try:
            credentials = pika.PlainCredentials('guest', password)
            self.conn = pika.BlockingConnection(pika.ConnectionParameters('localhost', 5672, '/', credentials))
        except pika.exceptions.ProbableAuthenticationError:
            exit(1)
        self.channel = self.conn.channel()
        self.channel.exchange_declare(
            exchange=self.exchange_name, 
            exchange_type='topic',
             durable=True)

    def consume(self, callback):
        result = self.channel.queue_declare('', exclusive=False, durable=True)
        queue_name = result.method.queue
        self.channel.queue_bind(exchange=exchange_name, queue=queue_name, routing_key='Camera.*')

        self.channel.basic_consume(
            queue=queue_name, 
            on_message_callback=callback, 
            auto_ack=True)

        self.channel.start_consuming()


    def __enter__(self):
        return self


    def __exit__(self, exc_type, exc_value, traceback):
        self.conn.close()

def start_consumer(num, q):

    def callback(ch, method, properties, body):
        if method.routing_key == "Camera.Start":
            q.put("Start")
        elif method.routing_key == "Camera.Stop":
            q.put("Stop")
    
    with PikaMassenger() as consumer:
        consumer.consume(callback=callback)