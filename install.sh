#!/bin/bash

# Exit immediately if something fails
set -e

if [ -d "$PWD/krunner-qalculator" ]; then
    # Update existing install
    cd krunner-qalculator/
    git pull -f
    echo "Files have been updated."
elif [[ $(basename "$PWD") !=  "krunner-qalculator"* ]]; then
    git clone https://github.com/kas-cor/krunner-qalculator.git
    cd krunner-qalculator/
fi

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

krunner_version=$(krunner --version | grep -oP "(?<=krunner )\d+")
if [[ "$krunner_version" == "6" ]]; then
    echo "Building for Plasma6"
    BUILD_QT6_OPTION="-DBUILD_WITH_QT6=ON"
else
    echo "Building for Plasma5"
    BUILD_QT6_OPTION=""
fi

# Configure project
echo "Configuring project..."
cmake ../src/ -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_USE_QT_SYS_PATHS=ON -DBUILD_TESTING=OFF $BUILD_QT6_OPTION

# Build project
echo "Building project..."
make -j$(nproc)

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

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    kquitapp$krunner_version krunner
fi

echo "Plugin installed successfully"
echo "KRunner restarted. Try using the plugin by pressing Alt+Space"