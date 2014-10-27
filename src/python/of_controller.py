#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
A liason between the gmonitor and oF slideshow
"""

import pdb
import sys
PATH = sys.argv[-1]

import gmonitor
import socket
import time

import threading

import calendar

UDP_IP = "127.0.0.1"
UDP_PORT = 7011
# UDP_PORT = Python -> oF text messags
# UDP_PORT + 1 = oF -> Python
# UDP_PORT + 2 = Python -> oF control messages

MATCH_LABEL = "SMS"
FILTERED_LABELS = ["Refuser", "RefuserAutomatique", "TRASH"]

gmail = gmonitor.Monitor(MATCH_LABEL, FILTERED_LABELS, verbose=True, response=True, path=PATH)
gmail.load(PATH+"message_database.xml")
NO_REPEAT_LENGTH = 5
EXCLUDED_MESSAGES = []
newest_id = ""

def GetNextMessage(excluded_messages):
    """
    A helper function to find the best message to next display
    """
    max_ratio = 0
    best_id = gmail.database.keys()[0]
    for message in gmail.database.values():
        if message in excluded_messages:
            continue
        sent_time = time.strptime(message.time, "%a, %d %b %Y %H:%M:%S +0000")
        time_since_sent = calendar.timegm(time.gmtime()) - calendar.timegm(sent_time)
        displayed_time = time.strptime(message.last_displayed, "%a, %d %b %Y %H:%M:%S +0000")
        time_since_displayed = calendar.timegm(time.gmtime()) - calendar.timegm(displayed_time)
        ratio = float(time_since_displayed) / time_since_sent
        if ratio > max_ratio:
            max_ratio = ratio
            best_id = message.id
    return gmail.database[best_id]

while True:

    gmail.update()
    input_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    input_sock.bind((UDP_IP, UDP_PORT+1))
    # It will hang on this line until oF sends a message
    # Which is unexpectedly brilliant
    data, addr = input_sock.recvfrom(1024)

    if data == "new message": #oF wants a message to display
        newest = False
        if len(gmail.database) == 0:
            continue
        if len(gmail.messages_to_add) > 0:
            message_object = gmail.messages_to_add.pop(0)
            newest_id = message_object.id
        else:
            message_object = GetNextMessage(EXCLUDED_MESSAGES)

        phone = message_object.sender
        # Add dashes to phone number
        phone = ('-').join([phone[-7:-4], phone[-4:]]) 
        output = phone + ": " + message_object.message
        if message_object.id == newest_id:
            output = "NEWEST" + output

        output_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        output_sock.sendto(output.encode('utf-8'), (UDP_IP, UDP_PORT))

        output_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  
        # Keep things in UTC
        message_object.last_displayed = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime())
        EXCLUDED_MESSAGES.append(message_object)
        if len(EXCLUDED_MESSAGES) > NO_REPEAT_LENGTH:
            EXCLUDED_MESSAGES.pop(0)

    if data == "save":
        gmail.save(PATH+"message_database.xml")

    if data == "load":
        gmail.load(PATH+"message_database.xml")

    if data == "shutdown":
        gmail.save(PATH+"message_database.xml")
        print "goodbye"
        break

    # time.sleep(2)


# Get time in RFC 2822 Internet email standard:
# time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime())

# get stored time as struct
# time.strptime(<MESSAGE_OBJECT>.time, "%a, %d %b %Y %H:%M:%S +0000")

# get time differences in seconds between TIME1 and TIME2
# calendar.timegm(<TIME1>) - calendar.timegm(<TIME2>)
