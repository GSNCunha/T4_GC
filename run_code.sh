#!/bin/bash

# Compile the program with debugging symbols
gcc -g -o servermain servermain.c buffer_code.c graph_server.c planta.c timer_utils.c serverUDP.c -I. -lSDL -lm -pthread

./servermain 8081

