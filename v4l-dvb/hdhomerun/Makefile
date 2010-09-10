dvb_hdhomerun-objs := dvb_hdhomerun_init.o dvb_hdhomerun_data.o
obj-$(CONFIG_DVB_HDHOMERUN) += dvb_hdhomerun.o

dvb_hdhomerun_core-objs := dvb_hdhomerun_control.o
obj-$(CONFIG_DVB_HDHOMERUN) += dvb_hdhomerun_core.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/ -Idrivers/media/dvb/frontends
