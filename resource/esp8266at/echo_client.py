#!/usr/bin/python

#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

import socket
 
SERVER_ADDR = '127.0.0.1'
SERVER_PORT = 9010
DATA_SIZE_MAX = 1024
 
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
try:
    sock.connect((SERVER_ADDR, SERVER_PORT))
    print('Connected')
    for seq_no in range(10):
        send_data = ("%03d hello" % seq_no).encode('utf-8')
        sock.sendall(send_data)
        print("Sent     : %s" % send_data)
        
        recv_data = sock.recv(DATA_SIZE_MAX)
        print("Received : %s" % recv_data)
        
        if (recv_data != send_data):
            print("Data is not match!")
            break;
 
except KeyboardInterrupt:
    print("Exit!")
finally:
    sock.close()
    del sock

