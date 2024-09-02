#!/bin/bash

# Source the NEC MPI environment variables
source /opt/nec/ve/mpi/3.6.0/bin/necmpivars.sh gnu

# Compile the C program with mpincc
mpincc -vh host.c -o host -I/opt/nec/ve/share/veda/include \
-L/opt/nec/ve/veos/lib64 -rdynamic -Wl,-rpath,$HOME/link/veda-lib -lveda

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation succeeded. Executable 'host' created."
else
    echo "Compilation failed."
fi
