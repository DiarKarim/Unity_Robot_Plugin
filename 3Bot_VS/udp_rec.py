import socket
import sys 

UDP_IP = "127.0.0.1"
UDP_PORT = 8899

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

while True:
    
    try:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print "received message:", data
    except KeyboardInterrupt:  
        print 'User stopped program'
        sys.exit()

