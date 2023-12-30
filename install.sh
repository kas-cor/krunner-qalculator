#!/bin/bash
set -e
rm -rf build
mkdir -p build
cd build
cmake ../src/ -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DKDE_INSTALL_QTPLUGINDIR=`kf5-config --qt-plugins`
make
sudo make install
kquitapp5 krunner
