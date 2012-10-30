/*
 * dvb_hdhomerun_core.h skeleton driver for the HDHomeRun devices
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

#ifndef __DVB_HDHOMERUN_CORE_H__
#define __DVB_HDHOMERUN_CORE_H__

#include <linux/kfifo.h>
#include <linux/wait.h>

#include "dvb_hdhomerun_control_messages.h"

#define HDHOMERUN_VERSION "0.0.14"

#define HDHOMERUN_MAX_TUNERS 8

extern struct kfifo control_fifo_user;
extern struct kfifo control_fifo_kernel;
extern int wait_for_write;
extern int userspace_ready;
extern wait_queue_head_t control_readq;
extern wait_queue_head_t inq;
extern wait_queue_head_t outq;
extern int control_bufsize;
extern spinlock_t control_spinlock_user;
extern spinlock_t control_spinlock_kernel;

extern int hdhomerun_debug_mask;

extern int hdhomerun_control_post_message(struct dvbhdhomerun_control_mesg *mesg);
extern int hdhomerun_control_wait_for_message(struct dvbhdhomerun_control_mesg *mesg);
extern int hdhomerun_control_post_and_wait(struct dvbhdhomerun_control_mesg *mesg);


#endif /* __DVB_HDHOMERUN_CORE_H__ */
