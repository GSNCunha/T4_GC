#!/bin/bash

# Compile the program with debugging symbols
gcc -g -o servermain servermain.c buffer_code.c graph.c planta.c timer_utils.c serverUDP.c -I. -lSDL -lm -pthread


# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running gdb..."
    # Run the program in gdb
    gdb ./servermain
else
    echo "Compilation failed. Please check for errors."
fi
