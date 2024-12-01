#!/bin/bash

# Function to print usage and exit
print_usage() {
    echo "Usage: $0 [--serial <serial>] | [<serial_number_index>]"
    exit 1
}

# Initialize an array to store unique serial numbers
declare -A serials

# Collect unique serial numbers from files ending with if02
for file in /dev/serial/by-id/usb-IBR_Black_Magic_Probe_IBR_Node_ibr_v0.5_*; do
    if [[ "$file" == *if02 ]]; then
        serial=$(echo "$file" | sed -n 's|.*/usb-IBR_Black_Magic_Probe_IBR_Node_ibr_v0.5_\([^/]*\)-if02|\1|p')
        serials["$serial"]=1
    fi
done

# Convert the associative array keys to an indexed array
serial_array=("${!serials[@]}")

# Handle the case where no argument is provided
if [ $# -eq 0 ]; then
    if [ "${#serial_array[@]}" -eq 1 ]; then
        # Only one board connected, use it directly
        make BOARD=ibr-node flash term
        exit 0
    else
        echo "Error: No argument provided and multiple boards detected. Please specify the serial or index."
        print_usage
    fi
fi

# Initialize variables for argument parsing
use_serial=false
arg_value=""

# Parse command line arguments
if [ "$1" == "--serial" ]; then
    if [ $# -ne 2 ]; then
        print_usage
    fi
    use_serial=true
    arg_value=$2
else
    if [ $# -ne 1 ]; then
        print_usage
    fi
    arg_value=$1
fi

# Initialize a variable to store the selected serial
selected_serial=""

# Handle --serial option
if $use_serial; then
    if [[ " ${serial_array[*]} " == *" $arg_value "* ]]; then
        selected_serial="$arg_value"
    else
        echo "Error: Serial number '$arg_value' not found."
        exit 1
    fi
else
    # Handle index option
    index=$arg_value

    # Check if the provided index is within the range of available serials
    if ! [[ "$index" =~ ^[0-9]+$ ]] || [ "$index" -lt 1 ] || [ "$index" -gt "${#serial_array[@]}" ]; then
        echo "Error: Index out of range. Please provide a number between 1 and ${#serial_array[@]}."
        exit 1
    fi

    # Get the serial number corresponding to the provided index
    selected_serial="${serial_array[$((index-1))]}"
fi

# Use the selected serial in the make commands
make BOARD=ibr-node flash SERIAL="$selected_serial"
make BOARD=ibr-node term PORT="/dev/serial/by-id/usb-IBR_Black_Magic_Probe_IBR_Node_ibr_v0.5_${selected_serial}-if02"
