#! /bin/bash

make -j4
if [ "$?" != "0" ]; then
    exit -1;
fi
echo 
echo 
sudo make install
if [ "$?" != "0" ]; then
    exit -1;
fi
echo 
echo
sudo rmmod dvb_hdhomerun
sudo rmmod dvb_hdhomerun_fe
sudo rmmod dvb_hdhomerun_core
echo 
echo
sudo modprobe dvb_hdhomerun

