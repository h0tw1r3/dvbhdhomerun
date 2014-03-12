KERNELRELEASE	:= `uname -r`
KVERMAJ         := $(shell echo $(KERNELRELEASE) | cut -f1 -d.)
KVERMIN         := $(shell echo $(KERNELRELEASE) | cut -f2 -d.)
PWD		:= $(shell pwd)
HEADERSUBFOLDER  = $(shell [ $(KVERMAJ) -gt 2 -a $(KVERMIN) -gt 6 ] && echo || echo /dvb)

dvb_hdhomerun-objs := dvb_hdhomerun_init.o dvb_hdhomerun_data.o dvb_hdhomerun_control.o
obj-m += dvb_hdhomerun.o
obj-m += dvb_hdhomerun_core.o
obj-m += dvb_hdhomerun_fe.o

ccflags-y	+= -I$(src)/headers/$(KVERMAJ).$(KVERMIN)

all: dlheaders dvb_hdhomerun

dlheaders: dlmain
	@echo Downloading kernel header dependencies...
	@$(foreach var,$(shell grep -h ^\\\#include\ \" $(PWD)/headers/$(KVERMAJ).$(KVERMIN)/*.[ch] 2>/dev/null | sort -u | sed 's/\#include "\(.*\)"/\1/'),wget -c --timeout 30 -nc -nv -P $(PWD)/headers/$(KVERMAJ).$(KVERMIN) https://github.com/torvalds/linux/raw/v$(KVERMAJ).$(KVERMIN)/drivers/media$(HEADERSUBFOLDER)/dvb-core/$(var);)

dlmain:
	@echo Downloading kernel headers...
	@mkdir -p $(PWD)/headers/$(KVERMAJ).$(KVERMIN)
	@$(foreach var,$(shell grep -h ^\\\#include\ \" *.[ch] 2>/dev/null | grep -v hdhomerun | sort -u | sed 's/\#include "\(.*\)"/\1/'),wget -c --timeout 30 -nc -nv -P $(PWD)/headers/$(KVERMAJ).$(KVERMIN) https://github.com/torvalds/linux/raw/v$(KVERMAJ).$(KVERMIN)/drivers/media$(HEADERSUBFOLDER)/dvb-core/$(var);)

dvb_hdhomerun:
	$(MAKE) -C /lib/modules/$(KERNELRELEASE)/build M=$(PWD) modules

install: dvb_hdhomerun
	$(MAKE) -C /lib/modules/$(KERNELRELEASE)/build M=$(PWD) modules_install

clean:
	$(MAKE) -C /lib/modules/$(KERNELRELEASE)/build M=$(PWD) clean
