#! /bin/bash

git clone https://github.com/vaimee/sepa-C-kpi.git
git clone https://github.com/arces-wot/WOT-Demo.git
git clone https://github.com/warmcat/libwebsockets.git
sudo apt-get install cmake libssl-dev libcurl4-gnutls-dev libglib2.0-dev
cd ./libwebsockets
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
