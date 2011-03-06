What is this?

A linux DVB driver for the HDHomeRun (http://www.silicondust.com).


What can the driver do?

Make the HDHomeRun work as any other DVB device under linux. Use the
HDHomeRun with Tvheadend, Kaffeine, the various dvb command line
tools, Mythtv (as a DVB device) etc.


How does it work?

The driver consists of two parts:

1) A user space application "userhdhomerun".
2) A kernel driver "dvb_hdhomerun".

The user space application "userhdhomerun" uses the LGPL library
provided by SiliconDust [1]. The library provides functionality to
setup the HDHomeRun, change channels, setup PID filtering, get signal
quality and so on. The result is UDP stream(s) from the HDHomeRun
containing MPEG transport stream(s) with the selected channels (PIDs).

The user space application connects to the kernel driver
"dvb_hdhomerun" and receives all its instructions from there.

The kernel driver "dvb_hdhomerun" is just a small skeleton driver. Its
sole purpose is to forward the various requests from dvb applications
(Kaffeine, Tvheadend etc.) to userhdhomerun which them handles
them. Essentially the requests boils down to PID filtering, start/stop
streams and signal quality requests.

The kernel driver creates /dev/dvb/adapterX/xyz devices via the linux
DVB subsystem (http://linuxtv.org). One adapter per tuner is created,
a dual tuner HDHomeRun will thus create two adapters.

The kernel driver also creates the following devices:

1) One /dev/hdhomerun_control device (which userhdhomerun connects to
in order to receive requests forwarded from the kernel driver from dvb
applications).

2) One or more /dev/hdhomerun_dataX device(s) one device per adapter.
The userhdhomerun connects to these to feed the MPEG transport
stream(s) from the HDHomeRun to the kernel driver. The kernel driver
then sends the MPEG transport stream to the DVB subsystem.


See build.txt for instructions of how to build kernel driver + user
app.

See installation.txt for usage/installation instructions.



[1] http://www.silicondust.com/support/hdhomerun/downloads/linux/
