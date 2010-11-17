/*
 * dvb_hdhomerun_compat.h, compat for various kernel versions
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

#ifndef __DVB_HDHOMERUN_COMPAT_H__
#define __DVB_HDHOMERUN_COMPAT_H__

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#define my_kfifo_len __kfifo_len
#define my_kfifo_put kfifo_put
#define my_kfifo_get kfifo_get
#else
#define my_kfifo_len kfifo_len
#define my_kfifo_get kfifo_out
#define my_kfifo_put kfifo_in
#endif

#endif /* __DVB_HDHOMERUN_COMPAT_H__ */
