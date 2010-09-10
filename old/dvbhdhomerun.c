/*
 * dvbhdhomerun.c, skeleton driver for the HDHomeRun devices
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
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/kfifo.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "dvbhdhomerun_control_messages.h"

MODULE_DESCRIPTION("DVB loopback device for hdhomerun device");
MODULE_VERSION("0.0.1");
MODULE_AUTHOR("Villy Thomsen");
MODULE_LICENSE("GPL");

/* #define DEBUG_OUT(debug_level, fmt, args...)  */
#define DEBUG_OUT(level, fmt, args...) if(level <= hdhomerun_debug_level)	\
    printk(KERN_DEBUG fmt, ## args);

/* #define DEBUG_FUNC(x) */
#define DEBUG_FUNC(x) DEBUG_OUT(x, "%s\n", __FUNCTION__);

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static struct dvb_adapter *dvb_adapter;
static struct hdhomerun_devinfo *devinfo;
static struct platform_device *platform_device;

struct hdhomerun_devinfo {
  struct dvb_device *frontend;
  struct dvb_device *demux;
  struct dvb_device *dvr;
  struct kfifo *fifo;
  spinlock_t fifo_spinlock;
  wait_queue_head_t readq;
  wait_queue_head_t writeq;
};

/* for the control device */
static struct kfifo *control_fifo_user;
static struct kfifo *control_fifo_kernel;
static spinlock_t control_spinlock;
static wait_queue_head_t control_readq;
static int control_bufsize = 32768;

/* For the loopback device */
static int fifo_bufsize = 188 * 1024 * 2; // 188 = the size of a mpeg TS packet.
static int fifo_current_size = 0;
static int fifo_max_size = 0;

/* Module params */
static int hdhomerun_debug_level = 3;
module_param(hdhomerun_debug_level, int, S_IRUGO | S_IWUSR);
module_param(fifo_bufsize, int, S_IRUGO);
module_param(fifo_current_size, int, S_IRUGO);
module_param(fifo_max_size, int, S_IRUGO);



static void dvbhdhomerun_post_message(struct dvbhdhomerun_control_mesg *mesg) {
  if(kfifo_put(control_fifo_user, (unsigned char*)mesg, sizeof(struct dvbhdhomerun_control_mesg)) < sizeof(struct dvbhdhomerun_control_mesg)) {
    printk(KERN_CRIT "No buffer space for hdhomerun control device!\n");
  }
}

static int dvbhdhomerun_wait_for_message(struct dvbhdhomerun_control_mesg *mesg) {
  do {
    if(wait_event_interruptible(control_readq, __kfifo_len(control_fifo_kernel) > 0)) {
      DEBUG_OUT(1,"read interrupted\n");
      return -ERESTARTSYS;
    }
  } while(__kfifo_len(control_fifo_kernel) == 0);

  return kfifo_get(control_fifo_kernel, (unsigned char*)mesg, sizeof(struct dvbhdhomerun_control_mesg));
}


static int hdhomerun_frontend_open(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_demux_open(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_dvr_open(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_frontend_release(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_demux_release(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_dvr_release(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static ssize_t my_read(struct file *f, char *buf,
		       size_t count, loff_t *offset)
{
  char *user_data;
  ssize_t retval;

  DEBUG_OUT(2, "Count: %d, offset %lld\n", count, *offset);
  DEBUG_OUT(2, "Buf size: %d\n", __kfifo_len(devinfo->fifo));

  if (!buf)
    return -EINVAL;

  if (count == 0)
    return 0;

  /* Block untill we have request count data. */
  do {
    if(__kfifo_len(devinfo->fifo) == 0) {
      if(f->f_flags & O_NONBLOCK) {
	DEBUG_OUT(0, "read O_NONBLOCK");
	return -EAGAIN;
      }
    }

    if(wait_event_interruptible(devinfo->readq, __kfifo_len(devinfo->fifo) >= count)) {
      DEBUG_OUT(1,"read interrupted\n");
      return -ERESTARTSYS;
    }
  } while(__kfifo_len(devinfo->fifo) == 0);
    
  user_data = kmalloc(count, GFP_KERNEL);
  if (!user_data)
    return -ENOMEM;

  retval = kfifo_get(devinfo->fifo, user_data, count);
  wake_up_interruptible(&devinfo->writeq);

  if(copy_to_user(buf, user_data, retval)) {
    retval = -EFAULT;
  } 

  DEBUG_OUT(2, "retval %d\n", retval);

  kfree(user_data);
  return retval;
}


static ssize_t my_write(struct file *f, const char __user *buf,
			       size_t count, loff_t *offset)
{
  char *user_data;
  ssize_t retval = -ENOMEM;

  DEBUG_OUT(2, "Count: %d, offset %lld\n", count, *offset);
  DEBUG_OUT(2, "Buf size: %d\n", __kfifo_len(devinfo->fifo));

  if(count == 0) {
    return 0;
  }

  user_data = kmalloc(count, GFP_KERNEL);
  if(!user_data) {
    goto error;
  }

  if (copy_from_user(user_data, buf, count)) {
    retval = -EFAULT;
    goto error;
  }

  do {
    retval = kfifo_put(devinfo->fifo, user_data, count);
    DEBUG_OUT(2, "retval %d\n", retval);

    fifo_current_size = __kfifo_len(devinfo->fifo);
    if(__kfifo_len(devinfo->fifo) > fifo_max_size) {
      fifo_max_size = __kfifo_len(devinfo->fifo);
    }

    if(__kfifo_len(devinfo->fifo) >= fifo_bufsize) {
      DEBUG_OUT(0, "BUF ERROR, size: %d\n", __kfifo_len(devinfo->fifo));
    }

    wake_up_interruptible(&devinfo->readq);

    if(retval == 0) {
      if(f->f_flags & O_NONBLOCK) {
	DEBUG_OUT(0, "write O_NONBLOCK");
	retval = -EAGAIN;
	goto error;
      }
      if(wait_event_interruptible(devinfo->writeq, __kfifo_len(devinfo->fifo) < fifo_bufsize)) {
	DEBUG_OUT(1, "write interrupted\n");
	retval = -ERESTARTSYS;
	goto error;
      }
    }
  } while (retval == 0);

 error:
  kfree(user_data);
  return retval;
}

static ssize_t hdhomerun_frontend_read(struct file *f, char *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_read(f, buf, count, offset);
}

static ssize_t hdhomerun_demux_read(struct file *f, char *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_read(f, buf, count, offset);
}

static ssize_t hdhomerun_dvr_read(struct file *f, char *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_read(f, buf, count, offset);
}


static ssize_t hdhomerun_frontend_write(struct file *f, const char __user *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_write(f, buf, count, offset);
}

static ssize_t hdhomerun_demux_write(struct file *f, const char __user *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_write(f, buf, count, offset);
}

static ssize_t hdhomerun_dvr_write(struct file *f, const char __user *buf,
			       size_t count, loff_t *offset)
{
  DEBUG_FUNC(2);

  return my_write(f, buf, count, offset);
}

struct dvb_frontend_info frontend_info = {
  .name             = "dvbhdhomerun",
  .type             = FE_QAM,
  .frequency_min    = 80000000,
  .frequency_max    = 990000000,
  .frequency_stepsize = 8000000,
  .symbol_rate_min  = 5217,
  .symbol_rate_max  = 6900,
};

static int hdhomerun_frontend_ioctl(struct inode *inode, struct file *f,
	unsigned int cmd, unsigned long arg)
{
  int retval = 0;
  struct dvbhdhomerun_control_mesg mesg;

  DEBUG_FUNC(0);

  switch(cmd) {
  case FE_GET_INFO: {
    fe_caps_t capabilities;

    DEBUG_OUT(0, "FE_GET_INFO\n");

    capabilities = FE_CAN_INVERSION_AUTO | FE_CAN_FEC_2_3;
    frontend_info.caps = capabilities;

    if(copy_to_user((char* __user)arg, &frontend_info, sizeof(struct dvb_frontend_info))) {
      retval = -EFAULT;
    } 

    break;
  }

  case FE_SET_FRONTEND: {
    struct dvb_frontend_parameters frontend_param;

    DEBUG_OUT(0,"FE_SET_FRONTEND\n");

    if(copy_from_user(&frontend_param, (struct dvb_frontend_parameters* __user)arg, sizeof(struct dvb_frontend_parameters))) {
      retval = -EFAULT;
    }

    DEBUG_OUT(0,"FE_SET_FRONTEND, freq: %d, inv: %d, symb rate: %d, fec: %d, mod: %d\n",
	      frontend_param.frequency,
	      frontend_param.inversion,
	      frontend_param.u.qam.symbol_rate,
	      frontend_param.u.qam.fec_inner,    
	      frontend_param.u.qam.modulation);    

    mesg.type = FE_SET_FRONTEND;
    mesg.u.frontend_parameters = frontend_param;

    dvbhdhomerun_post_message(&mesg);
    if(dvbhdhomerun_wait_for_message(&mesg) == 0) {
      DEBUG_OUT(0, "dvbhdhomerun_wait_for_message failed, shouldn't happen!\n");
    }
    
    break;
  }

  case FE_READ_STATUS: {
    fe_status_t status = 0;

    DEBUG_OUT(0,"FE_READ_STATUS\n");

    mesg.type = FE_READ_STATUS;
    dvbhdhomerun_post_message(&mesg);

    /* Now we wait for userspace to return to us */
    if(dvbhdhomerun_wait_for_message(&mesg) == 0) {
      DEBUG_OUT(0, "dvbhdhomerun_wait_for_message failed, shouldn't happen!\n");
    }

    status = mesg.u.fe_status;

    retval = __put_user(status, (int __user*)arg);
    break;
  }

  case FE_READ_BER:
    DEBUG_OUT(0,"FE_READ_BER\n");
    retval = __put_user(0, (uint32_t __user*)arg);
    break;

  case FE_READ_SIGNAL_STRENGTH: {
    DEBUG_OUT(0,"FE_READ_SIGNAL_STRENGTH\n");

    mesg.type = FE_READ_SIGNAL_STRENGTH;
    dvbhdhomerun_post_message(&mesg);

    /* Now we wait for userspace to return to us */
    if(dvbhdhomerun_wait_for_message(&mesg) == 0) {
      DEBUG_OUT(0, "dvbhdhomerun_wait_for_message failed, shouldn't happen!\n");
    }

    retval = __put_user(mesg.u.signal_strength, (uint16_t __user*)arg);
    break;
  }

  case FE_READ_SNR:
    DEBUG_OUT(0,"FE_READ_SNR\n");
    retval = __put_user(0, (uint16_t __user*)arg);
    break;

  case FE_READ_UNCORRECTED_BLOCKS:
    DEBUG_OUT(0,"FE_READ_UNCORRECTED_BLOCKS\n");
    
    retval = __put_user(0, (uint32_t __user*)arg);    
    break;

  default:
    retval = -ENOTTY;
    DEBUG_OUT(0,"Unknown/unhandled ioctl cmd: %x, nr:%d, type:%d\n", cmd, _IOC_NR(cmd), _IOC_TYPE(cmd));
  }

  return retval;
}

static int hdhomerun_demux_ioctl(struct inode *inode, struct file *f,
	unsigned int cmd, unsigned long arg)
{
  struct dvbhdhomerun_control_mesg mesg;
  int retval = 0;

  DEBUG_FUNC(0);

  switch(cmd) {
  case DMX_STOP:
    DEBUG_OUT(0,"DMX_STOP - NOT IMPLEMENTED\n");
    break;

  case DMX_SET_FILTER:
    DEBUG_OUT(0,"DMX_SET_FILTER - NOT IMPLEMENTED\n");
    break;

  case DMX_SET_PES_FILTER: {
    struct dmx_pes_filter_params pes_param;

    DEBUG_OUT(0,"DMX_SET_PES_FILTER - NOT IMPLEMENTED\n");

    if(copy_from_user(&pes_param, (struct dmx_pes_filter_params* __user)arg, sizeof(struct dmx_pes_filter_params))) {
      retval = -EFAULT;
    }

    mesg.type = DMX_SET_PES_FILTER;
    mesg.u.dmx_pes_filter = pes_param;

    dvbhdhomerun_post_message(&mesg);
    if(dvbhdhomerun_wait_for_message(&mesg) == 0) {
      DEBUG_OUT(0, "dvbhdhomerun_wait_for_message failed, shouldn't happen!\n");
    }


    break;
  }

  case DMX_SET_BUFFER_SIZE:
    DEBUG_OUT(0,"DMX_SET_BUFFER_SIZE - NOT IMPLEMENTED %lu\n", arg);
    break;

  default:
    retval = -ENOTTY;
    DEBUG_OUT(0,"Unknown/unhandled ioctl cmd: %x, nr:%d, type:%d\n", cmd, _IOC_NR(cmd), _IOC_TYPE(cmd));
  }

  return retval;
}

static int hdhomerun_dvr_ioctl(struct inode *inode, struct file *f,
	unsigned int cmd, unsigned long arg)
{
  int retval = 0;

  DEBUG_FUNC(0);

  switch(cmd) {
  default:
    retval = -ENOTTY;
    DEBUG_OUT(0,"Unknown/unhandled ioctl cmd: %x, nr:%d, type:%d\n", cmd, _IOC_NR(cmd), _IOC_TYPE(cmd));
  }

  return retval;
}


static unsigned int hdhomerun_frontend_poll(struct file *f, poll_table *wait) {
  int retval = 0;

  DEBUG_FUNC(0);
  DEBUG_OUT(0,"NOT IMPLEMENTED\n");

  return retval;
}

static unsigned int hdhomerun_demux_poll(struct file *f, poll_table *wait) {
  int mask = 0;

  DEBUG_FUNC(0);

  poll_wait(f, &devinfo->readq, wait);

  if(__kfifo_len(devinfo->fifo) > 0) {
    mask |= POLLIN | POLLRDNORM; /* readable */
  }

  return mask;
}

static unsigned int hdhomerun_dvr_poll(struct file *f, poll_table *wait) {
  int retval = 0;

  DEBUG_FUNC(0);
  DEBUG_OUT(0,"NOT IMPLEMENTED\n");

  return retval;
}

static int hdhomerun_kernel_ioctl(struct inode *inode, struct file *f,
	unsigned int cmd, void *parg)
{
  DEBUG_FUNC(0);
  return 0;
}

static struct file_operations hdhomerun_ops_frontend = {
	.owner		= THIS_MODULE,
	.open		= hdhomerun_frontend_open,
 	.release	= hdhomerun_frontend_release,
	.read		= hdhomerun_frontend_read,
	.write		= hdhomerun_frontend_write,
	.ioctl		= hdhomerun_frontend_ioctl,
	.poll		= hdhomerun_frontend_poll,
};

static struct file_operations hdhomerun_ops_demux = {
	.owner		= THIS_MODULE,
	.open		= hdhomerun_demux_open,
 	.release	= hdhomerun_demux_release,
	.read		= hdhomerun_demux_read,
	.write		= hdhomerun_demux_write,
	.ioctl		= hdhomerun_demux_ioctl,
	.poll		= hdhomerun_demux_poll,
};

static struct file_operations hdhomerun_ops_dvr = {
	.owner		= THIS_MODULE,
	.open		= hdhomerun_dvr_open,
 	.release	= hdhomerun_dvr_release,
	.read		= hdhomerun_dvr_read,
	.write		= hdhomerun_dvr_write,
	.ioctl		= hdhomerun_dvr_ioctl,
	.poll		= hdhomerun_dvr_poll,
};

static struct dvb_device hdhomerun_device_frontend = {
        .priv = NULL,
	.users = 1,
        .readers = 1,
        .writers = 1,
	.fops = &hdhomerun_ops_frontend,
	.kernel_ioctl  = hdhomerun_kernel_ioctl,
};

static struct dvb_device hdhomerun_device_demux = {
        .priv = NULL,
	.users = 1,
        .readers = 1,
        .writers = 1,
	.fops = &hdhomerun_ops_demux,
	.kernel_ioctl  = hdhomerun_kernel_ioctl,
};

static struct dvb_device hdhomerun_device_dvr = {
        .priv = NULL,
	.users = 1,
        .readers = 1,
        .writers = 1,
	.fops = &hdhomerun_ops_dvr,
	.kernel_ioctl  = hdhomerun_kernel_ioctl,
};

static void hdhomerun_register_adapter(void) {
  int ret;
  
  DEBUG_FUNC(0);
  
  ret = dvb_register_adapter(dvb_adapter, "HDHOMERUN", THIS_MODULE,
			     &platform_device->dev, adapter_nr);

  DEBUG_OUT(0, "ret %d\n", ret);
}


static void hdhomerun_register_device(void) {
  int retval;

  DEBUG_FUNC(0);
  
  retval = dvb_register_device(dvb_adapter, &devinfo->frontend,
			    &hdhomerun_device_frontend, 0, DVB_DEVICE_FRONTEND);
  DEBUG_OUT(0,"retval %d\n", retval);

  retval = dvb_register_device(dvb_adapter, &devinfo->demux,
			    &hdhomerun_device_demux, 0, DVB_DEVICE_DEMUX);
  DEBUG_OUT(0,"retval %d\n", retval);

  retval = dvb_register_device(dvb_adapter, &devinfo->dvr,
			    &hdhomerun_device_dvr, 0, DVB_DEVICE_DVR);
  DEBUG_OUT(0,"retval %d\n", retval);
}


static void hdhomerun_unregister_device(void) {
  DEBUG_FUNC(0);
  
  dvb_unregister_device(devinfo->frontend);
  dvb_unregister_device(devinfo->demux);
  dvb_unregister_device(devinfo->dvr);
}

static int hdhomerun_create_tuner(void) {
  devinfo = kmalloc(sizeof (struct hdhomerun_devinfo), GFP_KERNEL);

  spin_lock_init(&devinfo->fifo_spinlock);
  devinfo->fifo = kfifo_alloc(fifo_bufsize, GFP_KERNEL, &devinfo->fifo_spinlock);
  if (IS_ERR(devinfo->fifo)) {
    return PTR_ERR(devinfo->fifo);
  }

  init_waitqueue_head(&devinfo->readq);
  init_waitqueue_head(&devinfo->writeq);

  hdhomerun_register_adapter();
  hdhomerun_register_device();

  return 0;
}

static ssize_t hdhomerun_control_read(struct file *f, char *buf,
				      size_t count, loff_t *offset)
{
  char *user_data;
  ssize_t retval;

  DEBUG_FUNC(0);
  DEBUG_OUT(2, "Count: %d, offset %lld, buf size: %d\n", count, *offset, __kfifo_len(control_fifo_user));

  if (!buf)
    return -EINVAL;

  if (count == 0)
    return 0;

  if(__kfifo_len(control_fifo_user) == 0)
    return 0;

  user_data = kmalloc(count, GFP_KERNEL);
  if (!user_data)
    return -ENOMEM;

  retval = kfifo_get(control_fifo_user, user_data, count);

  if(copy_to_user(buf, user_data, retval)) {
    retval = -EFAULT;
  } 

  DEBUG_OUT(2, "retval %d\n", retval);

  kfree(user_data);
  return retval;
}

static ssize_t hdhomerun_control_write(struct file *f, const char __user *buf,
				       size_t count, loff_t *offset)
{
  char *user_data;
  ssize_t retval = -ENOMEM;

  DEBUG_FUNC(2);
  DEBUG_OUT(2, "Count: %d, offset %lld, buf size: %d\n", count, *offset, __kfifo_len(control_fifo_kernel));

  user_data = kmalloc(count, GFP_KERNEL);
  if(!user_data) {
    goto error;
  }

  if (copy_from_user(user_data, buf, count)) {
    retval = -EFAULT;
    goto error;
  }

  retval = kfifo_put(control_fifo_kernel, user_data, count);
  DEBUG_OUT(2, "retval %d\n", retval);

  wake_up_interruptible(&control_readq);

 error:
  kfree(user_data);
  return retval;
}

static int hdhomerun_control_open(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  return 0;
}

static int hdhomerun_control_release(struct inode *inode, struct file *file)
{
  DEBUG_FUNC(1);

  DEBUG_OUT(0, "Control buf size (user)  : %d\n", __kfifo_len(control_fifo_user));
  DEBUG_OUT(0, "Control buf size (kernel): %d\n", __kfifo_len(control_fifo_kernel));

  return 0;
}

static struct file_operations hdhomerun_control_fops = {
  .owner = THIS_MODULE,
  .read = hdhomerun_control_read,
  .write = hdhomerun_control_write,
  .open = hdhomerun_control_open,
  .release = hdhomerun_control_release,
};
  
static struct miscdevice hdhomerun_control_device = {
  MISC_DYNAMIC_MINOR,
  "hdhomerun_control",
  &hdhomerun_control_fops
};

static int __init hdhomerun_init(void) {
  int ret;

  DEBUG_FUNC(0);

  ret = misc_register(&hdhomerun_control_device);
  if (ret) {
    printk(KERN_ERR "Unable to register \"Hello, world!\" misc device\n");
    goto error;
  }

  // Buffer for sending message between kernel/userspace
  control_fifo_user = kfifo_alloc(control_bufsize, GFP_KERNEL, &control_spinlock);
  if (IS_ERR(control_fifo_user)) {
    return PTR_ERR(control_fifo_user);
  }
  control_fifo_kernel = kfifo_alloc(control_bufsize, GFP_KERNEL, &control_spinlock);
  if (IS_ERR(control_fifo_kernel)) {
    return PTR_ERR(control_fifo_kernel);
  }
  init_waitqueue_head(&control_readq);

  dvb_adapter = kmalloc(sizeof (struct dvb_adapter), GFP_KERNEL);

  platform_device = platform_device_alloc("dvbhdhomerun", -1);
  ret = platform_device_add(platform_device);

  hdhomerun_create_tuner();


 error:
  return ret;
}

static void __exit hdhomerun_exit(void) {
  DEBUG_FUNC(0);

  kfifo_free(devinfo->fifo);
  kfifo_free(control_fifo_user);
  kfifo_free(control_fifo_kernel);

  hdhomerun_unregister_device();
  dvb_unregister_adapter(dvb_adapter);
  platform_device_unregister(platform_device);

  misc_deregister(&hdhomerun_control_device);
}

module_init(hdhomerun_init);
module_exit(hdhomerun_exit);

