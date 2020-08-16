#!/usr/bin/python3
# MQTT email forwarder.
# J.Christensen 19Jun2020

__author__ = 'Jack Christensen'
__version__ = '1.0.0'
__versionDate__ = '19Jun2020'

import argparse
import configparser
import csv
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import logging
import logging.handlers
import os
import paho.mqtt.client as mqtt
import signal
import smtplib
import socket
import sys
import time

# program path and name for documentation and file naming purposes
progpath = os.path.dirname(os.path.realpath(__file__))
prognamepy = os.path.basename(__file__)     # name.py
progname = prognamepy.split(sep='.')[0]     # name only

# constants
VERSION_INFO = (prognamepy + ' v' + __version__ + ' by '
    + __author__ + ' ' + __versionDate__)
LOG_FILENAME = os.environ['HOME'] + os.sep + '.' + progname + '.log'

# configuration file parameters
EMAIL_SERVER = None
EMAIL_PORT = None
EMAIL_SENDER = None
EMAIL_AUTH = None
EMAIL_FROMADDR = None
EMAIL_REPLYTO = None

# signal handler for SIGINT
def sigint_handler(signal, frame):
    myLog.info('Received SIGINT, exiting.')
    sys.exit(0)

# signal handler for SIGTERM
def sigterm_handler(signal, frame):
    myLog.info('Received SIGTERM, exiting.')
    sys.exit(0)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(mqClient, userdata, flags, rc):
    global TOPIC
    myLog.info('Connected with result code ' + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    mqClient.subscribe(TOPIC)

# The callback for when a PUBLISH message is received from the server.
def on_message(mqClient, userdata, msg):
    msgText = msg.payload.decode('utf-8')
    myLog.debug('Received [' + msg.topic + '] ' + msgText)
    reader = csv.reader([msgText], quotechar='"')
    for row in reader:
        recipient = row[0]
        subject = row[1]
        body = row[2]
        sendEmail(subject, recipient, None, None, body)

def main() -> None:
    global EMAIL_SERVER, EMAIL_PORT, EMAIL_SENDER, EMAIL_AUTH, \
        EMAIL_FROMADDR, EMAIL_REPLYTO
    global TOPIC

    # register signal handlers
    signal.signal(signal.SIGINT, sigint_handler)
    signal.signal(signal.SIGTERM, sigterm_handler)

    # process command line arguments
    parser = argparse.ArgumentParser(
        description='MQTT email forwarder.',
        epilog='Connects to the given broker and subscribes to the'
        ' given topic. Sends messages published to the given topic as'
        ' email. Messages should be'
        ' in CSV format; fields that contain commas should be'
        ' quote-enclosed: "recipient","subject","body text".'
        ' This program runs until terminated with SIGTERM or SIGINT.')
    parser.add_argument('-b', '--broker', default='localhost', type=str, help='hostname of the mqtt broker (optional, defaults to localhost)')
    parser.add_argument('-t', '--topic', required=True, type=str, help='mqtt topic to subscribe to (required)')
    args = parser.parse_args()

    # set up logging
    global myLog
    myLog = logging.getLogger(prognamepy)
    myLog.setLevel(logging.DEBUG)
    handler = logging.handlers.RotatingFileHandler(
                LOG_FILENAME, maxBytes=1024*32, backupCount=3)
    myLog.addHandler(handler)
    f = logging.Formatter(
            fmt='%(asctime)s\t%(levelname)s\t%(name)s\t%(message)s',
            datefmt='%Y-%m-%d %H:%M:%S')
    handler.setFormatter(f)

    myLog.info('Start ' + VERSION_INFO)
    myLog.info('Broker: ' + args.broker + ', Topic: ' + args.topic \
        + ', PID: ' + str(os.getpid()))
    TOPIC = args.topic

    # get parameters from config file
    getParams()

    # now for the real work
    mqClient = mqtt.Client(client_id=prognamepy, clean_session=True)
    mqClient.on_connect = on_connect
    mqClient.on_message = on_message

    # try to connect to the broker
    retryInterval = 10
    nTry = 0
    connected = False
    while (not connected):
        try:
            nTry += 1
            mqClient.connect(args.broker)
            connected = True
        except Exception as e:
            logMsg = 'Connect to broker failed: ' + str(e) \
                + ', Retry in ' + str(retryInterval) + ' seconds.'
            myLog.error(logMsg)
            time.sleep(retryInterval)
            if (nTry == 36):
                retryInterval = 3600
            elif (nTry == 24):
                retryInterval = 300
            elif (nTry == 12):
                retryInterval = 60

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    mqClient.loop_forever()

def getParams() -> None:
    """read parameters from configuration file."""
    global EMAIL_SERVER, EMAIL_PORT, EMAIL_SENDER, EMAIL_AUTH, EMAIL_FROMADDR, EMAIL_REPLYTO
    try:
        config = configparser.ConfigParser()
        config.read(progpath + os.sep + progname + '.conf')
    except Exception as e:
        myLog.error('Cannot read config file: ' + str(e))
        sys.exit(1)

    # get required parameters
    try:
        EMAIL_SERVER = config['MQTTMAIL']['server']
        EMAIL_PORT = int(config['MQTTMAIL']['port'])
        EMAIL_SENDER = config['MQTTMAIL']['sender']
    except Exception as e:
        myLog.error('Configuration parameter error: ' + str(e))
        sys.exit(2)

    # get optional parameters
    if 'OPTIONS' in config:
        if 'auth' in config['OPTIONS']:
            EMAIL_AUTH = config['OPTIONS']['auth']
        if 'display_name' in config['OPTIONS']:
            EMAIL_FROMADDR = config['OPTIONS']['display_name']
        if 'reply_to' in config['OPTIONS']:
            EMAIL_REPLYTO = config['OPTIONS']['reply_to']

    # if auth not supplied in config file, try environment variable
    if not EMAIL_AUTH:
        try:
            EMAIL_AUTH = os.environ['PYMAIL_AUTH']
        except Exception as e:
            myLog.error('Missing auth parameter: ' + str(e))
            sys.exit(3)

def sendEmail(subject: str, to: str, cc: str, bcc: str, bodytext: str) -> None:
    """Send an email to one or more recipients."""

    # message headers
    recipients = []
    msg = MIMEMultipart()
    if to:
        msg['To'] = to
        recipients.append(to)
    if cc:
        msg['Cc'] = cc
        recipients.append(cc)
    if bcc:
        msg['Bcc'] = bcc
        recipients.append(bcc)
    if subject:
        msg['Subject'] = subject
    if EMAIL_FROMADDR:
        msg['From'] = EMAIL_FROMADDR
    else:
        msg['From'] = EMAIL_SENDER
    if EMAIL_REPLYTO:
        msg['Reply-To'] = EMAIL_REPLYTO
    msg.attach(MIMEText(bodytext, 'plain')) # message body

    # send the email
    try:
        with smtplib.SMTP(socket.gethostbyname(EMAIL_SERVER), EMAIL_PORT) as server:
            server.ehlo()
            server.starttls()
            server.ehlo()
            server.login(EMAIL_SENDER, EMAIL_AUTH)
            text = msg.as_string()
            server.send_message(msg)
            server.close()
        myLog.info('Email sent to ' + ','.join(recipients))
    except Exception as e:
        myLog.error('Error sending email: ' + str(e))
        sys.exit(4)

if __name__ == '__main__':
    main()
