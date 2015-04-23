import serial
import string
import time
import bluetooth

bt_mac = 'ur bt device mac address'
bt_port = 1

def connect():
    while(True):    
        try:
            bt_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
            bt_sock.connect((bt_mac, bt_port))
            break;
        except bluetooth.btcommon.BluetoothError as error:
            bt_sock.close()
            print "Could not connect: ", error, "; Retrying in 3s..."
            time.sleep(2)
    return bt_sock;
bt_sock = connect()
while(True):
    try:
        send_data = raw_input('sjsu-one$ ')
        bt_sock.sendall(send_data + '\r\n')
        print bt_sock.recv(128)
        print bt_sock.recv(128)
        print bt_sock.recv(128)
    except bluetooth.btcommon.BluetoothError as error:
        print "Caught BluetoothError: ", error
        time.sleep(3)
        bt_sock = connect()
        pass
bt_sock.close()