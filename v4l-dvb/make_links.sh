#! /bin/sh

current=`pwd`

V4L_DVB_PATH=/home/villyft/src/s2-liplianin

cd $V4L_DVB_PATH/linux/drivers/media/dvb/frontends
FRONTEND_FILES="dvb_hdhomerun_fe.c dvb_hdhomerun_fe.h"
for file in $FRONTEND_FILES
do
    rm -f $file
    cp $current/../kernel/$file $file
done

cd $V4L_DVB_PATH/linux/drivers/media/dvb
mkdir hdhomerun 2> /dev/null
cd hdhomerun
DRIVER_FILES="dvb_hdhomerun_compat.h dvb_hdhomerun_control.c dvb_hdhomerun_control.h dvb_hdhomerun_control_messages.h dvb_hdhomerun_core.c dvb_hdhomerun_core.h dvb_hdhomerun_data.c dvb_hdhomerun_data.h dvb_hdhomerun_debug.h dvb_hdhomerun_init.c dvb_hdhomerun_init.h"
for file in $DRIVER_FILES
do
    rm -f $file
    cp $current/../kernel/$file $file
done

rm -f Makefile
rm -f Kconfig
cp $current/hdhomerun/Makefile Makefile
cp $current/hdhomerun/Kconfig Kconfig
