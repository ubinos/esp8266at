#!/usr/bin/python

#
# Copyright (c) 2020 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

import socket

SERVER_ADDR = ''
SERVER_PORT = 9000
DATA_SIZE_MAX = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((SERVER_ADDR, SERVER_PORT))
sock.listen(1)

try:
    conn, addr = sock.accept()
    print('Connected')
    while True:
        recv_data = conn.recv(DATA_SIZE_MAX)
        if not recv_data:
            break;
        print("Received : %s" % recv_data)
        
        conn.sendall(recv_data)
        print("Sent     : %s" % recv_data)

except KeyboardInterrupt:
    print("Exit!")
finally:
    sock.close()
    del sock

