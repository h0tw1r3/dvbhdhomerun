/*
 * dvb_hdhomerun_data.h, skeleton driver for the HDHomeRun devices
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

#ifndef __DVB_HDHOMERUN_DATA_H__
#define __DVB_HDHOMERUN_DATA_H__

#include "dvb_demux.h"

#include "dvb_hdhomerun_control_messages.h"

extern int dvb_hdhomerun_data_init(int num_of_devices);
extern int dvb_hdhomerun_data_create_device(struct dvb_demux *dvb_demux, int id);
extern void dvb_hdhomerun_data_delete_device(int id);
extern void dvb_hdhomerun_data_exit(void);

#endif /* __DVB_HDHOMERUN_DATA_H__ */
