#!/bin/bash

# Check if a parameter is provided, if not, display an error and exit
if [ -z "$1" ]; then
    echo "Usage: $0 <program_name>"
    exit 1
fi

# Set the program name from the parameter
program_name="$1"

# Remove the old program files
rm -f "build/$program_name"
rm -f "../bin/$program_name"

# Create the build directory quietly
mkdir -p build > /dev/null 2>&1

# Navigate to the build directory
cd build

# Run cmake and make
cmake -DDEFINE_RELEASE=ON ..
make -j 4

# Check if the program was built successfully
if [ ! -e "../../bin/$program_name" ]; then
    # Display an error message in red and play a beep sound
    echo -e "\033[0;31mError: Compilation failed. $program_name not found in '../bin/'.\033[0m"
    echo -e "\a"  # Play a beep sound
    exit 1
fi

# Display a message about copying binaries
echo "Finished compile. Hopefully it's located in <app name>bin/$program_name now."

# Return to the parent directory
cd ..

# Make the program executable
chmod +x "../bin/$program_name"
