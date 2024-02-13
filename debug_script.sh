#!/bin/bash

# Set the path to your executable
EXECUTABLE_PATH="./as"

# Set the parameters for your program
PROGRAM_PARAMETERS="./as -o out.o main.s"

# Run GDB with the executable and parameters
gdb --args $EXECUTABLE_PATH $PROGRAM_PARAMETERS
