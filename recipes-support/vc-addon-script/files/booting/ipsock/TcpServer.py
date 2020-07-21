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
#------------------------------------------------------------------------------------------
# Instantiation
#   class TcpServer()
#     port                                    Number of TCP server port
#     maxConnections=1                        Number of parallel connections on the port
#     name="server"                           Instance name when printing debug messages
#
#     verboseColour=-1                        Color code (VT100) for socket operation verbose messages (-1 for off)
#     verboseRecv=0                           Print all received data
#     verboseSend=0                           Print all send data
#     verboseOut=sys.stdout                   Channel used for verbose messages
#     verboseSkipBlankSend=1                  Do not print blank lines on verbose channel
#
#     callbackOpen(handle, addr, port, param)=None
#                                             Function called when client conection established before rx/tx enabled.
#                                             Port is the incomming connection, myPort the servers port
#     callbackEstablished(handle, addr, port, param)=None
#                                             Function called when client conection established after rx/tx enabled.
#     callbackReject(handle, addr, port, param)=None
#                                             Function called when client conection rejected
#     callbackRecv(handle, data, param)=None  Function called on reception (data as received)
#     callbackClose(handle, name, param)=None Function called when port closes
#     callbackArgs=[]                         Parameter 'param' given to other callbacks
#
#     splitLines=1                            0: Data as received, 1=line by line (no \n)
#     lineSkipEmpty=0                         Empty lines are not provided (only when splitLines=1)
#     lineSuppressOlder=0                     Only the newest complete line provided (only when splitLines=1)
#     ignoreSendWhenDisconnected=0            Send does not throw an error when there is no connection
#------------------------------------------------------------------------------------------
# Member functions
#     connected(handle)                Return True is socket has been connected by client
#     send(handle, data)               Send data on socket
#     closeConnection(handle)          Triggers close of client connection
#     closeSocket()                    Closes all client connections and deregister the socket
#------------------------------------------------------------------------------------------
# There is an example/module test at the end of this file
#------------------------------------------------------------------------------------------
from time import sleep
import socket
import threading
import sys


#------------------------------------------------------------------------------------------
#Define enum for the connection state
DISCONNECTED, SHUTDOWN, CONNECTED = range(0,3)


#------------------------------------------------------------------------------------------
class TcpServer:
	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def __init__(self, port, maxConnections=1, name="server",
					   verboseColour=-1, verboseRecv=0, verboseSend=0, verboseOut=sys.stdout, verboseSkipBlankSend=1,
					   callbackOpen=None, callbackEstablished=None, callbackRecv=None, callbackClose=None, callbackReject=None,
					   callbackArgs=[],
					   splitLines=1, lineSkipEmpty=0, lineSuppressOlder=0,
					   ignoreSendWhenDisconnected=0
				):
		self.port = port
		self.name = name

		self.verboseColour = verboseColour
		self.verboseRecv = verboseRecv
		self.verboseSend = verboseSend
		self.verboseOut = verboseOut
		self.verboseSkipBlankSend = verboseSkipBlankSend

		self.callbackOpen = callbackOpen
		self.callbackEstablished = callbackEstablished
		self.callbackRecv = callbackRecv
		self.callbackClose = callbackClose
		self.callbackReject = callbackReject
		self.callbackArgs = callbackArgs

		self.splitLines = splitLines
		self.lineSkipEmpty = lineSkipEmpty
		self.lineSuppressOlder = lineSuppressOlder
		self.ignoreSendWhenDisconnected = ignoreSendWhenDisconnected

		#internal data structures
		self.socket = None
		self.connection = [None] * maxConnections
		self.threadListen = [None] * maxConnections
		self.isConnected = [DISCONNECTED] * maxConnections
		self.serverShutdown = False

		try:
			if (self.verboseColour!=-1):
				print("\033[%dm[%s] Create TCP server socket %d\033[m" %(30+self.verboseColour, self.name, self.port), file=self.verboseOut)
			self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)  #send imediately
			self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  #dont block server port after exit
			self.socket.bind(("", self.port))
			self.socket.listen()
		except Exception as e:
			if (self.verboseColour!=-1):
				print("\033[%d;%dm[%s] Socket %d setup failed\033[m (%s)"
					  %(40+self.verboseColour, 31, self.name, self.port, e), file=self.verboseOut)
			self.threadAccept = None
			raise   #and pass to the caller

		#Connect the listen task
		self.threadAccept = threading.Thread(target=self._accepting, name="AcceptThread")
		self.threadAccept.start()


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def __del__(self):
		self.closeSocket()


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def _accepting(self):
		while (self.socket != None):
			#this timeout prevents an hanging AcceptThread when program closes
			self.socket.settimeout(0.2)
			while (self.socket != None):
				try:
					con, (addr, port) = self.socket.accept()
					break
				except socket.timeout:
					pass
				except OSError:
					if (self.socket != None):
						raise
			if (self.socket == None):
				break

			#limit the number of incomming connections
			free = -1
			i = 0
			for c in self.connection:
				if (c == None):
					free = i
					break
				i = i+1
			if (free != -1 and not self.serverShutdown):
				if (self.verboseColour!=-1):
					print("\033[%dm[%s,%d] Got connection from client %s:%d on %d\033[m"
						  %(30+self.verboseColour, self.name, free, addr, port, self.port), file=self.verboseOut)
				if (self.callbackOpen):
					self.callbackOpen(free, addr, port, self.callbackArgs)
				self.connection[free] = con
				self.isConnected[free] = CONNECTED
				self.threadListen[free] = threading.Thread(target=self._listening, name="ReceiveThread", args=(free,))
				self.threadListen[free].start()
				if (self.callbackEstablished):
					self.callbackEstablished(free, addr, port, self.callbackArgs)

			else: #reject the 2nd connections as listen()/accept() is always active
				if (self.verboseColour!=-1):
					print("\033[%dm[%s] Reject connection from client %s on %d\033[m"
						  %(30+self.verboseColour, self.name, addr, self.port), file=self.verboseOut)
				con.shutdown(socket.SHUT_RDWR)
				con.close()
				if (self.callbackReject):
					self.callbackReject(free, addr, port, self.callbackArgs)

		#Mark the thread as closed
		self.threadAccept = None


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def _listening(self, handle):
		IncompleteLine = ""
		while True:
			try:
				Data = self.connection[handle].recv(2048).decode()
			except OSError:
				Data = '';
			if(Data == ''):
				break

			#process the data
			if (self.verboseRecv):
				print("\033[%dm[%s,%d] recv %s\033[m"
					  %(30+self.verboseColour, self.name, handle, repr(Data)), file=self.verboseOut)

			if (self.callbackRecv != None):
				if (not self.splitLines):
					self.callbackRecv(handle, Data, self.callbackArgs)
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
						self.callbackRecv(handle, Split[-1], self.callbackArgs)
					#we provide all lines
					else:
						for s in Split:
							self.callbackRecv(handle, s, self.callbackArgs)

		#and clean up this connection
		if (self.verboseColour!=-1):
			print("\033[%dm[%s,%d] Connection closed by client\033[m"
				  %(30+self.verboseColour, self.name, handle), file=self.verboseOut)
		self.isConnected[handle] = DISCONNECTED
		self.connection[handle].close()
		self.connection[handle] = None
		if (self.callbackClose):
			self.callbackClose(handle, self.name, self.callbackArgs)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def send(self, handle, data):
		if (self.isConnected[handle] != CONNECTED):
			if (not self.ignoreSendWhenDisconnected):
				raise ValueError ("Send on not established connection (handle=%d)" %handle)
			elif (self.verboseSend and (not self.verboseSkipBlankSend or data.strip() != "")):
				print("\033[%dm[%s,%d] skipSend %s\033[m" %(30+self.verboseColour, self.name, handle, repr(data)), file=self.verboseOut)
		else:
			if (self.verboseSend and (not self.verboseSkipBlankSend or data.strip() != "")):
				print("\033[%dm[%s,%d] send %s\033[m" %(30+self.verboseColour, self.name, handle, repr(data)), file=self.verboseOut)
			if (type(data) is str):
				data = data.encode()
			try:
				n = self.connection[handle].send(data)
				if (n != len(data)):
					raise ValueError ("Not all data were send.")
			except OSError as e:
				if (self.verboseSend):
					print("\033[%dm[%s] send fails. Close connection\033[m" %(30+self.verboseColour, self.name))
					print("Error:", e)
				self.closeConnection(handle)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def connected(self, handle):
		return (self.isConnected[handle] == CONNECTED)


	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def closeConnection(self, handle):
		if (self.isConnected[handle] == CONNECTED):
			self.isConnected[handle] = SHUTDOWN
			if (self.verboseColour!=-1):
				print("\033[%dm[%s,%d] Socket shutdown send by server\033[m"
					  %(30+self.verboseColour, self.name, handle), file=self.verboseOut)
			#this triggers the close inside the listen thread
			try:
				self.connection[handle].shutdown(socket.SHUT_RDWR)
			except OSError as e:
				sleep(0.5)
				print("TcpServer::closeConnection:", self.name, self.isConnected[handle])
				if (self.isConnected[handle] != DISCONNECTED):
					print("Error:", e)
					print("[%s] Shutdown failed. Force to DISCONNECTED"%self.name)
					self.isConnected[handle] = DISCONNECTED
					#raise

	#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	def closeSocket(self):
		self.serverShutdown = True
		if (self.socket != None):
			#close all connections
			for i in range(0,len(self.connection)):
				if (self.connection[i] != None):
					self.closeConnection(i)
			allClosed = False
			while (not allClosed):
				allClosed = True
				for i in range(0,len(self.connection)):
					if (self.isConnected[i] != DISCONNECTED):
						allClosed = False

				if (not allClosed):
					sleep(0.1)

			#close the server itself
			if (self.verboseColour!=-1):
				print("\033[%dm[%s] Close the server port %d\033[m"
					  %(30+self.verboseColour, self.name, self.port), file=self.verboseOut)
			help = self.socket  #to prevent issue in .accept task
			self.socket = None
			help.close()

		#wait for the AcceptThread
		while (self.threadAccept != None):
			sleep(0.1)
#class


#------------------------------------------------------------------------------------------
#Some test code for module testing (when called directly)
if (sys.argv[0] == __file__):
	def callbackRecv(handle, s, param):
		print("callback[%d]" %handle, s)

	def callbackClose(handle, name, param):
		print("close[%d]" %handle)

	def callbackOpen(handle, addr, port, param):
		print("open[%d]" %handle, addr, port, param)

	server = TcpServer(2222, maxConnections=3, verboseColour=2, callbackRecv=callbackRecv,
			callbackOpen=callbackOpen, callbackClose=callbackClose)
	sleep(10)
	try:
		server.send(0, "Hallo 0\n")
		server.send(1, "Hallo 1\n")
		server.send(2, "Hallo 2\n")
	except Exception as e:
		print(e)
	sleep(5)
	#as __del__ is delayed called when top space is closed, we need to call close_Socket() manually
	#del server
	server.closeSocket()
	print(threading.enumerate())
