#!/bin/bash

# Compile the program with debugging symbols
gcc -g -o clientmain clientmain.c clientUDP.c buffer_code.c timer_utils.c graph_client.c controller.c -I. -lSDL -lm -pthread -lrt

./clientmain 127.0.0.1 8100