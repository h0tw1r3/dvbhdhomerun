/*
 * dvb_hdhomerun_core.c, skeleton driver for the HDHomeRun devices
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

#include <linux/module.h>
#include <linux/sched.h>

#include "dvb_hdhomerun_control.h"
#include "dvb_hdhomerun_debug.h"

#include "dvb_hdhomerun_core.h"

/* for the control device */
struct kfifo *control_fifo_user;
EXPORT_SYMBOL(control_fifo_user);

struct kfifo *control_fifo_kernel;
EXPORT_SYMBOL(control_fifo_kernel);

spinlock_t control_spinlock_user;
EXPORT_SYMBOL(control_spinlock_user);

spinlock_t control_spinlock_kernel;
EXPORT_SYMBOL(control_spinlock_kernel);

wait_queue_head_t control_readq;
EXPORT_SYMBOL(control_readq);

int control_bufsize = 32768;
EXPORT_SYMBOL(control_bufsize);

/* Handles the case where the user space app ctrl-c's. I.E the hdhomerun app, shouldn't write to fifo.*/
int wait_for_write = 0; /* Need mutex on this */
EXPORT_SYMBOL(wait_for_write);

/* Handles the case where the hdhomerun app is not running */
int userspace_ready = 0; /* Need mutex on this */
EXPORT_SYMBOL(userspace_ready);

int hdhomerun_debug_mask = 0;
module_param(hdhomerun_debug_mask, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(hdhomerun_debug_mask, "Mask for debug output\n");
EXPORT_SYMBOL(hdhomerun_debug_mask);

MODULE_AUTHOR("Villy Thomsen");
MODULE_DESCRIPTION("HDHomeRun Driver Core Module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

int hdhomerun_control_post_message(struct dvbhdhomerun_control_mesg *mesg) {
	int ret = -1;

	DEBUG_FUNC(1);

	if(userspace_ready) {
		if(kfifo_put(control_fifo_user, (unsigned char*)mesg, sizeof(struct dvbhdhomerun_control_mesg)) < sizeof(struct dvbhdhomerun_control_mesg) ) {
			printk(KERN_CRIT "No buffer space for hdhomerun control device!\n");
		} else {
			ret = 1;
		}
	}
	return ret;
}
EXPORT_SYMBOL(hdhomerun_control_post_message);

int hdhomerun_control_wait_for_message(struct dvbhdhomerun_control_mesg *mesg) {
	DEBUG_FUNC(1);
	wait_for_write = 1;
	do {
		if(wait_event_interruptible(control_readq, __kfifo_len(control_fifo_kernel) > 0)) {
			DEBUG_OUT(HDHOMERUN_CONTROL,"%s read interrupted\n", __FUNCTION__);
			wait_for_write = 0;
			return -ERESTARTSYS;
		}
	} while(__kfifo_len(control_fifo_kernel) == 0);

	return kfifo_get(control_fifo_kernel, (unsigned char*)mesg, sizeof(struct dvbhdhomerun_control_mesg));
}
EXPORT_SYMBOL(hdhomerun_control_wait_for_message);

int hdhomerun_control_post_and_wait(struct dvbhdhomerun_control_mesg *mesg) {
	int ret;

	ret = hdhomerun_control_post_message(mesg);
	if(ret == 1) {
		/* Now we wait for userspace to return to us */
		ret = hdhomerun_control_wait_for_message(mesg);
		if(ret == 0) {
			DEBUG_OUT(HDHOMERUN_CONTROL, "dvbhdhomerun_wait_for_message failed, shouldn't happen!\n");
		}
	}

	return ret;
}
EXPORT_SYMBOL(hdhomerun_control_post_and_wait);


