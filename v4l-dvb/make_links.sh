#! /bin/sh

current=`pwd`

V4L_DVB_PATH=/home/villyft/src/v4l-dvb

cd $V4L_DVB_PATH/linux/drivers/media/dvb/frontends
rm dvb_hdhomerun_fe.c
rm dvb_hdhomerun_fe.h
ln -s $current/../kernel/dvb_hdhomerun_fe.c dvb_hdhomerun_fe.c
ln -s $current/../kernel/dvb_hdhomerun_fe.h dvb_hdhomerun_fe.h

cd $V4L_DVB_PATH/linux/drivers/media/dvb
mkdir hdhomerun
cd hdhomerun
rm dvb_hdhomerun.c
rm dvb_hdhomerun.h
ln -s $current/../kernel/dvb_hdhomerun.c dvb_hdhomerun.c
ln -s $current/../kernel/dvb_hdhomerun.h dvb_hdhomerun.h

rm dvb_hdhomerun_control.c
rm dvb_hdhomerun_control.h
ln -s $current/../kernel/dvb_hdhomerun_control.c dvb_hdhomerun_control.c
ln -s $current/../kernel/dvb_hdhomerun_control.h dvb_hdhomerun_control.h

rm dvb_hdhomerun_data.c
rm dvb_hdhomerun_data.h
ln -s $current/../kernel/dvb_hdhomerun_data.c dvb_hdhomerun_data.c
ln -s $current/../kernel/dvb_hdhomerun_data.h dvb_hdhomerun_data.h

rm dvbhdhomerun_control_messages.h
ln -s $current/../kernel/dvbhdhomerun_control_messages.h dvbhdhomerun_control_messages.h

rm Makefile
rm Kconfig
ln -s $current/hdhomerun/Makefile Makefile
ln -s $current/hdhomerun/Kconfig Kconfig

cd $V4L_DVB_PATH/v4l
rm load.sh
ln -s $current/load.sh load.sh
