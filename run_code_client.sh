#!/bin/bash

# Compile the program with debugging symbols
gcc -o clientmain clientmain.c clientUDP.c buffer_code.c timer_utils.c graph_client.c controller.c -I. -lSDL -lm -pthread

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running gdb..."
    # Run the program in gdb
    gdb ./clientmain
else
    echo "Compilation failed. Please check for errors."
fi
