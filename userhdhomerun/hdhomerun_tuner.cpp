/*
 * hdhomerun_tuner.cpp, skeleton driver for the HDHomeRun devices
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

#include "hdhomerun_tuner.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <sys/ioctl.h>

using namespace std;

HdhomerunTuner::HdhomerunTuner(int _device_id, int _device_ip, int _tuner) 
  : m_device(0), m_stop(false), m_stream(false), m_prevFreq(0),
    m_deviceId(_device_id), m_deviceIP(_device_ip), m_tuner(_tuner),
    m_kernelId(-1), m_type(HdhomerunTuner::ATSC)
{
  m_device = hdhomerun_device_create(m_deviceId, m_deviceIP, m_tuner, NULL);
  
  m_name = hdhomerun_device_get_name(m_device);
  cout << endl << "Name of device: " << m_name << endl;
  
  string type = hdhomerun_device_get_model_str(m_device);
  cout << "Type of device: " << type << endl;
  if(type == "hdhomerun_dvbt") {
    m_type = HdhomerunTuner::DVBC;
  } 

  int tuner = hdhomerun_device_get_tuner(m_device);
  cout << "Tuner: " << tuner << endl;

  int ret = hdhomerun_device_set_tuner_filter(m_device, "0x0000-0x1FFF");
  cout << "Set initial pass-all filter for tuner: " << ret << endl;  
}

HdhomerunTuner::~HdhomerunTuner()
{
  this->StopStreaming(0x2000);
  hdhomerun_device_destroy(m_device);
}

void HdhomerunTuner::run()
{
  uint8_t *data;
  size_t dataSize;
  ofstream ofs;

  ofs.open(m_nameDataDevice.c_str(), ios::out | ios::binary);
  if(!ofs) {
    cerr << "Couldn't open: " << m_nameDataDevice << endl;
    _exit(-1);
  }
  cout << "Open data device: " << m_nameDataDevice << endl;

  const int VIDEO_FOR_1_SEC = 20000000 / 8;  // Same number is used on hdhomerun_config. Don't know where they get that from.
  while(m_stream) {
    data = hdhomerun_device_stream_recv(m_device, VIDEO_FOR_1_SEC, &dataSize);

    if(dataSize > 0) {
      ofs.write((const char*)data, dataSize);
      //ofs.flush();
    }

    usleep(64000);
  }

  ofs.close();  
}

void HdhomerunTuner::AddPidToFilter(int _pid)
{
  if(_pid == 0x2000) {  // Means pass-all TS ?
    m_pidFilters.clear(); // When empty, GetStrFromPidFilter will return 0x0000-0x1FFF which = pass-all.    
  }
  else
  {
    vector<int>::iterator result;
    result = find(m_pidFilters.begin(), m_pidFilters.end(), _pid);
    
    if(result == m_pidFilters.end()) {
      m_pidFilters.push_back(_pid);
    }
  }
}
 
void HdhomerunTuner::RemovePidFromFilter(int _pid)
{
  vector<int>::iterator result;
  result = find(m_pidFilters.begin(), m_pidFilters.end(), _pid);

  if(result != m_pidFilters.end()) {
    m_pidFilters.erase(result);
  }
}

std::string HdhomerunTuner::GetStrFromPidFilter()
{
  string PidFilter;

  if(m_pidFilters.empty()) {
    PidFilter = "0x0000-0x1FFF";
  }
  else {
    ostringstream str;

    vector<int>::iterator it;
    for(it = m_pidFilters.begin(); it != m_pidFilters.end(); it++)  {
      str << "0x" << hex << uppercase << *it << " ";
    }
    
    PidFilter = str.str();
  }

  cout << "PidFilter: " << PidFilter << endl;
  return PidFilter;
}

void HdhomerunTuner::StartStreaming(int _pid)
{
  AddPidToFilter(_pid);

  // Setup PID filtering
  string StrPidFilter = GetStrFromPidFilter();

  int ret = hdhomerun_device_set_tuner_filter(m_device, StrPidFilter.c_str());

  // Need locking here too!

  // Start stream
  if(!m_stream) {
    int ret = hdhomerun_device_stream_start(m_device);
    cout << "hdhomerun_device_stream_start: " << ret << endl;
    hdhomerun_device_stream_flush(m_device); 
    
    m_stream = true;
    this->start();
  }
}
 
void HdhomerunTuner::StopStreaming(int _pid)
{
  RemovePidFromFilter(_pid);

  if(!m_pidFilters.empty()) {
    return;
  }

  if(m_stream) {
    cout << "Stop writing to dvr0" << endl;
    m_stream = false;
    while(!this->isFinished()) {
      usleep(100);
    }

    cout << "hdhomerun_device_stream_stop" << endl;
    hdhomerun_device_stream_stop(m_device);
    cout << "hdhomerun_device_stream_stop, stopped" << endl;
  }
}


int HdhomerunTuner::Tune(int _freq)
{
  int status = ReadStatus();
  if(m_prevFreq == _freq && status == (FE_HAS_SIGNAL | FE_HAS_CARRIER | FE_HAS_VITERBI | FE_HAS_SYNC | FE_HAS_LOCK) ) {
    return 0;
  }

  ostringstream is;
  is << "auto:" << _freq;

  int ret = hdhomerun_device_set_tuner_channel(m_device, is.str().c_str());
  cout << "hdhomerun_device_set_tuner_channel: " << ret << endl;

//   ret = hdhomerun_device_set_tuner_program(device, "237");
//   cout << "hdhomerun_device_set_tuner_program: " << ret << endl;

  struct hdhomerun_tuner_status_t hdhomerun_status;
  int ret2 = hdhomerun_device_wait_for_lock(m_device, &hdhomerun_status);
  cout << "hdhomerun_tuner_status_t: " << ret2 << endl;

  m_prevFreq = _freq;

  return ret;
}


int HdhomerunTuner::ReadStatus()
{
  int status = 0;

  struct hdhomerun_tuner_status_t hdhomerun_status;
  int ret = hdhomerun_device_get_tuner_status(m_device, NULL, &hdhomerun_status);
  if (ret > 0) {
    if (hdhomerun_status.symbol_error_quality == 100) {
      status = FE_HAS_SIGNAL
	| FE_HAS_CARRIER
	| FE_HAS_VITERBI
	| FE_HAS_SYNC
	| FE_HAS_LOCK; 
    }
  }

  cout << "sym qual: " << hdhomerun_status.symbol_error_quality << endl;

  return status;
}

int HdhomerunTuner::ReadSignalStrength()
{
  int status = 0x0000;

  struct hdhomerun_tuner_status_t hdhomerun_status;
  int ret = hdhomerun_device_get_tuner_status(m_device, NULL, &hdhomerun_status);
  cout << "strength: " << hdhomerun_status.signal_strength << " sym qual: " <<  hdhomerun_status.symbol_error_quality << " sig to noise:" << hdhomerun_status.signal_to_noise_quality << endl;

  status = (0xffff *  hdhomerun_status.signal_strength) / 100;

  return status;
}

int HdhomerunTuner::SetPesFilter(int _pid, int _output, int _pes_type)
{
  int ret = 0;

  _exit(0);

  ostringstream is;
  is << "0x" << hex << setfill('0') << setw(4) << _pid;
  
  if(m_pes_filter.empty()) {
    m_pes_filter = is.str();
  }
  else {
    m_pes_filter += " " + is.str();
  }
  cout << "filterstr: " << m_pes_filter << endl;

  ret = hdhomerun_device_set_tuner_filter(m_device, m_pes_filter.c_str());
  cout << "set_tuner_filter, ret = " << ret << endl;
					
  // type = DMX_PES_OTHER Means we don't want to send data to a the decoder.
  // output = DMX_OUT_TS_TAP create a /dev/dvb/adapterX/dvrY device - which is the one we are interested in.
  if(_output == DMX_OUT_TS_TAP && _pes_type != DMX_PES_OTHER) {
    //this->StartStreaming();
  }
  
  return ret;
}

const std::string& HdhomerunTuner::GetName()
{
  return m_name;
}

void HdhomerunTuner::SetDataDeviceName(const std::string& _name)
{
  m_nameDataDevice = _name;
}
