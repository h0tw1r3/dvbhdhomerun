/*
 * dvb_hdhomerun_tuner.h, skeleton driver for the HDHomeRun devices
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

#ifndef _hdhomerun_tuner_h_
#define _hdhomerun_tuner_h_

#include "thread_pthread.h"

#include <hdhomerun.h>

#include <string>
#include <vector>

class HdhomerunTuner : public ThreadPthread
{
 public:
  enum Type 
  {
    DVBC,
    ATSC
  };

 public:
  HdhomerunTuner(int _device_id, int _device_ip, int _tuner);
  ~HdhomerunTuner();
  
  void run();

  int Tune(int _freq);

  int ReadStatus();

  int ReadSignalStrength();

  int SetPesFilter(int _pid, int _output, int _pes_type);

  void StartStreaming(int _pid);
  void StopStreaming(int _pid);

  const std::string& GetName();

  void SetDataDeviceName(const std::string& _name);

  void SetKernelId(int _id) {
    m_kernelId = _id;
  }
  int GetKernelId() {
    return m_kernelId;
  }

  Type GetType() {
    return m_type;
  }

 private:
  void AddPidToFilter(int _pid);
  void RemovePidFromFilter(int _pid);
  std::string GetStrFromPidFilter();

 private:
  struct hdhomerun_device_t* m_device;

  bool m_stream;
  
  std::string m_pes_filter;

  std::vector<int> m_pidFilters;

  int m_prevFreq;

  int m_deviceId;
  int m_deviceIP;
  int m_tuner;

  int m_kernelId;

  Type m_type;

  // Name returned from hdhomerun lib
  std::string m_name;
  
  // /dev/hdhomerun_dataX device
  std::string m_nameDataDevice;
};

#endif // _hdhomerun_tuner_h_
