/*
 * dvb_hdhomerun_fe.h, skeleton driver for the HDHomeRun devices
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

#ifndef __DVB_HDHOMERUN_FE_H__
#define __DVB_HDHOMERUN_FE_H__

#include <linux/dvb/frontend.h>

#if defined(CONFIG_DVB_HDHOMERUN_FE) && defined(MODULE)
extern struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbc(int id);
extern struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbt(int id);
extern struct dvb_frontend *dvb_hdhomerun_fe_attach_atsc(int id);
#else
static inline
struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbc(int id) {
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
struct dvb_frontend *dvb_hdhomerun_fe_attach_atsc(int id) {
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbt(int id) {
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif /* CONFIG_DVB_HDHOMERUN_FE */

#endif /* __DVB_HDHOMERUN_FE_H__ */
