#!/bin/bash

# Function to restart KRunner
restart_krunner() {
    if pgrep -x "krunner" > /dev/null; then
        kquitapp6 krunner &>/dev/null || true
        sleep 1
    fi
    if command -v kstart6 &> /dev/null; then
        nohup kstart6 krunner &>/dev/null & disown
    else
        nohup krunner &>/dev/null & disown
    fi
    sleep 1
}

# Check for required commands
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is not installed"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Error: make is not installed"
    exit 1
fi

# Clean and create build directory
rm -rf build
mkdir -p build
cd build

# Configure project
echo "Configuring project..."
cmake ../src/ -DCMAKE_INSTALL_PREFIX=/usr

# Build project
echo "Building project..."
make

# Install
echo "Installing plugin..."
if sudo -n true 2>/dev/null; then
    sudo make install
else
    # Request password via systemd-ask-password if available
    if command -v systemd-ask-password &> /dev/null; then
        sudo -S make install <<< $(systemd-ask-password "Enter sudo password to install plugin: ")
    else
        # Use standard password input
        echo "Sudo password required to install plugin"
        sudo make install
    fi
fi

if [ $? -eq 0 ]; then
    echo "Plugin installed successfully"
    restart_krunner
    echo "KRunner restarted. Try using the plugin by pressing Alt+Space"
else
    echo "Error installing plugin"
    exit 1
fi
