How to build dvbhdhomerun from within the s2-liplianin drivers (http://mercurial.intuxication.org/hg/s2-liplianin/). 

1) Open dvbhdhomerun/v4l-dvb/make_links.sh and change the following to
   your s2-liplianin path.

  V4L_DVB_PATH=/home/tfylliv/src/s2-liplianin

2) Run make_links.sh. This copies dvbhdhomerun source code into the
   s2_liplianin driver source, including a couple of Makefiles and
   Kconfig's for building it.

3) Apply this patch: dvbhdhomerun/v4l-dvb/dvbhdhomerun_s2_liplianin.patch  
   to the s2-liplianin source. From within the root of s2-liplianin
   dir - same as the V4L_DVB_PATH from above, do this.

   patch -p1 < /home/tfylliv/src/dvbhdhomerun/v4l-dvb/dvbhdhomerun_s2_liplianin.patch

   This patch adds hdhomerun to the "make menuconfig" menus and add a
   define to the general makefile for frontends.

4) Enable hdhomerun via make menuconfig:

   Multimedia Support -> DVB/ATSC adapters -> HDHomeRun (as module).
   Multimedia Support -> DVB/ATSC adapters -> Customize DVB Frontends -> HDHomeRun FE (as module).

5) make; sudo make install

6) Test: sudo modprobe dvb_hdhomerun. Output from dmesg, should be:

[31591.612450] HDHomeRun: Begin init, version 0.0.9
[31591.612596] HDHomeRun: Waiting for userspace to connect
[31591.612597] HDHomeRun: End init

7) Profit! ;-)
