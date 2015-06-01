

import threading
import serial
import bluetooth
import string
import socket
import time
import re
import random as random
import Queue

bt_mac = '00:06:66:6C:60:E1'
bt_port = 1


class GameSocket(threading.Thread):
    def __init__(self):
        self.robot_re = r'A5B6_(\d+)'
        super(GameSocket, self).__init__()
        self.g_qt = Queue.Queue(maxsize=1)
        self.alive = threading.Event()
        self.alive.set()
        self.start()

    def _teardown(self):
        try:
            self.bt_sock.shutdown(2)
            self.bt_sock.recv(128, socket.MSG_DONTWAIT)
            self.bt_sock.close()
        except Exception:
            pass

    def run(self):
        data = ''
        self.connect()
        while self.alive.isSet():
            try:
                item = self.bt_sock.recv(128)
                if item is not None:
                    data += item
                    print item
                if re.search(self.robot_re, data):
                    match = re.search(self.robot_re, data)
                    data = ''
                    try:
                        self.g_qt.put(int(match.group(1)), block=True)
                    except Queue.Full:
                        time.sleep(1)
            except bluetooth.btcommon.BluetoothError as error:
                if 'timed out' in error:
                    continue
                print "Caught BluetoothError:", error
                self._teardown()
                time.sleep(3)
                self.connect()
            except Exception:
                self.alive.clear()
        self._teardown()
        #self.g_qt.join()

    def connect(self):
        while(True):
            try:
                self.bt_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
                self.bt_sock.connect((bt_mac, bt_port))
                self.bt_sock.setblocking(True)
                self.bt_sock.settimeout(0.5)
                break;
            except bluetooth.btcommon.BluetoothError as error:
                self._teardown()
                print "Could not connect: ", error, "; Retrying in 3s..."
                time.sleep(3)

    def _send_data(self, column):
        send_data = 'gameplay debug {}'.format(column)
        unsent = True
        while unsent:
            try:
                self.bt_sock.sendall(send_data + '\r\n')
                print send_data
                unsent = False
            except bluetooth.btcommon.BluetoothError as error:
                time.sleep(0.125)
            except AttributeError:
                time.sleep(0.125)

    def get_move(self):
        print 'block receive'
        move = self.g_qt.get(block=True)
        print 'Move:', move
        self.g_qt.task_done()
        return int(move)

