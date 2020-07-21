#------------------------------------------------------------------------------------------
# Class to bind a socket for a TCP server
# Support for
# - Callback on reception (binary or line oriented)
# - Method for transmission
# - Number of accepted clients configurable
# - Method for socket close handing
#
# Attention
# - TcpServer creates threads which need to be stopped before program ends.
#   Use closeSocket() before
# - The dummy parameter _ in callback functions is for symetrie with TcpServer
#------------------------------------------------------------------------------------------
# Instantiation
#   class TcpClient()
#     host                          Name of server ("" for local host)
#     port                          Number of TCP port
# - - - - -
#     tryConnect=NOTRY              Imediately try to connect
#                                       -2=BACKGROUND=endless try in background
#                                       -1=FOREGROUND=endless try in foreground (blocks the call)
#                                        0=NOTRY=no try
#                                        >=1 number of tries
#     tryGap=2                      Gap in seconds between two tries
#     name="client"                 Instance name when printing debug messages
# - - - - -
#     verboseColour=-1              Color code (VT100) for socket operation verbose messages (-1 for off)
#     verboseRecv=0                 Print all received data
#     verboseSend=0                 Print all send data
#     verboseSkipBlankSend=1        Do not print blank lines on verbose channel
# - - - - -
#     callbackOpen(_, host, port, param)=None Function called when conection established
#     callbackRecv(_, data, param)=None       Function called on reception (data as received)
#     callbackClose(_, name, param)=None      Function called when port closes
#     callbackSendTimeout(name)=None          Function called when timeout detected on send
#     callbackArgs=[]                         Parameter 'param' given to other callbacks
# - - - - -
#     splitLines=1                            0: Data as received, 1=line by line (no \n)
#     lineSkipEmpty=0                         Empty lines are not provided (only when splitLines=1)
#     lineSuppressOlder=0                     Only the newest complete line provided (only when splitLines=1)
#     ignoreSendWhenDisconnected=0            Send does not throw an error when there is no connection
#------------------------------------------------------------------------------------------
# Member functions
#     connect(tries=1)                        Triggers N connection tries (see init.tryConnect)
#     connected(_)                            Return True is socket has been connected by client
#     send(data, timeout=0)                   Send data on socket. timeout=0 means no timeout
#     closeConnection()                       Triggers close of client connection
#     closeSocket()                           Closes all client connections and deregister the socket
#     disableReconnect()                      Prevent automatic reconnection (when BACKGROUND was given on connect)
#------------------------------------------------------------------------------------------
# There is an example/module test at the end of this file
#------------------------------------------------------------------------------------------
from time import sleep
import socket
import threading

#------------------------------------------------------------------------------------------
#Define enum for the connection state
DISCONNECTED, SHUTDOWN, CONNECTED, CONNECTING, ENDCONNECTING = range(0,5)
BACKGROUND, FOREGROUND, NOTRY = [-2, -1, 0]


#------------------------------------------------------------------------------------------
class TcpClient:
	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def __init__(self, host, port, tryConnect=NOTRY,  tryGap=2,
					   name="client",
					   verboseColour=-1, verboseRecv=0, verboseSend=0, verboseSkipBlankSend=1,
					   callbackOpen=None, callbackRecv=None, callbackClose=None, callbackSendTimeout=None,
					   callbackArgs=[],
					   splitLines=1, lineSkipEmpty=0, lineSuppressOlder=0,
					   ignoreSendWhenDisconnected=0
				):
		self.host = host
		self.port = port
		self.tryGap = tryGap
		self.name = name

		self.verboseColour = verboseColour
		self.verboseRecv = verboseRecv
		self.verboseSend = verboseSend
		self.verboseSkipBlankSend = verboseSkipBlankSend

		self.callbackOpen = callbackOpen
		self.callbackRecv = callbackRecv
		self.callbackClose = callbackClose
		self.callbackSendTimeout = callbackSendTimeout
		self.callbackArgs = callbackArgs

		self.splitLines = splitLines
		self.lineSkipEmpty = lineSkipEmpty
		self.lineSuppressOlder = lineSuppressOlder
		self.ignoreSendWhenDisconnected = ignoreSendWhenDisconnected

		#internal data structures
		self.isConnected = DISCONNECTED
		self.backgroundConnect = (tryConnect == BACKGROUND)
		self.socket = None
		self.threadListen = None
		if (host==""):
			self.hostPrintName = "localhost"
		else:
			self.hostPrintName = host
		#for the timeout function
		self.timeoutID = 0
		self.pendingSend = {}
		self.lock = threading.Lock()
		self.closeLock = threading.Lock()

		#and connect if requested
		if (tryConnect != NOTRY):
			self.connect(tryConnect)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def connect(self, tries=1, _byTimer=False):
		#this is rare case when close/connect comes close together. Server need time to answer
		if (self.isConnected == SHUTDOWN):
			sleep(0.5)

		if (self.isConnected != DISCONNECTED and not _byTimer):
			raise ValueError ("Connection \"%s\" is already establised %d" %(self.name, self.isConnected))
		if (tries < BACKGROUND):
			raise ValueError ("Connection \"%s\" has tries=%d shall range (-2, -1, 0, ...)" %(self.name, tries))
		self.backgroundConnect = (tries == BACKGROUND)

		#Create a socket for the client
		if (self.socket == None):
			try:
				if (self.verboseColour!=-1):
					print("\033[%dm[%s] Create TCP client socket\033[m" %(30+self.verboseColour, self.name))
				self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
				self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
			except Exception as e:
				if (self.verboseColour!=-1):
					print("\033[%d;%d;2m[%s] Socket setup failed\033[m (%s)"
						  %(40+self.verboseColour, 31, self.name, e))
				self.socket = None
				raise   #and pass to the caller

		#Try to connect
		while (True):
			try:
				if (self.verboseColour!=-1):
					print("\033[%dm[%s] Try to connect to server %s:%d  #%d\033[m"
						  %(30+self.verboseColour, self.name, self.hostPrintName, self.port, tries))
				self.socket.connect((self.host, self.port))
				break
			except OSError:
				if (tries > 0):
					tries -= 1
				if (tries == NOTRY):
					if (self.verboseColour!=-1):
						print("\033[%d;%d;2m[%s] Connection to server failed\033[m"
							  %(40+self.verboseColour, 31, self.name))
					return False
				elif (tries == BACKGROUND):
					if (not _byTimer):
						self.isConnected = CONNECTING
					if (self.isConnected == CONNECTING):
						threading.Timer(self.tryGap, self.connect, args=(BACKGROUND,True)).start();
					else:
						self.isConnected = DISCONNECTED
					return False
				else:
					sleep(self.tryGap)

		if (self.verboseColour!=-1):
			print("\033[%dm[%s] Connected to server %s:%d\033[m"
				  %(30+self.verboseColour, self.name, self.hostPrintName, self.port))

		self.isConnected = CONNECTED
		if (self.callbackOpen):
				self.callbackOpen(0, self.host, self.port, self.callbackArgs)

		self.threadListen = threading.Thread(target=self._listening, name="TcpClient._listening{}:{}".format(self.hostPrintName, self.port))
		self.threadListen.start()
		return True


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def _listening(self):
		IncompleteLine = ""
		while True:
			try:
				Data = self.socket.recv(2048).decode()
			except OSError:
				Data = '';
			if(Data == ''):
				break

			#process the data
			if (self.verboseRecv):
				print("\033[%dm[%s] recv %s\033[m" %(30+self.verboseColour, self.name, repr(Data)))

			lock = False  #only to minimise locking when timeout not used
			for k, v in self.pendingSend.items():
				if (v):
					if (not lock):
						self.lock.acquire()
						lock = True
					self.pendingSend[k] = False
			if (lock):
				self.lock.release()

			if (self.callbackRecv != None):
				if (not self.splitLines):
					self.callbackRecv(0, Data, self.callbackArgs)
				else:
					Data = IncompleteLine + Data
					Split = Data.split("\n")
					#When read data does not end with \n we have a fragment of next read
					if (Data[-1:] != "\n"):
						IncompleteLine = Split[-1]
					else:
						IncompleteLine = ""
					Split.pop()

					#remove all lines which without content
					if (self.lineSkipEmpty):
						i = 0
						while (i < len(Split)):
							if (Split[i]==""):
								Split[i:] = Split[i+1:]
							else:
								i += 1

					#we take the last complete line for processing
					if (self.lineSuppressOlder):
						self.callbackRecv(0, Split[-1], self.callbackArgs)
					#we provide all lines
					else:
						for s in Split:
							self.callbackRecv(0, s, self.callbackArgs)

		#and clean up this connection
		with self.closeLock:
			if (self.verboseColour!=-1):
				print("\033[%dm[%s] Connection closed by server\033[m" %(30+self.verboseColour, self.name))
			self.socket.close()
			self.socket = None
			self.isConnected = DISCONNECTED
			if (self.callbackClose):
				self.callbackClose(0, self.name, self.callbackArgs)
			if (self.backgroundConnect):
				self.connect(BACKGROUND)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def send(self, data, timeout=0):
		if (self.isConnected != CONNECTED):
			if (not self.ignoreSendWhenDisconnected):
				raise ValueError ("Send on not established connection \"%s\"" %self.name)
			elif (self.verboseSend and (not self.verboseSkipBlankSend or data.strip() != "")):
				print("\033[%dm[%s] skipSend %s\033[m" %(30+self.verboseColour, self.name, repr(data)))
		else:
			if (self.verboseSend and (not self.verboseSkipBlankSend or data.strip() != "")):
				print("\033[%dm[%s] send %s\033[m" %(30+self.verboseColour, self.name, repr(data)))
			if (type(data) is str):
				data = data.encode()
			try:
				n = self.socket.send(data)
				if (n != len(data)):
					raise ValueError ("Not all data were send.")
			except OSError as e:
				if (self.verboseSend):
					print("\033[%dm[%s] send fails. Close connection\033[m" %(30+self.verboseColour, self.name))
					print("Error:", e)
				recon = self.backgroundConnect	#remember as close clears this
				self.closeConnection()
				if (recon):
					self.connect(BACKGROUND)
				return

			#timeout is limited to 4 parallel intervalls
			if (timeout and (len(self.pendingSend) < 4)):
				self.lock.acquire()
				self.pendingSend[self.timeoutID] = True
				toThread = threading.Thread(target=self._doTimeout, name="TcpClient::_doTimeout", args=(timeout, self.timeoutID))
				self.timeoutID += 1
				self.lock.release()
				toThread.start()


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def _doTimeout(self, timeout, id):
		sleep(timeout)
		self.lock.acquire()
		status = self.pendingSend.pop(id, None)
		self.lock.release()
		if (status):
			if (self.verboseSend):
				print("\033[%dm[%s] send-timeout detected\033[m" %(30+self.verboseColour, self.name))
			if (self.callbackSendTimeout != None):
				self.callbackSendTimeout(self.name)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def connected(self, _=None):
		return (self.isConnected == CONNECTED)

	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def disableReconnect(self):
		self.backgroundConnect = False

	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def closeConnection(self):
		with self.closeLock:
			self.backgroundConnect = False
			if (self.isConnected == CONNECTED):
				self.isConnected = SHUTDOWN
				if (self.verboseColour!=-1):
					print("\033[%dm[%s] Socket shutdown from client\033[m"
						  %(30+self.verboseColour, self.name))
				#this triggers the close inside the listen thread
				try:
					self.socket.shutdown(socket.SHUT_RDWR)
				except OSError as e:
					sleep(0.5)
					print("TcpClient::closeConnection:", self.name, self.isConnected)
					if (self.isConnected != DISCONNECTED):
						print("Error:", e)
						print("[%s] Shutdown failed. Force to DISCONNECTED"%self.name)
						self.isConnected = DISCONNECTED
						#raise
			elif (self.isConnected == CONNECTING):
				self.isConnected = ENDCONNECTING


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def closeSocket(self):
		if (self.socket != None):
			#close all connections
			self.closeConnection()
			while (self.isConnected != DISCONNECTED):
				sleep(0.1)

			#close the server itself
			if (self.verboseColour!=-1):
				print("\033[%dm[%s] Close TCP client socket\033[m" %(30+self.verboseColour, self.name))
#class


#------------------------------------------------------------------------------------------
#Some test code for module testing (when called directly)
import sys
if (sys.argv[0] == __file__):
	def callbackRecv(_, s, param):
		print("CB receive[]", s)

	def callbackClose(_, name, param):
		print("CB close[] ", name)

	def callbackOpen(_, addr, port, param):
		print("CB open[]", addr, port)

	client = TcpClient("", 2222, verboseColour=2, tryConnect=3, verboseSend=1,
				callbackRecv=callbackRecv, callbackOpen=callbackOpen, callbackClose=callbackClose,
				ignoreSendWhenDisconnected=1
			)
	if (not client.connected()):
		exit(1)
	sleep(2)
	try:
		client.send("Hallo 0\n")
		client.send("Hallo 1\n", 1)
		client.send("Hallo 2\n", 3)
		client.send("Hallo 3\n", 2)
		client.send("Hallo 4\n", 2)
		client.send("Hallo 5\n", 2)
		sleep(2)
		client.send("Hallo 4\n", 1.5)
	except Exception as e:
		print("Error:", e)
	sleep(6)
	client.closeConnection()
	client.send("Hallo Closed\n")
	client.closeConnection()
	client.connect(BACKGROUND)
	client.send("Hallo Wait\n")

	sleep(2)
	print("close")
	client.closeSocket()
	client.closeSocket()
	print(threading.enumerate())
