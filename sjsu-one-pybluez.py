import serial
import socket
import string
import time
import bluetooth

bt_mac = '00:06:66:6C:60:E1'
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

def _teardown():
    bt_sock.shutdown(2)
    try:
        bt_sock.recv(1024, socket.MSG_DONTWAIT)
    except:
        pass
    bt_sock.close()

while(True):
    try:
        send_data = raw_input('sjsu-one$ ')
        bt_sock.sendall(send_data + '\r\n')
        print bt_sock.recv(1024)
        print bt_sock.recv(1024)
        print bt_sock.recv(1024)
    except KeyboardInterrupt:
        _teardown()
        break
    except bluetooth.btcommon.BluetoothError as error:
        print "Caught BluetoothError: ", error
        time.sleep(3)
        bt_sock = connect()
    except Exception:
        _teardown()
        break
