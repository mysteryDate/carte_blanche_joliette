#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
A monitor for a gmail inbox using the gmailAPI
"""

import pdb

# for creating xml files
from lxml import etree

# for reading from gmail
from apiclient.discovery import build
from oauth2client.client import flow_from_clientsecrets
from oauth2client.file import Storage
from oauth2client.tools import run
import base64
import httplib2

# For sending messages
from email.mime.text import MIMEText

# For getting phone numbers out of senders
import re

class Monitor():
    """
    A monitor for our gmail inbox
    """
    max_history_id = 0  # The most recent historical change
    database = {} # Some message storage
    filtered_label_ids = []
    filtered_label_names = []
    # These are sent along to max
    messages_to_add = []

    EMAIL_ADDRESS = "carte.blanche.joliette@gmail.com"
    RESPONSE_ADDRESS = "@desksms.appspotmail.com"
    RESPONSE = u"Merci pour ton message! Regarde bien, il apparaîtra sous peu sur la Grande Carte Blanche! :)"

    def __init__(self, match_label, filtered_labels=[], verbose=False, response=False, path=""):
        """
        match_label is a string for the label we look under (e.x. "SMS")
        filteredLabels is a list of strings of label names to be filtered
        verbose turns on error logging
        For now, we need a match label
        """
        self.response = response
        self.path = path
        # Path to the client_secret.json file downloaded from the Developer Console
        CLIENT_SECRET_FILE = path+'client_secret.json'

        # Check https://developers.google.com/gmail/api/auth/scopes for all available scopes
        OAUTH_SCOPE = 'https://www.googleapis.com/auth/gmail.modify'

        # Location of the credentials storage file
        STORAGE = Storage(path+'gmail.storage')

        # Start the OAuth flow to retrieve credentials
        flow = flow_from_clientsecrets(CLIENT_SECRET_FILE, scope=OAUTH_SCOPE)
        http = httplib2.Http()

        # Try to retrieve credentials from storage or run the flow to generate them
        credentials = STORAGE.get()
        if credentials is None or credentials.invalid:
            credentials = run(flow, STORAGE, http=http)

        # Authorize the httplib2.Http object with our credentials
        http = credentials.authorize(http)

        # Build the Gmail service from discovery
        self.service = build('gmail', 'v1', http=http)

        # Get in the necessary labels
        label_data = self.service.users().labels().list(userId='me').execute()
        if label_data.has_key('labels'):
            for label in label_data['labels']:
                if label['name'] == match_label:
                    self.match_label_id = label['id']
                    self.match_label_name = match_label
                if label['name'] in filtered_labels:
                    self.filtered_label_ids.append(label['id'])
                    self.filtered_label_names.append(label['name'])

        # Initialize max_history_id
        threads = self.service.users().threads().list(userId='me').execute()
        if threads['threads']:
           for thread in threads['threads']:
               if thread['historyId'] > self.max_history_id:
                   self.max_history_id = thread['historyId']

        if verbose:
            self._verbose = True
        else:
            self._verbose = False

    def load(self, file_path):
        """
        Load in an xml file containing messages
        """
        tree = etree.parse(file_path)
        for entry in tree.findall('MESSAGE'):
            new_message = Message()
            new_message.id = unicode(entry.find("ID").text)
            new_message.time = unicode(entry.find("TIME_RECEIVED").text)
            new_message.sender = unicode(entry.find("SENDER").text)
            new_message.last_displayed = unicode(entry.find("LAST_DISPLAYED").text)
            new_message.message = entry.find("MESSAGE_TEXT").text
            self.database[new_message.id] = new_message
        self.max_history_id = unicode(tree.find("MAX_HISTORY_ID").text)
        if self._verbose:
            print "Loaded", len(tree.findall('MESSAGE')), "messages"

    def save(self, file_path):
        """
        Save current database to disk
        """
        db = etree.Element("DATABASE")
        max_history_id = etree.SubElement(db, "MAX_HISTORY_ID")
        max_history_id.text = self.max_history_id
        for message in self.database.values():
            elem = etree.SubElement(db, "MESSAGE")
            id_number = etree.SubElement(elem, "ID")
            sender = etree.SubElement(elem, "SENDER")
            time = etree.SubElement(elem, "TIME_RECEIVED")
            last_displayed = etree.SubElement(elem, "LAST_DISPLAYED")
            message_text = etree.SubElement(elem, "MESSAGE_TEXT")
            id_number.text = message.id
            sender.text = message.sender
            time.text = message.time
            last_displayed.text = message.last_displayed
            message_text.text = message.message
        tree = etree.ElementTree(db)
        tree.write(file_path, encoding="UTF-8", pretty_print=True)
        if self._verbose:
            print "Saved", len(self.database), "messages"

    def get_message(self, message_id):
        """
        Returns full data for a particular message
        """
        try:
            return self.service.users().messages().get(userId='me', id=message_id).execute()
        except Exception as e: 
            if self._verbose: print(e)
            return False

    def update(self):
        """
        Checks for new/changed messages and updates the database
        Can throw errors for too many requests, but will not crash
        """
        try:
            self.recentThreads = self.service.users().history().list(
                userId='me', startHistoryId=self.max_history_id, labelId=self.match_label_id).execute()
            self.max_history_id = self.recentThreads['historyId']
            # pdb.set_trace()
        except Exception as e: 
            if self._verbose: print (e)
            return
        if self.recentThreads.has_key('history'): # Something has changed
            modified_messages = set()
            # Get unique ids for every changed message
            for thread in self.recentThreads['history']:
                if thread.has_key('messages'):
                    for message in thread['messages']:
                        modified_messages.add(message['id'])
            if self._verbose: print "Number of new messages:", len(modified_messages)
            for message_id in modified_messages:
                # Check to see if it's moved to a filtered folder
                if message_id in self.database:
                    status = "Unchanged"
                    report_string = str(message_id)+"\t| "+self.database[message_id].message+"\t|"
                    try:
                        messageData = self.service.users().messages().get(
                            userId='me', id=message_id, format='minimal').execute()
                        if any([_ for _ in messageData['labelIds'] if _ in self.filtered_label_ids]):
                            # It has been moved somewhere where we no longer want it
                            status = "Deleted"
                            del self.database[message_id]
                    except Exception as e:
                        if self._verbose: print (e)
                        # It was fully deleted
                        status = "Deleted"
                        del self.database[message_id]
                    if self._verbose:   print (report_string).encode('utf-8'), status
                # Then it must be new
                else:
                    if self.add_message_to_database(message_id):
                        self.messages_to_add.append(self.database[message_id])
                        
    def add_message_to_database(self, message_id):
        """
        Takes the id of an inbox message, requests it and adds it to the database
        If the message is fully deleted, it will throw a 404 error
        Returns true if the message was added, false otherwise
        """
        new_message_entry = Message()
        new_message_entry.id = message_id
        new_message_data = self.get_message(message_id)
        if not new_message_data:
            return False # Couln't get the message, it's probably deleted
        if any([_ for _ in new_message_data['labelIds'] if _ in self.filtered_label_ids]):
            if self._verbose: print "Message", message_id, "has a filtered label"
            return False # We don't want that message
        new_message_entry.active = True
        for entry in new_message_data['payload']['headers']:
            if entry['name'] == 'From':
                # Phone number with area code
                try:
                    new_message_entry.sender = re.findall('\d{10,11}', entry['value'])[0]
                except Exception as e: 
                    if _self.verbose: print(e)
                    return False
            if entry['name'] == 'Date':
                new_message_entry.time = entry['value']
                new_message_entry.last_displayed = new_message_entry.time
        text = new_message_data['payload']['body']['data']
        text = base64.urlsafe_b64decode(text.encode('UTF'))
        new_message_entry.message = text.strip('\r\n ').split('====')[0].rstrip(
            '\r\n ').replace(' \r\n', '').replace('\r\n', '')
        new_message_entry.message = unicode(new_message_entry.message, 'utf-8')
        self.database[message_id] = new_message_entry
        if self._verbose:
            print (str(message_id)+"\t| "+new_message_entry.message+"\t| Added").encode('utf-8')
        if self.response:
            self.respond(message_id, self.RESPONSE)
        return True

    def print_database(self):
        """
        Print the current database to the terminal
        """
        print "\tId\t\t| Number\t|\tTime\t\t\t\t|\tMessage\t"
        print "-"*150 #A horizontal line
        for messageId in self.database.keys():
            mess = self.database[messageId]
            print messageId, "\t|", mess.sender, "\t|", mess.time, "\t|", (mess.message).encode('utf-8')

    def respond(self, message_id, response_text):
        """
        Send a response to the sender of message_id
        """
        response = MIMEText(response_text.encode('utf-8'))
        response['to'] = self.database[message_id].sender + self.RESPONSE_ADDRESS
        response['from'] = self.EMAIL_ADDRESS
        response = {'raw': base64.b64encode(response.as_string())}
        try:
            self.service.users().messages().send(userId='me', body=response).execute()
        except Exception as e: 
            if _self.verbose: print(e)

    def populate(self):
        """
        Gets all relevant messages in the inbox, regardless of history
        This is an expensive call, use it rarely (if ever)
        """
        try:
            query = "-label:{"+(' ').join(self.filtered_label_names)+"}"
            messages = self.service.users().messages().list(
                userId='me', labelIds=self.match_label_id, q=query).execute()
            if messages['messages']:
                for message in messages['messages']:
                    self.add_message_to_database(message['id'])
        except Exception as e:
            if self._verbose: print(e)

class Message():
    """
    A text message received into the mailbox
    """
    def __init__(self):
        self.id = "" # The gmail id of the message, will also be its key in the database
        self.sender = "" # The phone number of the sender
        self.time = "" # Gmail reports in GM time, so I don't want to do time.ctime()
        self.message = ""
        self.last_displayed = "" # The most recent time we displayed the message      