/*
 * dvb_hdhomerun_control.c, skeleton driver for the HDHomeRun devices
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

#include <dvbdev.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/kobject.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "dvb_hdhomerun_control_messages.h"
#include "dvb_hdhomerun_compat.h"
#include "dvb_hdhomerun_init.h"
#include "dvb_hdhomerun_data.h"
#include "dvb_hdhomerun_debug.h"
#include "dvb_hdhomerun_core.h"

#include "dvb_hdhomerun_control.h"

MODULE_AUTHOR("Villy Thomsen");
MODULE_DESCRIPTION("HDHomeRun Driver Core Module");
MODULE_LICENSE("GPL");
MODULE_VERSION(HDHOMERUN_VERSION);

static unsigned int hdhomerun_control_poll(struct file *f, struct poll_table_struct *p)
{
	unsigned int mask = 0;
	poll_wait(f, &inq, p);
	poll_wait(f, &outq, p);
	if (my_kfifo_len(&control_fifo_user) != 0) mask |= POLLIN | POLLRDNORM; /* readable */
	mask |= POLLOUT | POLLWRNORM; /* writable... always? */
	return mask;
}

static ssize_t hdhomerun_control_read(struct file *f, char *buf,
				      size_t count, loff_t *offset)
{
	char *user_data;
	ssize_t retval;

	DEBUG_FUNC(HDHOMERUN_CONTROL);
	DEBUG_OUT(HDHOMERUN_CONTROL, "Count: %Zu, offset %lld, buf size: %d\n", count, *offset, my_kfifo_len(&control_fifo_user));

	if (!buf)
		return -EINVAL;

	if (count == 0)
		return 0;

	while(my_kfifo_len(&control_fifo_user) <= 0) {
		if (f->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(inq, (my_kfifo_len(&control_fifo_user) != 0) ))
			return -ERESTARTSYS;
	}

	user_data = kmalloc(count, GFP_KERNEL);
	if (!user_data)
		return -ENOMEM;
	
	retval = my_kfifo_get(&control_fifo_user, user_data, count);

	if(copy_to_user(buf, user_data, retval)) {
		retval = -EFAULT;
	} 

	DEBUG_OUT(HDHOMERUN_CONTROL, "retval %Zu\n", retval);

	kfree(user_data);
	return retval;
}

static ssize_t hdhomerun_control_write(struct file *f, const char __user *buf,
				       size_t count, loff_t *offset)
{
	char *user_data;
	ssize_t retval = count;
	
	DEBUG_FUNC(1);
	DEBUG_OUT(HDHOMERUN_CONTROL, "Count: %Zu, offset %lld, buf size: %d\n", count, *offset, my_kfifo_len(&control_fifo_kernel));
	
	user_data = kmalloc(count, GFP_KERNEL);
	if(!user_data) {
		retval = -ENOMEM;
		goto error;
	}
	
	if (copy_from_user(user_data, buf, count)) {
		retval = -EFAULT;
		goto error;
	}
	
	if(wait_for_write) {
		while(my_kfifo_len(&control_fifo_kernel) == control_bufsize) {
			if (f->f_flags & O_NONBLOCK)
				return -EAGAIN;
			if (wait_event_interruptible(outq, (my_kfifo_len(&control_fifo_kernel) < control_bufsize) ))
				return -ERESTARTSYS;
		}
		retval = my_kfifo_put(&control_fifo_kernel, user_data, count);
	} else {
		DEBUG_OUT(HDHOMERUN_CONTROL, "%s ignoring write\n", __FUNCTION__);
	}	
	DEBUG_OUT(HDHOMERUN_CONTROL, "retval %Zu\n", retval);
	
	wake_up_interruptible(&control_readq);
	
error:
	kfree(user_data);
	return retval;
}

static int hdhomerun_control_open(struct inode *inode, struct file *file)
{
	DEBUG_FUNC(1);

	printk(KERN_INFO "hdhomerun: userhdhomerun connected\n");

	userspace_ready = 1; /*  need mutex here */

	return 0;
}

static int hdhomerun_control_release(struct inode *inode, struct file *file)
{
	DEBUG_FUNC(1);

	DEBUG_OUT(HDHOMERUN_CONTROL, "Control buf size (user)  : %d\n", my_kfifo_len(&control_fifo_user));
	DEBUG_OUT(HDHOMERUN_CONTROL, "Control buf size (kernel): %d\n", my_kfifo_len(&control_fifo_kernel));
	
	/* Clear the contents of the fifo when the user space program
	   disconnects. We want to start fresh when we reconnect
	   again. This is probably a problem for the programs using
	   the /dev/dvb/adapterX/etcY */
	kfifo_reset(&control_fifo_kernel);
	kfifo_reset(&control_fifo_user);
	
	wait_for_write = 0; /* need mutex here */
	userspace_ready =0; /* need mutex here */

   printk(KERN_INFO "hdhomerun: userhdhomerun disconnected\n");

   return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long hdhomerun_control_ioctl(struct file *f,
				   unsigned int cmd, unsigned long arg)
#else
static int hdhomerun_control_ioctl(struct inode *inode,struct file *f,
				    unsigned int cmd, unsigned long arg)
#endif
{
	int retval = 0;
	int err = 1;
	
	DEBUG_FUNC(1);

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}
	
	if (err) {
		printk(KERN_ERR "access_ok() fails\n");
		return -EFAULT;
	}

	switch(cmd) {
	case HDHOMERUN_REGISTER_TUNER: {
		struct hdhomerun_register_tuner_data tuner_data;

		retval = copy_from_user(&tuner_data, (struct hdhomerun_register_tuner_data*)arg, sizeof(struct hdhomerun_register_tuner_data));
		if(retval != 0) {
			printk(KERN_ERR "hdhomerun: get_user() failed, no dvb device created!\n");
		}
		else {
			printk(KERN_INFO "hdhomerun: creating dvb device for %s\n", tuner_data.name);

			retval = dvb_hdhomerun_data_init(tuner_data.num_of_devices);
			if(retval != 0) {
				printk(KERN_ERR "hdhomerun: hdhomerun_data_init() failed, no dvb device created\n");
				return -EFAULT;
			}

			retval = dvb_hdhomerun_register_hdhomerun(&tuner_data);
			if(retval != 0) {
				printk(KERN_ERR "hdhomerun: register_hdhomerun() failed, no dvb device created\n");
				return -EFAULT;
			}

			retval = copy_to_user((void *)arg, &tuner_data, sizeof(struct hdhomerun_register_tuner_data));
			break;
		}
	}
		
	default:
		retval = -ENOTTY;
		DEBUG_OUT(1,"Unknown/unhandled ioctl cmd: %x, nr:%d, type:%d\n", cmd, _IOC_NR(cmd), _IOC_TYPE(cmd));
	}
	
	return retval;
}

static struct file_operations hdhomerun_control_fops = {
	.owner = THIS_MODULE,
	.read = hdhomerun_control_read,
	.write = hdhomerun_control_write,
	.open = hdhomerun_control_open,
	.release = hdhomerun_control_release,
	.poll = hdhomerun_control_poll,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	.unlocked_ioctl = hdhomerun_control_ioctl,
#else
	.ioctl = hdhomerun_control_ioctl,
#endif
};
  
static struct miscdevice hdhomerun_control_device = {
	MISC_DYNAMIC_MINOR,
	"hdhomerun_control",
	&hdhomerun_control_fops
};

int dvb_hdhomerun_control_init() {
	int ret = misc_register(&hdhomerun_control_device);
	
	DEBUG_FUNC(1);

	if (ret) {
		printk(KERN_ERR "Unable to register hdhomerun_control device\n");
		goto error;
	}

	/* Buffer for sending message between kernel/userspace */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	control_fifo_user = *(kfifo_alloc(control_bufsize, GFP_KERNEL, &control_spinlock_user));
	if (IS_ERR(&control_fifo_user)) {
		return PTR_ERR(&control_fifo_user);
	}
#else
	ret = kfifo_alloc(&control_fifo_user, control_bufsize, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "Error kfifo_alloc\n");
		return PTR_ERR(&control_fifo_user);
	}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	control_fifo_kernel = *(kfifo_alloc(control_bufsize, GFP_KERNEL, &control_spinlock_kernel));
	if (IS_ERR(&control_fifo_kernel)) {
		return PTR_ERR(&control_fifo_kernel);
	}
#else
	ret = kfifo_alloc(&control_fifo_kernel, control_bufsize, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "Error kfifo_alloc\n");
		return PTR_ERR(&control_fifo_kernel);
	}
#endif 

	init_waitqueue_head(&control_readq);
	init_waitqueue_head(&inq);
	init_waitqueue_head(&outq);

error:
	return ret;
}
EXPORT_SYMBOL(dvb_hdhomerun_control_init);

void dvb_hdhomerun_control_exit() {
	DEBUG_FUNC(1);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
	kfifo_free(&control_fifo_user);
	kfifo_free(&control_fifo_kernel);
#endif

	misc_deregister(&hdhomerun_control_device);
}
EXPORT_SYMBOL(dvb_hdhomerun_control_exit);
