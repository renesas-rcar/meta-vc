#!/usr/bin/env python3
import sys
import fileinput
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), 'ipsock'))
from TcpServer import TcpServer
from time import sleep


#----------------------------------------------------------------------
PORTNR1 = 2222   #the continious status
PORTNR2 = 5555   #only sync/nosync

MAX_CONNECTS = 10
connections = []

SYNC_MSG_BEFORE_REPORT_SYNC = 5


#----------------------------------------------------------------------
def callbackOpen(handle, addr, port, param):
    global connections
    #print("CB open[%d]" %handle, addr, port, param)
    connections.append(handle)

def callbackClose(handle, name, param):
    global connections
    #print("CB close[%d]" %handle)
    connections.remove(handle)

def callbackRecv(handle, s, param):
    print("CB recv[%d]" %handle, s)

server = TcpServer(PORTNR1, maxConnections=MAX_CONNECTS, verboseColour=2, verboseRecv=0, verboseSend=0,
            callbackRecv=callbackRecv, callbackOpen=callbackOpen, callbackClose=callbackClose,
            ignoreSendWhenDisconnected=1
        )


#----------------------------------------------------------------------
def sendStatus(handle, addr, port, param):
    #print("sendStatus[%d]" %handle, addr, port, param)
    status.send(handle, "%d\n" %(synccnt==SYNC_MSG_BEFORE_REPORT_SYNC))
    status.closeConnection(handle)

status = TcpServer(PORTNR2, maxConnections=MAX_CONNECTS, verboseColour=-1, verboseRecv=0, verboseSend=0,
            callbackRecv=None, callbackEstablished=sendStatus, callbackClose=None,
            ignoreSendWhenDisconnected=1
        )


#----------------------------------------------------------------------
synccnt = 0
for line in fileinput.input():
    print(connections, synccnt, repr(line))
    for handle in connections:
        server.send(handle, line)
    if (line.find(": clock_sync master offset ") != -1):
        if (synccnt < SYNC_MSG_BEFORE_REPORT_SYNC):
            synccnt += 1
    else:
        synccnt = 0
