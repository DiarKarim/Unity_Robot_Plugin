import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5005

k = 0

print ("UDP target IP:", UDP_IP)
print ("UDP target port:", UDP_PORT)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP



while True:

	message = str(0.0 + k/100000.0).encode('ASCII') + ",".encode('ASCII') + str(0.0).encode('ASCII') + ",".encode('ASCII') + str(0.5).encode('ASCII')
	
	try: 
		sock.sendto(message, (UDP_IP, UDP_PORT))
		print(message)
	except Exception as e:
		print(e)

	k = k+1