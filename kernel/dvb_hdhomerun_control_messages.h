/*
 * dvb_hdhomerun_control_messages.h, skeleton driver for the HDHomeRun devices
 *
 * Copyright (C) 2010 Villy Thomsen <tfylliv@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __DVBHDHOMERUN_CONTROL_MESSAGES_H__
#define __DVBHDHOMERUN_CONTROL_MESSAGES_H__

#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <linux/ioctl.h>

#ifndef __KERNEL__
#include <stdint.h>
#endif

typedef enum {
	DVB_HDHOMERUN_FE_READ_STATUS = 0,
	DVB_HDHOMERUN_FE_READ_BER,
	DVB_HDHOMERUN_FE_READ_UNCORRECTED_BLOCKS,
	DVB_HDHOMERUN_FE_SET_FRONTEND,
	DVB_HDHOMERUN_FE_READ_SIGNAL_STRENGTH,
	DVB_HDHOMERUN_START_FEED,
	DVB_HDHOMERUN_STOP_FEED,
	DVB_HDHOMERUN_DMX_SET_PES_FILTER,
	DVB_HDHOMERUN_INT_REGISTER_DEVICE  
} hdhomerun_control_mesg_type_t;


struct hdhomerun_dvb_demux_feed {
	uint16_t pid;
	unsigned int index;
};

struct hdhomerun_register_tuner_data {
   uint8_t num_of_devices;
   char name[12];
   int id;
   int type;
   bool use_full_name;
};

struct dvbhdhomerun_control_mesg {
	unsigned int type;
	union {
		unsigned int frequency;
		fe_status_t fe_status;
		int16_t signal_strength;
		struct dmx_pes_filter_params dmx_pes_filter;
		struct hdhomerun_dvb_demux_feed demux_feed;
		struct hdhomerun_register_tuner_data reg_data;
	} u;
	int id;
};


/* Use 'v' as magic number */
#define HDHOMERUN_IOC_MAGIC  'v'
#define HDHOMERUN_REGISTER_TUNER _IOWR(HDHOMERUN_IOC_MAGIC, 0, struct hdhomerun_register_tuner_data)

#endif
