/*
 * hdhomerun_controller.h, skeleton driver for the HDHomeRun devices
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

#ifndef _hdhomerun_controller_h_
#define _hdhomerun_controller_h_

#include <string>
#include <vector>

class HdhomerunTuner;
class Control;

class HdhomerunController
{
 public:
  HdhomerunController(int _maxDevices);
  ~HdhomerunController();

  HdhomerunTuner* GetTuner(int id);
  
 private:
  std::vector<HdhomerunTuner*> m_tuners;

  int m_maxDevices;

  Control* m_control;
};

#endif // _hdhomerun_controller_h_
