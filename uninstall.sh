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

# Check for build directory
if [ ! -d "build" ]; then
    echo "Error: build directory not found"
    echo "Run install.sh at least once before uninstalling"
    exit 1
fi

# Change to build directory
cd build

# Remove plugin
echo "Removing plugin..."
if sudo -n true 2>/dev/null; then
    sudo make uninstall
else
    # Request password via systemd-ask-password if available
    if command -v systemd-ask-password &> /dev/null; then
        sudo -S make uninstall <<< $(systemd-ask-password "Enter sudo password to remove plugin: ")
    else
        # Use standard password input
        echo "Sudo password required to remove plugin"
        sudo make uninstall
    fi
fi

if [ $? -eq 0 ]; then
    echo "Plugin removed successfully"
    restart_krunner
    echo "KRunner restarted"
else
    echo "Error removing plugin"
    exit 1
fi
