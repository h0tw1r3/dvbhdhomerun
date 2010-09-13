/*
 * hdhomerun_controller.cpp, skeleton driver for the HDHomeRun devices
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

#include "hdhomerun_controller.h"

#include "hdhomerun_control.h"
#include "hdhomerun_tuner.h"

#include <iostream>
#include <sstream>

using namespace std;

HdhomerunController::HdhomerunController(int _maxDevices) 
  : m_maxDevices(_maxDevices)
{
  //
  // Discover HDHomeRun's 
  //
  struct hdhomerun_discover_device_t devices[m_maxDevices];

  int numOfDevices = hdhomerun_discover_find_devices_custom(0, HDHOMERUN_DEVICE_TYPE_TUNER, HDHOMERUN_DEVICE_ID_WILDCARD, devices, m_maxDevices);
  cout << "Num of devices = " << numOfDevices << endl;

  if(numOfDevices == 0) {
    cerr << "No HDHomeRun devices found! Exiting" << endl;
    _exit(-1);
  }

  //
  // Create an object for each tuner to handle data streaming/filtering/etc.
  //
  const int MAX_TUNERS = 2; 
  for(int i = 0; i < numOfDevices; ++i) {
    for(int j = 0; j < MAX_TUNERS; ++j) {
      HdhomerunTuner* tuner = new HdhomerunTuner(devices[i].device_id, devices[i].ip_addr, j);
      m_tuners.push_back(tuner);
    }
  }

  cout << endl;

  //
  // Create DVB devices 
  //
  m_control = new Control(this);

  vector<HdhomerunTuner*>::iterator it;
  for(it = m_tuners.begin(); it != m_tuners.end(); ++it) {
    int kernelId = 0;

    if(m_control->Ioctl(numOfDevices * MAX_TUNERS, (*it)->GetName(), kernelId, (*it)->GetType())) {
      ostringstream stream;
      stream << "/dev/hdhomerun_data" << kernelId;
      (*it)->SetDataDeviceName(stream.str());
      (*it)->SetKernelId(kernelId);
    }
  }

  // Begin receiving request from the /dev/dvb/xx/yy devices.
  m_control->start();
}
 
HdhomerunController::~HdhomerunController()
{
  // Need to catch ctrl-c somewhere so we can close down nicely.

  // Need to stop threads!
}


HdhomerunTuner* HdhomerunController::GetTuner(int _id)
{
  HdhomerunTuner* tuner = 0;

  vector<HdhomerunTuner*>::iterator it;
  for(it = m_tuners.begin(); it != m_tuners.end(); ++it) {
    if((*it)->GetKernelId() == _id) {
      tuner = (*it);
      break;
    }
  }

  return tuner;
}
