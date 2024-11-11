#!/bin/bash

# Default IP address
ip="192.168.1.159"

# Parse command-line options
while getopts ":i:" opt; do
    case ${opt} in
        # -i
        i )
            ip=$OPTARG
            ;;
        \? )
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done

# Execute commands
make clean && make && nxlink -a "$ip" enablechatcheatsnx.nro