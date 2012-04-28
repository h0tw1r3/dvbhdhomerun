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
sudo rmmod -v dvb_hdhomerun
sudo rmmod -v dvb_hdhomerun_fe
sudo rmmod -v dvb_hdhomerun_core
echo
sudo modprobe -v dvb_hdhomerun

