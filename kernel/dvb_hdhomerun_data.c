/*
 * dvb_hdhomerun_data.c, skeleton driver for the HDHomeRun devices
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
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "dvb_hdhomerun_core.h"
#include "dvb_hdhomerun_debug.h"
#include "dvb_hdhomerun_data.h"

struct hdhomerun_data_state *hdhomerun_data_states[HDHOMERUN_MAX_TUNERS];

struct hdhomerun_data_state {
	struct dvb_demux *dvb_demux;
	int id;
	dev_t dev;
	struct cdev cdev;
	struct device *device;
};

static dev_t hdhomerun_major = -1;
static struct class *hdhomerun_class;
static int hdhomerun_num_of_devices = 0;

MODULE_AUTHOR("Villy Thomsen");
MODULE_DESCRIPTION("HDHomeRun driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(HDHOMERUN_VERSION);

static ssize_t hdhomerun_data_write(struct file *f, const char __user *buf,
				       size_t count, loff_t *offset)
{
	char *user_data;
	ssize_t retval = count;

	struct hdhomerun_data_state *state = f->private_data;
	
	DEBUG_FUNC(1);
	DEBUG_OUT(HDHOMERUN_DATA, "Count: %Zu, offset %lld\n", count, *offset);
	
	user_data = kmalloc(count, GFP_KERNEL);
	if(!user_data) {
		retval = -ENOMEM;
		goto error;
	}
	
	if (copy_from_user(user_data, buf, count)) {
		retval = -EFAULT;
		goto error;
	}
	
	/* Feed stuff to V4l-DVB */
	dvb_dmx_swfilter(state->dvb_demux, user_data, count);

error:
	kfree(user_data);
	return retval;
}

static int hdhomerun_data_open(struct inode *inode, struct file *file)
{
	struct hdhomerun_data_state *state;

	DEBUG_FUNC(1);

	state = container_of(inode->i_cdev, struct hdhomerun_data_state, cdev);
	DEBUG_OUT(HDHOMERUN_DATA, "Open major: %d\n", MAJOR(hdhomerun_major));

	file->private_data = state;

	return 0;
}

static int hdhomerun_data_release(struct inode *inode, struct file *file)
{
	DEBUG_FUNC(1);

	return 0;
}

static struct file_operations hdhomerun_data_fops = {
	.owner = THIS_MODULE,
	.write = hdhomerun_data_write,
	.open = hdhomerun_data_open,
	.release = hdhomerun_data_release,
};

int dvb_hdhomerun_data_init(int num_of_devices) {
	int ret = 0;

	if(hdhomerun_major == -1) {
		/* Create class (should I use an existing?) */
		hdhomerun_class = class_create(THIS_MODULE, "hdhomerun");
		if (IS_ERR(hdhomerun_class)) {
			ret = PTR_ERR(hdhomerun_class);
			goto fail_class_create;
		}
	
		/* Create major */
		ret = alloc_chrdev_region(&hdhomerun_major, 0, num_of_devices, "hdhomerun_data");
		if(ret < 0) {
			printk(KERN_WARNING "hdhomerun: Can't get major: %d, num of devices: %d\n", MAJOR(hdhomerun_major), num_of_devices);
		}

		hdhomerun_num_of_devices = num_of_devices;
	}

	return ret;

fail_class_create:
	printk(KERN_ERR "unable to create class for hdhomerun\n");
	return ret;
}
EXPORT_SYMBOL(dvb_hdhomerun_data_init);
  
int dvb_hdhomerun_data_create_device(struct dvb_demux *dvb_demux, int id) {
	struct hdhomerun_data_state *state;
	int major;
	int minor;
	int ret = 0;

	DEBUG_FUNC(1);

	if(hdhomerun_major == -1) {
		printk(KERN_ERR "hdhomerun: class not created yet!\n");
		return -1;
	}

	/* setup internal structure for storing data */
	state = kzalloc(sizeof(struct hdhomerun_data_state), GFP_KERNEL);
	if (state == NULL) {
		printk(KERN_ERR
		       "HDHomeRun: out of memory for data device %d\n",
		       id);
		return -ENOMEM;
	}

	state->dvb_demux = dvb_demux;
	state->id = id;

	/* Create dev_t for this tuner */
	major = MAJOR(hdhomerun_major);
	minor = MINOR(hdhomerun_major);
	state->dev = MKDEV(major, minor + id);

	/* Create the character device */
	cdev_init(&state->cdev, &hdhomerun_data_fops);
	state->cdev.owner = THIS_MODULE;
	state->cdev.ops = &hdhomerun_data_fops;

	ret = cdev_add(&state->cdev, state->dev, 1);
	if(ret < 0) {
		printk(KERN_WARNING "hdhomerun: Can't add char device: %d %d\n", major, minor);
	}

	/* Create device file and sysfs entry */
	state->device = device_create(hdhomerun_class, NULL, state->dev, NULL, "hdhomerun_data%d", id);
	if(IS_ERR(state->device)) {
		ret = PTR_ERR(state->device);
		goto fail_device_create;
	}
	printk(KERN_INFO "hdhomerun: device /dev/hdhomerun_data%d created\n", id);
	
	hdhomerun_data_states[id] = state;

	return ret;

fail_device_create:
	printk(KERN_ERR "unable to create device /dev/hdhomerun%d\n", id);
	return ret;
}
EXPORT_SYMBOL(dvb_hdhomerun_data_create_device);

void dvb_hdhomerun_data_delete_device(int id) {
	DEBUG_FUNC(1);

	cdev_del(&hdhomerun_data_states[id]->cdev);
	device_destroy(hdhomerun_class, hdhomerun_data_states[id]->dev);
}
EXPORT_SYMBOL(dvb_hdhomerun_data_delete_device);

void dvb_hdhomerun_data_exit() {
	DEBUG_FUNC(1);

	if(hdhomerun_major != -1) {
		unregister_chrdev_region(hdhomerun_major, hdhomerun_num_of_devices);
		hdhomerun_major = -1;

		class_destroy(hdhomerun_class);
	}
}
EXPORT_SYMBOL(dvb_hdhomerun_data_exit);
