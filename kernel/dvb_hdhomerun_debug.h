/*
 * dvb_hdhomerun_debug.c, skeleton driver for the HDHomeRun devices
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

#ifndef __DVB_HDHOMERUN_DEBUG_H__
#define __DVB_HDHOMERUN_DEBUG_H__

extern int hdhomerun_debug_mask;

enum enum_hdhomerun_debug_mask {
	HDHOMERUN_FUNCTION = 0x1,
	HDHOMERUN_FE = 0x2,
	HDHOMERUN_DATA = 0x4,
	HDHOMERUN_CONTROL = 0x8
};

#define DEBUG_OUT(level, fmt, args...) if( level & hdhomerun_debug_mask )	\
    printk(KERN_DEBUG fmt, ## args);

#define DEBUG_FUNC(x) DEBUG_OUT(x, "%s\n", __FUNCTION__);

#endif /* __DVB_HDHOMERUN_DEBUG_H__ */
